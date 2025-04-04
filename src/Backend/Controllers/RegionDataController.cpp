// RegionDataController.cpp
#include "RegionDataController.h"

#include <QDebug>
#include <QReadLocker>
#include <QWriteLocker>
#include <stdexcept>

namespace CargoNetSim
{
namespace Backend
{

// RegionData implementation

RegionData::RegionData(const QString     &regionName,
                       NetworkController *networkController,
                       QObject           *parent)
    : QObject(parent)
    , m_region(regionName)
    , m_networkController(networkController)
{
    // Initialize networks lists from provided
    // NetworkController
    auto trainNetworksMap =
        m_networkController->trainNetworksInRegion(
            regionName);
    m_trainNetworksList = trainNetworksMap.keys();

    auto truckNetworksMap =
        m_networkController->truckNetworkConfigsInRegion(
            regionName);
    m_truckNetworksList = truckNetworksMap.keys();
}

RegionData::~RegionData()
{
    m_networkController = nullptr;
}

bool RegionData::checkNetworkNameConflict(
    const QString &name) const
{
    return trainNetworkExists(name)
           || truckNetworkExists(name);
}

void RegionData::setRegionName(const QString &name)
{
    if (m_region == name)
    {
        return; // No change needed
    }

    // We need to move all networks from old region to new
    // region This is a complex operation that requires
    // careful handling

    // First, capture the current list of networks
    QStringList currentTrainNetworks = m_trainNetworksList;
    QStringList currentTruckNetworks = m_truckNetworksList;

    // Create temporary storage for networks and configs
    // we'll move
    QVector<TrainClient::NeTrainSimNetwork *>
        trainNetworksToMove;
    QVector<TruckClient::IntegrationSimulationConfig *>
        truckConfigsToMove;

    // Get references to all networks to move
    for (const QString &networkName : currentTrainNetworks)
    {
        TrainClient::NeTrainSimNetwork *network =
            m_networkController->trainNetwork(networkName,
                                              m_region);
        if (network)
        {
            trainNetworksToMove.append(network);
        }
    }

    for (const QString &networkName : currentTruckNetworks)
    {
        TruckClient::IntegrationSimulationConfig *config =
            m_networkController->truckNetworkConfig(
                networkName, m_region);
        if (config)
        {
            truckConfigsToMove.append(config);
        }
    }

    // Remove networks from old region
    for (const QString &networkName : currentTrainNetworks)
    {
        m_networkController->removeTrainNetwork(networkName,
                                                m_region);
    }

    for (const QString &networkName : currentTruckNetworks)
    {
        m_networkController->removeTruckNetworkConfig(
            networkName, m_region);
    }

    // Update our region name
    m_region = name;

    // Add networks to new region
    for (int i = 0; i < trainNetworksToMove.size(); ++i)
    {
        const QString &networkName =
            currentTrainNetworks[i];
        TrainClient::NeTrainSimNetwork *network =
            trainNetworksToMove[i];
        m_networkController->addTrainNetwork(networkName,
                                             name, network);
    }

    for (int i = 0; i < truckConfigsToMove.size(); ++i)
    {
        const QString &networkName =
            currentTruckNetworks[i];
        TruckClient::IntegrationSimulationConfig *config =
            truckConfigsToMove[i];
        m_networkController->addTruckNetworkConfig(
            networkName, name, config);
    }

    // Our internal lists are already up to date, so no need
    // to update them
}

void RegionData::addTrainNetwork(const QString &networkName,
                                 const QString &nodeFile,
                                 const QString &linkFile)
{
    if (checkNetworkNameConflict(networkName))
    {
        throw std::runtime_error(
            QString("Network name '%1' already exists in "
                    "train or "
                    "truck networks")
                .arg(networkName)
                .toStdString());
    }

    try
    {
        // Create a new train network
        TrainClient::NeTrainSimNetwork *network =
            new TrainClient::NeTrainSimNetwork();
        network->loadNetwork(nodeFile, linkFile);

        // Add the network to NetworkController
        if (!m_networkController->addTrainNetwork(
                networkName, m_region, network))
        {
            delete network; // Clean up if adding failed
            throw std::runtime_error(
                "Failed to register train network "
                "with NetworkController");
        }

        // Add to our local list
        m_trainNetworksList.append(networkName);

        // Notify listeners
        emit trainNetworkAdded(networkName);
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(
            QString("Failed to create train network: %1")
                .arg(e.what())
                .toStdString());
    }
}

void RegionData::addTruckNetwork(const QString &networkName,
                                 const QString &configFile)
{
    if (checkNetworkNameConflict(networkName))
    {
        throw std::runtime_error(
            QString("Network name '%1' already exists "
                    "in train or truck networks")
                .arg(networkName)
                .toStdString());
    }

    try
    {
        // Parse the config file
        auto configReader =
            TruckClient::IntegrationSimulationConfigReader(
                configFile);
        TruckClient::IntegrationSimulationConfig *config =
            configReader.getConfig();

        // Add the config to NetworkController
        if (!m_networkController->addTruckNetworkConfig(
                networkName, m_region, config))
        {
            delete config; // Clean up if adding failed
            throw std::runtime_error(
                "Failed to register truck network "
                "config with NetworkController");
        }

        // Add to our local list
        m_truckNetworksList.append(networkName);

        // Notify listeners
        emit truckNetworkAdded(networkName);
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(
            QString("Failed to create truck network: %1")
                .arg(e.what())
                .toStdString());
    }
}

void RegionData::renameTrainNetwork(const QString &oldName,
                                    const QString &newName)
{
    if (!trainNetworkExists(oldName))
    {
        throw std::runtime_error(
            QString(
                "Train network '%1' not found in region")
                .arg(oldName)
                .toStdString());
    }

    if (oldName != newName
        && checkNetworkNameConflict(newName))
    {
        throw std::runtime_error(
            QString("Network name '%1' already exists")
                .arg(newName)
                .toStdString());
    }

    try
    {
        // Get the network from NetworkController
        TrainClient::NeTrainSimNetwork *network =
            m_networkController->trainNetwork(oldName,
                                              m_region);
        if (!network)
        {
            throw std::runtime_error(
                "Network exists in local list but "
                "not in NetworkController");
        }

        // Remove network with old name from
        // NetworkController
        if (!m_networkController->removeTrainNetwork(
                oldName, m_region))
        {
            throw std::runtime_error(
                "Failed to remove network with "
                "old name from NetworkController");
        }

        // Add network with new name to NetworkController
        if (!m_networkController->addTrainNetwork(
                newName, m_region, network))
        {
            // Try to restore the old name if adding with
            // new name fails
            if (!m_networkController->addTrainNetwork(
                    oldName, m_region, network))
            {
                // Critical error - network is now not
                // registered at all
                delete network;
                throw std::runtime_error(
                    "Failed to re-register network "
                    "with new name and failed to "
                    "restore old name");
            }
            throw std::runtime_error(
                "Failed to register network "
                "with new name");
        }

        // Update our local list
        int index = m_trainNetworksList.indexOf(oldName);
        if (index >= 0)
        {
            m_trainNetworksList[index] = newName;
        }

        // Notify listeners
        emit trainNetworkRenamed(oldName, newName);
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(
            QString("Failed to rename train network: %1")
                .arg(e.what())
                .toStdString());
    }
}

void RegionData::renameTruckNetwork(const QString &oldName,
                                    const QString &newName)
{
    if (!truckNetworkExists(oldName))
    {
        throw std::runtime_error(
            QString(
                "Truck network '%1' not found in region")
                .arg(oldName)
                .toStdString());
    }

    if (oldName != newName
        && checkNetworkNameConflict(newName))
    {
        throw std::runtime_error(
            QString("Network name '%1' already exists")
                .arg(newName)
                .toStdString());
    }

    try
    {
        // Get the config from NetworkController
        TruckClient::IntegrationSimulationConfig *config =
            m_networkController->truckNetworkConfig(
                oldName, m_region);
        if (!config)
        {
            throw std::runtime_error(
                "Network exists in local list but "
                "not in NetworkController");
        }

        // Remove config with old name from
        // NetworkController
        if (!m_networkController->removeTruckNetworkConfig(
                oldName, m_region))
        {
            throw std::runtime_error(
                "Failed to remove network with old "
                "name from NetworkController");
        }

        // Add config with new name to NetworkController
        if (!m_networkController->addTruckNetworkConfig(
                newName, m_region, config))
        {
            // Try to restore the old name if adding with
            // new name fails
            if (!m_networkController->addTruckNetworkConfig(
                    oldName, m_region, config))
            {
                // Critical error - network is now not
                // registered at all
                delete config;
                throw std::runtime_error(
                    "Failed to re-register network "
                    "with new name and failed to "
                    "restore old name");
            }
            throw std::runtime_error(
                "Failed to register network "
                "with new name");
        }

        // Update our local list
        int index = m_truckNetworksList.indexOf(oldName);
        if (index >= 0)
        {
            m_truckNetworksList[index] = newName;
        }

        // Notify listeners
        emit truckNetworkRenamed(oldName, newName);
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(
            QString("Failed to rename truck network: %1")
                .arg(e.what())
                .toStdString());
    }
}

bool RegionData::removeTrainNetwork(const QString &name)
{
    if (!trainNetworkExists(name))
    {
        throw std::runtime_error(
            QString(
                "Train network '%1' not found in region")
                .arg(name)
                .toStdString());
        return false;
    }

    try
    {
        // Remove from NetworkController (this will delete
        // the network object)
        if (!m_networkController->removeTrainNetwork(
                name, m_region))
        {
            throw std::runtime_error(
                "Failed to remove network from "
                "NetworkController");
            return false;
        }

        // Remove from our local list
        m_trainNetworksList.removeAll(name);

        // Notify listeners
        emit trainNetworkRemoved(name);

        return true;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(
            QString("Failed to remove train network: %1")
                .arg(e.what())
                .toStdString());
        return false;
    }
}

bool RegionData::removeTruckNetwork(const QString &name)
{
    if (!truckNetworkExists(name))
    {
        throw std::runtime_error(
            QString(
                "Truck network '%1' not found in region")
                .arg(name)
                .toStdString());
        return false;
    }

    try
    {
        // Remove from NetworkController (this will delete
        // the config)
        if (!m_networkController->removeTruckNetworkConfig(
                name, m_region))
        {
            throw std::runtime_error(
                "Failed to remove network config "
                "from NetworkController");

            return false;
        }

        // Remove from our local list
        m_truckNetworksList.removeAll(name);

        // Notify listeners
        emit truckNetworkRemoved(name);

        return true;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(
            QString("Failed to remove truck network: %1")
                .arg(e.what())
                .toStdString());

        return false;
    }
}

bool RegionData::trainNetworkExists(
    const QString &name) const
{
    return m_trainNetworksList.contains(name);
}

bool RegionData::truckNetworkExists(
    const QString &name) const
{
    return m_truckNetworksList.contains(name);
}

TrainClient::NeTrainSimNetwork *
RegionData::getTrainNetwork(const QString &name) const
{
    if (!trainNetworkExists(name))
    {
        return nullptr;
    }
    return m_networkController->trainNetwork(name,
                                             m_region);
}

TruckClient::IntegrationNetwork *
RegionData::getTruckNetwork(const QString &name) const
{
    if (!truckNetworkExists(name))
    {
        return nullptr;
    }
    return m_networkController->truckNetwork(name,
                                             m_region);
}

TruckClient::IntegrationSimulationConfig *
RegionData::getTruckNetworkConfig(const QString &name) const
{
    if (!truckNetworkExists(name))
    {
        return nullptr;
    }
    return m_networkController->truckNetworkConfig(
        name, m_region);
    return m_networkController->truckNetworkConfig(
        name, m_region);
}

QStringList RegionData::getTrainNetworks() const
{
    return m_trainNetworksList;
}

QStringList RegionData::getTruckNetworks() const
{
    return m_truckNetworksList;
}

QMap<QString, QVariant> RegionData::toMap() const
{
    QMap<QString, QVariant> map;

    map["region"] = m_region;

    // Serialize the network lists
    QVariantList trainList;
    for (const QString &network : m_trainNetworksList)
    {
        trainList.append(network);
    }
    map["rail_networks"] = trainList;

    QVariantList truckList;
    for (const QString &network : m_truckNetworksList)
    {
        truckList.append(network);
    }
    map["truck_networks"] = truckList;

    // Add the variables map
    map["variables"] = QVariant::fromValue(m_variables);

    return map;
}

RegionData *
RegionData::fromMap(const QMap<QString, QVariant> &data,
                    NetworkController *networkController,
                    QObject           *parent)
{
    QString regionName = data["region"].toString();

    RegionData *regionData = new RegionData(
        regionName, networkController, parent);

    // Deserialize the network lists
    QVariantList trainList = data["rail_networks"].toList();
    for (const QVariant &network : trainList)
    {
        regionData->m_trainNetworksList.append(
            network.toString());
    }

    QVariantList truckList =
        data.value("truck_networks").toList();
    for (const QVariant &network : truckList)
    {
        regionData->m_truckNetworksList.append(
            network.toString());
    }

    // Deserialize the variables if they exist
    if (data.contains("variables"))
    {
        regionData->m_variables = data["variables"].toMap();
    }

    return regionData;
}

// ---------------------------------------------------------------------------

// RegionDataController implementation

RegionDataController::RegionDataController(
    NetworkController *networkController, QObject *parent)
    : QObject(parent)
    , m_networkController(networkController)
{
}

RegionDataController::~RegionDataController()
{
    clear();
    m_networkController = nullptr;
}

RegionData *
RegionDataController::getRegionData(const QString &name)
{
    return m_regions.value(name, nullptr);
}

QStringList RegionDataController::getAllRegionNames() const
{
    return m_regions.keys();
}

bool RegionDataController::addRegion(const QString &name)
{
    if (m_regions.contains(name))
    {
        return false;
    }

    m_regions[name] =
        new RegionData(name, m_networkController, this);

    // Emit signal that a new region was added
    emit regionAdded(name);

    return true;
}

bool RegionDataController::renameRegion(
    const QString &oldName, const QString &newName)
{
    if (!m_regions.contains(oldName)
        || m_regions.contains(newName))
    {
        return false;
    }

    RegionData *data = m_regions.take(oldName);
    data->setRegionName(newName);
    m_regions[newName] = data;

    if (m_currentRegion == oldName)
    {
        m_currentRegion = newName;
        // Emit that current region has changed
        emit currentRegionChanged(newName);
    }

    // Emit signal that a region was renamed
    emit regionRenamed(oldName, newName);

    return true;
}

bool RegionDataController::removeRegion(const QString &name)
{
    if (!m_regions.contains(name))
    {
        return false;
    }

    // Check if we're removing the current region
    bool isCurrentRegion = (m_currentRegion == name);

    // Get region data to clean up all networks
    RegionData *data = m_regions.take(name);

    // Remove all train networks in this region
    QStringList trainNetworks = data->getTrainNetworks();
    for (const QString &network : trainNetworks)
    {
        try
        {
            data->removeTrainNetwork(network);
        }
        catch (const std::exception &e)
        {
            qWarning()
                << "Error removing train network" << network
                << "during region removal:" << e.what();
        }
    }

    // Remove all truck networks in this region
    QStringList truckNetworks = data->getTruckNetworks();
    for (const QString &network : truckNetworks)
    {
        try
        {
            data->removeTruckNetwork(network);
        }
        catch (const std::exception &e)
        {
            qWarning()
                << "Error removing truck network" << network
                << "during region removal:" << e.what();
        }
    }

    // Delete the region data
    delete data;

    // Emit signal that a region was removed
    emit regionRemoved(name);

    // If current region was removed, update it and notify
    if (isCurrentRegion)
    {
        m_currentRegion = QString();
        emit currentRegionChanged(m_currentRegion);
    }

    return true;
}

RegionData *
RegionDataController::getCurrentRegionData() const
{
    if (m_currentRegion.isEmpty())
    {
        return nullptr;
    }
    return m_regions.value(m_currentRegion, nullptr);
}

bool RegionDataController::setCurrentRegion(
    const QString &name)
{
    // If name is empty, clear the current region
    if (name.isEmpty())
    {
        if (!m_currentRegion.isEmpty())
        {
            m_currentRegion = QString();
            emit currentRegionChanged(m_currentRegion);
        }
        return true;
    }

    // Check if the region exists
    if (!m_regions.contains(name))
    {
        return false;
    }

    // Set the current region if it's different
    if (m_currentRegion != name)
    {
        m_currentRegion = name;
        emit currentRegionChanged(m_currentRegion);
    }

    return true;
}

void RegionDataController::clear()
{
    // Delete all RegionData objects
    for (RegionData *data : m_regions.values())
    {
        // Remove all train networks in this region
        QStringList trainNetworks =
            data->getTrainNetworks();
        for (const QString &network : trainNetworks)
        {
            try
            {
                data->removeTrainNetwork(network);
            }
            catch (const std::exception &e)
            {
                qWarning() << "Error removing train network"
                           << network
                           << "during clear:" << e.what();
            }
        }

        // Remove all truck networks in this region
        QStringList truckNetworks =
            data->getTruckNetworks();
        for (const QString &network : truckNetworks)
        {
            try
            {
                data->removeTruckNetwork(network);
            }
            catch (const std::exception &e)
            {
                qWarning() << "Error removing truck network"
                           << network
                           << "during clear:" << e.what();
            }
        }

        delete data;
    }

    m_regions.clear();
    m_currentRegion = QString(); // Reset current region
    m_globalVariables.clear();   // Clear global variables

    // Emit signals
    emit regionsCleared();
    emit currentRegionChanged(m_currentRegion);
}

QMap<QString, QVariant> RegionDataController::toMap() const
{
    QMap<QString, QVariant> map;
    QMap<QString, QVariant> regionsMap;

    for (auto it = m_regions.constBegin();
         it != m_regions.constEnd(); ++it)
    {
        regionsMap[it.key()] = it.value()->toMap();
    }

    map["regions"]        = QVariant::fromValue(regionsMap);
    map["current_region"] = m_currentRegion;

    // Add global variables
    map["global_variables"] =
        QVariant::fromValue(m_globalVariables);

    return map;
}

bool RegionDataController::fromMap(
    NetworkController             *networkController,
    const QMap<QString, QVariant> &data)
{
    // Clear existing data (this will emit regionsCleared)
    clear();

    m_networkController = networkController;

    try
    {
        QMap<QString, QVariant> regionsMap =
            data["regions"].toMap();

        for (auto it = regionsMap.constBegin();
             it != regionsMap.constEnd(); ++it)
        {
            QString                 regionName = it.key();
            QMap<QString, QVariant> regionData =
                it.value().toMap();

            RegionData *region = RegionData::fromMap(
                regionData, networkController, this);
            m_regions[regionName] = region;

            // Emit signal for each region added
            emit regionAdded(regionName);
        }

        // Restore current region if it exists
        if (data.contains("current_region"))
        {
            QString newCurrentRegion =
                data["current_region"].toString();
            if (!newCurrentRegion.isEmpty()
                && m_regions.contains(newCurrentRegion))
            {
                m_currentRegion = newCurrentRegion;
                emit currentRegionChanged(m_currentRegion);
            }
        }

        // Restore global variables if they exist
        if (data.contains("global_variables"))
        {
            m_globalVariables =
                data["global_variables"].toMap();
        }

        return true;
    }
    catch (const std::exception &e)
    {
        qWarning() << "Error deserializing regions data:"
                   << e.what();
        clear();
        return false;
    }
}

} // namespace Backend
} // namespace CargoNetSim
