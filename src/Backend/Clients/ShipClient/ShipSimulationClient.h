#pragma once

#include <QObject>
#include <QMutex>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <QMap>
#include <QList>
#include <QString>
#include <QDateTime>

#include "Backend/Clients/ShipClient/ShipState.h"
#include "Backend/Commons/ClientType.h"
#include "Backend/Clients/ShipClient/SimulationResults.h"
#include "Backend/Clients/ShipClient/ShipState.h"
#include "Backend/Clients/BaseClient/SimulationClientBase.h"
#include "Backend/Models/ShipSystem.h"

// Forward declarations
namespace CargoNetSim {
namespace Backend {
namespace ShipClient {
class TerminalGraphServer;
class SimulatorTimeServer;
class ProgressBarManager;
class ApplicationLogger;
}}}

namespace CargoNetSim {
namespace Backend {
namespace ShipClient {

/**
 * @brief Client for interacting with the ship simulator
 *
 * This class provides specialized functionality for working with the
 * ship simulation module, including ship creation, container
 * management, and simulation control.
 */
class ShipSimulationClient : public SimulationClientBase {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject
     * @param host RabbitMQ host
     * @param port RabbitMQ port
     */
    explicit ShipSimulationClient(
        QObject* parent = nullptr,
        const QString& host = "localhost",
        int port = 5672);

    /**
     * @brief Destructor
     */
    ~ShipSimulationClient() override;

    /**
     * @brief Reset the simulator server
     * @return True if successful
     */
    bool resetServer();

    /**
     * @brief Initializes the ship simulation client in its thread
     *
     * Overrides `SimulationClientBase::initializeClient` to set up
     * thread-specific resources for the ship simulation client. It
     * runs after the object is moved to its thread and the thread
     * starts, ensuring proper thread context for initialization.
     *
     * Tasks performed:
     * - Calls base class init to set up shared client resources.
     * - Sets up RabbitMQ handler’s heartbeat for connection health.
     * - Logs init details for debugging and verification.
     *
     * @note Intended for auto-call via `QThread::started` signal.
     *       Avoid manual calls unless explicitly needed and synced.
     *
     * @warning Call only once, after thread start. Multiple calls
     *          or pre-thread calls may cause conflicts or errors.
     *
     * @throws std::runtime_error If base init fails or RabbitMQ
     *                            handler config fails due to state.
     *
     * @see SimulationClientBase::initializeClient
     * @see QThread::started
     * @see RabbitMQHandler::setupHeartbeat
     */
    void initializeClient() override;

    /**
     * @brief Define a new ship simulator
     * @param networkName Name for the network
     * @param timeStep Simulation time step
     * @param ships List of ship definitions
     * @param destinationTerminalIds Map of ship IDs to terminal IDs
     * @param networkPath Path to network definition file
     * @return True if successful
     */
    bool defineSimulator(
        const QString& networkName,
        double timeStep,
        const QList<QJsonObject>& ships,
        const QMap<QString, QStringList>& destinationTerminalIds,
        const QString& networkPath = "Default");

    /**
     * @brief Run the simulator
     * @param networkNames List of networks to run, or "*" for all
     * @param byTimeSteps Time steps to run for (-1 for unlimited)
     * @return True if successful
     */
    bool runSimulator(
        const QStringList& networkNames,
        double byTimeSteps = -1.0);

    /**
     * @brief End the simulator
     * @param networkNames Networks to end, or "*" for all
     * @return True if successful
     */
    bool endSimulator(const QStringList& networkNames);

    /**
     * @brief Add ships to an existing simulator
     * @param networkName Network to add ships to
     * @param ships List of ship definitions
     * @param destinationTerminalIds Map of ship IDs to terminal IDs
     * @return True if successful
     */
    bool addShipsToSimulator(
        const QString& networkName,
        const QList<QJsonObject>& ships,
        const QMap<QString, QStringList>& destinationTerminalIds);

    /**
     * @brief Add containers to a ship
     * @param networkName Network name
     * @param shipId Ship ID
     * @param containers List of container JSON strings
     * @return True if successful
     */
    bool addContainersToShip(
        const QString& networkName,
        const QString& shipId,
        const QStringList& containers);

    /**
     * @brief Unload containers from ship at terminals
     * @param networkName Network name
     * @param shipId Ship ID
     * @param terminalNames Terminal names
     * @return True if successful
     */
    bool unloadContainersFromShipAtTerminals(
        const QString& networkName,
        const QString& shipId,
        const QStringList& terminalNames);

    /**
     * @brief Get network terminal nodes
     * @param networkName Network name
     */
    void getNetworkTerminalNodes(const QString& networkName);

    /**
     * @brief Get shortest path between nodes
     * @param networkName Network name
     * @param startNode Start node ID
     * @param endNode End node ID
     */
    void getShortestPath(
        const QString& networkName,
        const QString& startNode,
        const QString& endNode);

    /**
     * @brief Get state of a specific ship
     * @param networkName Network name
     * @param shipId Ship ID
     * @return Ship state as JSON or empty object if not found
     */
    QJsonObject getShipState(
        const QString& networkName,
        const QString& shipId) const;

    /**
     * @brief Get states of all ships in a network
     * @param networkName Network name
     * @return List of ship states as JSON
     */
    QJsonArray getAllNetworkShipsStates(
        const QString& networkName) const;

    /**
     * @brief Get states of all ships in all networks
     * @return Map of network names to ship state lists
     */
    QJsonObject getAllShipsStates() const;

protected:
    /**
     * @brief Process message from server
     *
     * Override from SimulationClientBase to handle ship-specific events
     *
     * @param message JSON message object
     */
    void processMessage(const QJsonObject& message) override;

private:
    /**
     * @brief Unload containers (internal implementation)
     * @param networkName Network name
     * @param shipId Ship ID
     * @param terminalNames Terminal names
     * @return True if the command was sent successfully
     */
    bool unloadContainersFromShipAtTerminalsPrivate(
        const QString& networkName,
        const QString& shipId,
        const QStringList& terminalNames);

    /**
     * @brief Process a simulation network loaded event
     * @param message Event data
     */
    void onSimulationNetworkLoaded(const QJsonObject& message);

    /**
     * @brief Process a simulation created event
     * @param message Event data
     */
    void onSimulationCreated(const QJsonObject& message);

    /**
     * @brief Process a simulation paused event
     * @param message Event data
     */
    void onSimulationPaused(const QJsonObject& message);

    /**
     * @brief Process a simulation resumed event
     * @param message Event data
     */
    void onSimulationResumed(const QJsonObject& message);

    /**
     * @brief Process a simulation restarted event
     * @param message Event data
     */
    void onSimulationRestarted(const QJsonObject& message);

    /**
     * @brief Process a simulation ended event
     * @param message Event data
     */
    void onSimulationEnded(const QJsonObject& message);

    /**
     * @brief Process a simulation advanced event
     * @param message Event data
     */
    void onSimulationAdvanced(const QJsonObject& message);

    /**
     * @brief Process a simulation progress update event
     * @param message Event data
     */
    void onSimulationProgressUpdate(const QJsonObject& message);

    /**
     * @brief Process a ship added to simulator event
     * @param message Event data
     */
    void onShipAddedToSimulator(const QJsonObject& message);

    /**
     * @brief Process an all ships reached destination event
     * @param message Event data
     */
    void onAllShipsReachedDestination(const QJsonObject& message);

    /**
     * @brief Process a ship reached destination event
     * @param message Event data
     */
    void onShipReachedDestination(const QJsonObject& message);

    /**
     * @brief Process a ship reached seaport event
     * @param message Event data
     */
    void onShipReachedSeaport(const QJsonObject& message);

    /**
     * @brief Process a containers unloaded event
     * @param message Event data
     */
    void onContainersUnloaded(const QJsonObject& message);

    /**
     * @brief Process a simulation results available event
     * @param message Event data
     */
    void onSimulationResultsAvailable(const QJsonObject& message);

    /**
     * @brief Process a ship state available event
     * @param message Event data
     */
    void onShipStateAvailable(const QJsonObject& message);

    /**
     * @brief Process a simulator state available event
     * @param message Event data
     */
    void onSimulatorStateAvailable(const QJsonObject& message);

    /**
     * @brief Process an error occurred event
     * @param message Event data
     */
    void onErrorOccurred(const QJsonObject& message);

    /**
     * @brief Process a server reset event
     */
    void onServerReset();

    /**
     * @brief Process a containers added event
     * @param message Event data
     */
    void onContainersAdded(const QJsonObject& message);

    // Data storage - protected by mutex for thread safety
    mutable QMutex m_dataAccessMutex;
    QMap<QString, QList<SimulationResults*>> m_networkData;
    QMap<QString, QList<ShipState*>> m_shipState;
    QMap<QString, Backend::Ship*> m_loadedShips;
    QMap<QString, QStringList> m_shipsDestinationTerminals;
};

} // namespace ShipClient
} // namespace Backend
} // namespace CargoNetSim

Q_DECLARE_METATYPE(CargoNetSim::Backend::ShipClient::ShipSimulationClient)
Q_DECLARE_METATYPE(CargoNetSim::Backend::ShipClient::ShipSimulationClient*)
