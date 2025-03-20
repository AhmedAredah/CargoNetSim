#include "ResponseProcessor.h"
#include <QDebug>
#include <QJsonValue>
#include <QDateTime>
#include <QThread>

namespace CargoNetSim {
namespace Backend {

/**
 * Constructor
 */
ResponseProcessor::ResponseProcessor(
    ClientType clientType,
    QObject* parent)
    : QObject(parent),
      m_clientType(clientType)
{
    qDebug() << "Response processor initialized for" 
             << getClientTypeString();
}

/**
 * Destructor
 */
ResponseProcessor::~ResponseProcessor()
{
    qDebug() << "Response processor destroyed for" 
             << getClientTypeString();
}

/**
 * Waits for any of the expected events to be received.
 * If timeout is negative, waits indefinitely.
 */
bool ResponseProcessor::waitForEvent(
    const QStringList& expectedEvents,
    int timeoutMs)
{
    QMutexLocker locker(&m_eventMutex);
    
    // Normalize expected event names
    QStringList normalizedEvents;
    for (const QString& event : expectedEvents) {
        normalizedEvents.append(normalizeEventName(event));
    }
    
    // Check if any expected event is already received
    for (const QString& eventName : normalizedEvents) {
        if (m_receivedEvents.contains(eventName)) {
            // Found an expected event, remove it from registry
            m_receivedEvents.remove(eventName);
            return true;
        }
    }
    
    // Wait for event with timeout
    if (timeoutMs < 0) {
        // Wait indefinitely
        while (true) {
            m_eventCondition.wait(&m_eventMutex);
            
            // Check if any expected event arrived
            for (const QString& eventName : normalizedEvents) {
                if (m_receivedEvents.contains(eventName)) {
                    m_receivedEvents.remove(eventName);
                    return true;
                }
            }
        }
    } else {
        // Wait with timeout
        QDateTime deadline = QDateTime::currentDateTime()
                            .addMSecs(timeoutMs);
        
        while (QDateTime::currentDateTime() < deadline) {
            int remainingMs = QDateTime::currentDateTime()
                             .msecsTo(deadline);
            
            if (remainingMs <= 0 || 
                !m_eventCondition.wait(&m_eventMutex, remainingMs)) {
                // Timeout occurred
                return false;
            }
            
            // Check if any expected event arrived
            for (const QString& eventName : normalizedEvents) {
                if (m_receivedEvents.contains(eventName)) {
                    m_receivedEvents.remove(eventName);
                    return true;
                }
            }
        }
        
        return false; // Timeout
    }
}

/**
 * Checks if a specific event has been received
 */
bool ResponseProcessor::hasReceivedEvent(
    const QString& eventName) const
{
    QMutexLocker locker(&m_eventMutex);
    return m_receivedEvents.contains(normalizeEventName(eventName));
}

/**
 * Gets the data associated with a received event
 */
QJsonObject ResponseProcessor::getEventData(
    const QString& eventName) const
{
    QMutexLocker locker(&m_eventMutex);
    QString normalized = normalizeEventName(eventName);
    
    if (m_receivedEvents.contains(normalized)) {
        return m_receivedEvents.value(normalized);
    }
    
    return QJsonObject(); // Empty object if event not found
}

/**
 * Clears all received events from the registry
 */
void ResponseProcessor::clearEvents()
{
    QMutexLocker locker(&m_eventMutex);
    m_receivedEvents.clear();
}

/**
 * Normalizes an event name for consistent lookup
 */
QString ResponseProcessor::normalizeEventName(const QString& eventName)
{
    return eventName.toLower().simplified().remove(' ');
}

/**
 * Processes a message received from RabbitMQ
 * Base implementation just extracts the event name and registers it
 * Derived classes must override this to handle client-specific events
 */
void ResponseProcessor::processMessage(const QJsonObject& message)
{
    // Extract event name
    QString eventName = message.value("event").toString();
    if (eventName.isEmpty()) {
        qWarning() << "Received message without event name:" 
                   << message;
        return;
    }
    
    QString normalizedEvent = normalizeEventName(eventName);
    
    // Register event
    registerEvent(normalizedEvent, message);
    
    // Base implementation just emits the generic event signal
    // Derived classes should override this method to handle
    // client-specific events
    emit eventReceived(normalizedEvent, message);
}

/**
 * Registers an event with the event system
 */
void ResponseProcessor::registerEvent(
    const QString& eventName,
    const QJsonObject& eventData)
{
    QMutexLocker locker(&m_eventMutex);
    m_receivedEvents[eventName] = eventData;
    m_eventCondition.wakeAll();
    
    qDebug() << "Registered event:" << eventName;
}

/**
 * Helper method to get client type as string for logging
 */
QString ResponseProcessor::getClientTypeString() const
{
    switch (m_clientType) {
        case ClientType::ShipClient:
            return "ShipClient";
        case ClientType::TrainClient:
            return "TrainClient";
        case ClientType::TruckClient:
            return "TruckClient";
        default:
            return "BaseClient";
    }
}

} // namespace Backend
} // namespace CargoNetSim
