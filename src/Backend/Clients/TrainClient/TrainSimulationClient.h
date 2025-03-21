/**
 * @file TrainSimulationClient.h
 * @brief Defines TrainSimulationClient for train simulation
 *
 * This header declares the TrainSimulationClient class, which
 * manages train simulation interactions within the CargoNetSim
 * framework, including simulation setup, train management, and
 * state retrieval.
 *
 * @author Ahmed Aredah
 * @date March 20, 2025
 */

#pragma once

#include <QObject>
#include <QMutex>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QList>
#include <QString>
#include <containerLib/container.h>
#include "Backend/Models/TrainSystem.h"
#include "SimulationResults.h"
#include "TrainState.h"
#include "Backend/Clients/BaseClient/SimulationClientBase.h"
#include "Backend/Commons/ClientType.h"

// Forward declarations for dependencies
namespace CargoNetSim {
namespace Backend {
namespace TrainClient {
class TerminalGraphServer;
class SimulatorTimeServer;
class ProgressBarManager;
class ApplicationLogger;
}}}

/**
 * @namespace CargoNetSim::Backend::TrainClient
 * @brief Namespace for train simulation client components
 *
 * Encapsulates classes and utilities related to train simulation
 * within the CargoNetSim backend infrastructure.
 */
namespace CargoNetSim {
namespace Backend {
namespace TrainClient {

/**
 * @class TrainSimulationClient
 * @brief Manages train simulation server interactions
 *
 * Extends SimulationClientBase to provide train-specific
 * functionality, such as defining simulators, managing trains and
 * containers, and handling server events. Ensures thread safety
 * using mutexes.
 *
 * @ingroup TrainSimulation
 */
class TrainSimulationClient : public SimulationClientBase {
    Q_OBJECT

public:
    /**
     * @brief Constructs a TrainSimulationClient instance
     *
     * Initializes the client with RabbitMQ connection parameters
     * and optional parent object.
     *
     * @param parent Parent QObject, defaults to nullptr
     * @param host RabbitMQ hostname, defaults to "localhost"
     * @param port RabbitMQ port number, defaults to 5672
     */
    explicit TrainSimulationClient(
        QObject* parent = nullptr,
        const QString& host = "localhost",
        int port = 5672);

    /**
     * @brief Destroys the TrainSimulationClient instance
     *
     * Frees dynamically allocated resources, ensuring proper
     * cleanup of trains, states, and simulation data.
     */
    ~TrainSimulationClient() override;

    /**
     * @brief Resets the train simulation server
     *
     * Sends a reset command to clear all simulation data and state
     * on the server.
     *
     * @return True if reset is successful, false otherwise
     */
    bool resetServer();

    /**
     * @brief Initializes the client in its thread
     *
     * Configures thread-specific resources, such as RabbitMQ
     * heartbeat, after moving to its thread. Called automatically
     * via QThread::started signal.
     *
     * @param logger Optional logger for initialization logging
     * @throws std::runtime_error If RabbitMQ handler setup fails
     * @note Avoid manual invocation unless synchronized
     * @warning Call only once after thread start
     */
    void initializeClient(LoggerInterface* logger) override;

    /**
     * @brief Defines a simulator using a network name
     *
     * Sets up a train simulation using a predefined network name
     * and optional train list.
     *
     * @param networkName Unique identifier for the network
     * @param timeStep Simulation time increment, defaults to 1.0
     * @param trains List of Train pointers, defaults to empty
     * @return True if simulator definition succeeds
     */
    bool defineSimulatorByNetworkName(
        const QString& networkName,
        const double timeStep = 1.0,
        const QList<Train*>& trains = {});

    /**
     * @brief Defines a new simulator with custom topology
     *
     * Configures a train simulation with specified nodes, links,
     * and trains.
     *
     * @param nodesJson JSON array of network nodes
     * @param linksJson JSON array of network links
     * @param networkName Unique identifier for the network
     * @param timeStep Simulation time increment, defaults to 1.0
     * @param trains List of Train pointers, defaults to empty
     * @return True if simulator definition succeeds
     */
    bool defineSimulator(
        const QJsonArray& nodesJson,
        const QJsonArray& linksJson,
        const QString& networkName,
        const double timeStep = 1.0,
        const QList<Train*>& trains = {});

    /**
     * @brief Runs the simulator for specified networks
     *
     * Initiates simulation execution for given networks or all if
     * "*" is specified.
     *
     * @param networkNames List of network names or "*" for all
     * @param byTimeSteps Steps to run, -1 for unlimited, defaults to -1
     * @return True if simulation starts successfully
     */
    bool runSimulator(
        const QStringList& networkNames,
        double byTimeSteps = -1.0);

    /**
     * @brief Terminates the simulator for specified networks
     *
     * Stops simulation execution for given networks or all if "*"
     * is specified.
     *
     * @param networkNames List of network names or "*" for all
     * @return True if simulation ends successfully
     */
    bool endSimulator(const QStringList& networkNames);

    /**
     * @brief Adds trains to an existing simulator
     *
     * Incorporates additional trains into a specified simulation
     * network.
     *
     * @param networkName Target network identifier
     * @param trains List of Train pointers to add
     * @return True if trains are added successfully
     */
    bool addTrainsToSimulator(
        const QString& networkName,
        const QList<Train*>& trains);

    /**
     * @brief Adds containers to a specified train
     *
     * Assigns containers to a train within a given network.
     *
     * @param networkName Network containing the train
     * @param trainId Unique identifier of the train
     * @param containers List of Container pointers to add
     * @return True if containers are added successfully
     */
    bool addContainersToTrain(
        const QString& networkName,
        const QString& trainId,
        const QList<ContainerCore::Container*>& containers);

    /**
     * @brief Unloads containers from a train
     *
     * Transfers containers from a train to specified destination
     * terminals.
     *
     * @param networkName Network containing the train
     * @param trainId Unique identifier of the train
     * @param containersDestinationNames List of destination terminals
     * @return True if unloading succeeds
     */
    bool unloadTrain(
        const QString& networkName,
        const QString& trainId,
        const QStringList& containersDestinationNames);

    /**
     * @brief Retrieves the state of a specific train
     *
     * Fetches the current state of a train within a network.
     *
     * @param networkName Network containing the train
     * @param trainId Unique identifier of the train
     * @return Pointer to TrainState or nullptr if not found
     */
    const TrainState* getTrainState(
        const QString& networkName,
        const QString& trainId) const;

    /**
     * @brief Retrieves states of all trains in a network
     *
     * Returns a list of states for all trains in a network.
     *
     * @param networkName Network to query
     * @return List of TrainState pointers, empty if none found
     */
    QList<const TrainState*> getAllNetworkTrainStates(
        const QString& networkName) const;

    /**
     * @brief Retrieves states of all trains across networks
     *
     * Returns a map of network names to lists of train states.
     *
     * @return Map of network names to TrainState pointer lists
     */
    QMap<QString, QList<const TrainState*>> getAllTrainsStates() const;

protected:
    /**
     * @brief Processes messages from the server
     *
     * Handles incoming server messages by dispatching them to
     * appropriate event handlers.
     *
     * @param message JSON object containing the server message
     */
    void processMessage(const QJsonObject& message) override;

private:
    /**
     * @brief Internal method to unload containers from a train
     *
     * Sends an unload command without waiting for a response.
     *
     * @param networkName Network containing the train
     * @param trainId Unique identifier of the train
     * @param containersDestinationNames List of destination terminals
     * @return True if command is sent successfully
     */
    bool unloadTrainPrivate(
        const QString& networkName,
        const QString& trainId,
        const QStringList& containersDestinationNames);

    /**
     * @brief Handles simulation created event
     *
     * Processes the event when a simulation is created.
     *
     * @param message Event data in JSON format
     */
    void onSimulationCreated(const QJsonObject& message);

    /**
     * @brief Handles simulation ended event
     *
     * Processes the event when a simulation ends.
     *
     * @param message Event data in JSON format
     */
    void onSimulationEnded(const QJsonObject& message);

    /**
     * @brief Handles train reached destination event
     *
     * Processes the event when a train reaches its destination.
     *
     * @param message Event data in JSON format
     */
    void onTrainReachedDestination(const QJsonObject& message);

    /**
     * @brief Handles all trains reached destination event
     *
     * Processes the event when all trains reach their destinations.
     *
     * @param message Event data in JSON format
     */
    void onAllTrainsReachedDestination(const QJsonObject& message);

    /**
     * @brief Handles simulation results available event
     *
     * Processes the event when simulation results are available.
     *
     * @param message Event data in JSON format
     */
    void onSimulationResultsAvailable(const QJsonObject& message);

    /**
     * @brief Handles trains added to simulator event
     *
     * Processes the event when trains are added to the simulator.
     *
     * @param message Event data in JSON format
     */
    void onTrainsAddedToSimulator(const QJsonObject& message);

    /**
     * @brief Handles error occurred event
     *
     * Processes the event when an error occurs on the server.
     *
     * @param message Event data in JSON format
     */
    void onErrorOccurred(const QJsonObject& message);

    /**
     * @brief Handles server reset event
     *
     * Processes the event when the server is reset.
     */
    void onServerReset();

    /**
     * @brief Handles simulation advanced event
     *
     * Processes the event when a simulation advances in time.
     *
     * @param message Event data in JSON format
     */
    void onSimulationAdvanced(const QJsonObject& message);

    /**
     * @brief Handles containers added event
     *
     * Processes the event when containers are added to a train.
     *
     * @param message Event data in JSON format
     */
    void onContainersAdded(const QJsonObject& message);

    /**
     * @brief Handles simulation progress update event
     *
     * Processes the event when simulation progress updates.
     *
     * @param message Event data in JSON format
     */
    void onSimulationProgressUpdate(const QJsonObject& message);

    /**
     * @brief Handles simulation paused event
     *
     * Processes the event when a simulation is paused.
     *
     * @param message Event data in JSON format
     */
    void onSimulationPaused(const QJsonObject& message);

    /**
     * @brief Handles simulation resumed event
     *
     * Processes the event when a simulation is resumed.
     *
     * @param message Event data in JSON format
     */
    void onSimulationResumed(const QJsonObject& message);

    /**
     * @brief Handles train reached terminal event
     *
     * Processes the event when a train reaches a terminal.
     *
     * @param message Event data in JSON format
     */
    void onTrainReachedTerminal(const QJsonObject& message);

    /**
     * @brief Handles containers unloaded event
     *
     * Processes the event when containers are unloaded from a train.
     *
     * @param message Event data in JSON format
     */
    void onContainersUnloaded(const QJsonObject& message);

    /**
     * @var m_dataAccessMutex
     * @brief Mutex for thread-safe data access
     *
     * Protects internal data structures from concurrent access.
     */
    mutable QMutex m_dataAccessMutex;

    /**
     * @var m_networkData
     * @brief Stores simulation results by network
     *
     * Maps network names to SimulationResults pointers.
     */
    QMap<QString, SimulationResults*> m_networkData;

    /**
     * @var m_trainState
     * @brief Stores train states by network
     *
     * Maps network names to lists of TrainState pointers.
     */
    QMap<QString, QList<TrainState*>> m_trainState;

    /**
     * @var m_loadedTrains
     * @brief Stores loaded train objects
     *
     * Maps train IDs to Train pointers for the simulation.
     */
    QMap<QString, Backend::Train*> m_loadedTrains;
};

} // namespace TrainClient
} // namespace Backend
} // namespace CargoNetSim

Q_DECLARE_METATYPE(CargoNetSim::Backend::TrainClient::TrainSimulationClient)
Q_DECLARE_METATYPE(CargoNetSim::Backend::TrainClient::TrainSimulationClient*)
