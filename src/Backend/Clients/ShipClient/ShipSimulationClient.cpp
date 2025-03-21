#include "ShipSimulationClient.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QThread>
#include <QCoreApplication>
#include <QFile>
#include <QDir>

// Placeholder includes
#include "Backend/Models/ShipSystem.h"
#include "Backend/Commons/LoggerInterface.h"
// #include "TerminalGraphServer.h"
// #include "SimulatorTimeServer.h"
// #include "ProgressBarManager.h"
// #include "ApplicationLogger.h"

namespace CargoNetSim {
namespace Backend {
namespace ShipClient {

/**
 * Constructor initializes the ship client
 */
ShipSimulationClient::ShipSimulationClient(
    QObject* parent,
    const QString& host,
    int port)
    : SimulationClientBase(
          parent,
          host,
          port,
          "CargoNetSim.Exchange",
          "CargoNetSim.CommandQueue.ShipNetSim",
          "CargoNetSim.ResponseQueue.ShipNetSim",
          "CargoNetSim.Command.ShipNetSim",
          QStringList{"CargoNetSim.Response.ShipNetSim"},
          ClientType::ShipClient)
{
    // Add progress bar placeholder
    // ProgressBarManager::getInstance()->addProgressBar(
    //     ClientType::ShipClient, "Ship Simulation", 100);

    // Log initialization using logger if available
    if (m_logger) {
        m_logger->log("ShipSimulationClient initialized",
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "ShipSimulatorClient initialized";
    }
}

/**
 * Destructor cleans up resources
 */
ShipSimulationClient::~ShipSimulationClient()
{
    QMutexLocker locker(&m_dataAccessMutex);

    // Clean up SimulationResults objects
    for (auto& resultsList : m_networkData) {
        qDeleteAll(resultsList);
    }
    m_networkData.clear();

    // Clean up ShipState objects
    for (auto& stateList : m_shipState) {
        qDeleteAll(stateList);
    }
    m_shipState.clear();

    // Clean up Ship objects
    qDeleteAll(m_loadedShips);
    m_loadedShips.clear();

    // Log destruction using logger if available
    if (m_logger) {
        m_logger->log("ShipSimulationClient destroyed",
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "ShipSimulatorClient destroyed";
    }
}

/**
 * Reset the server to its initial state
 */
bool ShipSimulationClient::resetServer()
{
    return executeSerializedCommand([this]() {
        bool success = sendCommandAndWait(
            "resetServer",
            QJsonObject(),
            {"serverReset"});

        // Log result of reset operation
        if (m_logger) {
            if (success) {
                m_logger->log("Server reset successful",
                              static_cast<int>(m_clientType));
            } else {
                m_logger->logError("Server reset failed",
                                   static_cast<int>(m_clientType));
            }
        }
        return success;
    });
}

void ShipSimulationClient::initializeClient(LoggerInterface *logger)
{
    // Call base class initialization to set up shared resources
    SimulationClientBase::initializeClient(logger);

    // Configure RabbitMQ handler's heartbeat for connection health
    if (m_rabbitMQHandler == nullptr) {
        if (m_logger) {
            m_logger->logError("Cannot execute command: RabbitMQ "
                               "handler not initialized",
                               static_cast<int>(m_clientType));
        }
        throw std::runtime_error("RabbitMQ handler not initialized");
    }
    m_rabbitMQHandler->setupHeartbeat(5);

    // Log initialization details using logger if available
    if (m_logger) {
        m_logger->log("Initialized in thread: " +
                          QString::number(reinterpret_cast<quintptr>(
                              QThread::currentThreadId())),
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "ShipSimulationClient initialized in thread:"
                 << QThread::currentThreadId();
    }
}

/**
 * Define a new ship simulator with specified parameters
 */
bool ShipSimulationClient::defineSimulator(
    const QString& networkName,
    double timeStep,
    const QList<QJsonObject>& ships,
    const QMap<QString, QStringList>& destinationTerminalIds,
    const QString& networkPath)
{
    return executeSerializedCommand([&]() {
        try {
            // Prepare ship data
            QJsonArray shipsArray;
            QList<Backend::Ship*> shipObjects;

            // Convert ship data to JSON
            try {
                for (const QJsonObject& shipData : ships) {
                    // Create Ship objects
                    Backend::Ship* ship = new Backend::Ship(shipData);
                    shipObjects.append(ship);

                    // Use ship's toJson method to get JSON representation
                    shipsArray.append(ship->toJson());
                }
            } catch (const std::exception& e) {
                if (m_logger) {
                    m_logger->logError("Error converting ships: " +
                                           QString(e.what()),
                                       static_cast<int>(m_clientType));
                } else {
                    qCritical() << "Error converting ships:" << e.what();
                }                qDeleteAll(shipObjects);
                return false;
            }

            // Create command parameters
            QJsonObject params;
            params["networkFilePath"] = networkPath;
            params["networkName"] = networkName;
            params["timeStep"] = timeStep;

            if (!ships.isEmpty()) {
                params["ships"] = shipsArray;
            }

            // Send command and wait for response
            bool success = sendCommandAndWait(
                "defineSimulator",
                params,
                {"simulationcreated"});

            if (success) {
                QMutexLocker locker(&m_dataAccessMutex);

                // Store ship objects
                for (Backend::Ship* ship : shipObjects) {
                    m_loadedShips[ship->getUserId()] = ship;

                    QStringList terminals =
                        destinationTerminalIds.value(ship->getUserId());
                    m_shipsDestinationTerminals[ship->getUserId()] =
                        terminals;
                }
            } else {
                qDeleteAll(shipObjects);
                return false;
            }

            return true;

        } catch (const std::exception& e) {
            if (m_logger) {
                m_logger->logError("Exception in defineSimulator: " +
                                       QString(e.what()),
                                   static_cast<int>(m_clientType));
            } else {
                qCritical() << "Exception in defineSimulator:" << e.what();
            }
            return false;
        }
    });
}

/**
 * Run the simulator for specified networks
 */
bool ShipSimulationClient::runSimulator(
    const QStringList& networkNames,
    double byTimeSteps)
{
    return executeSerializedCommand([&]() {
        QStringList networks = networkNames;

        // If "*" specified, use all networks
        if (networks.contains("*")) {
            QMutexLocker locker(&m_dataAccessMutex);
            networks = m_networkData.keys();
        }

        // Create command parameters
        QJsonObject params;

        QJsonArray networksArray;
        for (const QString& network : networks) {
            networksArray.append(network);
        }
        params["networkNames"] = networksArray;
        params["byTimeSteps"] = byTimeSteps;

        // Send command and wait for response
        bool success = sendCommandAndWait(
            "runSimulator",
            params,
            {"allshipsreacheddestination"});

        // Log result of run operation
        if (m_logger) {
            if (success) {
                m_logger->log("Simulator run for " + networks.join(", "),
                              static_cast<int>(m_clientType));
            } else {
                m_logger->logError("Failed to run simulator for " +
                                       networks.join(", "),
                                   static_cast<int>(m_clientType));
            }
        }
        return success;
    });
}

/**
 * End the simulator for specified networks
 */
bool ShipSimulationClient::endSimulator(
    const QStringList& networkNames)
{
    return executeSerializedCommand([&]() {
        QStringList networks = networkNames;

        // If "*" specified, use all networks
        if (networks.contains("*")) {
            QMutexLocker locker(&m_dataAccessMutex);
            networks = m_networkData.keys();
        }

        // Create command parameters
        QJsonObject params;

        QJsonArray networksArray;
        for (const QString& network : networks) {
            networksArray.append(network);
        }
        params["network"] = networksArray;

        // Send command and wait for response
        bool success = sendCommandAndWait(
            "endSimulator",
            params,
            {"simulationended"});

        // Log result of end operation
        if (m_logger) {
            if (success) {
                m_logger->log("Simulator ended for " + networks.join(", "),
                              static_cast<int>(m_clientType));
            } else {
                m_logger->logError("Failed to end simulator for " +
                                       networks.join(", "),
                                   static_cast<int>(m_clientType));
            }
        }
        return success;
    });
}

/**
 * Add ships to an existing simulator
 */
bool ShipSimulationClient::addShipsToSimulator(
    const QString& networkName,
    const QList<QJsonObject>& ships,
    const QMap<QString, QStringList>& destinationTerminalIds)
{
    return executeSerializedCommand([&]() {
        // Prepare ship data
        QJsonArray shipsArray;
        QList<Ship*> shipObjects;

        // Convert ship data to JSON
        try {
            for (const QJsonObject& shipData : ships) {
                // Create Ship objects
                Ship* ship = new Ship(shipData);
                shipObjects.append(ship);

                // Use ship's toJson method to get JSON representation
                shipsArray.append(ship->toJson());
            }
        } catch (const std::exception& e) {
            if (m_logger) {
                m_logger->logError("Error converting ships: " +
                                       QString(e.what()),
                                   static_cast<int>(m_clientType));
            } else {
                qCritical() << "Error converting ships:" << e.what();
            }
            qDeleteAll(shipObjects);
            return false;
        }

        // Create command parameters
        QJsonObject params;
        params["networkName"] = networkName;
        params["ships"] = shipsArray;

        // Send command and wait for response
        bool success = sendCommandAndWait(
            "addShipsToSimulator",
            params,
            {"shipaddedtosimulator"});

        if (success) {
            QMutexLocker locker(&m_dataAccessMutex);

            // Store ship objects
            for (Ship* ship : shipObjects) {
                m_loadedShips[ship->getUserId()] = ship;

                QStringList terminals =
                    destinationTerminalIds.value(ship->getUserId());
                m_shipsDestinationTerminals[ship->getUserId()] = terminals;
            }
            if (m_logger) {
                m_logger->log("Ships added to " + networkName,
                              static_cast<int>(m_clientType));
            }
        } else {
            qDeleteAll(shipObjects);
            return false;
        }

        return true;
    });
}

/**
 * Add containers to a ship in the simulator
 */
bool ShipSimulationClient::addContainersToShip(
    const QString& networkName,
    const QString& shipId,
    const QStringList& containers)
{
    return executeSerializedCommand([&]() {
        // Parse container data
        QJsonArray containersArray;

        for (const QString& containerStr : containers) {
            // Replace NaN with null in JSON strings
            QString fixedStr = containerStr;
            fixedStr.replace("\"addedTime\": NaN",
                             "\"addedTime\": null");

            // Parse JSON
            QJsonDocument doc =
                QJsonDocument::fromJson(fixedStr.toUtf8());

            if (doc.isNull() || !doc.isObject()) {
                if (m_logger) {
                    m_logger->logError("Error parsing container JSON: " +
                                           containerStr,
                                       static_cast<int>(m_clientType));
                } else {
                    qCritical() << "Error parsing container JSON:"
                                << containerStr;
                }
                return false;
            }

            containersArray.append(doc.object());
        }

        // Create command parameters
        QJsonObject params;
        params["networkName"] = networkName;
        params["shipID"] = shipId;
        params["containers"] = containersArray;

        // Send command and wait for response
        bool success = sendCommandAndWait(
            "addContainersToShip",
            params,
            {"containersaddedtoship"});

        // Log result of adding containers
        if (m_logger) {
            if (success) {
                m_logger->log("Containers added to ship " + shipId,
                              static_cast<int>(m_clientType));
            } else {
                m_logger->logError("Failed to add containers to " + shipId,
                                   static_cast<int>(m_clientType));
            }
        }
        return success;
    });
}

/**
 * Unload containers internal implementation
 */
bool ShipSimulationClient::unloadContainersFromShipAtTerminalsPrivate(
    const QString& networkName,
    const QString& shipId,
    const QStringList& terminalNames)
{
    // Create command parameters
    QJsonObject params;
    params["networkName"] = networkName;
    params["shipID"] = shipId;

    QJsonArray terminalsArray;
    for (const QString& terminal : terminalNames) {
        terminalsArray.append(terminal);
    }
    params["terminalNames"] = terminalsArray;

    // Send command
    bool success = sendCommand("unloadContainersFromShipAtTerminal", params);

    // Log result of private unload
    if (m_logger && !success) {
        m_logger->logError("Private unload failed for " + shipId,
                           static_cast<int>(m_clientType));
    }
    return success;
}

/**
 * Unload containers from a ship at specified terminals
 */
bool ShipSimulationClient::unloadContainersFromShipAtTerminals(
    const QString& networkName,
    const QString& shipId,
    const QStringList& terminalNames)
{
    return executeSerializedCommand([&]() {
        // Create command parameters
        QJsonObject params;
        params["networkName"] = networkName;
        params["shipID"] = shipId;

        QJsonArray terminalsArray;
        for (const QString& terminal : terminalNames) {
            terminalsArray.append(terminal);
        }
        params["terminalNames"] = terminalsArray;

        // Send command and wait for response
        bool success = sendCommandAndWait(
            "unloadContainersFromShipAtTerminal",
            params,
            {"shipunloadedcontainers"});

        // Log result of unload operation
        if (m_logger) {
            if (success) {
                m_logger->log("Ship " + shipId + " unloaded",
                              static_cast<int>(m_clientType));
            } else {
                m_logger->logError("Failed to unload ship " + shipId,
                                   static_cast<int>(m_clientType));
            }
        }
        return success;
    });
}

/**
 * Get terminal nodes in a network
 */
void ShipSimulationClient::getNetworkTerminalNodes(
    const QString& networkName)
{
    executeSerializedCommand([&]() {
        // Create command parameters
        QJsonObject params;
        params["network"] = networkName;

        // Send command
        bool success = sendCommand("getNetworkSeaPorts", params);

        // Log request if logger available
        if (m_logger) {
            m_logger->log("Requested terminal nodes for " + networkName,
                          static_cast<int>(m_clientType));
        }
        return success;
    });
}

/**
 * Get the shortest path between two nodes in a network
 */
void ShipSimulationClient::getShortestPath(
    const QString& networkName,
    const QString& startNode,
    const QString& endNode)
{
    executeSerializedCommand([&]() {
        // Create command parameters
        QJsonObject params;
        params["network"] = networkName;
        params["startNode"] = startNode;
        params["endNode"] = endNode;

        // Send command
        bool success = sendCommand("getShortestPath", params);

        // Log request if logger available
        if (m_logger) {
            m_logger->log("Requested shortest path in " + networkName,
                          static_cast<int>(m_clientType));
        }
        return success;
    });
}

/**
 * Get the state of a specific ship
 */
QJsonObject ShipSimulationClient::getShipState(
    const QString& networkName,
    const QString& shipId) const
{
    QMutexLocker locker(&m_dataAccessMutex);

    if (!m_shipState.contains(networkName)) {
        if (m_logger) {
            m_logger->log("No ship state for network " + networkName,
                          static_cast<int>(m_clientType));
        }
        return QJsonObject();
    }

    const QList<ShipState*>& states = m_shipState[networkName];

    for (const ShipState* state : states) {
        if (state->shipId() == shipId) {
            return state->toJson();
        }
    }

    if (m_logger) {
        m_logger->log("Ship " + shipId + " not found in " + networkName,
                      static_cast<int>(m_clientType));
    }
    return QJsonObject();
}

/**
 * Get states of all ships in a network
 */
QJsonArray ShipSimulationClient::getAllNetworkShipsStates(
    const QString& networkName) const
{
    QMutexLocker locker(&m_dataAccessMutex);

    QJsonArray states;

    if (!m_shipState.contains(networkName)) {
        if (m_logger) {
            m_logger->log("No ship states for " + networkName,
                          static_cast<int>(m_clientType));
        }
        return states;
    }

    const QList<ShipState*>& networkStates = m_shipState[networkName];

    for (const ShipState* state : networkStates) {
        states.append(state->toJson());
    }

    return states;
}

/**
 * Get states of all ships in all networks
 */
QJsonObject ShipSimulationClient::getAllShipsStates() const
{
    QMutexLocker locker(&m_dataAccessMutex);

    QJsonObject allStates;

    for (auto it = m_shipState.constBegin();
         it != m_shipState.constEnd();
         ++it) {
        QJsonArray networkStates;

        for (const ShipState* state : it.value()) {
            networkStates.append(state->toJson());
        }

        allStates[it.key()] = networkStates;
    }

    return allStates;
}

/**
 * Process messages received from the server
 */
void ShipSimulationClient::processMessage(const QJsonObject& message)
{
    // Call parent implementation
    SimulationClientBase::processMessage(message);

    // Extract event type
    if (!message.contains("event")) {
        if (m_logger) {
            m_logger->log("Received message without event",
                          static_cast<int>(m_clientType));
        }
        return; // Not an event message
    }

    QString eventType = message.value("event").toString();
    QString normalizedEvent = normalizeEventName(eventType);

    // Handle specific event types
    if (normalizedEvent == "simulationnetworkloaded") {
        onSimulationNetworkLoaded(message);
    } else if (normalizedEvent == "simulationcreated") {
        onSimulationCreated(message);
    } else if (normalizedEvent == "simulationended") {
        onSimulationEnded(message);
    } else if (normalizedEvent == "simulationadvanced") {
        onSimulationAdvanced(message);
    } else if (normalizedEvent == "simulationprogressupdate") {
        onSimulationProgressUpdate(message);
    } else if (normalizedEvent == "shipaddedtosimulator") {
        onShipAddedToSimulator(message);
    } else if (normalizedEvent == "shipreacheddestination") {
        onShipReachedDestination(message);
    } else if (normalizedEvent == "allshipsreacheddestination") {
        onAllShipsReachedDestination(message);
    } else if (normalizedEvent == "simulationresultsavailable") {
        onSimulationResultsAvailable(message);
    } else if (normalizedEvent == "shipstate") {
        onShipStateAvailable(message);
    } else if (normalizedEvent == "simulatorstate") {
        onSimulatorStateAvailable(message);
    } else if (normalizedEvent == "containersaddedtoship") {
        onContainersAdded(message);
    } else if (normalizedEvent == "containersunloaded") {
        onContainersUnloaded(message);
    } else if (normalizedEvent == "shipreachedseaport") {
        onShipReachedSeaport(message);
    } else if (normalizedEvent == "erroroccurred") {
        onErrorOccurred(message);
    } else if (normalizedEvent == "serverreset") {
        onServerReset();
    } else if (normalizedEvent == "simulationpaused") {
        onSimulationPaused(message);
    } else if (normalizedEvent == "simulationresumed") {
        onSimulationResumed(message);
    } else if (normalizedEvent == "simulationrestarted") {
        onSimulationRestarted(message);
    } else {
        if (m_logger) {
            m_logger->log("Unrecognized event: " + eventType,
                          static_cast<int>(m_clientType));
        } else {
            qWarning() << "Unrecognized event:" << eventType;
        }
    }
}

// Event handling methods

void ShipSimulationClient::onSimulationNetworkLoaded(
    const QJsonObject& message)
{
    if (m_logger) {
        m_logger->log("Simulation network loaded",
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "Simulation network loaded.";
    }
}

void ShipSimulationClient::onSimulationCreated(
    const QJsonObject& message)
{
    qDebug() << "Simulation created.";

    QString networkName = message.value("networkName").toString();

    QMutexLocker locker(&m_dataAccessMutex);
    m_networkData[networkName] = QList<SimulationResults*>();

    if (m_logger) {
        m_logger->log("Simulation created for " + networkName,
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "Simulation created.";
    }
}

void ShipSimulationClient::onSimulationPaused(
    const QJsonObject& message)
{
    if (m_logger) {
        m_logger->log("Simulation paused",
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "Simulation paused.";
    }
}

void ShipSimulationClient::onSimulationResumed(
    const QJsonObject& message)
{
    if (m_logger) {
        m_logger->log("Simulation resumed",
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "Simulation resumed.";
    }
}

void ShipSimulationClient::onSimulationRestarted(
    const QJsonObject& message)
{
    if (m_logger) {
        m_logger->log("Simulation restarted",
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "Simulation restarted.";
    }
}

void ShipSimulationClient::onSimulationEnded(
    const QJsonObject& message)
{
    if (m_logger) {
        m_logger->log("Simulation ended",
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "Simulation ended.";
    }
}

void ShipSimulationClient::onSimulationAdvanced(
    const QJsonObject& message)
{
    double newTime = message.value("newSimulationTime").toDouble();
    qDebug() << "Simulation advanced to time:" << newTime;

    QJsonObject networkProgresses =
        message.value("networkNamesProgress").toObject();

    if (!networkProgresses.isEmpty()) {
        double totalProgress = 0.0;
        QStringList networks;

        for (auto it = networkProgresses.constBegin();
             it != networkProgresses.constEnd();
             ++it) {
            networks.append(it.key());
            totalProgress += it.value().toDouble();
        }

        double average = networks.isEmpty() ? 0.0 :
                             totalProgress / networks.size();

        // ProgressBarManager::getInstance()->updateProgress(
        //    ClientType::ShipClient, average);

        if (m_logger) {
            m_logger->log("Simulation advanced to time: " +
                              QString::number(newTime),
                          static_cast<int>(m_clientType));
            m_logger->log("Simulations advanced for " + networks.join(", "),
                          static_cast<int>(m_clientType));
        } else {
            qDebug() << "Simulation advanced to time:" << newTime;
            qDebug() << "Simulations advanced for" << networks.join(", ");
        }
    } else {
        if (m_logger) {
            m_logger->log("Invalid 'networkNamesProgress' in message",
                          static_cast<int>(m_clientType));
        } else {
            qWarning() << "Invalid or missing 'networkNamesProgress' "
                       << "in the message.";
        }
    }
}

void ShipSimulationClient::onSimulationProgressUpdate(
    const QJsonObject& message)
{
    double progress = message.value("newProgress").toDouble();

    // ProgressBarManager::getInstance()->updateProgress(
    //    ClientType::ShipClient, progress);

    if (m_logger) {
        m_logger->updateProgress(progress,
                                 static_cast<int>(m_clientType));
    }
}

void ShipSimulationClient::onShipAddedToSimulator(
    const QJsonObject& message)
{
    QString shipId = message.value("shipID").toString();

    if (m_logger) {
        m_logger->log("Ship " + shipId + " added to simulator",
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "Ship" << shipId << "added to simulator.";
    }
}

void ShipSimulationClient::onAllShipsReachedDestination(
    const QJsonObject& message)
{
    QString networkName = message.value("networkName").toString();

    if (m_logger) {
        m_logger->log("All ships reached destination in " + networkName,
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "All ships reached destination of network:"
                 << networkName;
    }
}

void ShipSimulationClient::onShipReachedDestination(
    const QJsonObject& message)
{
    QMutexLocker locker(&m_dataAccessMutex);

    QJsonObject shipStatus = message.value("state").toObject();
    QStringList shipIds;

    for (auto it = shipStatus.constBegin();
         it != shipStatus.constEnd();
         ++it) {
        QString networkName = it.key();

        // Ensure the key exists in m_shipState
        if (!m_shipState.contains(networkName)) {
            m_shipState[networkName] = QList<ShipState*>();
        }

        QJsonObject networkStatus = it.value().toObject();
        if (networkStatus.contains("shipStates")) {
            QJsonObject shipData =
                networkStatus.value("shipStates").toObject();

            QString shipId = shipData.value("shipID").toString();
            int containersCount =
                shipData.value("containersCount").toInt();

            QStringList terminalIds =
                m_shipsDestinationTerminals.value(shipId);

            ShipState* shipState = new ShipState(shipData);
            m_shipState[networkName].append(shipState);
            shipIds.append(shipId);

            // We must release the mutex while calling other methods
            // to avoid deadlocks
            locker.unlock();

            // Put containers in terminal
            bool foundTerminal = false;
            for (const QString& terminalId : terminalIds) {
                // In real implementation with terminal graph server:
                // if (graphServer->terminalExists(terminalId)) {
                //    foundTerminal = true;
                //    bool canAdd = graphServer->terminal(terminalId)
                //        ->checkCapacityStatus(containersCount);
                //    if (canAdd) {
                //        unloadContainersFromShipAtTerminalsPrivate(
                //            networkName, shipId,
                //            QStringList{terminalId});
                //    }
                // }

                // Placeholder for terminal check
                foundTerminal = true;
                unloadContainersFromShipAtTerminalsPrivate(
                    networkName, shipId, QStringList{terminalId});
            }

            if (!foundTerminal && m_logger) {
                m_logger->log("No terminal of [" + terminalIds.join(", ") +
                                  "] exists",
                              static_cast<int>(m_clientType));
            } else if (!foundTerminal) {
                qWarning() << "No terminal of ["
                           << terminalIds.join(", ")
                           << "] exist in the terminal manager!";
            }

            // Reacquire the mutex for continued access to shared data
            locker.relock();
        }
    }

    if (m_logger) {
        m_logger->log("Ships [" + shipIds.join(", ") +
                          "] reached destinations",
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "Ships [" << shipIds.join(", ")
        << "] reached destinations";
    }
}

void ShipSimulationClient::onShipReachedSeaport(
    const QJsonObject& message)
{
    QString terminalId = message.value("seaPortCode").toString();
    int containersCount = message.value("containersCount").toInt();
    QString networkName = message.value("networkName").toString();
    QString shipId = message.value("shipID").toString();

    // In real implementation with terminal graph server:
    // Terminal capacity check would go here
    // if (graphServer->terminalExists(terminalId)) {
    //    bool canAdd = graphServer->terminal(terminalId)
    //        ->checkCapacityStatus(containersCount);
    //    if (canAdd) {
    //        unloadContainersFromShipAtTerminalsPrivate(
    //            networkName, shipId, QStringList{terminalId});
    //    }
    // }

    // Placeholder implementation
    bool success = unloadContainersFromShipAtTerminalsPrivate(
        networkName, shipId, QStringList{terminalId});

    if (m_logger) {
        m_logger->log("Ship " + shipId + " reached seaport " + terminalId,
                      static_cast<int>(m_clientType));
    }
}

void ShipSimulationClient::onContainersUnloaded(
    const QJsonObject& message)
{
    QJsonArray containers = message.value("containers").toArray();
    QString portName = message.value("portName").toString();

    // Create JSON document with containers
    QJsonObject containersObj;
    containersObj["containers"] = containers;
    QJsonDocument containersDoc(containersObj);
    QString containersJson =
        containersDoc.toJson(QJsonDocument::Compact);

    // In real implementation using time server:
    // double currentTime =
    //    SimulatorTimeServer::getInstance()->getCurrentSimulationTime();
    double currentTime = 0.0;

    // In real implementation using terminal graph server:
    // if (TerminalGraphServer::getInstance()->terminalExists(portName)) {
    //    try {
    //        TerminalGraphServer::getInstance()->terminal(portName)
    //            ->addContainersFromJson(
    //                containersJson, currentTime);
    //    } catch (const std::runtime_error& e) {
    //        qWarning() << "Error unloading containers:" << e.what();
    //    }
    // }

    if (m_logger) {
        m_logger->log("Containers unloaded at port: " + portName,
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "Containers unloaded at port:" << portName;
    }
}

void ShipSimulationClient::onSimulationResultsAvailable(
    const QJsonObject& message)
{
    QJsonObject results = message.value("results").toObject();

    if (m_logger) {
        m_logger->log("Simulation results available",
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "Simulation results available";
    }
}

void ShipSimulationClient::onShipStateAvailable(
    const QJsonObject& message)
{
    QJsonObject shipState = message.value("state").toObject();

    if (m_logger) {
        m_logger->log("Ship state available",
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "Ship state available";
    }
}

void ShipSimulationClient::onSimulatorStateAvailable(
    const QJsonObject& message)
{
    QJsonObject simulatorState = message.value("state").toObject();

    if (m_logger) {
        m_logger->log("Simulator state available",
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "Simulator state available";
    }
}

void ShipSimulationClient::onErrorOccurred(
    const QJsonObject& message)
{
    QString errorMessage = message.value("errorMessage").toString();
    if (m_logger) {
        m_logger->logError("Error occurred: " + errorMessage,
                           static_cast<int>(m_clientType));
    } else {
        qCritical() << "Error occurred:" << errorMessage;
    }
}

void ShipSimulationClient::onServerReset()
{
    if (m_logger) {
        m_logger->log("Server reset successfully",
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "Server Reset Successfully";
    }
}

void ShipSimulationClient::onContainersAdded(
    const QJsonObject& message)
{
    QString network = message.value("networkName").toString();
    QString shipId = message.value("shipID").toString();

    if (m_logger) {
        m_logger->log("Containers added to ship " + shipId +
                          " on " + network,
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "Containers added to ship" << shipId
                 << "on network" << network;
    }
}

} // namespace ShipClient
} // namespace Backend
} // namespace CargoNetSim
