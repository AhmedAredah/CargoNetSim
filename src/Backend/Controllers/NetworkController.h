/**
 * @file NetworkController.h
 * @brief Singleton controller for transportation network
 *        management.
 * @author Ahmed Aredah
 * @date 2025-03-19
 * @copyright Copyright (c) 2025
 */

#pragma once

#include <QMap>
#include <QObject>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <QString>
#include <memory>

#include "Backend/Clients/TrainClient/TrainNetwork.h"
#include "Backend/Clients/TruckClient/TruckNetwork.h"

namespace CargoNetSim
{
namespace Backend
{

/**
 * @class NetworkController
 * @brief Singleton controller for managing train and truck
 *        networks by name and region.
 *
 * This class provides a centralized way to manage
 * different types of networks across multiple regions.
 * It ensures thread-safety and prevents name conflicts
 * within the same region.
 */
class NetworkController : public QObject
{
    Q_OBJECT

    // Make the RegionData a friend
    friend class RegionData;

    // Make the RegionDataController a friend
    friend class RegionDataController;

public:
    /**
     * @brief Constructor for NetworkController.
     * @param parent Optional parent QObject.
     */
    explicit NetworkController(QObject *parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~NetworkController();

    // Delete copy and move constructors and operators
    NetworkController(const NetworkController &) = delete;
    NetworkController &
    operator=(const NetworkController &)    = delete;
    NetworkController(NetworkController &&) = delete;
    NetworkController &
    operator=(NetworkController &&) = delete;

    /**
     * @brief Add a train network to the controller.
     * @param name Unique name for the network within its
     *        region.
     * @param region Region to which the network belongs.
     * @param network Pointer to the train network
     *        (ownership is transferred).
     * @return True if added successfully, false if
     *         a network with the same name already exists
     *         in the region.
     */
    bool addTrainNetwork(
        const QString &name, const QString &region,
        TrainClient::NeTrainSimNetwork *network);

    /**
     * @brief Add a truck network config to the
     *        controller.
     * @param name Unique name for the network within its
     *        region.
     * @param region Region to which the network belongs.
     * @param config Pointer to the truck network
     *        configuration (ownership is transferred).
     * @return True if added successfully, false if
     *         a network with the same name already exists
     *         in the region.
     */
    bool addTruckNetworkConfig(
        const QString &name, const QString &region,
        TruckClient::IntegrationSimulationConfig *config);

    /**
     * @brief Get a train network by name and region.
     * @param name Name of the network.
     * @param region Region of the network.
     * @return Pointer to the train network, or nullptr
     *         if not found.
     */
    TrainClient::NeTrainSimNetwork *
    trainNetwork(const QString &name,
                 const QString &region) const;

    /**
     * @brief Get a truck network configuration by name
     *        and region.
     * @param name Name of the network.
     * @param region Region of the network.
     * @return Pointer to the truck network configuration,
     *         or nullptr if not found.
     */
    TruckClient::IntegrationSimulationConfig *
    truckNetworkConfig(const QString &name,
                       const QString &region) const;

    /**
     * @brief Get a truck network by name and region.
     * @param name Name of the network.
     * @param region Region of the network.
     * @return Pointer to the truck network, or nullptr
     *         if not found.
     * @note This method extracts the network from the
     *       associated configuration.
     */
    TruckClient::IntegrationNetwork *
    truckNetwork(const QString &name,
                 const QString &region) const;

    /**
     * @brief Remove a train network from the controller.
     * @param name Name of the network.
     * @param region Region of the network.
     * @return True if removed successfully, false if not
     *         found.
     */
    bool removeTrainNetwork(const QString &name,
                            const QString &region);

    /**
     * @brief Remove a truck network configuration from
     *        the controller.
     * @param name Name of the network.
     * @param region Region of the network.
     * @return True if removed successfully, false if not
     *         found.
     */
    bool removeTruckNetworkConfig(const QString &name,
                                  const QString &region);

    /**
     * @brief Rename a train network in a specific region.
     * @param oldName Current name of the network.
     * @param newName New name for the network.
     * @param region Region where the network is located.
     * @return True if renamed successfully, false if old
     * name doesn't exist or new name already exists.
     */
    bool renameTrainNetwork(const QString &oldName,
                            const QString &newName,
                            const QString &region);

    /**
     * @brief Rename a truck network configuration in a
     * specific region.
     * @param oldName Current name of the network.
     * @param newName New name for the network.
     * @param region Region where the network is located.
     * @return True if renamed successfully, false if old
     * name doesn't exist or new name already exists.
     */
    bool renameTruckNetworkConfig(const QString &oldName,
                                  const QString &newName,
                                  const QString &region);

    /**
     * @brief Get all train networks in a region.
     * @param region Region to query.
     * @return QMap of network names to network pointers.
     */
    QMap<QString, TrainClient::NeTrainSimNetwork *>
    trainNetworksInRegion(const QString &region) const;

    /**
     * @brief Get all truck network configurations in a
     *        region.
     * @param region Region to query.
     * @return QMap of network names to network
     *         configuration pointers.
     */
    QMap<QString,
         TruckClient::IntegrationSimulationConfig *>
    truckNetworkConfigsInRegion(
        const QString &region) const;

    /**
     * @brief Get all regions with networks.
     * @return List of region names.
     */
    QStringList regions() const;

    /**
     * @brief Clear all networks from the controller.
     *
     * Removes and deletes all train networks and truck
     * network configurations from all regions.
     */
    void clear();

    /**
     * @brief Clear all train networks from the controller.
     *
     * Removes and deletes all train networks from all
     * regions. This operation is thread-safe.
     *
     * @return The number of train networks that were
     * removed.
     */
    int clearTrainNetworks();

    /**
     * @brief Clear all truck networks from the controller.
     *
     * Removes and deletes all truck network configurations
     * from all regions. This operation is thread-safe.
     *
     * @return The number of truck network configurations
     * that were removed.
     */
    int clearTruckNetworks();

    /**
     * @brief Check if any network (train or truck) with the
     * given name exists in the region.
     * @param name The network name to check.
     * @param region The region to check in.
     * @return True if a network with this name exists in
     * the region.
     */
    bool networkExistsInRegion(const QString &name,
                               const QString &region) const;

    /**
     * @brief Rename a region and transfer all its networks
     * to the new region name.
     * @param oldRegion The current region name.
     * @param newRegion The new region name.
     * @return True if successful, false if newRegion
     * already exists or operation fails.
     */
    bool renameRegion(const QString &oldRegion,
                      const QString &newRegion);

    /**
     * @brief Get train network names in a region.
     * @param region The region to query.
     * @return QStringList containing all train network
     * names in the region.
     */
    QStringList
    trainNetworkNamesInRegion(const QString &region) const;

    /**
     * @brief Get truck network names in a region.
     * @param region The region to query.
     * @return QStringList containing all truck network
     * names in the region.
     */
    QStringList
    truckNetworkNamesInRegion(const QString &region) const;

    /**
     * @brief Check if a train network exists in a region.
     * @param name The network name to check.
     * @param region The region to check in.
     * @return True if the train network exists.
     */
    bool trainNetworkExists(const QString &name,
                            const QString &region) const;

    /**
     * @brief Check if a truck network exists in a region.
     * @param name The network name to check.
     * @param region The region to check in.
     * @return True if the truck network exists.
     */
    bool truckNetworkExists(const QString &name,
                            const QString &region) const;

    /**
     * @brief Clear all networks in a specific region.
     * @param region The region to clear.
     * @return The number of networks removed.
     */
    int clearRegion(const QString &region);

signals:
    /**
     * @brief Emitted when a train network is added.
     * @param name Name of the network.
     * @param region Region of the network.
     */
    void trainNetworkAdded(const QString &name,
                           const QString &region);

    /**
     * @brief Emitted when a truck network configuration
     *        is added.
     * @param name Name of the network.
     * @param region Region of the network.
     */
    void truckNetworkConfigAdded(const QString &name,
                                 const QString &region);

    /**
     * @brief Emitted when a train network is removed.
     * @param name Name of the network.
     * @param region Region of the network.
     */
    void trainNetworkRemoved(const QString &name,
                             const QString &region);

    /**
     * @brief Emitted when a truck network configuration
     *        is removed.
     * @param name Name of the network.
     * @param region Region of the network.
     */
    void truckNetworkConfigRemoved(const QString &name,
                                   const QString &region);

    /**
     * @brief Emitted when a train network is renamed.
     * @param oldName Previous name of the network.
     * @param newName New name of the network.
     * @param region Region of the network.
     */
    void trainNetworkRenamed(const QString &oldName,
                             const QString &newName,
                             const QString &region);

    /**
     * @brief Emitted when a truck network configuration
     *        is renamed.
     * @param oldName Previous name of the network.
     * @param newName New name of the network.
     * @param region Region of the network.
     */
    void truckNetworkConfigRenamed(const QString &oldName,
                                   const QString &newName,
                                   const QString &region);

    /**
     * @brief Emitted when a region is renamed.
     * @param oldRegion The previous name of the region.
     * @param newRegion The new name of the region.
     */
    void regionRenamed(const QString &oldRegion,
                       const QString &newRegion);

    /**
     * @brief Emitted when a region is cleared.
     * @param region The name of the cleared region.
     */
    void regionCleared(const QString &region);

private:
    /** @brief Lock for thread safety of train networks */
    mutable QReadWriteLock m_trainNetworksLock;

    /** @brief Lock for thread safety of truck networks */
    mutable QReadWriteLock m_truckNetworkConfigsLock;

    /**
     * Storage for networks by region and name.
     * Using QMap<QString, QMap<QString, T*>> structure
     * where:
     * - Outer key is the region
     * - Inner key is the network name
     * - Value is the pointer to the network/configuration
     */
    QMap<QString,
         QMap<QString, TrainClient::NeTrainSimNetwork *>>
        m_trainNetworks;

    QMap<QString,
         QMap<QString,
              TruckClient::IntegrationSimulationConfig *>>
        m_truckNetworkConfigs;
};

} // namespace Backend
} // namespace CargoNetSim

// Register custom types with Qt's meta-object system
Q_DECLARE_METATYPE(CargoNetSim::Backend::NetworkController)
Q_DECLARE_METATYPE(
    CargoNetSim::Backend::NetworkController *)
