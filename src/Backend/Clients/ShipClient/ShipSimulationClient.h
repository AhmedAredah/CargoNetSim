/**
 * @file ShipSimulationClient.h
 * @brief Header for the ShipSimulationClient class
 *
 * This file defines the ShipSimulationClient class, which
 * manages interactions with the ship simulation server
 * within the CargoNetSim framework. It provides interfaces
 * for defining simulators, managing ships and containers,
 * and retrieving simulation states.
 *
 * @author Ahmed Aredah
 * @date March 19, 2025
 */

#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QString>
#include <containerLib/container.h>

#include "Backend/Clients/BaseClient/SimulationClientBase.h"
#include "Backend/Clients/ShipClient/ShipState.h"
#include "Backend/Clients/ShipClient/SimulationResults.h"
#include "Backend/Commons/ClientType.h"
#include "Backend/Models/ShipSystem.h"

// Forward declarations
namespace CargoNetSim {
namespace Backend {
namespace ShipClient {
class TerminalGraphServer;
class SimulatorTimeServer;
class ProgressBarManager;
class ApplicationLogger;
} // namespace ShipClient
} // namespace Backend
} // namespace CargoNetSim

/**
 * @namespace CargoNetSim::Backend::ShipClient
 * @brief Namespace for ship simulation client components
 *
 * Contains classes and utilities for managing ship
 * simulation operations within the CargoNetSim backend.
 */
namespace CargoNetSim {
namespace Backend {
namespace ShipClient {

/**
 * @class ShipSimulationClient
 * @brief Manages interactions with the ship simulation
 * server
 *
 * This class extends SimulationClientBase to provide
 * specialized functionality for ship simulation, including
 * simulator setup, ship and container management, and state
 * retrieval. It uses RabbitMQ for communication and ensures
 * thread-safe operations.
 *
 * @ingroup ShipSimulation
 */
class ShipSimulationClient : public SimulationClientBase {
    Q_OBJECT

public:
    /**
     * @brief Constructs a ShipSimulationClient instance
     *
     * Initializes the client with a parent object and
     * RabbitMQ connection details. Defaults to localhost
     * and port 5672.
     *
     * @param parent Parent QObject, defaults to nullptr
     * @param host RabbitMQ server hostname, defaults to
     * "localhost"
     * @param port RabbitMQ server port, defaults to 5672
     */
    explicit ShipSimulationClient(
        QObject       *parent = nullptr,
        const QString &host = "localhost", int port = 5672);

    /**
     * @brief Destroys the ShipSimulationClient instance
     *
     * Cleans up resources, including dynamically allocated
     * objects and thread-safe data structures.
     */
    ~ShipSimulationClient() override;

    /**
     * @brief Resets the ship simulation server
     *
     * Sends a reset command to the server, clearing all
     * current simulation data and state.
     *
     * @return True if the reset command succeeds, false
     * otherwise
     */
    bool resetServer();

    /**
     * @brief Initializes the client within its thread
     *
     * Sets up thread-specific resources after the object is
     * moved to its thread. Automatically invoked via
     * QThread::started.
     *
     * @param logger Optional logger for initialization
     * logging
     * @throws std::runtime_error If RabbitMQ setup fails
     * @note Avoid manual calls unless synchronized
     * @warning Call only once after thread start
     */
    void initializeClient(
        LoggerInterface *logger = nullptr) override;

    /**
     * @brief Defines a new ship simulator
     *
     * Configures a simulation network with specified ships
     * and parameters, sending the setup to the server.
     *
     * @param networkName Unique name for the simulation
     * network
     * @param timeStep Time increment for simulation steps
     * @param ships List of Ship pointers to include in the
     * simulator
     * @param destinationTerminalIds Map of ship IDs to
     * terminal IDs
     * @param networkPath Path to network file, defaults to
     * "Default"
     * @return True if the simulator is defined successfully
     */
    bool
    defineSimulator(const QString       &networkName,
                    const double         timeStep,
                    const QList<Ship *> &ships,
                    const QMap<QString, QStringList>
                                  &destinationTerminalIds,
                    const QString &networkPath = "Default");

    /**
     * @brief Runs the simulator for specified networks
     *
     * Starts the simulation for given networks or all if
     * "*" is specified, with an optional time step limit.
     *
     * @param networkNames List of network names or "*" for
     * all
     * @param byTimeSteps Steps to run, -1 for unlimited,
     * defaults to -1
     * @return True if the simulation starts successfully
     */
    bool runSimulator(const QStringList &networkNames,
                      double byTimeSteps = -1.0);

    /**
     * @brief Ends the simulator for specified networks
     *
     * Terminates the simulation for given networks or all
     * if "*" is specified.
     *
     * @param networkNames List of network names or "*" for
     * all
     * @return True if the simulation ends successfully
     */
    bool endSimulator(const QStringList &networkNames);

    /**
     * @brief Adds ships to an existing simulator
     *
     * Incorporates additional ships into a running
     * simulation network, associating them with destination
     * terminals.
     *
     * @param networkName Target network name
     * @param ships List of Ship pointers to add
     * @param destinationTerminalIds Map of ship IDs to
     * terminal IDs
     * @return True if ships are added successfully
     */
    bool
    addShipsToSimulator(const QString       &networkName,
                        const QList<Ship *> &ships,
                        const QMap<QString, QStringList>
                            &destinationTerminalIds);

    /**
     * @brief Adds containers to a ship
     *
     * Assigns containers to a specified ship within a
     * network, sending the command to the server.
     *
     * @param networkName Network containing the ship
     * @param shipId Unique identifier of the target ship
     * @param containers List of Container pointers to add
     * @return True if containers are added successfully
     */
    bool addContainersToShip(
        const QString &networkName, const QString &shipId,
        const QList<ContainerCore::Container *>
            &containers);

    /**
     * @brief Unloads containers from a ship at terminals
     *
     * Removes containers from a ship and assigns them to
     * specified terminals within a network.
     *
     * @param networkName Network containing the ship
     * @param shipId Unique identifier of the target ship
     * @param terminalNames List of terminal names for
     * unloading
     * @return True if unloading succeeds
     */
    bool unloadContainersFromShipAtTerminals(
        const QString &networkName, const QString &shipId,
        const QStringList &terminalNames);

    /**
     * @brief Requests terminal nodes for a network
     *
     * Sends a command to retrieve the terminal nodes of a
     * specified simulation network.
     *
     * @param networkName Name of the network to query
     */
    void
    getNetworkTerminalNodes(const QString &networkName);

    /**
     * @brief Requests shortest path between nodes
     *
     * Sends a command to compute the shortest path between
     * two nodes in a specified network.
     *
     * @param networkName Network to query
     * @param startNode Starting node ID
     * @param endNode Ending node ID
     */
    void getShortestPath(const QString &networkName,
                         const QString &startNode,
                         const QString &endNode);

    /**
     * @brief Retrieves the state of a specific ship
     *
     * Returns the current state of a ship within a network.
     *
     * @param networkName Network containing the ship
     * @param shipId Unique identifier of the ship
     * @return Pointer to ShipState or nullptr if not found
     */
    const ShipState *
    getShipState(const QString &networkName,
                 const QString &shipId) const;

    /**
     * @brief Retrieves states of all ships in a network
     *
     * Returns a list of states for all ships in a specified
     * network.
     *
     * @param networkName Network to query
     * @return List of ShipState pointers, empty if none
     * found
     */
    QList<const ShipState *> getAllNetworkShipsStates(
        const QString &networkName) const;

    /**
     * @brief Retrieves states of all ships across all
     * networks
     *
     * Returns a map of network names to lists of ship
     * states.
     *
     * @return Map of network names to ShipState pointer
     * lists
     */
    QMap<QString, QList<const ShipState *>>
    getAllShipsStates() const;

protected:
    /**
     * @brief Processes messages from the server
     *
     * Handles incoming server messages, dispatching them to
     * appropriate event handlers.
     *
     * @param message JSON object containing the server
     * message
     */
    void
    processMessage(const QJsonObject &message) override;

private:
    /**
     * @brief Internal method to unload containers
     *
     * Executes the unloading process without waiting for a
     * response.
     *
     * @param networkName Network name
     * @param shipId Ship ID
     * @param terminalNames Terminal names for unloading
     * @return True if the command is sent successfully
     */
    bool unloadContainersFromShipAtTerminalsPrivate(
        const QString &networkName, const QString &shipId,
        const QStringList &terminalNames);

    /**
     * @brief Handles simulation network loaded event
     *
     * Processes the event when a simulation network is
     * loaded.
     *
     * @param message Event data in JSON format
     */
    void
    onSimulationNetworkLoaded(const QJsonObject &message);

    /**
     * @brief Handles simulation created event
     *
     * Processes the event when a simulation is created.
     *
     * @param message Event data in JSON format
     */
    void onSimulationCreated(const QJsonObject &message);

    /**
     * @brief Handles simulation paused event
     *
     * Processes the event when a simulation is paused.
     *
     * @param message Event data in JSON format
     */
    void onSimulationPaused(const QJsonObject &message);

    /**
     * @brief Handles simulation resumed event
     *
     * Processes the event when a simulation is resumed.
     *
     * @param message Event data in JSON format
     */
    void onSimulationResumed(const QJsonObject &message);

    /**
     * @brief Handles simulation restarted event
     *
     * Processes the event when a simulation is restarted.
     *
     * @param message Event data in JSON format
     */
    void onSimulationRestarted(const QJsonObject &message);

    /**
     * @brief Handles simulation ended event
     *
     * Processes the event when a simulation ends.
     *
     * @param message Event data in JSON format
     */
    void onSimulationEnded(const QJsonObject &message);

    /**
     * @brief Handles simulation advanced event
     *
     * Processes the event when a simulation advances in
     * time.
     *
     * @param message Event data in JSON format
     */
    void onSimulationAdvanced(const QJsonObject &message);

    /**
     * @brief Handles simulation progress update event
     *
     * Processes the event when simulation progress updates.
     *
     * @param message Event data in JSON format
     */
    void
    onSimulationProgressUpdate(const QJsonObject &message);

    /**
     * @brief Handles ship added to simulator event
     *
     * Processes the event when a ship is added to the
     * simulator.
     *
     * @param message Event data in JSON format
     */
    void onShipAddedToSimulator(const QJsonObject &message);

    /**
     * @brief Handles all ships reached destination event
     *
     * Processes the event when all ships reach their
     * destinations.
     *
     * @param message Event data in JSON format
     */
    void onAllShipsReachedDestination(
        const QJsonObject &message);

    /**
     * @brief Handles ship reached destination event
     *
     * Processes the event when a ship reaches its
     * destination.
     *
     * @param message Event data in JSON format
     */
    void
    onShipReachedDestination(const QJsonObject &message);

    /**
     * @brief Handles ship reached seaport event
     *
     * Processes the event when a ship reaches a seaport.
     *
     * @param message Event data in JSON format
     */
    void onShipReachedSeaport(const QJsonObject &message);

    /**
     * @brief Handles containers unloaded event
     *
     * Processes the event when containers are unloaded from
     * a ship.
     *
     * @param message Event data in JSON format
     */
    void onContainersUnloaded(const QJsonObject &message);

    /**
     * @brief Handles simulation results available event
     *
     * Processes the event when simulation results are
     * available.
     *
     * @param message Event data in JSON format
     */
    void onSimulationResultsAvailable(
        const QJsonObject &message);

    /**
     * @brief Handles ship state available event
     *
     * Processes the event when a ship's state is available.
     *
     * @param message Event data in JSON format
     */
    void onShipStateAvailable(const QJsonObject &message);

    /**
     * @brief Handles simulator state available event
     *
     * Processes the event when the simulator state is
     * available.
     *
     * @param message Event data in JSON format
     */
    void
    onSimulatorStateAvailable(const QJsonObject &message);

    /**
     * @brief Handles error occurred event
     *
     * Processes the event when an error occurs on the
     * server.
     *
     * @param message Event data in JSON format
     */
    void onErrorOccurred(const QJsonObject &message);

    /**
     * @brief Handles server reset event
     *
     * Processes the event when the server is reset.
     */
    void onServerReset();

    /**
     * @brief Handles containers added event
     *
     * Processes the event when containers are added to a
     * ship.
     *
     * @param message Event data in JSON format
     */
    void onContainersAdded(const QJsonObject &message);

    /**
     * @var m_dataAccessMutex
     * @brief Mutex for thread-safe data access
     *
     * Ensures synchronized access to internal data
     * structures.
     */
    mutable QMutex m_dataAccessMutex;

    /**
     * @var m_networkData
     * @brief Stores simulation results by network
     *
     * Maps network names to lists of SimulationResults
     * pointers.
     */
    QMap<QString, QList<SimulationResults *>> m_networkData;

    /**
     * @var m_shipState
     * @brief Stores ship states by network
     *
     * Maps network names to lists of ShipState pointers.
     */
    QMap<QString, QList<ShipState *>> m_shipState;

    /**
     * @var m_loadedShips
     * @brief Stores loaded ship objects
     *
     * Maps ship IDs to Ship pointers for the simulation.
     */
    QMap<QString, Backend::Ship *> m_loadedShips;

    /**
     * @var m_shipsDestinationTerminals
     * @brief Maps ship IDs to destination terminals
     *
     * Associates each ship with its target terminal IDs.
     */
    QMap<QString, QStringList> m_shipsDestinationTerminals;
};

} // namespace ShipClient
} // namespace Backend
} // namespace CargoNetSim

Q_DECLARE_METATYPE(
    CargoNetSim::Backend::ShipClient::ShipSimulationClient)
Q_DECLARE_METATYPE(
    CargoNetSim::Backend::ShipClient::ShipSimulationClient
        *)
