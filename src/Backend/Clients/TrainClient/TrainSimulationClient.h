#pragma once

#include <QObject>
#include <QMutex>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QList>
#include <QString>
#include "Backend/Models/TrainSystem.h"
#include "SimulationResults.h"
#include "TrainState.h"
#include "Backend/Clients/BaseClient/SimulationClientBase.h"
#include "Backend/Commons/ClientType.h"

/**
 * @file TrainSimulatorClient.h
 * @brief Header for TrainSimulatorClient class
 * @author Ahmed Aredah
 * @date March 20, 2025
 *
 * Declares TrainSimulatorClient, managing train simulation
 * interactions in the CargoNetSim backend.
 */

// Forward declarations
namespace CargoNetSim {
namespace Backend {
namespace TrainClient {

class TerminalGraphServer;
class SimulatorTimeServer;
class ProgressBarManager;
class ApplicationLogger;
}}}

namespace CargoNetSim {
namespace Backend {
namespace TrainClient {

/**
 * @class TrainSimulatorClient
 * @brief Client for interacting with the train simulator
 *
 * Provides functionality for train simulation management,
 * including defining simulations, handling trains and containers,
 * and processing server events. Inherits from SimulationClientBase.
 */
class TrainSimulationClient : public SimulationClientBase {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject, defaults to nullptr
     * @param host RabbitMQ host, defaults to "localhost"
     * @param port RabbitMQ port, defaults to 5672
     *
     * Initializes the train simulator client with connection details.
     */
    explicit TrainSimulationClient(
        QObject* parent = nullptr,
        const QString& host = "localhost",
        int port = 5672);

    /**
     * @brief Destructor
     *
     * Cleans up allocated resources like trains and states.
     */
    ~TrainSimulationClient() override;

    /**
     * @brief Reset the simulator server
     * @return True if reset succeeds
     *
     * Resets the server to its initial state.
     */
    bool resetServer();

    /**
     * @brief Initialize the client in its thread
     *
     * Sets up thread-specific resources like RabbitMQ heartbeat.
     */
    void initializeClient() override;

    /**
     * @brief Define simulator by network name
     * @param networkName Name of the network
     * @param timeStep Simulation time step, defaults to 1.0
     * @param trains List of train definitions, defaults to empty
     * @return True if successful
     *
     * Defines a simulator using a predefined network name.
     */
    bool defineSimulatorByNetworkName(
        const QString& networkName,
        double timeStep = 1.0,
        const QList<QJsonObject>& trains = {});

    /**
     * @brief Define a new simulator
     * @param nodesJson JSON array of network nodes
     * @param linksJson JSON array of network links
     * @param networkName Name of the network
     * @param timeStep Simulation time step, defaults to 1.0
     * @param trains List of train definitions, defaults to empty
     * @return True if successful
     *
     * Defines a simulator with custom network topology.
     */
    bool defineSimulator(
        const QJsonArray& nodesJson,
        const QJsonArray& linksJson,
        const QString& networkName,
        double timeStep = 1.0,
        const QList<QJsonObject>& trains = {});

    /**
     * @brief Run the simulator
     * @param networkNames Networks to run, "*" for all
     * @param byTimeSteps Steps to run, -1 for unlimited
     * @return True if successful
     *
     * Starts simulation for specified networks.
     */
    bool runSimulator(
        const QStringList& networkNames,
        double byTimeSteps = -1.0);

    /**
     * @brief End the simulator
     * @param networkNames Networks to end, "*" for all
     * @return True if successful
     *
     * Stops simulation for specified networks.
     */
    bool endSimulator(const QStringList& networkNames);

    /**
     * @brief Add trains to simulator
     * @param networkName Target network name
     * @param trains List of train definitions
     * @return True if successful
     *
     * Adds trains to an existing simulator network.
     */
    bool addTrainsToSimulator(
        const QString& networkName,
        const QList<QJsonObject>& trains);

    /**
     * @brief Add containers to a train
     * @param networkName Network name
     * @param trainId Train ID
     * @param containers List of container JSON strings
     * @return True if successful
     *
     * Adds containers to a specified train.
     */
    bool addContainersToTrain(
        const QString& networkName,
        const QString& trainId,
        const QStringList& containers);

    /**
     * @brief Unload containers from a train
     * @param networkName Network name
     * @param trainId Train ID
     * @param containersDestinationNames Destination terminals
     * @return True if successful
     *
     * Unloads containers at specified terminals.
     */
    bool unloadTrain(
        const QString& networkName,
        const QString& trainId,
        const QStringList& containersDestinationNames);

    /**
     * @brief Get state of a specific train
     * @param networkName Network name
     * @param trainId Train ID
     * @return Train state as JSON, empty if not found
     *
     * Retrieves the state of a specific train.
     */
    QJsonObject getTrainState(
        const QString& networkName,
        const QString& trainId) const;

    /**
     * @brief Get states of all trains in a network
     * @param networkName Network name
     * @return List of train states as JSON
     *
     * Retrieves states of all trains in a network.
     */
    QJsonArray getAllNetworkTrainStates(
        const QString& networkName) const;

    /**
     * @brief Get states of all trains across networks
     * @return Map of network names to train states
     *
     * Retrieves states of all trains in all networks.
     */
    QJsonObject getAllTrainsStates() const;

protected:
    /**
     * @brief Process server messages
     * @param message JSON message from server
     *
     * Handles train-specific events from the server.
     */
    void processMessage(const QJsonObject& message) override;

private:
    /**
     * @brief Internal unload train method
     * @param networkName Network name
     * @param trainId Train ID
     * @param containersDestinationNames Destination terminals
     * @return True if command sent successfully
     *
     * Private implementation for unloading containers.
     */
    bool unloadTrainPrivate(
        const QString& networkName,
        const QString& trainId,
        const QStringList& containersDestinationNames);

    /// @brief Handle simulation created event
    void onSimulationCreated(const QJsonObject& message);

    /// @brief Handle simulation ended event
    void onSimulationEnded(const QJsonObject& message);

    /// @brief Handle train reached destination event
    void onTrainReachedDestination(const QJsonObject& message);

    /// @brief Handle all trains reached destination event
    void onAllTrainsReachedDestination(const QJsonObject& message);

    /// @brief Handle simulation results available event
    void onSimulationResultsAvailable(const QJsonObject& message);

    /// @brief Handle trains added to simulator event
    void onTrainsAddedToSimulator(const QJsonObject& message);

    /// @brief Handle error occurred event
    void onErrorOccurred(const QJsonObject& message);

    /// @brief Handle server reset event
    void onServerReset();

    /// @brief Handle simulation advanced event
    void onSimulationAdvanced(const QJsonObject& message);

    /// @brief Handle containers added event
    void onContainersAdded(const QJsonObject& message);

    /// @brief Handle simulation progress update event
    void onSimulationProgressUpdate(const QJsonObject& message);

    /// @brief Handle simulation paused event
    void onSimulationPaused(const QJsonObject& message);

    /// @brief Handle simulation resumed event
    void onSimulationResumed(const QJsonObject& message);

    /// @brief Handle train reached terminal event
    void onTrainReachedTerminal(const QJsonObject& message);

    /// @brief Handle containers unloaded event
    void onContainersUnloaded(const QJsonObject& message);

    /// @brief Mutex for thread-safe data access
    mutable QMutex m_dataAccessMutex;

    /// @brief Map of network names to simulation results
    QMap<QString, SimulationResults*> m_networkData;

    /// @brief Map of network names to train states
    QMap<QString, QList<TrainState*>> m_trainState;

    /// @brief Map of train IDs to Train objects
    QMap<QString, Backend::Train*> m_loadedTrains;
};

} // namespace TrainClient
} // namespace Backend
} // namespace CargoNetSim

Q_DECLARE_METATYPE(CargoNetSim::Backend::TrainClient::TrainSimulationClient)
Q_DECLARE_METATYPE(CargoNetSim::Backend::TrainClient::TrainSimulationClient*)
