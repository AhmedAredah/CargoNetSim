/**
 * @file VehicleController.h
 * @brief Singleton controller for managing vehicles in the
 *        CargoNetSim system.
 * @author Ahmed Aredah
 * @date 2025-03-19
 * @copyright Copyright (c) 2025
 */

#pragma once
#include "../Models/ShipSystem.h"
#include "../Models/TrainSystem.h"
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QVector>
#include <QtCore/qreadwritelock.h>

// Forward declarations
namespace CargoNetSim
{
class CargoNetSimController;
}

namespace CargoNetSim
{
namespace Backend
{

/**
 * @class VehicleControllerCleanup
 * @brief Utility class to handle singleton cleanup.
 */
class VehicleControllerCleanup
{
public:
    /**
     * @brief Cleanup the VehicleController singleton
     *        instance.
     */
    static void cleanup();
};

/**
 * @class VehicleController
 * @brief Singleton controller that manages all ship and
 *        train vehicles.
 *
 * This class provides centralized management of all
 * transportation vehicles in the simulation, including
 * loading from files, tracking, and updating vehicle
 * states.
 */
class VehicleController : public QObject
{
    Q_OBJECT

    friend class VehicleControllerCleanup;

    // Make the CargoNetSimController a friend
    friend class CargoNetSim::CargoNetSimController;

public:
    /**
     * @brief Constructor for VehicleController.
     * @param parent Optional parent QObject.
     */
    explicit VehicleController(QObject *parent = nullptr);

    /**
     * @brief destructor for VehicleController.
     */
    ~VehicleController();

    // Disable copy constructor and assignment operator
    VehicleController(const VehicleController &) = delete;
    VehicleController &
    operator=(const VehicleController &) = delete;

    // Ship Management

    /**
     * @brief Load ships from a configuration file.
     * @param filePath Path to the ship configuration file.
     * @return True if ships were loaded successfully.
     */
    bool loadShipsFromFile(const QString &filePath);

    /**
     * @brief Get a ship by its identifier.
     * @param shipId The unique identifier of the ship.
     * @return Pointer to the ship, or nullptr if not found.
     */
    Ship *getShip(const QString &shipId) const;

    /**
     * @brief Get all ships in the system.
     * @return Vector of pointers to all ships.
     */
    QVector<Ship *> getAllShips() const;

    /**
     * @brief Add a new ship to the system.
     * @param ship Pointer to the ship to add.
     * @return True if the ship was added successfully.
     */
    bool addShip(Ship *ship);

    /**
     * @brief Remove a ship from the system.
     * @param shipId The unique identifier of the ship.
     * @return True if the ship was removed successfully.
     */
    bool removeShip(const QString &shipId);

    /**
     * @brief Update an existing ship's properties.
     * @param ship Pointer to the updated ship.
     * @return True if the ship was updated successfully.
     */
    bool updateShip(Ship *ship);

    /**
     * @brief Update multiple ships at once.
     * @param ships Vector of pointers to updated ships.
     * @return True if all ships were updated successfully.
     */
    bool updateShips(QVector<Ship *> ships);

    /**
     * @brief Get the total number of ships in the system.
     * @return Count of ships.
     */
    int shipCount() const;

    // Train Management

    /**
     * @brief Load trains from a configuration file.
     * @param filePath Path to the train configuration file.
     * @return True if trains were loaded successfully.
     */
    bool loadTrainsFromFile(const QString &filePath);

    /**
     * @brief Get a train by its identifier.
     * @param userId The unique identifier of the train.
     * @return Pointer to the train, or nullptr if not
     *         found.
     */
    Train *getTrain(const QString &userId) const;

    /**
     * @brief Get all trains in the system.
     * @return Vector of pointers to all trains.
     */
    QVector<Train *> getAllTrains() const;

    /**
     * @brief Add a new train to the system.
     * @param train Pointer to the train to add.
     * @return True if the train was added successfully.
     */
    bool addTrain(Train *train);

    /**
     * @brief Remove a train from the system.
     * @param userId The unique identifier of the train.
     * @return True if the train was removed successfully.
     */
    bool removeTrain(const QString &userId);

    /**
     * @brief Update an existing train's properties.
     * @param train Pointer to the updated train.
     * @return True if the train was updated successfully.
     */
    bool updateTrain(Train *train);

    /**
     * @brief Update multiple trains at once.
     * @param trains Vector of pointers to updated trains.
     * @return True if all trains were updated successfully.
     */
    bool updateTrains(QVector<Train *> trains);

    /**
     * @brief Get the total number of trains in the system.
     * @return Count of trains.
     */
    int trainCount() const;

    /**
     * @brief Clear all vehicles from the system.
     */
    void clear();

signals:
    /**
     * @brief Signal emitted when a ship is added.
     * @param ship Pointer to the added ship.
     */
    void shipAdded(CargoNetSim::Backend::Ship *ship);

    /**
     * @brief Signal emitted when a ship is removed.
     * @param shipId The identifier of the removed ship.
     */
    void shipRemoved(const QString &shipId);

    /**
     * @brief Signal emitted when a ship is updated.
     * @param ship Pointer to the updated ship.
     */
    void shipUpdated(CargoNetSim::Backend::Ship *ship);

    /**
     * @brief Signal emitted when ships are loaded.
     * @param count Number of ships loaded.
     */
    void shipsLoaded(int count);

    /**
     * @brief Signal emitted when all ships are cleared.
     */
    void shipsCleared();

    /**
     * @brief Signal emitted when a train is added.
     * @param train Pointer to the added train.
     */
    void trainAdded(CargoNetSim::Backend::Train *train);

    /**
     * @brief Signal emitted when a train is removed.
     * @param userId The identifier of the removed train.
     */
    void trainRemoved(const QString &userId);

    /**
     * @brief Signal emitted when a train is updated.
     * @param train Pointer to the updated train.
     */
    void trainUpdated(CargoNetSim::Backend::Train *train);

    /**
     * @brief Signal emitted when trains are loaded.
     * @param count Number of trains loaded.
     */
    void trainsLoaded(int count);

    /**
     * @brief Signal emitted when all trains are cleared.
     */
    void trainsCleared();

private:

    /** @brief Map of ship identifiers to ship objects */
    QMap<QString, Ship *> m_ships;

    /** @brief Map of train identifiers to train objects */
    QMap<QString, Train *> m_trains;
};

} // namespace Backend
} // namespace CargoNetSim

// Register custom types with Qt's meta-object system
Q_DECLARE_METATYPE(CargoNetSim::Backend::VehicleController)
Q_DECLARE_METATYPE(
    CargoNetSim::Backend::VehicleController *)
