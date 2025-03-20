#pragma once

#include <QObject>
#include <QMutex>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <QMap>
#include <QList>
#include <QString>
#include <QThread>
#include <QDateTime>
#include <QWaitCondition>

#include "Backend/Clients/ShipClient/ShipState.h"
#include "Backend/Commons/ClientType.h"
#include "Backend/Clients/ShipClient/SimulationResults.h"
#include "Backend/Clients/ShipClient/ShipState.h"
#include "Backend/Clients/BaseClient/SimulationClientBase.h"

// Forward declarations
namespace CargoNetSim {
namespace Backend {
class Ship;
class TerminalGraphServer;
class SimulatorTimeServer;
class ProgressBarManager;
class ApplicationLogger;
}}

namespace CargoNetSim {
namespace Backend {

/**
 * @brief Client for interacting with the ship simulator
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
     */
    void resetServer();
    
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
     * @brief Wait for specified events
     * @param expectedEvents List of events to wait for
     * @param timeout Timeout in ms (-1 for indefinite)
     * @return True if any expected event was received
     */
    bool waitForEvent(
        const QStringList& expectedEvents,
        int timeout = -1);
    
    /**
     * @brief Handle message from RabbitMQ
     * @param message Message content
     */
    void handleMessage(const QJsonObject& message) override;
    
    // Event handling methods
    void onSimulationNetworkLoaded(const QJsonObject& message);
    void onSimulationCreated(const QJsonObject& message);
    void onSimulationPaused(const QJsonObject& message);
    void onSimulationResumed(const QJsonObject& message);
    void onSimulationRestarted(const QJsonObject& message);
    void onSimulationEnded(const QJsonObject& message) override;
    void onSimulationAdvanced(
        const QJsonObject& message) override;
    void onSimulationProgressUpdate(
        const QJsonObject& message) override;
    void onShipAddedToSimulator(const QJsonObject& message);
    void onAllShipsReachedDestination(const QJsonObject& message);
    void onShipReachedDestination(const QJsonObject& message);
    void onShipReachedSeaport(const QJsonObject& message);
    void onContainersUnloaded(const QJsonObject& message);
    void onSimulationResultsAvailable(const QJsonObject& message);
    void onShipStateAvailable(const QJsonObject& message);
    void onSimulatorStateAvailable(const QJsonObject& message);
    void onErrorOccurred(const QJsonObject& message) override;
    void onServerReset() override;
    void onContainersAdded(const QJsonObject& message);

private:
    /**
     * @brief Unload containers (internal implementation)
     * @param networkName Network name
     * @param shipId Ship ID
     * @param terminalNames Terminal names
     */
    void unloadContainersFromShipAtTerminalsPrivate(
        const QString& networkName,
        const QString& shipId,
        const QStringList& terminalNames);
    
    // Queue for synchronizing events
    QMutex m_eventQueueMutex;
    QWaitCondition m_eventWaitCondition;
    QStringList m_eventQueue;
    
    // Data storage
    QMap<QString, QList<SimulationResults>> m_networkData;
    QMap<QString, QList<ShipState>> m_shipState;
    QMap<QString, Ship*> m_loadedShips;
    QMap<QString, QStringList> m_shipsDestinationTerminals;
};

} // namespace Backend
} // namespace CargoNetSim


Q_DECLARE_METATYPE(CargoNetSim::Backend::ShipSimulationClient)
Q_DECLARE_METATYPE(CargoNetSim::Backend::ShipSimulationClient*)
