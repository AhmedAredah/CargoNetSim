// NetworkController.cpp
#include "NetworkController.h"

namespace CargoNetSim {
namespace Backend {


void NetworkControllerCleanup::cleanup() {
    QWriteLocker writeLocker(&NetworkController::m_instanceLock);
    if (NetworkController::m_instance) {
        delete NetworkController::m_instance;
        NetworkController::m_instance = nullptr;
    }
}


// Initialize static members
NetworkController* NetworkController::m_instance = nullptr;
QReadWriteLock NetworkController::m_instanceLock;

NetworkController& NetworkController::getInstance(QObject* parent) {
    QReadLocker locker(&m_instanceLock);
    if (!m_instance) {
        locker.unlock();
        QWriteLocker writeLocker(&m_instanceLock);
        // Double-check pattern to ensure thread safety
        if (!m_instance) {
            m_instance = new NetworkController(parent);
        }
        writeLocker.unlock();
        locker.relock();
    }
    return *m_instance;
}

NetworkController::NetworkController(QObject* parent)
    : QObject(parent) {
}

NetworkController::~NetworkController() {
    // Clean up all train networks
    QWriteLocker trainLocker(&m_trainNetworksLock);
    for (auto& regionNetworks : m_trainNetworks) {
        qDeleteAll(regionNetworks);
    }
    m_trainNetworks.clear();
    trainLocker.unlock();

    // Clean up all truck network configs
    QWriteLocker truckLocker(&m_truckNetworkConfigsLock);
    for (auto& regionConfigs : m_truckNetworkConfigs) {
        qDeleteAll(regionConfigs);
    }
    m_truckNetworkConfigs.clear();
}

bool NetworkController::addTrainNetwork(const QString& name,
                                        const QString& region,
                                        NeTrainSimNetworkBase* network) {
    if (!network) {
        qWarning() << "Attempted to add null train network:"
                   << name << "in region" << region;
        return false;
    }

    QWriteLocker locker(&m_trainNetworksLock);

    // Check if network with this name already exists in the region
    if (m_trainNetworks.contains(region) &&
        m_trainNetworks[region].contains(name)) {
        qWarning() << "Train network with name"
                   << name << "already exists in region"
                   << region;
        return false;
    }

    // Add the network
    m_trainNetworks[region][name] = network;
    locker.unlock();

    // Take ownership of the network
    network->setParent(this);

    // Emit signal
    emit trainNetworkAdded(name, region);
    return true;
}

bool
    NetworkController::
    addTruckNetworkConfig(const QString& name,
                          const QString& region,
                          IntegrationSimulationFormatIConfigBase* config) {
    if (!config) {
        qWarning() << "Attempted to add null truck network config:"
                   << name << "in region"
                   << region;
        return false;
    }

    QWriteLocker locker(&m_truckNetworkConfigsLock);

    // Check if config with this name already exists in the region
    if (m_truckNetworkConfigs.contains(region) &&
        m_truckNetworkConfigs[region].contains(name)) {
        qWarning() << "Truck network config with name"
                   << name << "already exists in region"
                   << region;
        return false;
    }

    // Add the config
    m_truckNetworkConfigs[region][name] = config;
    locker.unlock();

    // Take ownership of the config
    config->setParent(this);

    // Emit signal
    emit truckNetworkConfigAdded(name, region);
    return true;
}

NeTrainSimNetworkBase*
NetworkController::trainNetwork(const QString& name,
                                const QString& region) const {
    QReadLocker locker(&m_trainNetworksLock);

    if (!m_trainNetworks.contains(region) ||
        !m_trainNetworks[region].contains(name)) {
        return nullptr;
    }

    return m_trainNetworks[region][name];
}

IntegrationSimulationFormatIConfigBase*
NetworkController::truckNetworkConfig(const QString& name,
                                      const QString& region) const {
    QReadLocker locker(&m_truckNetworkConfigsLock);

    if (!m_truckNetworkConfigs.contains(region) ||
        !m_truckNetworkConfigs[region].contains(name)) {
        return nullptr;
    }

    return m_truckNetworkConfigs[region][name];
}

IntegrationNetworkBase*
NetworkController::truckNetwork(const QString& name,
                                const QString& region) const {
    QReadLocker locker(&m_truckNetworkConfigsLock);

    if (!m_truckNetworkConfigs.contains(region) ||
        !m_truckNetworkConfigs[region].contains(name)) {
        return nullptr;
    }

    IntegrationSimulationFormatIConfigBase* config =
        m_truckNetworkConfigs[region][name];
    locker.unlock();

    // Get the network from the config
    return config->getNetwork();
}

bool NetworkController::removeTrainNetwork(const QString& name,
                                           const QString& region) {
    QWriteLocker locker(&m_trainNetworksLock);

    if (!m_trainNetworks.contains(region) ||
        !m_trainNetworks[region].contains(name)) {
        return false;
    }

    // Store pointer to delete after removal from map
    NeTrainSimNetworkBase* network = m_trainNetworks[region][name];

    // Remove from map
    m_trainNetworks[region].remove(name);

    // Clean up empty regions
    if (m_trainNetworks[region].isEmpty()) {
        m_trainNetworks.remove(region);
    }

    locker.unlock();

    // Delete the network
    delete network;

    // Emit signal
    emit trainNetworkRemoved(name, region);
    return true;
}

bool NetworkController::removeTruckNetworkConfig(const QString& name,
                                                 const QString& region) {
    QWriteLocker locker(&m_truckNetworkConfigsLock);

    if (!m_truckNetworkConfigs.contains(region) ||
        !m_truckNetworkConfigs[region].contains(name)) {
        return false;
    }

    // Store pointer to delete after removal from map
    IntegrationSimulationFormatIConfigBase* config =
        m_truckNetworkConfigs[region][name];

    // Remove from map
    m_truckNetworkConfigs[region].remove(name);

    // Clean up empty regions
    if (m_truckNetworkConfigs[region].isEmpty()) {
        m_truckNetworkConfigs.remove(region);
    }

    locker.unlock();

    // Delete the config (which will also delete the network)
    delete config;

    // Emit signal
    emit truckNetworkConfigRemoved(name, region);
    return true;
}

QMap<QString, NeTrainSimNetworkBase*>
NetworkController::trainNetworksInRegion(const QString& region) const {
    QReadLocker locker(&m_trainNetworksLock);

    if (!m_trainNetworks.contains(region)) {
        return QMap<QString, NeTrainSimNetworkBase*>();
    }

    return m_trainNetworks[region];
}

QMap<QString, IntegrationSimulationFormatIConfigBase*>
NetworkController::truckNetworkConfigsInRegion(const QString& region) const {
    QReadLocker locker(&m_truckNetworkConfigsLock);

    if (!m_truckNetworkConfigs.contains(region)) {
        return QMap<QString, IntegrationSimulationFormatIConfigBase*>();
    }

    return m_truckNetworkConfigs[region];
}


QStringList NetworkController::regions() const {
    QStringList result;

    {
        QReadLocker trainLocker(&m_trainNetworksLock);
        result = m_trainNetworks.keys();
    }

    {
        QReadLocker truckLocker(&m_truckNetworkConfigsLock);
        for (const QString& region : m_truckNetworkConfigs.keys()) {
            if (!result.contains(region)) {
                result.append(region);
            }
        }
    }

    return result;
}

} // namespace Backend
} // namespace CargoNetSim
