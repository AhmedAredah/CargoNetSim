// NetworkController.h
#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <memory>

#include "../Models/TrainNetwork.h"
#include "../Models/TruckNetwork.h"

namespace CargoNetSim {
namespace Backend {

// Forward declare the cleanup class
class NetworkControllerCleanup {
public:
    /**
     * @brief Cleanup the RegionDataController singleton instance
     */
    static void cleanup();
};

/**
 * @class NetworkController
 * @brief Singleton controller for managing train and truck networks by name and region.
 *
 * This class provides a centralized way to manage different types of networks across
 * multiple regions. It ensures thread-safety and prevents name conflicts within the same region.
 */
class NetworkController : public QObject {
    Q_OBJECT

    // Make the cleanup class a friend
    friend class NetworkControllerCleanup;

public:
    /**
     * @brief Get the singleton getInstance of NetworkController
     * @param parent Optional parent QObject
     * @return Reference to the singleton getInstance
     */
    static NetworkController& getInstance(QObject* parent = nullptr);

    // Delete copy and move constructors and operators
    NetworkController(const NetworkController&) = delete;
    NetworkController& operator=(const NetworkController&) = delete;
    NetworkController(NetworkController&&) = delete;
    NetworkController& operator=(NetworkController&&) = delete;

    /**
     * @brief Add a train network to the controller
     * @param name Unique name for the network within its region
     * @param region Region to which the network belongs
     * @param network Pointer to the train network (ownership is transferred)
     * @return True if added successfully, false if a network with the same name already exists in the region
     */
    bool addTrainNetwork(const QString& name, const QString& region, NeTrainSimNetworkBase* network);

    /**
     * @brief Add a truck network config to the controller
     * @param name Unique name for the network within its region
     * @param region Region to which the network belongs
     * @param config Pointer to the truck network configuration (ownership is transferred)
     * @return True if added successfully, false if a network with the same name already exists in the region
     */
    bool addTruckNetworkConfig(const QString& name, const QString& region, IntegrationSimulationFormatIConfigBase* config);

    /**
     * @brief Get a train network by name and region
     * @param name Name of the network
     * @param region Region of the network
     * @return Pointer to the train network, or nullptr if not found
     */
    NeTrainSimNetworkBase* trainNetwork(const QString& name, const QString& region) const;

    /**
     * @brief Get a truck network configuration by name and region
     * @param name Name of the network
     * @param region Region of the network
     * @return Pointer to the truck network configuration, or nullptr if not found
     */
    IntegrationSimulationFormatIConfigBase* truckNetworkConfig(const QString& name, const QString& region) const;

    /**
     * @brief Get a truck network by name and region
     * @param name Name of the network
     * @param region Region of the network
     * @return Pointer to the truck network, or nullptr if not found
     * @note This method extracts the network from the associated configuration
     */
    IntegrationNetworkBase* truckNetwork(const QString& name, const QString& region) const;

    /**
     * @brief Remove a train network from the controller
     * @param name Name of the network
     * @param region Region of the network
     * @return True if removed successfully, false if not found
     */
    bool removeTrainNetwork(const QString& name, const QString& region);

    /**
     * @brief Remove a truck network configuration from the controller
     * @param name Name of the network
     * @param region Region of the network
     * @return True if removed successfully, false if not found
     */
    bool removeTruckNetworkConfig(const QString& name, const QString& region);

    /**
     * @brief Get all train networks in a region
     * @param region Region to query
     * @return QMap of network names to network pointers
     */
    QMap<QString, NeTrainSimNetworkBase*> trainNetworksInRegion(const QString& region) const;

    /**
     * @brief Get all truck network configurations in a region
     * @param region Region to query
     * @return QMap of network names to network configuration pointers
     */
    QMap<QString, IntegrationSimulationFormatIConfigBase*> truckNetworkConfigsInRegion(const QString& region) const;

    /**
     * @brief Get all regions with networks
     * @return List of region names
     */
    QStringList regions() const;

signals:
    /**
     * @brief Emitted when a train network is added
     * @param name Name of the network
     * @param region Region of the network
     */
    void trainNetworkAdded(const QString& name, const QString& region);

    /**
     * @brief Emitted when a truck network configuration is added
     * @param name Name of the network
     * @param region Region of the network
     */
    void truckNetworkConfigAdded(const QString& name, const QString& region);

    /**
     * @brief Emitted when a train network is removed
     * @param name Name of the network
     * @param region Region of the network
     */
    void trainNetworkRemoved(const QString& name, const QString& region);

    /**
     * @brief Emitted when a truck network configuration is removed
     * @param name Name of the network
     * @param region Region of the network
     */
    void truckNetworkConfigRemoved(const QString& name, const QString& region);

protected:
    // Singleton instance
    static NetworkController* m_instance;

private:
    /**
     * @brief Private constructor for singleton pattern
     * @param parent Optional parent QObject
     */
    explicit NetworkController(QObject* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~NetworkController();

    // Lock for thread safety of the singleton creation
    static QReadWriteLock m_instanceLock;

    // Locks for thread safety of network operations
    mutable QReadWriteLock m_trainNetworksLock;
    mutable QReadWriteLock m_truckNetworkConfigsLock;

    // Storage for networks by region and name
    // Using QMap<QString, QMap<QString, T*>> structure where:
    // - Outer key is the region
    // - Inner key is the network name
    // - Value is the pointer to the network or configuration
    QMap<QString, QMap<QString, NeTrainSimNetworkBase*>> m_trainNetworks;
    QMap<QString, QMap<QString, IntegrationSimulationFormatIConfigBase*>> m_truckNetworkConfigs;
};

} // namespace Backend
} // namespace CargoNetSim
