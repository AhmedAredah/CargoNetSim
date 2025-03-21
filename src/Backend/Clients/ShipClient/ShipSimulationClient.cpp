/**
 * @file ShipSimulationClient.cpp
 * @brief Implementation of the ShipSimulationClient class
 *
 * This file contains the implementation of the ShipSimulationClient
 * class, providing functionality for managing ship simulations,
 * including setup, control, and state retrieval within the
 * CargoNetSim framework.
 *
 * @author Ahmed Aredah
 * @date March 19, 2025
 */

#include "ShipSimulationClient.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QThread>
#include <QCoreApplication>
#include <QFile>
#include <QDir>

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
 * @brief Constructs a ShipSimulationClient instance
 *
 * Initializes the client with RabbitMQ connection parameters and
 * sets up base simulation client properties.
 *
 * @param parent Parent QObject, defaults to nullptr
 * @param host RabbitMQ hostname, defaults to "localhost"
 * @param port RabbitMQ port, defaults to 5672
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
    if (m_logger) {
        m_logger->log("ShipSimulationClient initialized",
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "ShipSimulatorClient initialized";
    }
}

/**
 * @brief Destroys the ShipSimulationClient instance
 *
 * Frees all dynamically allocated resources and logs destruction.
 */
ShipSimulationClient::~ShipSimulationClient()
{
    QMutexLocker locker(&m_dataAccessMutex);
    for (auto& resultsList : m_networkData) {
        qDeleteAll(resultsList);
    }
    m_networkData.clear();
    for (auto& stateList : m_shipState) {
        qDeleteAll(stateList);
    }
    m_shipState.clear();
    qDeleteAll(m_loadedShips);
    m_loadedShips.clear();
    if (m_logger) {
        m_logger->log("ShipSimulationClient destroyed",
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "ShipSimulatorClient destroyed";
    }
}

/**
 * @brief Resets the ship simulation server
 *
 * Clears all simulation data on the server and logs the result.
 *
 * @return True if reset succeeds, false otherwise
 */
bool ShipSimulationClient::resetServer()
{
    return executeSerializedCommand([this]() {
        bool success = sendCommandAndWait(
            "resetServer",
            QJsonObject(),
            {"serverReset"});
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

/**
 * @brief Initializes the client in its thread
 *
 * Configures RabbitMQ heartbeat and logs initialization details.
 *
 * @param logger Optional logger for initialization logging
 * @throws std::runtime_error If RabbitMQ handler is not set
 */
void ShipSimulationClient::initializeClient(LoggerInterface* logger)
{
    SimulationClientBase::initializeClient(logger);
    if (m_rabbitMQHandler == nullptr) {
        if (m_logger) {
            m_logger->logError("RabbitMQ handler not initialized",
                               static_cast<int>(m_clientType));
        }
        throw std::runtime_error("RabbitMQ handler not initialized");
    }
    m_rabbitMQHandler->setupHeartbeat(5);
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
 * @brief Defines a new ship simulator
 *
 * Sets up a simulation with ships and parameters, storing ship data.
 *
 * @param networkName Network name
 * @param timeStep Simulation time step
 * @param ships List of Ship pointers
 * @param destinationTerminalIds Ship ID to terminal IDs map
 * @param networkPath Network file path
 * @return True if successful
 */
bool ShipSimulationClient::defineSimulator(
    const QString& networkName,
    const double timeStep,
    const QList<Ship*>& ships,
    const QMap<QString, QStringList>& destinationTerminalIds,
    const QString& networkPath)
{
    return executeSerializedCommand([&]() {
        try {
            QJsonArray shipsArray;
            for (const auto* ship : ships) {
                if (ship) {
                    shipsArray.append(ship->toJson());
                }
            }
            QJsonObject params;
            params["networkFilePath"] = networkPath;
            params["networkName"] = networkName;
            params["timeStep"] = timeStep;
            if (!ships.isEmpty()) {
                params["ships"] = shipsArray;
            }
            bool success = sendCommandAndWait(
                "defineSimulator",
                params,
                {"simulationcreated"});
            if (success) {
                QMutexLocker locker(&m_dataAccessMutex);
                for (auto* ship : ships) {
                    if (ship) {
                        m_loadedShips[ship->getUserId()] = ship;
                        auto terminals = destinationTerminalIds.value(
                            ship->getUserId());
                        m_shipsDestinationTerminals[ship->getUserId()] =
                            terminals;
                    }
                }
            }
            return success;
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
 * @brief Runs the simulator for specified networks
 *
 * Initiates simulation execution for given networks.
 *
 * @param networkNames Networks to run or "*" for all
 * @param byTimeSteps Time steps to run, -1 for unlimited
 * @return True if successful
 */
bool ShipSimulationClient::runSimulator(
    const QStringList& networkNames,
    double byTimeSteps)
{
    return executeSerializedCommand([&]() {
        QStringList networks = networkNames;
        if (networks.contains("*")) {
            QMutexLocker locker(&m_dataAccessMutex);
            networks = m_networkData.keys();
        }
        QJsonObject params;
        QJsonArray networksArray;
        for (const QString& network : networks) {
            networksArray.append(network);
        }
        params["networkNames"] = networksArray;
        params["byTimeSteps"] = byTimeSteps;
        bool success = sendCommandAndWait(
            "runSimulator",
            params,
            {"allshipsreacheddestination"});
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
 * @brief Ends the simulator for specified networks
 *
 * Stops simulation execution for given networks.
 *
 * @param networkNames Networks to end or "*" for all
 * @return True if successful
 */
bool ShipSimulationClient::endSimulator(
    const QStringList& networkNames)
{
    return executeSerializedCommand([&]() {
        QStringList networks = networkNames;
        if (networks.contains("*")) {
            QMutexLocker locker(&m_dataAccessMutex);
            networks = m_networkData.keys();
        }
        QJsonObject params;
        QJsonArray networksArray;
        for (const QString& network : networks) {
            networksArray.append(network);
        }
        params["network"] = networksArray;
        bool success = sendCommandAndWait(
            "endSimulator",
            params,
            {"simulationended"});
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
 * @brief Adds ships to an existing simulator
 *
 * Integrates new ships into a running simulation network.
 *
 * @param networkName Target network name
 * @param ships List of Ship pointers to add
 * @param destinationTerminalIds Ship ID to terminal IDs map
 * @return True if successful
 */
bool ShipSimulationClient::addShipsToSimulator(
    const QString& networkName,
    const QList<Ship*>& ships,
    const QMap<QString, QStringList>& destinationTerminalIds)
{
    return executeSerializedCommand([&]() {
        QJsonArray shipsArray;
        for (const auto* ship : ships) {
            if (ship) {
                shipsArray.append(ship->toJson());
            }
        }
        QJsonObject params;
        params["networkName"] = networkName;
        params["ships"] = shipsArray;
        bool success = sendCommandAndWait(
            "addShipsToSimulator",
            params,
            {"shipaddedtosimulator"});
        if (success) {
            QMutexLocker locker(&m_dataAccessMutex);
            for (auto* ship : ships) {
                if (ship) {
                    m_loadedShips[ship->getUserId()] = ship;
                    auto terminals = destinationTerminalIds.value(
                        ship->getUserId());
                    m_shipsDestinationTerminals[ship->getUserId()] = terminals;
                }
            }
            if (m_logger) {
                m_logger->log("Ships added to " + networkName,
                              static_cast<int>(m_clientType));
            }
        }
        return success;
    });
}

/**
 * @brief Adds containers to a ship
 *
 * Assigns containers to a ship in a specified network.
 *
 * @param networkName Network name
 * @param shipId Ship identifier
 * @param containers List of Container pointers
 * @return True if successful
 */
bool ShipSimulationClient::addContainersToShip(
    const QString& networkName,
    const QString& shipId,
    const QList<ContainerCore::Container*>& containers)
{
    return executeSerializedCommand([&]() {
        QJsonArray containersArray;
        for (const auto* container : containers) {
            if (container) {
                containersArray.append(container->toJson());
            }
        }
        QJsonObject params;
        params["networkName"] = networkName;
        params["shipID"] = shipId;
        params["containers"] = containersArray;
        bool success = sendCommandAndWait(
            "addContainersToShip",
            params,
            {"containersaddedtoship"});
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
 * @brief Internal method to unload containers
 *
 * Sends an unload command without waiting for a response.
 *
 * @param networkName Network name
 * @param shipId Ship identifier
 * @param terminalNames Terminal names for unloading
 * @return True if command is sent successfully
 */
bool ShipSimulationClient::unloadContainersFromShipAtTerminalsPrivate(
    const QString& networkName,
    const QString& shipId,
    const QStringList& terminalNames)
{
    QJsonObject params;
    params["networkName"] = networkName;
    params["shipID"] = shipId;
    QJsonArray terminalsArray;
    for (const QString& terminal : terminalNames) {
        terminalsArray.append(terminal);
    }
    params["terminalNames"] = terminalsArray;
    bool success = sendCommand("unloadContainersFromShipAtTerminal", params);
    if (m_logger && !success) {
        m_logger->logError("Private unload failed for " + shipId,
                           static_cast<int>(m_clientType));
    }
    return success;
}

/**
 * @brief Unloads containers from a ship at terminals
 *
 * Executes and waits for unloading of containers to terminals.
 *
 * @param networkName Network name
 * @param shipId Ship identifier
 * @param terminalNames Terminal names
 * @return True if successful
 */
bool ShipSimulationClient::unloadContainersFromShipAtTerminals(
    const QString& networkName,
    const QString& shipId,
    const QStringList& terminalNames)
{
    return executeSerializedCommand([&]() {
        QJsonObject params;
        params["networkName"] = networkName;
        params["shipID"] = shipId;
        QJsonArray terminalsArray;
        for (const QString& terminal : terminalNames) {
            terminalsArray.append(terminal);
        }
        params["terminalNames"] = terminalsArray;
        bool success = sendCommandAndWait(
            "unloadContainersFromShipAtTerminal",
            params,
            {"shipunloadedcontainers"});
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
 * @brief Requests terminal nodes for a network
 *
 * Sends a request for terminal nodes in a specified network.
 *
 * @param networkName Network name
 */
void ShipSimulationClient::getNetworkTerminalNodes(
    const QString& networkName)
{
    executeSerializedCommand([&]() {
        QJsonObject params;
        params["network"] = networkName;
        bool success = sendCommand("getNetworkSeaPorts", params);
        if (m_logger) {
            m_logger->log("Requested terminal nodes for " + networkName,
                          static_cast<int>(m_clientType));
        }
        return success;
    });
}

/**
 * @brief Requests shortest path between nodes
 *
 * Sends a request for the shortest path in a network.
 *
 * @param networkName Network name
 * @param startNode Start node ID
 * @param endNode End node ID
 */
void ShipSimulationClient::getShortestPath(
    const QString& networkName,
    const QString& startNode,
    const QString& endNode)
{
    executeSerializedCommand([&]() {
        QJsonObject params;
        params["network"] = networkName;
        params["startNode"] = startNode;
        params["endNode"] = endNode;
        bool success = sendCommand("getShortestPath", params);
        if (m_logger) {
            m_logger->log("Requested shortest path in " + networkName,
                          static_cast<int>(m_clientType));
        }
        return success;
    });
}

/**
 * @brief Retrieves the state of a specific ship
 *
 * Fetches the current state of a ship from stored data.
 *
 * @param networkName Network name
 * @param shipId Ship identifier
 * @return ShipState pointer or nullptr if not found
 */
const ShipState* ShipSimulationClient::getShipState(
    const QString& networkName,
    const QString& shipId) const
{
    QMutexLocker locker(&m_dataAccessMutex);
    if (!m_shipState.contains(networkName)) {
        if (m_logger) {
            m_logger->log("No ship state for network " + networkName,
                          static_cast<int>(m_clientType));
        }
        return nullptr;
    }
    const auto& states = m_shipState[networkName];
    for (const auto* state : states) {
        if (state && state->shipId() == shipId) {
            return state;
        }
    }
    if (m_logger) {
        m_logger->log("Ship " + shipId + " not found in " + networkName,
                      static_cast<int>(m_clientType));
    }
    return nullptr;
}

/**
 * @brief Retrieves states of all ships in a network
 *
 * Fetches all ship states for a specified network.
 *
 * @param networkName Network name
 * @return List of ShipState pointers, empty if none
 */
QList<const ShipState*> ShipSimulationClient::getAllNetworkShipsStates(
    const QString& networkName) const
{
    QMutexLocker locker(&m_dataAccessMutex);
    QList<const ShipState*> statesList;
    if (!m_shipState.contains(networkName)) {
        if (m_logger) {
            m_logger->log("No ship states for " + networkName,
                          static_cast<int>(m_clientType));
        }
        return statesList;
    }
    const auto& states = m_shipState[networkName];
    for (const auto* state : states) {
        if (state) {
            statesList.append(state);
        }
    }
    return statesList;
}

/**
 * @brief Retrieves states of all ships across networks
 *
 * Fetches ship states for all networks in a mapped structure.
 *
 * @return Map of network names to ShipState pointer lists
 */
QMap<QString, QList<const ShipState*>>
ShipSimulationClient::getAllShipsStates() const
{
    QMutexLocker locker(&m_dataAccessMutex);
    QMap<QString, QList<const ShipState*>> allStates;
    for (auto it = m_shipState.constBegin();
         it != m_shipState.constEnd(); ++it) {
        QList<const ShipState*> networkStates;
        for (const auto* state : it.value()) {
            if (state) {
                networkStates.append(state);
            }
        }
        allStates[it.key()] = networkStates;
    }
    return allStates;
}

/**
 * @brief Processes server messages
 *
 * Dispatches incoming messages to appropriate event handlers.
 *
 * @param message JSON message from the server
 */
void ShipSimulationClient::processMessage(const QJsonObject& message)
{
    SimulationClientBase::processMessage(message);
    if (!message.contains("event")) {
        if (m_logger) {
            m_logger->log("Received message without event",
                          static_cast<int>(m_clientType));
        }
        return;
    }
    QString eventType = message.value("event").toString();
    QString normalizedEvent = normalizeEventName(eventType);
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

/**
 * @brief Handles simulation network loaded event
 *
 * Logs the event when a simulation network is loaded.
 *
 * @param message Event data
 */
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

/**
 * @brief Handles simulation created event
 *
 * Initializes network data when a simulation is created.
 *
 * @param message Event data
 */
void ShipSimulationClient::onSimulationCreated(
    const QJsonObject& message)
{
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

/**
 * @brief Handles simulation paused event
 *
 * Logs the event when a simulation is paused.
 *
 * @param message Event data
 */
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

/**
 * @brief Handles simulation resumed event
 *
 * Logs the event when a simulation is resumed.
 *
 * @param message Event data
 */
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

/**
 * @brief Handles simulation restarted event
 *
 * Logs the event when a simulation is restarted.
 *
 * @param message Event data
 */
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

/**
 * @brief Handles simulation ended event
 *
 * Logs the event when a simulation ends.
 *
 * @param message Event data
 */
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

/**
 * @brief Handles simulation advanced event
 *
 * Logs progress when a simulation advances in time.
 *
 * @param message Event data
 */
void ShipSimulationClient::onSimulationAdvanced(
    const QJsonObject& message)
{
    double newTime = message.value("newSimulationTime").toDouble();
    QJsonObject networkProgresses =
        message.value("networkNamesProgress").toObject();
    if (!networkProgresses.isEmpty()) {
        double totalProgress = 0.0;
        QStringList networks;
        for (auto it = networkProgresses.constBegin();
             it != networkProgresses.constEnd(); ++it) {
            networks.append(it.key());
            totalProgress += it.value().toDouble();
        }
        double average = networks.isEmpty() ? 0.0 :
                             totalProgress / networks.size();
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
            qWarning() << "Invalid or missing 'networkNamesProgress'";
        }
    }
}

/**
 * @brief Handles simulation progress update event
 *
 * Updates progress logging when simulation progress changes.
 *
 * @param message Event data
 */
void ShipSimulationClient::onSimulationProgressUpdate(
    const QJsonObject& message)
{
    double progress = message.value("newProgress").toDouble();
    if (m_logger) {
        m_logger->updateProgress(progress,
                                 static_cast<int>(m_clientType));
    }
}

/**
 * @brief Handles ship added to simulator event
 *
 * Logs when a ship is added to the simulator.
 *
 * @param message Event data
 */
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

/**
 * @brief Handles all ships reached destination event
 *
 * Logs when all ships reach their destinations.
 *
 * @param message Event data
 */
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

/**
 * @brief Handles ship reached destination event
 *
 * Updates ship state and unloads containers when a ship arrives.
 *
 * @param message Event data
 */
void ShipSimulationClient::onShipReachedDestination(
    const QJsonObject& message)
{
    QMutexLocker locker(&m_dataAccessMutex);
    QJsonObject shipStatus = message.value("state").toObject();
    QStringList shipIds;
    for (auto it = shipStatus.constBegin();
         it != shipStatus.constEnd(); ++it) {
        QString networkName = it.key();
        if (!m_shipState.contains(networkName)) {
            m_shipState[networkName] = QList<ShipState*>();
        }
        QJsonObject networkStatus = it.value().toObject();
        if (networkStatus.contains("shipStates")) {
            QJsonObject shipData =
                networkStatus.value("shipStates").toObject();
            QString shipId = shipData.value("shipID").toString();
            int containersCount = shipData.value("containersCount").toInt();
            QStringList terminalIds =
                m_shipsDestinationTerminals.value(shipId);
            ShipState* shipState = new ShipState(shipData);
            m_shipState[networkName].append(shipState);
            shipIds.append(shipId);
            locker.unlock();
            bool foundTerminal = false;
            for (const QString& terminalId : terminalIds) {
                foundTerminal = true;
                unloadContainersFromShipAtTerminalsPrivate(
                    networkName, shipId, QStringList{terminalId});
            }
            if (!foundTerminal && m_logger) {
                m_logger->log("No terminal of [" + terminalIds.join(", ") +
                                  "] exists",
                              static_cast<int>(m_clientType));
            } else if (!foundTerminal) {
                qWarning() << "No terminal of [" << terminalIds.join(", ")
                << "] exists!";
            }
            locker.relock();
        }
    }
    if (m_logger) {
        m_logger->log("Ships [" + shipIds.join(", ") +
                          "] reached destinations",
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "Ships [" << shipIds.join(", ") << "] reached destinations";
    }
}

/**
 * @brief Handles ship reached seaport event
 *
 * Unloads containers when a ship reaches a seaport.
 *
 * @param message Event data
 */
void ShipSimulationClient::onShipReachedSeaport(
    const QJsonObject& message)
{
    QString terminalId = message.value("seaPortCode").toString();
    int containersCount = message.value("containersCount").toInt();
    QString networkName = message.value("networkName").toString();
    QString shipId = message.value("shipID").toString();
    bool success = unloadContainersFromShipAtTerminalsPrivate(
        networkName, shipId, QStringList{terminalId});
    if (m_logger) {
        m_logger->log("Ship " + shipId + " reached seaport " + terminalId,
                      static_cast<int>(m_clientType));
    }
}

/**
 * @brief Handles containers unloaded event
 *
 * Logs when containers are unloaded from a ship.
 *
 * @param message Event data
 */
void ShipSimulationClient::onContainersUnloaded(
    const QJsonObject& message)
{
    QJsonArray containers = message.value("containers").toArray();
    QString portName = message.value("portName").toString();
    QJsonObject containersObj;
    containersObj["containers"] = containers;
    QJsonDocument containersDoc(containersObj);
    QString containersJson =
        containersDoc.toJson(QJsonDocument::Compact);
    double currentTime = 0.0;
    if (m_logger) {
        m_logger->log("Containers unloaded at port: " + portName,
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "Containers unloaded at port:" << portName;
    }
}

/**
 * @brief Handles simulation results available event
 *
 * Logs when simulation results are available.
 *
 * @param message Event data
 */
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

/**
 * @brief Handles ship state available event
 *
 * Logs when a ship's state becomes available.
 *
 * @param message Event data
 */
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

/**
 * @brief Handles simulator state available event
 *
 * Logs when the simulator state becomes available.
 *
 * @param message Event data
 */
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

/**
 * @brief Handles error occurred event
 *
 * Logs an error message from the server.
 *
 * @param message Event data
 */
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

/**
 * @brief Handles server reset event
 *
 * Logs when the server is successfully reset.
 */
void ShipSimulationClient::onServerReset()
{
    if (m_logger) {
        m_logger->log("Server reset successfully",
                      static_cast<int>(m_clientType));
    } else {
        qDebug() << "Server Reset Successfully";
    }
}

/**
 * @brief Handles containers added event
 *
 * Logs when containers are added to a ship.
 *
 * @param message Event data
 */
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
