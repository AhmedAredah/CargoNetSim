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

    qDebug() << "ShipSimulatorClient initialized";
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

    qDebug() << "ShipSimulatorClient destroyed";
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

        return success;
    });
}

void ShipSimulationClient::initializeClient()
{
    // Call base class initialization to set up shared resources
    SimulationClientBase::initializeClient();

    // Configure RabbitMQ handler's heartbeat for connection health
    if (m_rabbitMQHandler == nullptr) {
        throw std::runtime_error("RabbitMQ handler not initialized");
    }
    m_rabbitMQHandler->setupHeartbeat(5);

    // Log initialization details for debugging
    qDebug() << "ShipSimulationClient initialized in thread:"
             << QThread::currentThreadId();
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
                qCritical() << "Error converting ships:" << e.what();
                qDeleteAll(shipObjects);
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
            qCritical() << "Exception in defineSimulator:" << e.what();
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
        return sendCommandAndWait(
            "runSimulator",
            params,
            {"allshipsreacheddestination"});
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
        return sendCommandAndWait(
            "endSimulator",
            params,
            {"simulationended"});
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
            qCritical() << "Error converting ships:" << e.what();
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
                qCritical() << "Error parsing container JSON:"
                            << containerStr;
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
        return sendCommandAndWait(
            "addContainersToShip",
            params,
            {"containersaddedtoship"});
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
    return sendCommand("unloadContainersFromShipAtTerminal", params);
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
        return sendCommandAndWait(
            "unloadContainersFromShipAtTerminal",
            params,
            {"shipunloadedcontainers"});
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
        sendCommand("getNetworkSeaPorts", params);
        return true;
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
        sendCommand("getShortestPath", params);
        return true;
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
        return QJsonObject();
    }

    const QList<ShipState*>& states = m_shipState[networkName];

    for (const ShipState* state : states) {
        if (state->shipId() == shipId) {
            return state->toJson();
        }
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
        qWarning() << "Unrecognized event:" << eventType;
    }
}

// Event handling methods

void ShipSimulationClient::onSimulationNetworkLoaded(
    const QJsonObject& message)
{
    qDebug() << "Simulation network loaded.";
}

void ShipSimulationClient::onSimulationCreated(
    const QJsonObject& message)
{
    qDebug() << "Simulation created.";

    QString networkName = message.value("networkName").toString();

    QMutexLocker locker(&m_dataAccessMutex);
    m_networkData[networkName] = QList<SimulationResults*>();
}

void ShipSimulationClient::onSimulationPaused(
    const QJsonObject& message)
{
    qDebug() << "Simulation paused.";
}

void ShipSimulationClient::onSimulationResumed(
    const QJsonObject& message)
{
    qDebug() << "Simulation resumed.";
}

void ShipSimulationClient::onSimulationRestarted(
    const QJsonObject& message)
{
    qDebug() << "Simulation restarted.";
}

void ShipSimulationClient::onSimulationEnded(
    const QJsonObject& message)
{
    qDebug() << "Simulation ended.";
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

        qDebug() << "Simulations advanced for" << networks.join(", ");
    } else {
        qWarning() << "Invalid or missing 'networkNamesProgress' "
                   << "in the message.";
    }
}

void ShipSimulationClient::onSimulationProgressUpdate(
    const QJsonObject& message)
{
    double progress = message.value("newProgress").toDouble();

    // ProgressBarManager::getInstance()->updateProgress(
    //    ClientType::ShipClient, progress);
}

void ShipSimulationClient::onShipAddedToSimulator(
    const QJsonObject& message)
{
    QString shipId = message.value("shipID").toString();
    qDebug() << "Ship" << shipId << "added to simulator.";
}

void ShipSimulationClient::onAllShipsReachedDestination(
    const QJsonObject& message)
{
    QString networkName = message.value("networkName").toString();
    qDebug() << "All ships reached destination of network:"
             << networkName;
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

            if (!foundTerminal) {
                qWarning() << "No terminal of ["
                           << terminalIds.join(", ")
                           << "] exist in the terminal manager!";
            }

            // Reacquire the mutex for continued access to shared data
            locker.relock();
        }
    }

    qDebug() << "Ships [" << shipIds.join(", ")
             << "] reached destinations";
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
    unloadContainersFromShipAtTerminalsPrivate(
        networkName, shipId, QStringList{terminalId});
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

    qDebug() << "Containers unloaded at port:" << portName;
}

void ShipSimulationClient::onSimulationResultsAvailable(
    const QJsonObject& message)
{
    QJsonObject results = message.value("results").toObject();
    qDebug() << "Simulation results available";
}

void ShipSimulationClient::onShipStateAvailable(
    const QJsonObject& message)
{
    QJsonObject shipState = message.value("state").toObject();
    qDebug() << "Ship state available";
}

void ShipSimulationClient::onSimulatorStateAvailable(
    const QJsonObject& message)
{
    QJsonObject simulatorState = message.value("state").toObject();
    qDebug() << "Simulator state available";
}

void ShipSimulationClient::onErrorOccurred(
    const QJsonObject& message)
{
    QString errorMessage = message.value("errorMessage").toString();
    qCritical() << "Error occurred:" << errorMessage;
}

void ShipSimulationClient::onServerReset()
{
    qDebug() << "Server Reset Successfully";
}

void ShipSimulationClient::onContainersAdded(
    const QJsonObject& message)
{
    QString network = message.value("networkName").toString();
    QString shipId = message.value("shipID").toString();

    qDebug() << "Containers added to ship" << shipId
             << "on network" << network;
}

} // namespace ShipClient
} // namespace Backend
} // namespace CargoNetSim
