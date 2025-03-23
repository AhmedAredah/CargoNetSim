#include "TrainSimulationClient.h"
#include "Backend/Models/TrainSystem.h"
#include <QDebug>
#include <QJsonDocument>
// Placeholder includes (uncomment as needed)
// #include "TerminalGraphServer.h"
// #include "SimulatorTimeServer.h"
// #include "ProgressBarManager.h"

/**
 * @file TrainSimulationClient.cpp
 * @brief Implementation of TrainSimulationClient class
 * @author Ahmed Aredah
 * @date March 20, 2025
 *
 * Implements TrainSimulationClient with detailed comments
 * for audit and review, managing train simulation.
 */

namespace CargoNetSim
{
namespace Backend
{
namespace TrainClient
{

TrainSimulationClient::TrainSimulationClient(
    QObject *parent, const QString &host, int port)
    : SimulationClientBase(
          parent, host, port, "CargoNetSim.Exchange",
          "CargoNetSim.CommandQueue.NeTrainSim",
          "CargoNetSim.ResponseQueue.NeTrainSim",
          "CargoNetSim.Command.NeTrainSim",
          QStringList{"CargoNetSim.Response.NeTrainSim"},
          ClientType::TrainClient)
{
    // Initialize progress bar (placeholder)
    // ProgressBarManager::getInstance()->addProgressBar(
    //     ClientType::TrainClient, "Train Simulation",
    //     100);

    // Log initialization using logger if available
    if (m_logger)
    {
        m_logger->log("TrainSimulationClient initialized",
                      static_cast<int>(m_clientType));
    }
    else
    {
        qDebug() << "TrainSimulationClient initialized";
    }
}

TrainSimulationClient::~TrainSimulationClient()
{
    // Lock mutex to safely clean up resources
    QMutexLocker locker(&m_dataAccessMutex);

    // Delete all simulation results
    for (auto &results : m_networkData)
    {
        delete results; // Delete the pointer directly
    }
    m_networkData.clear();

    // Delete all train states
    for (auto &states : m_trainState)
    {
        qDeleteAll(
            states); // states is a QList<TrainState*>
    }
    m_trainState.clear();

    // Delete all loaded trains
    qDeleteAll(m_loadedTrains); // m_loadedTrains is a
                                // QMap<QString, Train*>
    m_loadedTrains.clear();

    // Log destruction using logger if available
    if (m_logger)
    {
        m_logger->log("TrainSimulationClient destroyed",
                      static_cast<int>(m_clientType));
    }
    else
    {
        qDebug() << "TrainSimulatorClient destroyed";
    }
}

bool TrainSimulationClient::resetServer()
{
    // Execute reset command in a serialized manner
    return executeSerializedCommand([this]() {
        // Send reset command and wait for confirmation
        bool success = sendCommandAndWait(
            "resetServer", QJsonObject(), {"serverReset"});

        // Log result of reset operation
        if (m_logger)
        {
            if (success)
            {
                m_logger->log(
                    "Server reset successful",
                    static_cast<int>(m_clientType));
            }
            else
            {
                m_logger->logError(
                    "Server reset failed",
                    static_cast<int>(m_clientType));
            }
        }
        return success;
    });
}

void TrainSimulationClient::initializeClient(
    LoggerInterface *logger)
{
    // Call base class initialization first
    SimulationClientBase::initializeClient(logger);

    // Ensure RabbitMQ handler exists
    if (!m_rabbitMQHandler)
    {
        throw std::runtime_error(
            "RabbitMQ handler not initialized");
    }

    // Ensure RabbitMQ handler exists
    if (!m_rabbitMQHandler)
    {
        if (m_logger)
        {
            m_logger->logError(
                "Cannot execute command: RabbitMQ "
                "handler not initialized",
                static_cast<int>(m_clientType));
        }
        throw std::runtime_error(
            "RabbitMQ handler not initialized");
    }

    // Set up heartbeat for connection health
    m_rabbitMQHandler->setupHeartbeat(5);

    // Log thread initialization
    if (m_logger)
    {
        m_logger->log(
            "Initialized in thread: "
                + QString::number(
                    reinterpret_cast<quintptr>(
                        QThread::currentThreadId())),
            static_cast<int>(m_clientType));
    }
    else
    {
        qDebug() << "TrainSimulationClient initialized in "
                    "thread:"
                 << QThread::currentThreadId();
    }
}

bool TrainSimulationClient::defineSimulatorByNetworkName(
    const QString &networkName, const double timeStep,
    const QList<Train *> &trains)
{
    // Placeholder for network registry (uncomment if
    // available) Network* network =
    // getNeTrainSimNetworkRegistry().getNetwork(networkName);
    // if (!network) {
    //     qCritical() << "Network" << networkName << "not
    //     found"; return false;
    // }
    // QJsonArray nodesJson = network->nodesToJson();
    // QJsonArray linksJson = network->linksToJson();
    QJsonArray nodesJson;
    QJsonArray linksJson;

    // Delegate to full defineSimulator method
    return defineSimulator(nodesJson, linksJson,
                           networkName, timeStep, trains);
}

bool TrainSimulationClient::defineSimulator(
    const QJsonArray &nodesJson,
    const QJsonArray &linksJson, const QString &networkName,
    const double timeStep, const QList<Train *> &trains)
{
    return executeSerializedCommand([&]() {
        // Prepare train data for JSON
        QJsonArray trainsArray;
        for (const auto *train : trains)
        {
            if (train)
            {
                trainsArray.append(train->toJson());
            }
        }

        // Build command parameters
        QJsonObject params;
        params["nodesJson"]   = nodesJson;
        params["linksJson"]   = linksJson;
        params["networkName"] = networkName;
        params["timeStep"]    = timeStep;
        if (!trains.isEmpty())
        {
            params["trains"] = trainsArray;
        }

        // Send command and wait for response
        bool success =
            sendCommandAndWait("defineSimulator", params,
                               {"simulationCreated"});

        // If successful, store train objects
        if (success)
        {
            QMutexLocker locker(&m_dataAccessMutex);
            for (const auto *train : trains)
            {
                if (train)
                {
                    m_loadedTrains[train->getUserId()] =
                        const_cast<Train *>(train);
                }
            }
            if (m_logger)
            {
                m_logger->log(
                    "Simulator defined for " + networkName,
                    static_cast<int>(m_clientType));
            }
        }
        else if (m_logger)
        {
            m_logger->logError(
                "Failed to define simulator for "
                    + networkName,
                static_cast<int>(m_clientType));
        }

        return success;
    });
}

bool TrainSimulationClient::runSimulator(
    const QStringList &networkNames, double byTimeSteps)
{
    // Execute in a serialized command block
    return executeSerializedCommand([&]() {
        // Handle wildcard for all networks
        QStringList networks = networkNames;
        if (networks.contains("*"))
        {
            QMutexLocker locker(&m_dataAccessMutex);
            networks = m_networkData.keys();
        }

        // Build command parameters
        QJsonObject params;
        QJsonArray  networksArray;
        for (const QString &network : networks)
        {
            networksArray.append(network);
        }
        params["networkNames"] = networksArray;
        params["byTimeSteps"]  = byTimeSteps;

        // Send command and wait for response
        bool success = sendCommandAndWait(
            "runSimulator", params,
            {"allTrainsReachedDestination"});

        // Log result of run operation
        if (m_logger)
        {
            if (success)
            {
                m_logger->log(
                    "Simulator run for "
                        + networks.join(", "),
                    static_cast<int>(m_clientType));
            }
            else
            {
                m_logger->logError(
                    "Failed to run simulator for "
                        + networks.join(", "),
                    static_cast<int>(m_clientType));
            }
        }

        return success;
    });
}

bool TrainSimulationClient::endSimulator(
    const QStringList &networkNames)
{
    // Execute in a serialized command block
    return executeSerializedCommand([&]() {
        // Handle wildcard for all networks
        QStringList networks = networkNames;
        if (networks.contains("*"))
        {
            QMutexLocker locker(&m_dataAccessMutex);
            networks = m_networkData.keys();
        }

        // Build command parameters
        QJsonObject params;
        QJsonArray  networksArray;
        for (const QString &network : networks)
        {
            networksArray.append(network);
        }
        params["networkNames"] = networksArray;

        // Send command and wait for response
        bool success = sendCommandAndWait(
            "endSimulator", params, {"simulationEnded"});

        // Log result of end operation
        if (m_logger)
        {
            if (success)
            {
                m_logger->log(
                    "Simulator ended for "
                        + networks.join(", "),
                    static_cast<int>(m_clientType));
            }
            else
            {
                m_logger->logError(
                    "Failed to end simulator for "
                        + networks.join(", "),
                    static_cast<int>(m_clientType));
            }
        }
        return success;
    });
}

bool TrainSimulationClient::addTrainsToSimulator(
    const QString        &networkName,
    const QList<Train *> &trains)
{
    return executeSerializedCommand([&]() {
        // Prepare train data for JSON
        QJsonArray trainsArray;
        for (const auto *train : trains)
        {
            if (train)
            {
                trainsArray.append(train->toJson());
            }
        }

        // Build command parameters
        QJsonObject params;
        params["network"] = networkName;
        params["trains"]  = trainsArray;

        // Send command and wait for response
        bool success = sendCommandAndWait(
            "addTrainsToSimulator", params,
            {"trainAddedToSimulator"});

        // If successful, store train objects
        if (success)
        {
            QMutexLocker locker(&m_dataAccessMutex);
            for (const auto *train : trains)
            {
                if (train)
                {
                    m_loadedTrains[train->getUserId()] =
                        const_cast<Train *>(train);
                }
            }
            if (m_logger)
            {
                m_logger->log(
                    "Trains added to " + networkName,
                    static_cast<int>(m_clientType));
            }
        }
        else if (m_logger)
        {
            m_logger->logError(
                "Failed to add trains to " + networkName,
                static_cast<int>(m_clientType));
        }

        return success;
    });
}

bool TrainSimulationClient::addContainersToTrain(
    const QString &networkName, const QString &trainId,
    const QList<ContainerCore::Container *> &containers)
{
    return executeSerializedCommand([&]() {
        // Prepare containers array for JSON
        QJsonArray containersArray;
        for (const auto *container : containers)
        {
            if (container)
            {
                containersArray.append(container->toJson());
            }
        }

        // Build command parameters
        QJsonObject params;
        params["networkName"] = networkName;
        params["trainID"]     = trainId;
        params["containers"]  = containersArray;

        // Send command and wait for response
        bool success = sendCommandAndWait(
            "addContainersToTrain", params,
            {"containersAddedToTrain"});

        // Log result of adding containers
        if (m_logger)
        {
            if (success)
            {
                m_logger->log(
                    "Containers added to train " + trainId,
                    static_cast<int>(m_clientType));
            }
            else
            {
                m_logger->logError(
                    "Failed to add containers to "
                        + trainId,
                    static_cast<int>(m_clientType));
            }
        }
        return success;
    });
}

bool TrainSimulationClient::unloadTrain(
    const QString &networkName, const QString &trainId,
    const QStringList &containersDestinationNames)
{
    // Execute in a serialized command block
    return executeSerializedCommand([&]() {
        // Attempt private unload first
        if (!unloadTrainPrivate(networkName, trainId,
                                containersDestinationNames))
        {
            return false;
        }

        // Build command parameters
        QJsonObject params{
            {"networkName", networkName},
            {"trainID", trainId},
            {"ContainersDestinationNames",
             QJsonArray::fromStringList(
                 containersDestinationNames)}};

        // Send command and wait for response
        bool success = sendCommandAndWait(
            "unloadContainersFromTrainAtCurrentTerminal",
            params, {"containersUnloaded"});

        // Log result of unload operation
        if (m_logger)
        {
            if (success)
            {
                m_logger->log(
                    "Train " + trainId + " unloaded",
                    static_cast<int>(m_clientType));
            }
            else
            {
                m_logger->logError(
                    "Failed to unload train " + trainId,
                    static_cast<int>(m_clientType));
            }
        }
        return success;
    });
}

bool TrainSimulationClient::unloadTrainPrivate(
    const QString &networkName, const QString &trainId,
    const QStringList &containersDestinationNames)
{
    // Build command parameters
    QJsonObject params;
    params["networkName"] = networkName;
    params["trainID"]     = trainId;
    params["ContainersDestinationNames"] =
        QJsonArray::fromStringList(
            containersDestinationNames);

    // Send command without waiting
    bool success = sendCommand(
        "unloadContainersFromTrainAtCurrentTerminal",
        params);

    // Log result of private unload
    if (m_logger && !success)
    {
        m_logger->logError("Private unload failed for "
                               + trainId,
                           static_cast<int>(m_clientType));
    }
    return success;
}

const TrainState *TrainSimulationClient::getTrainState(
    const QString &networkName,
    const QString &trainId) const
{
    QMutexLocker locker(&m_dataAccessMutex);
    if (!m_trainState.contains(networkName))
    {
        if (m_logger)
        {
            m_logger->log("No train state for network "
                              + networkName,
                          static_cast<int>(m_clientType));
        }
        return nullptr;
    }

    for (const auto *state : m_trainState[networkName])
    {
        if (state && state->m_trainUserId == trainId)
        {
            return state;
        }
    }

    if (m_logger)
    {
        m_logger->log("Train " + trainId + " not found in "
                          + networkName,
                      static_cast<int>(m_clientType));
    }
    return nullptr;
}

QList<const TrainState *>
TrainSimulationClient::getAllNetworkTrainStates(
    const QString &networkName) const
{
    QMutexLocker              locker(&m_dataAccessMutex);
    QList<const TrainState *> states;

    if (m_trainState.contains(networkName))
    {
        for (const auto *state : m_trainState[networkName])
        {
            if (state)
            {
                states.append(state);
            }
        }
    }
    else if (m_logger)
    {
        m_logger->log("No train states for network "
                          + networkName,
                      static_cast<int>(m_clientType));
    }

    return states;
}

QMap<QString, QList<const TrainState *>>
TrainSimulationClient::getAllTrainsStates() const
{
    QMutexLocker locker(&m_dataAccessMutex);
    QMap<QString, QList<const TrainState *>> allStates;

    for (auto it = m_trainState.constBegin();
         it != m_trainState.constEnd(); ++it)
    {
        QList<const TrainState *> networkStates;
        for (const auto *state : it.value())
        {
            if (state)
            {
                networkStates.append(state);
            }
        }
        allStates[it.key()] = networkStates;
    }

    return allStates;
}

void TrainSimulationClient::processMessage(
    const QJsonObject &message)
{
    // Call base class message processing
    SimulationClientBase::processMessage(message);

    // Check if message contains an event
    if (!message.contains("event"))
    {
        if (m_logger)
        {
            m_logger->log("Received message without event",
                          static_cast<int>(m_clientType));
        }
        return;
    }

    // Normalize event name for comparison
    QString event = message["event"]
                        .toString()
                        .toLower()
                        .trimmed()
                        .replace(" ", "");

    // Dispatch event to appropriate handler
    if (event == "simulationcreated")
    {
        onSimulationCreated(message);
    }
    else if (event == "simulationended")
    {
        onSimulationEnded(message);
    }
    else if (event == "trainreacheddestination")
    {
        onTrainReachedDestination(message);
    }
    else if (event == "alltrainsreacheddestination")
    {
        onAllTrainsReachedDestination(message);
    }
    else if (event == "simulationresultsavailable")
    {
        onSimulationResultsAvailable(message);
    }
    else if (event == "trainaddedtosimulator")
    {
        onTrainsAddedToSimulator(message);
    }
    else if (event == "erroroccurred")
    {
        onErrorOccurred(message);
    }
    else if (event == "serverreset")
    {
        onServerReset();
    }
    else if (event == "simulationadvanced")
    {
        onSimulationAdvanced(message);
    }
    else if (event == "containersaddedtotrain")
    {
        onContainersAdded(message);
    }
    else if (event == "simulationprogressupdate")
    {
        onSimulationProgressUpdate(message);
    }
    else if (event == "simulationpaused")
    {
        onSimulationPaused(message);
    }
    else if (event == "simulationresumed")
    {
        onSimulationResumed(message);
    }
    else if (event == "trainreachedterminal")
    {
        onTrainReachedTerminal(message);
    }
    else if (event == "containersunloaded")
    {
        onContainersUnloaded(message);
    }
    else
    {
        qWarning() << "Unrecognized event:" << event;
    }
}

void TrainSimulationClient::onSimulationCreated(
    const QJsonObject &message)
{
    // Extract network name from message
    QString network = message["network"].toString();

    // Lock mutex for safe data access
    QMutexLocker locker(&m_dataAccessMutex);

    // Initialize new results for network
    m_networkData[network] = new SimulationResults();

    // Log event using logger if available
    if (m_logger)
    {
        m_logger->log("Simulation created for network: "
                          + network,
                      static_cast<int>(m_clientType));
    }
    else
    {
        qDebug() << "Simulation created for network:"
                 << network;
    }
}

void TrainSimulationClient::onSimulationEnded(
    const QJsonObject &message)
{
    // Log event using logger if available
    if (m_logger)
    {
        m_logger->log("Simulation ended",
                      static_cast<int>(m_clientType));
    }
    else
    {
        qDebug() << "Simulation ended";
    }
}

void TrainSimulationClient::onTrainReachedDestination(
    const QJsonObject &message)
{
    // Lock mutex for safe data access
    QMutexLocker locker(&m_dataAccessMutex);

    // Extract train status from message
    QJsonObject trainStatus = message["state"].toObject();

    // List to track train IDs
    QStringList trainIds;

    // Process each network in the status
    for (auto it = trainStatus.begin();
         it != trainStatus.end(); ++it)
    {
        QString network = it.key();

        // Ensure network exists in train states
        if (!m_trainState.contains(network))
        {
            m_trainState[network] = QList<TrainState *>();
        }

        // Extract train state data
        QJsonObject data =
            it.value().toObject()["trainState"].toObject();

        // Create and store new train state
        TrainState *state = new TrainState(data);
        m_trainState[network].append(state);
        trainIds.append(state->m_trainUserId);

        // Check for containers to unload
        int containersCount =
            data["containersCount"].toInt();
        if (containersCount > 0
            && m_loadedTrains.contains(
                state->m_trainUserId))
        {
            // Get destination ID from train path
            QString destinationId = QString::number(
                m_loadedTrains[state->m_trainUserId]
                    ->getTrainPathOnNodeIds()
                    .last());

            // Construct terminal ID
            QString terminalId =
                QString("__network_details_$$||||$$%1$$||||"
                        "$$%2")
                    .arg(network, destinationId);

            // Placeholder for TerminalGraphServer
            // if
            // (TerminalGraphServer::getInstance()->terminalExists(terminalId))
            // {
            //     QStringList aliases =
            //     TerminalGraphServer::getInstance()
            //                               ->getAliasesOfTerminal(terminalId);
            //     if
            //     (TerminalGraphServer::getInstance()->terminal(terminalId)
            //             ->checkCapacityStatus(containersCount))
            //             {
            //         locker.unlock();
            //         unloadTrainPrivate(network,
            //         state->m_trainUserId, aliases);
            //         locker.relock();
            //     }
            // }
        }
    }

    // Log event using logger if available
    if (m_logger)
    {
        m_logger->log("Trains [" + trainIds.join(", ")
                          + "] reached destinations",
                      static_cast<int>(m_clientType));
    }
    else
    {
        qDebug() << "Trains [" << trainIds.join(", ")
                 << "] reached destinations";
    }
}

void TrainSimulationClient::onAllTrainsReachedDestination(
    const QJsonObject &message)
{
    // Extract network name from message
    QString network = message["networkName"].toString();

    // Log event using logger if available
    if (m_logger)
    {
        m_logger->log("All trains reached destination in: "
                          + network,
                      static_cast<int>(m_clientType));
    }
    else
    {
        qDebug() << "All trains reached destination in:"
                 << network;
    }
}

void TrainSimulationClient::onSimulationResultsAvailable(
    const QJsonObject &message)
{
    // Extract results from message
    QJsonObject results = message["results"].toObject();

    // Lock mutex for safe data access
    QMutexLocker locker(&m_dataAccessMutex);

    // Update results for each network
    for (auto it = results.begin(); it != results.end();
         ++it)
    {
        delete m_networkData[it.key()];
        m_networkData[it.key()] = new SimulationResults(
            SimulationResults::fromJson(
                it.value().toObject()));
    }

    // Log event using logger if available
    if (m_logger)
    {
        m_logger->log("Simulation results available for: "
                          + results.keys().join(", "),
                      static_cast<int>(m_clientType));
    }
    else
    {
        qDebug()
            << "Simulation results available for networks:"
            << results.keys().join(", ");
    }
}

void TrainSimulationClient::onTrainsAddedToSimulator(
    const QJsonObject &message)
{
    // Extract network name from message
    QString network = message["networkNames"].toString();

    // Log event using logger if available
    if (m_logger)
    {
        m_logger->log("Trains added to network: " + network,
                      static_cast<int>(m_clientType));
    }
    else
    {
        qDebug() << "Trains added to network:" << network;
    }
}

void TrainSimulationClient::onErrorOccurred(
    const QJsonObject &message)
{
    // Extract error message from JSON
    QString error = message["errorMessage"].toString();

    // Log error using logger if available
    if (m_logger)
    {
        m_logger->logError("Error occurred: " + error,
                           static_cast<int>(m_clientType));
    }
    else
    {
        qCritical() << "Error occurred:" << error;
    }
}

void TrainSimulationClient::onServerReset()
{
    // Lock mutex for safe data access
    QMutexLocker locker(&m_dataAccessMutex);

    // Clean up simulation results
    for (auto &results : m_networkData)
    {
        delete results;
    }
    m_networkData.clear();

    // Clean up train states
    for (auto &states : m_trainState)
    {
        qDeleteAll(states);
    }
    m_trainState.clear();

    // Clean up loaded trains
    qDeleteAll(m_loadedTrains);
    m_loadedTrains.clear();

    // Log event using logger if available
    if (m_logger)
    {
        m_logger->log("Server reset successfully",
                      static_cast<int>(m_clientType));
    }
    else
    {
        qDebug() << "Server reset successfully";
    }
}

void TrainSimulationClient::onSimulationAdvanced(
    const QJsonObject &message)
{
    // Extract progress data from message
    QJsonObject progresses =
        message["networkNamesProgress"].toObject();

    // Process progress if data exists
    if (!progresses.isEmpty())
    {
        double      totalProgress = 0.0;
        QStringList networks;

        // Calculate total progress and list networks
        for (auto it = progresses.begin();
             it != progresses.end(); ++it)
        {
            networks.append(it.key());
            totalProgress += it.value().toDouble();
        }

        // Compute average progress
        double average = totalProgress / networks.size();

        // Update progress bar (placeholder)
        // ProgressBarManager::getInstance()->updateProgress(
        //     ClientType::TrainClient, average);

        // Log event using logger if available
        if (m_logger)
        {
            m_logger->log("Simulation advanced for: "
                              + networks.join(", "),
                          static_cast<int>(m_clientType));
        }
        else
        {
            qDebug() << "Simulation advanced for:"
                     << networks.join(", ");
        }
    }
}

void TrainSimulationClient::onContainersAdded(
    const QJsonObject &message)
{
    // Extract network and train ID from message
    QString network = message["networkName"].toString();
    QString trainId = message["trainID"].toString();

    // Log event using logger if available
    if (m_logger)
    {
        m_logger->log("Containers added to train " + trainId
                          + " in " + network,
                      static_cast<int>(m_clientType));
    }
    else
    {
        qDebug() << "Containers added to train" << trainId
                 << "in" << network;
    }
}

void TrainSimulationClient::onSimulationProgressUpdate(
    const QJsonObject &message)
{
    // Extract progress value from message
    double progress = message["newProgress"].toDouble();

    // Update progress bar (placeholder)
    // ProgressBarManager::getInstance()->updateProgress(
    //     ClientType::TrainClient, progress);

    // Log progress update if logger available
    if (m_logger)
    {
        m_logger->updateProgress(
            progress, static_cast<int>(m_clientType));
    }
}

void TrainSimulationClient::onSimulationPaused(
    const QJsonObject &message)
{
    // Extract network names from message
    QJsonArray networks = message["networkNames"].toArray();

    // Log event using logger if available
    if (m_logger)
    {
        m_logger->log("Simulation paused for networks",
                      static_cast<int>(m_clientType));
    }
    else
    {
        qDebug() << "Simulation paused for:" << networks;
    }
}

void TrainSimulationClient::onSimulationResumed(
    const QJsonObject &message)
{
    // Extract network names from message
    QJsonArray networks = message["networkNames"].toArray();

    // Log event using logger if available
    if (m_logger)
    {
        m_logger->log("Simulation resumed for networks",
                      static_cast<int>(m_clientType));
    }
    else
    {
        qDebug() << "Simulation resumed for:" << networks;
    }
}

void TrainSimulationClient::onTrainReachedTerminal(
    const QJsonObject &message)
{
    // Extract terminal and container data
    QString terminalId = message["terminalID"].toString();
    int     containersCount =
        message["containersCount"].toInt();
    QString network = message["networkName"].toString();
    QString trainId = message["trainID"].toString();

    // Process if containers exist and terminal is valid
    if (containersCount > 0 && !terminalId.isEmpty())
    {
        // Construct full terminal ID
        QString fullTerminalId =
            QString(
                "__network_details_$$||||$$%1$$||||$$%2")
                .arg(network, terminalId);

        // Placeholder for TerminalGraphServer
        // if
        // (TerminalGraphServer::getInstance()->terminalExists(fullTerminalId))
        // {
        //     QStringList aliases =
        //     TerminalGraphServer::getInstance()
        //                               ->getAliasesOfTerminal(fullTerminalId);
        //     if
        //     (TerminalGraphServer::getInstance()->terminal(fullTerminalId)
        //             ->checkCapacityStatus(containersCount))
        //             {
        //         unloadTrainPrivate(network, trainId,
        //         aliases);
        //     }
        // }
    }

    // Log event using logger if available
    if (m_logger)
    {
        m_logger->log("Train " + trainId
                          + " reached terminal "
                          + terminalId,
                      static_cast<int>(m_clientType));
    }
}

void TrainSimulationClient::onContainersUnloaded(
    const QJsonObject &message)
{
    // Extract terminal and container data
    QString terminalId = message["terminalID"].toString();
    QJsonArray containers = message["containers"].toArray();

    // Wrap containers in a JSON object
    QJsonObject   containersObj{{"containers", containers}};
    QJsonDocument doc(containersObj);

    // Convert to compact JSON string
    QString containersJson =
        doc.toJson(QJsonDocument::Compact);

    // Placeholder for simulation time
    // double currentTime =
    // SimulatorTimeServer::getInstance()
    //                          ->getCurrentSimulationTime();
    double currentTime = 0.0;

    // Placeholder for TerminalGraphServer
    // if
    // (TerminalGraphServer::getInstance()->terminalExists(terminalId))
    // {
    //     TerminalGraphServer::getInstance()->terminal(terminalId)
    //         ->addContainersFromJson(containersJson,
    //         currentTime);
    // }

    // Log event using logger if available
    if (m_logger)
    {
        m_logger->log("Containers unloaded at terminal: "
                          + terminalId,
                      static_cast<int>(m_clientType));
    }
    else
    {
        qDebug() << "Containers unloaded at terminal:"
                 << terminalId;
    }
}

} // namespace TrainClient
} // namespace Backend
} // namespace CargoNetSim
