#include "SimulationClientBase.h"
#include <QDebug>
#include <QJsonDocument>
#include <QThread>

namespace CargoNetSim {
namespace Backend {

SimulationClientBase::SimulationClientBase(
    QObject* parent,
    const QString& host,
    int port,
    const QString& exchange,
    const QString& commandQueue,
    const QString& responseQueue,
    const QString& sendingRoutingKey,
    const QStringList& receivingRoutingKeys,
    ClientType clientType)
    : QObject(parent),
      m_clientType(clientType),
      m_host(host),
      m_port(port),
      m_exchange(exchange),
      m_commandQueue(commandQueue),
      m_responseQueue(responseQueue),
      m_sendingRoutingKey(sendingRoutingKey.isEmpty() 
                         ? "default_key" 
                         : sendingRoutingKey),
      m_receivingRoutingKeys(receivingRoutingKeys.isEmpty() 
                            ? QStringList{"default_key"} 
                            : receivingRoutingKeys)
{
    // Create RabbitMQ handler
    m_rabbitMQHandler = new RabbitMQHandler(
        this, host, port, exchange, commandQueue,
        responseQueue, m_sendingRoutingKey,
        m_receivingRoutingKeys);
    
    // Connect signals and slots
    QObject::connect(m_rabbitMQHandler, &RabbitMQHandler::messageReceived,
            this, &SimulationClientBase::handleMessage);
    QObject::connect(m_rabbitMQHandler, &RabbitMQHandler::connectionChanged,
            this, &SimulationClientBase::connectionStatusChanged);
    QObject::connect(m_rabbitMQHandler, &RabbitMQHandler::errorOccurred,
            this, &SimulationClientBase::errorOccurred);
    
    // Connect to RabbitMQ
    m_rabbitMQHandler->establishConnection();
    
    // Setup heartbeat
    m_rabbitMQHandler->setupHeartbeat();
}

SimulationClientBase::~SimulationClientBase()
{
    // Cleanup will be handled by parent-child relationships
}

bool SimulationClientBase::sendCommand(
    const QString& command,
    const QJsonObject& params,
    const QString& routingKey)
{
    QJsonObject message;
    message["command"] = command;
    if (!params.isEmpty()) {
        message["params"] = params;
    }
    message["clientType"] = static_cast<int>(m_clientType);
    message["timestamp"] = QDateTime::currentDateTime()
                          .toString(Qt::ISODate);
    
    return m_rabbitMQHandler->sendCommand(
        message, routingKey.isEmpty() 
                ? m_sendingRoutingKey : routingKey);
}

bool SimulationClientBase::waitForEvent(
    const QStringList& expectedEvents,
    int timeoutMs)
{
    QMutexLocker locker(&m_eventMutex);
    
    // Check if event already received
    for (const QString& event : expectedEvents) {
        QString normalizedEvent = event.toLower()
                                 .simplified().remove(' ');
        if (m_receivedEvents.contains(normalizedEvent)) {
            m_receivedEvents.remove(normalizedEvent);
            return true;
        }
    }
    
    // Wait for event
    bool eventReceived = false;
    if (timeoutMs < 0) {
        // Wait indefinitely
        while (!eventReceived) {
            m_eventCondition.wait(&m_eventMutex);
            for (const QString& event : expectedEvents) {
                QString normalizedEvent = event.toLower()
                                         .simplified()
                                         .remove(' ');
                if (m_receivedEvents.contains(normalizedEvent)) {
                    m_receivedEvents.remove(normalizedEvent);
                    eventReceived = true;
                    break;
                }
            }
        }
    } else {
        // Wait with timeout
        QTime deadline = QTime::currentTime()
                        .addMSecs(timeoutMs);
        while (!eventReceived) {
            if (!m_eventCondition.wait(
                    &m_eventMutex, 
                    QTime::currentTime().msecsTo(deadline))) {
                // Timeout
                return false;
            }
            
            for (const QString& event : expectedEvents) {
                QString normalizedEvent = event.toLower()
                                         .simplified()
                                         .remove(' ');
                if (m_receivedEvents.contains(normalizedEvent)) {
                    m_receivedEvents.remove(normalizedEvent);
                    eventReceived = true;
                    break;
                }
            }
        }
    }
    
    return eventReceived;
}

void SimulationClientBase::resetServer()
{
    // Start waiting for the event
    // Send command in a separate thread to avoid blocking
    QThread* commandThread = QThread::create([this]() {
        this->sendCommand("resetServer");
    });
    commandThread->start();
    
    // Wait for server reset event
    waitForEvent({"serverReset"});
    
    // Wait for thread to finish and clean up
    commandThread->wait();
    delete commandThread;
}

void SimulationClientBase::handleMessage(
    const QJsonObject& message)
{
    QString event = message.value("event").toString();
    if (event.isEmpty()) {
        return;
    }
    
    QString normalizedEvent = event.toLower()
                             .simplified().remove(' ');
    
    // Store event and signal waiting threads
    QMutexLocker locker(&m_eventMutex);
    m_receivedEvents[normalizedEvent] = message;
    m_eventCondition.wakeAll();
    
    // Emit signal for listeners
    emit eventReceived(normalizedEvent, message);
    
    // Handle specific events
    if (normalizedEvent == "serverreset") {
        onServerReset();
    } else if (normalizedEvent == "simulationcreated") {
        onSimulationCreated(message);
    } else if (normalizedEvent == "simulationended") {
        onSimulationEnded(message);
    } else if (normalizedEvent == "simulationadvanced") {
        onSimulationAdvanced(message);
    } else if (normalizedEvent == "simulationprogressupdate") {
        onSimulationProgressUpdate(message);
    } else if (normalizedEvent == "erroroccurred") {
        onErrorOccurred(message);
    }
}

} // namespace Backend
} // namespace CargoNetSim
