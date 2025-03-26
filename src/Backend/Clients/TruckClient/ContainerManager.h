/**
 * @file ContainerManager.h
 * @brief Manages container tracking across vehicles
 * @author Ahmed Aredah
 * @date 2025-03-22
 */

#pragma once

#include <QMap>
#include <QObject>
#include <QString>
#include <QVector>
#include <containerLib/container.h>

namespace CargoNetSim
{
namespace Backend
{
namespace TruckClient
{

/**
 * @class ContainerManager
 * @brief Tracks containers across transportation vehicles
 *
 * Provides a central registry for container location
 * tracking and facilitates container transfers between
 * vehicles.
 */
class ContainerManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent The parent QObject
     */
    explicit ContainerManager(QObject *parent = nullptr);

    /**
     * @brief Assigns containers to a vehicle
     * @param vehicleId The vehicle identifier
     * @param containers List of containers to assign
     */
    void assignContainersToVehicle(
        const QString &vehicleId,
        const QList<ContainerCore::Container *>
            &containers);

    /**
     * @brief Removes containers from a vehicle
     * @param vehicleId The vehicle identifier
     * @param containers List of containers to remove
     * @return List of removed containers
     */
    QList<ContainerCore::Container *>
    removeContainersFromVehicle(
        const QString &vehicleId,
        const QList<ContainerCore::Container *>
            &containers);

    /**
     * @brief Removes all containers from a vehicle
     * @param vehicleId The vehicle identifier
     * @return List of all removed containers
     */
    QList<ContainerCore::Container *>
    removeAllContainersFromVehicle(
        const QString &vehicleId);

    /**
     * @brief Transfers containers between vehicles
     * @param sourceVehicleId Source vehicle identifier
     * @param destVehicleId Destination vehicle identifier
     * @param containers List of containers to transfer
     * @return True if transfer was successful
     */
    bool transferContainers(
        const QString &sourceVehicleId,
        const QString &destVehicleId,
        const QList<ContainerCore::Container *>
            &containers);

    /**
     * @brief Gets containers assigned to a vehicle
     * @param vehicleId The vehicle identifier
     * @return List of assigned containers
     */
    QList<ContainerCore::Container *>
    getContainersForVehicle(const QString &vehicleId) const;

    /**
     * @brief Gets the vehicle ID for a container
     * @param container The container to locate
     * @return Vehicle ID or empty string if not found
     */
    QString getVehicleForContainer(
        const ContainerCore::Container *container) const;

    /**
     * @brief Checks if a container is assigned to a vehicle
     * @param vehicleId The vehicle identifier
     * @param container The container to check
     * @return True if the container is assigned to the
     * vehicle
     */
    bool isContainerAssignedToVehicle(
        const QString                  &vehicleId,
        const ContainerCore::Container *container) const;

signals:
    /**
     * @brief Signal emitted when containers are assigned
     * @param vehicleId The vehicle identifier
     * @param containerIds List of container identifiers
     */
    void
    containersAssigned(const QString     &vehicleId,
                       const QStringList &containerIds);

    /**
     * @brief Signal emitted when containers are removed
     * @param vehicleId The vehicle identifier
     * @param containerIds List of container identifiers
     */
    void containersRemoved(const QString     &vehicleId,
                           const QStringList &containerIds);

    /**
     * @brief Signal emitted when containers are transferred
     * @param sourceId Source vehicle identifier
     * @param destId Destination vehicle identifier
     * @param containerIds List of container identifiers
     */
    void
    containersTransferred(const QString     &sourceId,
                          const QString     &destId,
                          const QStringList &containerIds);

private:
    // Map of vehicle IDs to lists of containers
    QMap<QString, QList<ContainerCore::Container *>>
        m_containersByVehicle;

    // Map of container pointers to vehicle IDs for quick
    // lookup
    QMap<const ContainerCore::Container *, QString>
        m_vehicleByContainer;
};

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
