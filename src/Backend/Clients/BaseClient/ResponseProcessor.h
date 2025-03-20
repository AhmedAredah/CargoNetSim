#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QMutex>
#include <QWaitCondition>
#include <QStringList>
#include "Backend/Commons/ClientType.h"

namespace CargoNetSim {
namespace Backend {


/**
 * @brief Base class for processing responses from the simulation server
 * 
 * This class provides the basic infrastructure for handling responses.
 * Client-specific processors should inherit from this class and
 * override the processMessage method.
 */
class ResponseProcessor : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param clientType Type of client this processor belongs to
     * @param parent Parent QObject
     */
    explicit ResponseProcessor(
        ClientType clientType,
        QObject* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    virtual ~ResponseProcessor();
    
    /**
     * @brief Waits for specified events to be received
     * @param expectedEvents List of event names to wait for
     * @param timeoutMs Timeout in milliseconds (-1 for indefinite)
     * @return True if any of the expected events was received
     */
    bool waitForEvent(
        const QStringList& expectedEvents,
        int timeoutMs = -1);
    
    /**
     * @brief Checks if an event has been received
     * @param eventName Name of the event to check
     * @return True if the event has been received
     */
    bool hasReceivedEvent(const QString& eventName) const;
    
    /**
     * @brief Gets data for a received event
     * @param eventName Name of the event
     * @return JSON object containing event data or empty object
     */
    QJsonObject getEventData(const QString& eventName) const;
    
    /**
     * @brief Clears all received events
     */
    void clearEvents();
    
    /**
     * @brief Normalizes an event name (lowercase, no spaces)
     * @param eventName Raw event name
     * @return Normalized event name
     */
    static QString normalizeEventName(const QString& eventName);

public slots:
    /**
     * @brief Processes a received message from RabbitMQ
     * Derived classes must override this method to handle
     * client-specific events
     * @param message JSON message object
     */
    virtual void processMessage(const QJsonObject& message);

signals:
    /**
     * @brief Emitted when any event is received
     * @param eventName Name of the event
     * @param eventData JSON data associated with the event
     */
    void eventReceived(
        const QString& eventName,
        const QJsonObject& eventData);
    
    /**
     * @brief Emitted when an error occurs
     * @param errorMessage Error message
     */
    void errorOccurred(const QString& errorMessage);

protected:
    /**
     * @brief Register an event with the event system
     * @param eventName Name of the event
     * @param eventData Event data
     */
    void registerEvent(
        const QString& eventName,
        const QJsonObject& eventData);
    
    /**
     * @brief Get client type as string
     * @return String representation of client type
     */
    QString getClientTypeString() const;

    // Client type
    ClientType m_clientType;
    
    // Event registry for synchronization
    QMap<QString, QJsonObject> m_receivedEvents;
    mutable QMutex m_eventMutex;
    QWaitCondition m_eventCondition;
};

} // namespace Backend
} // namespace CargoNetSim

Q_DECLARE_METATYPE(CargoNetSim::Backend::ResponseProcessor)
Q_DECLARE_METATYPE(CargoNetSim::Backend::ResponseProcessor*)
