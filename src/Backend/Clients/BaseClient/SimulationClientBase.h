#pragma once

#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QMap>
#include <QJsonObject>
#include <QStringList>
#include <QEventLoop>
#include <QTimer>
#include <QFuture>
#include <QPromise>
#include <QQueue>
#include <atomic>
#include "Backend/Commons/ClientType.h"
#include "Backend/Clients/BaseClient/RabbitMQHandler.h"
#include "Backend/Commons/LoggerInterface.h"

namespace CargoNetSim {
namespace Backend {

/**
 * @brief Base class for simulation clients
 *
 * Provides a unified interface for sending commands and processing
 * responses. Ensures commands are processed one at a time with proper
 * waiting for server responses.
 */
class SimulationClientBase : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject
     * @param host RabbitMQ host
     * @param port RabbitMQ port
     * @param exchange RabbitMQ exchange
     * @param commandQueue Command queue name
     * @param responseQueue Response queue name
     * @param sendingRoutingKey Routing key for sent messages
     * @param receivingRoutingKeys Routing keys for received messages
     * @param clientType Type of client
     */
    explicit SimulationClientBase(
        QObject* parent = nullptr,
        const QString& host = "localhost",
        int port = 5672,
        const QString& exchange = "simulation_exchange",
        const QString& commandQueue = "command_queue",
        const QString& responseQueue = "response_queue",
        const QString& sendingRoutingKey = "default_key",
        const QStringList& receivingRoutingKeys =
        QStringList{"default_key"},
        ClientType clientType = ClientType::TerminalClient);

    /**
     * @brief Destructor
     */
    virtual ~SimulationClientBase();

    /**
     * @brief Initializes the client object in its target thread context.
     *
     * This function is responsible for performing initialization tasks
     * that require the client object to be in its designated thread,
     * such as setting up signal-slot connections or initializing
     * thread-specific resources. It should be called once, after the
     * object has been moved to its target thread and the thread has started.
     *
     * The function performs the following tasks:
     * - Initializes internal state variables that depend on the thread
     *   context.
     * - Sets up connections to external resources, such as message handlers
     *   or network clients.
     * - Configures any necessary thread-safe callbacks or event handlers.
     *
     * @param logger Optional logger interface for logging messages
     *              (default is nullptr).
     *              If provided, it will be used for logging during
     *              initialization.
     *              If not provided, no logging will occur.
     *              This allows for flexible logging options depending
     *              on the client's needs.
     * @note This function is virtual and can be overridden by derived
     *       classes to add subclass-specific initialization logic.
     *       When overriding, it is recommended to call the base class
     *       implementation first.
     *
     * @warning This function should only be called once, after the object
     *          has been moved to its target thread and the thread has
     *          started. Calling it multiple times or
     *          from the wrong thread may result in undefined behavior.
     *
     * @throws std::runtime_error If initialization fails due to resource
     *          unavailability or configuration errors.
     *
     * @see SimulationClientBase::SimulationClientBase
     * @see QThread::started
     */
    virtual void initializeClient(LoggerInterface* logger = nullptr);

    /**
     * @brief Checks if client is connected to server
     * @return True if connected
     */
    bool isConnected() const;

    /**
     * @brief Connects to the simulation server
     * @return True if connection was successful
     */
    bool connectToServer();

    /**
     * @brief Disconnects from the simulation server
     */
    void disconnectFromServer();

    /**
     * @brief Get client type enumeration
     * @return The client type
     */
    ClientType getClientType() const;

    /**
     * @brief Get client type as string
     * @return String representation of client type
     */
    QString getClientTypeString() const;

signals:
    /**
     * @brief Emitted when an event is received
     * @param event Event name (normalized)
     * @param data Event data as JSON
     */
    void eventReceived(
        const QString& event,
        const QJsonObject& data);

    /**
     * @brief Emitted when a command is sent
     * @param commandId Unique ID of the command
     * @param command Command name
     */
    void commandSent(
        const QString& commandId,
        const QString& command);

    /**
     * @brief Emitted when a command result is received
     * @param commandId Unique ID of the command
     * @param success Whether the command was successful
     * @param result Result data
     */
    void commandResultReceived(
        const QString& commandId,
        bool success,
        const QJsonObject& result);

    /**
     * @brief Emitted when an error occurs
     * @param errorMessage Error message
     */
    void errorOccurred(
        const QString& errorMessage);

    /**
     * @brief Emitted when connection status changes
     * @param connected True if connected, false otherwise
     */
    void connectionStatusChanged(bool connected);

protected:
    /**
     * @brief Send a command and wait for a specific response event
     *
     * This method sends a command and blocks until the expected
     * response event is received or a timeout occurs.
     *
     * @param command Command name
     * @param params Command parameters (optional)
     * @param expectedEvents Events to wait for (at least one needed)
     * @param timeoutMs Timeout in milliseconds (use -1 for no timeout)
     * @param routingKey Custom routing key (optional)
     * @return True if command was processed successfully
     */
    virtual bool sendCommandAndWait(
        const QString& command,
        const QJsonObject& params,
        const QStringList& expectedEvents,
        int timeoutMs = 1800000, // 30 minutes
        const QString& routingKey = QString());

    /**
     * @brief Send a command without waiting for response
     * @param command Command name
     * @param params Command parameters (optional)
     * @param routingKey Custom routing key (optional)
     * @return True if command was sent successfully
     */
    virtual bool sendCommand(
        const QString& command,
        const QJsonObject& params = QJsonObject(),
        const QString& routingKey = QString());

    /**
     * @brief Creates a command object with parameters
     *
     * Override this in derived classes to add client-specific
     * fields to command objects.
     *
     * @param command Command name
     * @param params Command parameters
     * @return Command object as JSON
     */
    virtual QJsonObject createCommandObject(
        const QString& command,
        const QJsonObject& params = QJsonObject());

    /**
     * @brief Wait for a specific event to occur
     *
     * This method blocks until one of the expected events is
     * received or a timeout occurs.
     *
     * @param expectedEvents List of events to wait for
     * @param timeoutMs Timeout in milliseconds (-1 for infinite)
     * @return True if an event was received before timeout
     */
    virtual bool waitForEvent(
        const QStringList& expectedEvents,
        int timeoutMs = -1);

    /**
     * @brief Check if a specific event has been received
     * @param eventName Name of the event to check
     * @return True if the event has been received
     */
    bool hasReceivedEvent(const QString& eventName) const;

    /**
     * @brief Get data for a received event
     * @param eventName Name of the event
     * @return Event data as JSON object
     */
    QJsonObject getEventData(const QString& eventName) const;

    /**
     * @brief Process message from server
     *
     * This method determines if a message is a command response
     * or an event and routes it accordingly. Override in derived
     * classes to add custom processing.
     *
     * @param message Message JSON object
     */
    virtual void processMessage(const QJsonObject& message);

    /**
     * @brief Normalize an event name
     * @param eventName Event name to normalize
     * @return Normalized event name
     */
    static QString normalizeEventName(const QString& eventName);

    /**
     * @brief Register a received event
     * @param eventName Name of the event (normalized)
     * @param eventData Event data
     */
    void registerEvent(
        const QString& eventName,
        const QJsonObject& eventData);

    /**
     * @brief Clear all registered events
     */
    void clearEvents();

    /**
     * @brief Execute a function while ensuring serialized command execution
     *
     * This method ensures that only one command is processed at a time.
     * All command execution should go through this method.
     *
     * @param func Function to execute
     * @return Result of the function
     */
    template<typename Func>
    auto executeSerializedCommand(Func func) -> decltype(func())
    {
        // Check if m_rabbitMQHandler exists and is connected
        if (m_rabbitMQHandler == nullptr || !isConnected()) {
            qWarning() << "Cannot execute command: RabbitMQ handler"
                          " not initialized or not connected";
            throw std::runtime_error("Client not ready for "
                                     "command execution");
        }

        QMutexLocker locker(&m_commandSerializationMutex);
        return func();
    }

    // RabbitMQ handler
    RabbitMQHandler* m_rabbitMQHandler;

    // Client type identification
    ClientType m_clientType;

    // Event registry for synchronization
    QMap<QString, QJsonObject> m_receivedEvents;
    mutable QMutex m_eventMutex;
    QWaitCondition m_eventCondition;

    // Connection parameters
    QString m_host;
    int m_port;
    QString m_exchange;
    QString m_commandQueue;
    QString m_responseQueue;
    QString m_sendingRoutingKey;
    QStringList m_receivingRoutingKeys;

    // Logging interface
    LoggerInterface* m_logger;

    // Command timeout constant
    static const int COMMAND_TIMEOUT_MS = 1800000; // 30 minutes

private slots:
    /**
     * @brief Handle messages from RabbitMQ
     * @param message Message JSON object
     */
    void handleMessage(const QJsonObject& message);

private:
    // Command serialization
    QMutex m_commandSerializationMutex;

    // Currently processing flag for preventing concurrent operations
    std::atomic<bool> m_processingCommand;
};

} // namespace Backend
} // namespace CargoNetSim

Q_DECLARE_METATYPE(CargoNetSim::Backend::SimulationClientBase)
Q_DECLARE_METATYPE(CargoNetSim::Backend::SimulationClientBase*)
