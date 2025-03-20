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
    // In real code, you'd use your actual implementation
    // ProgressBarManager::getInstance()->addProgressBar(
    //     ClientType::ShipClient, "Ship Simulation", 100);
    
    // Start heartbeat mechanism
    m_rabbitMQHandler->setupHeartbeat(5);
    
    qDebug() << "ShipSimulatorClient initialized";
}

ShipSimulationClient::~ShipSimulationClient()
{
    // Clean up resources
    qDeleteAll(m_loadedShips);
    m_loadedShips.clear();
    
    qDebug() << "ShipSimulatorClient destroyed";
}

void ShipSimulationClient::resetServer()
{
    // Use parent class implementation
    SimulationClientBase::resetServer();
}

bool ShipSimulationClient::waitForEvent(
    const QStringList& expectedEvents,
    int timeout)
{
    QMutexLocker locker(&m_eventQueueMutex);
    
    // Normalize expected event names
    QStringList normalizedEvents;
    for (const QString& event : expectedEvents) {
        normalizedEvents.append(
            event.trimmed().toLower().remove(' '));
    }
    
    // Check if event already in queue
    for (const QString& event : m_eventQueue) {
        if (normalizedEvents.contains(event)) {
            m_eventQueue.removeOne(event);
            return true;
        }
    }
    
    // Wait for event with timeout
    QDateTime deadline = QDateTime::currentDateTime();
    if (timeout > 0) {
        deadline = deadline.addMSecs(timeout);
    }
    
    bool eventReceived = false;
    
    while (!eventReceived) {
        // Check if timeout has been reached
        if (timeout > 0 && QDateTime::currentDateTime() > deadline) {
            qWarning() << "Timeout waiting for events:" 
                       << expectedEvents.join(", ");
            return false;
        }
        
        // Wait for new event
        if (timeout < 0) {
            // Wait indefinitely
            m_eventWaitCondition.wait(&m_eventQueueMutex);
        } else {
            // Wait with timeout
            int remainingMs = QDateTime::currentDateTime()
                             .msecsTo(deadline);
            if (remainingMs <= 0 || 
                !m_eventWaitCondition.wait(
                    &m_eventQueueMutex, remainingMs)) {
                return false;
            }
        }
        
        // Check if any expected event has arrived
        for (const QString& event : normalizedEvents) {
            int index = m_eventQueue.indexOf(event);
            if (index >= 0) {
                m_eventQueue.removeAt(index);
                eventReceived = true;
                break;
            }
        }
    }
    
    return true;
}

bool ShipSimulationClient::defineSimulator(
    const QString& networkName,
    double timeStep,
    const QList<QJsonObject>& ships,
    const QMap<QString, QStringList>& destinationTerminalIds,
    const QString& networkPath)
{
    try {
        // Create a new thread for waiting for the event
        QThread* waitThread = QThread::create([this]() {
            this->waitForEvent({"simulationcreated"});
        });
        
        waitThread->start();
        
        // Prepare ship data
        QJsonArray shipsArray;
        QList<Ship*> shipObjects;
        
        // Convert ship data to JSON
        try {
            for (const QJsonObject& shipData : ships) {
                // In real implementation, create Ship objects
                Ship* ship = new Ship(shipData);
                shipObjects.append(ship);
                
                // Use ship's toJson method to get JSON representation
                shipsArray.append(ship->toJson());
            }
        } catch (const std::exception& e) {
            // ApplicationLogger::logError(
            //    QString("Error converting ships: %1")
            //    .arg(e.what()),
            //    ClientType::ShipClient);
            qCritical() << "Error converting ships:" << e.what();
            
            // Clean up
            qDeleteAll(shipObjects);
            waitThread->terminate();
            waitThread->wait();
            delete waitThread;
            
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
        
        // Send command
        try {
            sendCommand("defineSimulator", params);
            
            // Store ship objects
            for (Ship* ship : shipObjects) {
                m_loadedShips[ship->getUserId()] = ship;
                
                QStringList terminals = 
                    destinationTerminalIds.value(ship->getUserId());
                m_shipsDestinationTerminals[ship->getUserId()] = terminals;
            }
        } catch (const std::exception& e) {
            // ApplicationLogger::logError(
            //    QString("Error sending command: %1").arg(e.what()),
            //    ClientType::ShipClient);
            qCritical() << "Error sending command:" << e.what();
            
            // Clean up
            qDeleteAll(shipObjects);
            waitThread->terminate();
            waitThread->wait();
            delete waitThread;
            
            return false;
        }
        
        // Wait for thread to complete
        waitThread->wait();
        delete waitThread;
        
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception in defineSimulator:" << e.what();
        return false;
    }
}

bool ShipSimulationClient::runSimulator(
    const QStringList& networkNames,
    double byTimeSteps)
{
    QStringList networks = networkNames;
    
    // If "*" specified, use all networks
    if (networks.contains("*")) {
        networks = m_networkData.keys();
    }
    
    // Create a thread for waiting for the event
    QThread* waitThread = QThread::create([this]() {
        this->waitForEvent({"allshipsreacheddestination"});
    });
    
    waitThread->start();
    
    // Create command parameters
    QJsonObject params;
    
    QJsonArray networksArray;
    for (const QString& network : networks) {
        networksArray.append(network);
    }
    params["networkNames"] = networksArray;
    params["byTimeSteps"] = byTimeSteps;
    
    // Send command
    sendCommand("runSimulator", params);
    
    // Wait for thread to complete
    waitThread->wait();
    delete waitThread;
    
    return true;
}

bool ShipSimulationClient::endSimulator(
    const QStringList& networkNames)
{
    QStringList networks = networkNames;
    
    // If "*" specified, use all networks
    if (networks.contains("*")) {
        networks = m_networkData.keys();
    }
    
    // Create a thread for waiting for the event
    QThread* waitThread = QThread::create([this]() {
        this->waitForEvent({"simulationended"});
    });
    
    waitThread->start();
    
    // Create command parameters
    QJsonObject params;
    
    QJsonArray networksArray;
    for (const QString& network : networks) {
        networksArray.append(network);
    }
    params["network"] = networksArray;
    
    // Send command
    sendCommand("endSimulator", params);
    
    // Wait for thread to complete
    waitThread->wait();
    delete waitThread;
    
    return true;
}

bool ShipSimulationClient::addShipsToSimulator(
    const QString& networkName,
    const QList<QJsonObject>& ships,
    const QMap<QString, QStringList>& destinationTerminalIds)
{
    // Create a thread for waiting for the event
    QThread* waitThread = QThread::create([this]() {
        this->waitForEvent({"shipaddedtosimulator"});
    });
    
    waitThread->start();
    
    // Prepare ship data
    QJsonArray shipsArray;
    QList<Ship*> shipObjects;
    
    // Convert ship data to JSON
    for (const QJsonObject& shipData : ships) {
        // In real implementation, create Ship objects
        Ship* ship = new Ship(shipData);
        shipObjects.append(ship);
        
        // Use ship's toJson method to get JSON representation
        shipsArray.append(ship->toJson());
    }
    
    // Create command parameters
    QJsonObject params;
    params["networkName"] = networkName;
    params["ships"] = shipsArray;
    
    // Send command
    sendCommand("addShipsToSimulator", params);
    
    // Store ship objects
    for (Ship* ship : shipObjects) {
        m_loadedShips[ship->getUserId()] = ship;
        
        QStringList terminals = 
            destinationTerminalIds.value(ship->getUserId());
        m_shipsDestinationTerminals[ship->getUserId()] = terminals;
    }
    
    // Wait for thread to complete
    waitThread->wait();
    delete waitThread;
    
    return true;
}

bool ShipSimulationClient::addContainersToShip(
    const QString& networkName,
    const QString& shipId,
    const QStringList& containers)
{
    // Create a thread for waiting for the event
    QThread* waitThread = QThread::create([this]() {
        this->waitForEvent({"containersaddedtoship"});
    });
    
    waitThread->start();
    
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
            // ApplicationLogger::logError(
            //    QString("Error parsing container JSON: %1")
            //        .arg(containerStr),
            //    ClientType::ShipClient);
            qCritical() << "Error parsing container JSON:" 
                        << containerStr;
            
            // Clean up
            waitThread->terminate();
            waitThread->wait();
            delete waitThread;
            
            return false;
        }
        
        containersArray.append(doc.object());
    }
    
    // Create command parameters
    QJsonObject params;
    params["networkName"] = networkName;
    params["shipID"] = shipId;
    params["containers"] = containersArray;
    
    // Send command
    sendCommand("addContainersToShip", params);
    
    // Wait for thread to complete
    waitThread->wait();
    delete waitThread;
    
    return true;
}

void ShipSimulationClient::unloadContainersFromShipAtTerminalsPrivate(
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
    sendCommand("unloadContainersFromShipAtTerminal", params);
}

bool ShipSimulationClient::unloadContainersFromShipAtTerminals(
    const QString& networkName,
    const QString& shipId,
    const QStringList& terminalNames)
{
    // Create a thread for waiting for the event
    QThread* waitThread = QThread::create([this]() {
        this->waitForEvent({"shipunloadedcontainers"});
    });
    
    waitThread->start();
    
    // Call private implementation
    unloadContainersFromShipAtTerminalsPrivate(
        networkName, shipId, terminalNames);
    
    // Wait for thread to complete
    waitThread->wait();
    delete waitThread;
    
    return true;
}

void ShipSimulationClient::getNetworkTerminalNodes(
    const QString& networkName)
{
    // Create command parameters
    QJsonObject params;
    params["network"] = networkName;
    
    // Send command
    sendCommand("getNetworkSeaPorts", params);
}

void ShipSimulationClient::getShortestPath(
    const QString& networkName,
    const QString& startNode,
    const QString& endNode)
{
    // Create command parameters
    QJsonObject params;
    params["network"] = networkName;
    params["startNode"] = startNode;
    params["endNode"] = endNode;
    
    // Send command
    sendCommand("getShortestPath", params);
}

QJsonObject ShipSimulationClient::getShipState(
    const QString& networkName,
    const QString& shipId) const
{
    if (!m_shipState.contains(networkName)) {
        return QJsonObject();
    }
    
    const QList<ShipState>& states = m_shipState[networkName];
    
    for (const ShipState& state : states) {
        if (state.shipId() == shipId) {
            return state.toJson();
        }
    }
    
    return QJsonObject();
}

QJsonArray ShipSimulationClient::getAllNetworkShipsStates(
    const QString& networkName) const
{
    QJsonArray states;
    
    if (!m_shipState.contains(networkName)) {
        return states;
    }
    
    const QList<ShipState>& networkStates = m_shipState[networkName];
    
    for (const ShipState& state : networkStates) {
        states.append(state.toJson());
    }
    
    return states;
}

QJsonObject ShipSimulationClient::getAllShipsStates() const
{
    QJsonObject allStates;
    
    for (auto it = m_shipState.constBegin(); 
         it != m_shipState.constEnd(); 
         ++it) {
        QJsonArray networkStates;
        
        for (const ShipState& state : it.value()) {
            networkStates.append(state.toJson());
        }
        
        allStates[it.key()] = networkStates;
    }
    
    return allStates;
}

void ShipSimulationClient::handleMessage(const QJsonObject& message)
{
    QString eventType = message.value("event").toString();
    QString normalizedEvent = 
        eventType.trimmed().toLower().remove(' ');
    
    // Add to event queue and signal waiting threads
    {
        QMutexLocker locker(&m_eventQueueMutex);
        m_eventQueue.append(normalizedEvent);
        m_eventWaitCondition.wakeAll();
    }
    
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
        // ApplicationLogger::logError(
        //    QString("Unrecognized event: %1").arg(eventType),
        //    ClientType::ShipClient);
        qWarning() << "Unrecognized event:" << eventType;
    }
    
    // Signal the event to listeners
    emit eventReceived(normalizedEvent, message);
}

void ShipSimulationClient::onSimulationNetworkLoaded(
    const QJsonObject& message)
{
    // ApplicationLogger::log(
    //    "Simulation network loaded.",
    //    ClientType::ShipClient);
    qDebug() << "Simulation network loaded.";
}

void ShipSimulationClient::onSimulationCreated(
    const QJsonObject& message)
{
    // ApplicationLogger::log(
    //    "Simulation created.",
    //    ClientType::ShipClient);
    qDebug() << "Simulation created.";
    
    QString networkName = message.value("networkName").toString();
    m_networkData[networkName] = QList<SimulationResults>();
}

void ShipSimulationClient::onSimulationPaused(
    const QJsonObject& message)
{
    // ApplicationLogger::log(
    //    "Simulation paused.",
    //    ClientType::ShipClient);
    qDebug() << "Simulation paused.";
}

void ShipSimulationClient::onSimulationResumed(
    const QJsonObject& message)
{
    // ApplicationLogger::log(
    //    "Simulation resumed.",
    //    ClientType::ShipClient);
    qDebug() << "Simulation resumed.";
}

void ShipSimulationClient::onSimulationRestarted(
    const QJsonObject& message)
{
    // ApplicationLogger::log(
    //    "Simulation restarted.",
    //    ClientType::ShipClient);
    qDebug() << "Simulation restarted.";
}

void ShipSimulationClient::onSimulationEnded(
    const QJsonObject& message)
{
    // ApplicationLogger::log(
    //    "Simulation ended.",
    //    ClientType::ShipClient);
    qDebug() << "Simulation ended.";
    
    // Call parent implementation
    SimulationClientBase::onSimulationEnded(message);
}

void ShipSimulationClient::onSimulationAdvanced(
    const QJsonObject& message)
{
    double newTime = message.value("newSimulationTime").toDouble();
    
    // ApplicationLogger::log(
    //    QString("Simulation advanced to time: %1").arg(newTime),
    //    ClientType::ShipClient);
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
        
        // ApplicationLogger::log(
        //    QString("Simulations advanced for %1")
        //        .arg(networks.join(", ")),
        //    ClientType::ShipClient);
        qDebug() << "Simulations advanced for" << networks.join(", ");
    } else {
        // ApplicationLogger::log(
        //    "Invalid or missing 'networkNamesProgress' in message.");
        qWarning() << "Invalid or missing 'networkNamesProgress' "
                   << "in the message.";
    }
    
    // Call parent implementation
    SimulationClientBase::onSimulationAdvanced(message);
}

void ShipSimulationClient::onSimulationProgressUpdate(
    const QJsonObject& message)
{
    double progress = message.value("newProgress").toDouble();
    
    // ProgressBarManager::getInstance()->updateProgress(
    //    ClientType::ShipClient, progress);
    
    // Call parent implementation
    SimulationClientBase::onSimulationProgressUpdate(message);
}

void ShipSimulationClient::onShipAddedToSimulator(
    const QJsonObject& message)
{
    QString shipId = message.value("shipID").toString();
    
    // ApplicationLogger::log(
    //    QString("Ship %1 added to simulator.").arg(shipId),
    //    ClientType::ShipClient);
    qDebug() << "Ship" << shipId << "added to simulator.";
}

void ShipSimulationClient::onAllShipsReachedDestination(
    const QJsonObject& message)
{
    QString networkName = message.value("networkName").toString();
    
    // ApplicationLogger::log(
    //    QString("All ships reached destination of network: %1")
    //        .arg(networkName),
    //    ClientType::ShipClient);
    qDebug() << "All ships reached destination of network:" 
             << networkName;
}

void ShipSimulationClient::onShipReachedDestination(
    const QJsonObject& message)
{
    QJsonObject shipStatus = message.value("state").toObject();
    QStringList shipIds;
    
    for (auto it = shipStatus.constBegin(); 
         it != shipStatus.constEnd(); 
         ++it) {
        QString networkName = it.key();
        
        // Ensure the key exists in m_shipState
        if (!m_shipState.contains(networkName)) {
            m_shipState[networkName] = QList<ShipState>();
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
            
            ShipState shipState(shipData);
            m_shipState[networkName].append(shipState);
            shipIds.append(shipId);
            
            // Put containers in terminal
            // TerminalGraphServer* graphServer = 
            //    TerminalGraphServer::getInstance();
            
            bool foundTerminal = false;
            for (const QString& terminalId : terminalIds) {
                // Check if terminal exists and has capacity
                // In real implementation, use your terminal graph server
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
                // ApplicationLogger::log(
                //    QString("No terminal of [%1] exist in the "
                //            "terminal manager!")
                //        .arg(terminalIds.join(", ")),
                //    ClientType::ShipClient);
                qWarning() << "No terminal of [" 
                           << terminalIds.join(", ")
                           << "] exist in the terminal manager!";
            }
        }
    }
    
    // ApplicationLogger::log(
    //    QString("Ships [ %1 ] reached destinations")
    //        .arg(shipIds.join(", ")),
    //    ClientType::ShipClient);
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
    
    // TerminalGraphServer* graphServer = 
    //    TerminalGraphServer::getInstance();
    
    // In real implementation, check terminal capacity
    // if (graphServer->terminalExists(terminalId)) {
    //    bool canAdd = graphServer->terminal(terminalId)
    //        ->checkCapacityStatus(containersCount);
    //    if (canAdd) {
    //        unloadContainersFromShipAtTerminalsPrivate(
    //            networkName, shipId, QStringList{terminalId});
    //    }
    // } else {
    //    ApplicationLogger::log(
    //        QString("Terminal %1 does not exist in the "
    //                "terminal manager!")
    //            .arg(terminalId),
    //        ClientType::ShipClient);
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
    
    // Get current simulation time
    // In real implementation, use your time server
    // double currentTime = 
    //    SimulatorTimeServer::getInstance()->getCurrentSimulationTime();
    double currentTime = 0.0;
    
    // Add containers to terminal
    // In real implementation, use your terminal graph server
    // if (TerminalGraphServer::getInstance()->terminalExists(portName)) {
    //    try {
    //        TerminalGraphServer::getInstance()->terminal(portName)
    //            ->addContainersFromJson(
    //                containersJson, currentTime);
    //    } catch (const std::runtime_error& e) {
    //        qWarning() << "Error unloading containers:" << e.what();
    //    }
    // } else {
    //    ApplicationLogger::log(
    //        QString("Terminal %1 does not exist in the "
    //                "terminal manager!")
    //            .arg(portName),
    //        ClientType::ShipClient);
    // }
    
    // Placeholder implementation
    qDebug() << "Containers unloaded at port:" << portName;
}

void ShipSimulationClient::onSimulationResultsAvailable(
    const QJsonObject& message)
{
    QJsonObject results = message.value("results").toObject();
    
    // ApplicationLogger::log(
    //    QString("Simulation results available"),
    //    ClientType::ShipClient);
    qDebug() << "Simulation results available";
}

void ShipSimulationClient::onShipStateAvailable(
    const QJsonObject& message)
{
    QJsonObject shipState = message.value("state").toObject();
    
    // ApplicationLogger::log(
    //    "Ship state available",
    //    ClientType::ShipClient);
    qDebug() << "Ship state available";
}

void ShipSimulationClient::onSimulatorStateAvailable(
    const QJsonObject& message)
{
    QJsonObject simulatorState = message.value("state").toObject();
    
    // ApplicationLogger::log(
    //    "Simulator state available",
    //    ClientType::ShipClient);
    qDebug() << "Simulator state available";
}

void ShipSimulationClient::onErrorOccurred(
    const QJsonObject& message)
{
    QString errorMessage = message.value("errorMessage").toString();
    
    // ApplicationLogger::logError(
    //    QString("Error occurred: %1").arg(errorMessage),
    //    ClientType::ShipClient);
    qCritical() << "Error occurred:" << errorMessage;
    
    // Call parent implementation
    SimulationClientBase::onErrorOccurred(message);
}

void ShipSimulationClient::onServerReset()
{
    // ApplicationLogger::log(
    //    "Server Reset Successfully",
    //    ClientType::ShipClient);
    qDebug() << "Server Reset Successfully";
    
    // Call parent implementation
    SimulationClientBase::onServerReset();
}

void ShipSimulationClient::onContainersAdded(
    const QJsonObject& message)
{
    QString network = message.value("networkName").toString();
    QString shipId = message.value("shipID").toString();
    
    // ApplicationLogger::log(
    //    QString("Containers added to ship %1 on network %2.")
    //        .arg(shipId, network),
    //    ClientType::ShipClient);
    qDebug() << "Containers added to ship" << shipId 
             << "on network" << network;
}

} // namespace Backend
} // namespace CargoNetSim
