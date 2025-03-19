// RegionDataController.cpp
#include "RegionDataController.h"

#include <QDebug>
#include <stdexcept>
#include <QReadLocker>
#include <QWriteLocker>

namespace CargoNetSim {
namespace Backend {


void RegionDataControllerCleanup::cleanup() {
    if (RegionDataController::instance) {
        delete RegionDataController::instance;
        RegionDataController::instance = nullptr;
    }
}



// Initialize static members
RegionDataController* RegionDataController::instance = nullptr;

// RegionData implementation

RegionData::RegionData(const QString& regionName, QObject* parent)
    : QObject(parent), region(regionName) {
    // Initialize networks lists from NetworkController
    auto trainNetworksMap =
        NetworkController::getInstance().trainNetworksInRegion(regionName);
    trainNetworksList = trainNetworksMap.keys();

    auto truckNetworksMap =
        NetworkController::getInstance().truckNetworkConfigsInRegion(regionName);
    truckNetworksList = truckNetworksMap.keys();
}

bool RegionData::checkNetworkNameConflict(const QString& name) const {
    return trainNetworkExists(name) || truckNetworkExists(name);
}

void RegionData::setRegionName(const QString& name) {
    if (region == name) {
        return; // No change needed
    }

    // We need to move all networks from old region to new region
    // This is a complex operation that requires careful handling

    // First, capture the current list of networks
    QStringList currentTrainNetworks = trainNetworksList;
    QStringList currentTruckNetworks = truckNetworksList;

    // Create temporary storage for networks and configs we'll move
    QVector<NeTrainSimNetworkBase*> trainNetworksToMove;
    QVector<IntegrationSimulationFormatIConfigBase*> truckConfigsToMove;

    // Get references to all networks to move
    auto& controller = NetworkController::getInstance();
    for (const QString& networkName : currentTrainNetworks) {
        NeTrainSimNetworkBase* network =
            controller.trainNetwork(networkName, region);
        if (network) {
            trainNetworksToMove.append(network);
        }
    }

    for (const QString& networkName : currentTruckNetworks) {
        IntegrationSimulationFormatIConfigBase* config =
            controller.truckNetworkConfig(networkName, region);
        if (config) {
            truckConfigsToMove.append(config);
        }
    }

    // Remove networks from old region
    for (const QString& networkName : currentTrainNetworks) {
        controller.removeTrainNetwork(networkName, region);
    }

    for (const QString& networkName : currentTruckNetworks) {
        controller.removeTruckNetworkConfig(networkName, region);
    }

    // Update our region name
    region = name;

    // Add networks to new region
    for (int i = 0; i < trainNetworksToMove.size(); ++i) {
        const QString& networkName = currentTrainNetworks[i];
        NeTrainSimNetworkBase* network = trainNetworksToMove[i];
        controller.addTrainNetwork(networkName, name, network);
    }

    for (int i = 0; i < truckConfigsToMove.size(); ++i) {
        const QString& networkName = currentTruckNetworks[i];
        IntegrationSimulationFormatIConfigBase* config = truckConfigsToMove[i];
        controller.addTruckNetworkConfig(networkName, name, config);
    }

    // Our internal lists are already up to date, so no need to update them
}

void RegionData::addTrainNetwork(const QString& networkName,
                                 const QString& nodeFile,
                                 const QString& linkFile) {
    if (checkNetworkNameConflict(networkName)) {
        throw std::runtime_error(
            QString("Network name '%1' already exists in train or "
                    "truck networks").arg(networkName).toStdString()
            );
    }

    try {
        // Create a new train network
        NeTrainSimNetworkBase* network = new NeTrainSimNetworkBase();
        network->loadNetwork(nodeFile, linkFile);

        // Add the network to NetworkController
        if (!NetworkController::getInstance().
             addTrainNetwork(networkName, region, network)) {
            delete network; // Clean up if adding failed
            throw std::runtime_error("Failed to register train network "
                                     "with NetworkController");
        }

        // Add to our local list
        trainNetworksList.append(networkName);

        // Notify listeners
        emit trainNetworkAdded(networkName);

    } catch (const std::exception& e) {
        throw std::runtime_error(
            QString("Failed to create train network: %1")
                .arg(e.what()).toStdString()
            );
    }
}

void RegionData::addTruckNetwork(const QString& networkName,
                                 const QString& configFile) {
    if (checkNetworkNameConflict(networkName)) {
        throw std::runtime_error(
            QString("Network name '%1' already exists "
                    "in train or truck networks")
                .arg(networkName).toStdString()
            );
    }

    try {
        // Parse the config file
        QJsonObject configData =
            IntegrationSimulationConfigReader::readConfig(configFile);
        IntegrationSimulationFormatIConfigBase* config =
            IntegrationSimulationFormatIConfigBase::fromDict(configData);

        // Add the config to NetworkController
        if (!NetworkController::getInstance().addTruckNetworkConfig(networkName,
                                                                 region,
                                                                 config)) {
            delete config; // Clean up if adding failed
            throw std::runtime_error("Failed to register truck network "
                                     "config with NetworkController");
        }

        // Add to our local list
        truckNetworksList.append(networkName);

        // Notify listeners
        emit truckNetworkAdded(networkName);

    } catch (const std::exception& e) {
        throw std::runtime_error(
            QString("Failed to create truck network: %1")
                .arg(e.what()).toStdString()
            );
    }
}

void RegionData::renameTrainNetwork(const QString& oldName,
                                    const QString& newName) {
    if (!trainNetworkExists(oldName)) {
        throw std::runtime_error(
            QString("Train network '%1' not found in region")
                .arg(oldName).toStdString()
            );
    }

    if (oldName != newName && checkNetworkNameConflict(newName)) {
        throw std::runtime_error(
            QString("Network name '%1' already exists")
                .arg(newName).toStdString()
            );
    }

    try {
        // Get the network from NetworkController
        NeTrainSimNetworkBase* network =
            NetworkController::getInstance().trainNetwork(oldName, region);
        if (!network) {
            throw std::runtime_error("Network exists in local list but "
                                     "not in NetworkController");
        }

        // Remove network with old name from NetworkController
        if (!NetworkController::getInstance().removeTrainNetwork(oldName,
                                                              region)) {
            throw std::runtime_error("Failed to remove network with "
                                     "old name from NetworkController");
        }

        // Add network with new name to NetworkController
        if (!NetworkController::getInstance().addTrainNetwork(newName,
                                                           region,
                                                           network)) {
            // Try to restore the old name if adding with new name fails
            if (!NetworkController::getInstance().addTrainNetwork(oldName,
                                                               region,
                                                               network)) {
                // Critical error - network is now not registered at all
                delete network;
                throw std::runtime_error("Failed to re-register network "
                                         "with new name and failed to "
                                         "restore old name");
            }
            throw std::runtime_error("Failed to register network "
                                     "with new name");
        }

        // Update our local list
        int index = trainNetworksList.indexOf(oldName);
        if (index >= 0) {
            trainNetworksList[index] = newName;
        }

        // Notify listeners
        emit trainNetworkRenamed(oldName, newName);

    } catch (const std::exception& e) {
        throw std::runtime_error(
            QString("Failed to rename train network: %1")
                .arg(e.what()).toStdString()
            );
    }
}

void
RegionData::renameTruckNetwork(const QString& oldName,
                                    const QString& newName) {
    if (!truckNetworkExists(oldName)) {
        throw std::runtime_error(
            QString("Truck network '%1' not found in region")
                .arg(oldName).toStdString()
            );
    }

    if (oldName != newName && checkNetworkNameConflict(newName)) {
        throw std::runtime_error(
            QString("Network name '%1' already exists")
                .arg(newName).toStdString()
            );
    }

    try {
        // Get the config from NetworkController
        IntegrationSimulationFormatIConfigBase* config =
            NetworkController::getInstance().truckNetworkConfig(oldName,
                                                             region);
        if (!config) {
            throw std::runtime_error("Network exists in local list but "
                                     "not in NetworkController");
        }

        // Remove config with old name from NetworkController
        if (!NetworkController::getInstance().removeTruckNetworkConfig(oldName,
                                                                    region)) {
            throw std::runtime_error("Failed to remove network with old "
                                     "name from NetworkController");
        }

        // Add config with new name to NetworkController
        if (!NetworkController::getInstance().addTruckNetworkConfig(newName,
                                                                 region,
                                                                 config)) {
            // Try to restore the old name if adding with new name fails
            if (!NetworkController::getInstance().addTruckNetworkConfig(oldName,
                                                                     region,
                                                                     config)) {
                // Critical error - network is now not registered at all
                delete config;
                throw std::runtime_error("Failed to re-register network "
                                         "with new name and failed to "
                                         "restore old name");
            }
            throw std::runtime_error("Failed to register network "
                                     "with new name");
        }

        // Update our local list
        int index = truckNetworksList.indexOf(oldName);
        if (index >= 0) {
            truckNetworksList[index] = newName;
        }

        // Notify listeners
        emit truckNetworkRenamed(oldName, newName);

    } catch (const std::exception& e) {
        throw std::runtime_error(
            QString("Failed to rename truck network: %1").arg(e.what()).toStdString()
            );
    }
}

void RegionData::removeTrainNetwork(const QString& name) {
    if (!trainNetworkExists(name)) {
        throw std::runtime_error(
            QString("Train network '%1' not found in region")
                .arg(name).toStdString()
            );
    }

    try {
        // Remove from NetworkController (this will delete the network object)
        if (!NetworkController::getInstance().removeTrainNetwork(name,
                                                              region)) {
            throw std::runtime_error("Failed to remove network from "
                                     "NetworkController");
        }

        // Remove from our local list
        trainNetworksList.removeAll(name);

        // Notify listeners
        emit trainNetworkRemoved(name);

    } catch (const std::exception& e) {
        throw std::runtime_error(
            QString("Failed to remove train network: %1")
                .arg(e.what()).toStdString()
            );
    }
}

void RegionData::removeTruckNetwork(const QString& name) {
    if (!truckNetworkExists(name)) {
        throw std::runtime_error(
            QString("Truck network '%1' not found in region")
                .arg(name).toStdString()
            );
    }

    try {
        // Remove from NetworkController (this will delete the config)
        if (!NetworkController::getInstance().removeTruckNetworkConfig(name,
                                                                    region)) {
            throw std::runtime_error("Failed to remove network config "
                                     "from NetworkController");
        }

        // Remove from our local list
        truckNetworksList.removeAll(name);

        // Notify listeners
        emit truckNetworkRemoved(name);

    } catch (const std::exception& e) {
        throw std::runtime_error(
            QString("Failed to remove truck network: %1")
                .arg(e.what()).toStdString()
            );
    }
}

bool RegionData::trainNetworkExists(const QString& name) const {
    return trainNetworksList.contains(name);
}

bool RegionData::truckNetworkExists(const QString& name) const {
    return truckNetworksList.contains(name);
}

NeTrainSimNetworkBase*
RegionData::getTrainNetwork(const QString& name) const {
    if (!trainNetworkExists(name)) {
        return nullptr;
    }
    return NetworkController::getInstance().trainNetwork(name, region);
}

IntegrationNetworkBase*
RegionData::getTruckNetwork(const QString& name) const {
    if (!truckNetworkExists(name)) {
        return nullptr;
    }
    return NetworkController::getInstance().truckNetwork(name, region);
}

IntegrationSimulationFormatIConfigBase*
RegionData::getTruckNetworkConfig(const QString& name) const {
    if (!truckNetworkExists(name)) {
        return nullptr;
    }
    return NetworkController::getInstance().truckNetworkConfig(name, region);
}

QStringList RegionData::getTrainNetworks() const {
    return trainNetworksList;
}

QStringList RegionData::getTruckNetworks() const {
    return truckNetworksList;
}

QMap<QString, QVariant> RegionData::toMap() const {
    QMap<QString, QVariant> map;

    map["region"] = region;

    // Serialize the network lists
    QVariantList trainList;
    for (const QString& network : trainNetworksList) {
        trainList.append(network);
    }
    map["train_networks"] = trainList;

    QVariantList truckList;
    for (const QString& network : truckNetworksList) {
        truckList.append(network);
    }
    map["truck_networks"] = truckList;

    // Add the variables map
    map["variables"] = QVariant::fromValue(variables);

    return map;
}

RegionData* RegionData::fromMap(const QMap<QString, QVariant>& data,
                                QObject* parent) {
    QString regionName = data["region"].toString();

    RegionData* regionData = new RegionData(regionName, parent);

    // Deserialize the network lists
    QVariantList trainList = data["train_networks"].toList();
    for (const QVariant& network : trainList) {
        regionData->trainNetworksList.append(network.toString());
    }

    QVariantList truckList = data.value("truck_networks").toList();
    for (const QVariant& network : truckList) {
        regionData->truckNetworksList.append(network.toString());
    }

    // Deserialize the variables if they exist
    if (data.contains("variables")) {
        regionData->variables = data["variables"].toMap();
    }

    return regionData;
}

// ---------------------------------------------------------------------------

// RegionDataController implementation

RegionDataController& RegionDataController::getInstance(QObject* parent) {
    if (!instance) {
        instance = new RegionDataController(parent);
    }
    return *instance;
}

RegionDataController::RegionDataController(QObject* parent)
    : QObject(parent) {
}

RegionDataController::~RegionDataController() {
    clear();
    instance = nullptr;
}

RegionData* RegionDataController::getRegionData(const QString& name) {
    return regions.value(name, nullptr);
}

QStringList RegionDataController::getAllRegionNames() const {
    return regions.keys();
}

bool RegionDataController::addRegion(const QString& name) {
    if (regions.contains(name)) {
        return false;
    }

    regions[name] = new RegionData(name, this);

    // Emit signal that a new region was added
    emit regionAdded(name);

    return true;
}

bool RegionDataController::renameRegion(const QString& oldName, const QString& newName) {
    if (!regions.contains(oldName) || regions.contains(newName)) {
        return false;
    }

    RegionData* data = regions.take(oldName);
    data->setRegionName(newName);
    regions[newName] = data;

    if (currentRegion == oldName) {
        currentRegion = newName;
        // Emit that current region has changed
        emit currentRegionChanged(newName);
    }

    // Emit signal that a region was renamed
    emit regionRenamed(oldName, newName);

    return true;
}

bool RegionDataController::removeRegion(const QString& name) {
    if (!regions.contains(name)) {
        return false;
    }

    // Check if we're removing the current region
    bool isCurrentRegion = (currentRegion == name);

    // Get region data to clean up all networks
    RegionData* data = regions.take(name);

    // Remove all train networks in this region
    QStringList trainNetworks = data->getTrainNetworks();
    for (const QString& network : trainNetworks) {
        try {
            data->removeTrainNetwork(network);
        } catch (const std::exception& e) {
            qWarning() << "Error removing train network" << network << "during region removal:" << e.what();
        }
    }

    // Remove all truck networks in this region
    QStringList truckNetworks = data->getTruckNetworks();
    for (const QString& network : truckNetworks) {
        try {
            data->removeTruckNetwork(network);
        } catch (const std::exception& e) {
            qWarning() << "Error removing truck network" << network << "during region removal:" << e.what();
        }
    }

    // Delete the region data
    delete data;

    // Emit signal that a region was removed
    emit regionRemoved(name);

    // If current region was removed, update it and notify
    if (isCurrentRegion) {
        currentRegion = QString();
        emit currentRegionChanged(currentRegion);
    }

    return true;
}

RegionData* RegionDataController::getCurrentRegionData() const {
    if (currentRegion.isEmpty()) {
        return nullptr;
    }
    return regions.value(currentRegion, nullptr);
}

bool RegionDataController::setCurrentRegion(const QString& name) {
    // If name is empty, clear the current region
    if (name.isEmpty()) {
        if (!currentRegion.isEmpty()) {
            currentRegion = QString();
            emit currentRegionChanged(currentRegion);
        }
        return true;
    }

    // Check if the region exists
    if (!regions.contains(name)) {
        return false;
    }

    // Set the current region if it's different
    if (currentRegion != name) {
        currentRegion = name;
        emit currentRegionChanged(currentRegion);
    }

    return true;
}

void RegionDataController::clear() {
    // Delete all RegionData objects
    for (RegionData* data : regions.values()) {
        // Remove all train networks in this region
        QStringList trainNetworks = data->getTrainNetworks();
        for (const QString& network : trainNetworks) {
            try {
                data->removeTrainNetwork(network);
            } catch (const std::exception& e) {
                qWarning() << "Error removing train network" << network << "during clear:" << e.what();
            }
        }

        // Remove all truck networks in this region
        QStringList truckNetworks = data->getTruckNetworks();
        for (const QString& network : truckNetworks) {
            try {
                data->removeTruckNetwork(network);
            } catch (const std::exception& e) {
                qWarning() << "Error removing truck network" << network << "during clear:" << e.what();
            }
        }

        delete data;
    }

    regions.clear();
    currentRegion = QString(); // Reset current region
    globalVariables.clear();   // Clear global variables

    // Emit signals
    emit regionsCleared();
    emit currentRegionChanged(currentRegion);
}

QMap<QString, QVariant> RegionDataController::toMap() const {
    QMap<QString, QVariant> map;
    QMap<QString, QVariant> regionsMap;

    for (auto it = regions.constBegin(); it != regions.constEnd(); ++it) {
        regionsMap[it.key()] = it.value()->toMap();
    }

    map["regions"] = QVariant::fromValue(regionsMap);
    map["current_region"] = currentRegion;

    // Add global variables
    map["global_variables"] = QVariant::fromValue(globalVariables);

    return map;
}

bool RegionDataController::fromMap(const QMap<QString, QVariant>& data) {
    // Clear existing data (this will emit regionsCleared)
    clear();

    try {
        QMap<QString, QVariant> regionsMap = data["regions"].toMap();

        for (auto it = regionsMap.constBegin(); it != regionsMap.constEnd(); ++it) {
            QString regionName = it.key();
            QMap<QString, QVariant> regionData = it.value().toMap();

            RegionData* region = RegionData::fromMap(regionData, this);
            regions[regionName] = region;

            // Emit signal for each region added
            emit regionAdded(regionName);
        }

        // Restore current region if it exists
        if (data.contains("current_region")) {
            QString newCurrentRegion = data["current_region"].toString();
            if (!newCurrentRegion.isEmpty() && regions.contains(newCurrentRegion)) {
                currentRegion = newCurrentRegion;
                emit currentRegionChanged(currentRegion);
            }
        }

        // Restore global variables if they exist
        if (data.contains("global_variables")) {
            globalVariables = data["global_variables"].toMap();
        }

        return true;
    } catch (const std::exception& e) {
        qWarning() << "Error deserializing regions data:" << e.what();
        clear();
        return false;
    }
}

} // namespace GUI
} // namespace CargoNetSim
