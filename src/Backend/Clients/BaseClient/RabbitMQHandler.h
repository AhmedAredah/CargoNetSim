#pragma once

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QThread>
#include <atomic>
#include <rabbitmq-c/amqp.h>

namespace CargoNetSim {
namespace Backend {

/**
 * @brief Handles RabbitMQ communication for the simulation
 *
 * Manages connections to RabbitMQ, message sending and
 * receiving, and connection maintenance through heartbeats.
 */
class RabbitMQHandler : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject
     * @param host RabbitMQ host
     * @param port RabbitMQ port
     * @param exchange RabbitMQ exchange name
     * @param commandQueue Command queue name
     * @param responseQueue Response queue name
     * @param sendingRoutingKey Default sending routing key
     * @param receivingRoutingKeys List of routing keys to
     *                             receive on
     */
    explicit RabbitMQHandler(
        QObject       *parent = nullptr,
        const QString &host = "localhost", int port = 5672,
        const QString &exchange     = "simulation_exchange",
        const QString &commandQueue = "command_queue",
        const QString &responseQueue     = "response_queue",
        const QString &sendingRoutingKey = "default_key",
        const QStringList &receivingRoutingKeys =
            QStringList{"default_key"});

    /**
     * @brief Destructor
     */
    virtual ~RabbitMQHandler();

    /**
     * @brief Establishes connection to RabbitMQ
     * @return True if connection successful
     */
    bool establishConnection();

    /**
     * @brief Disconnects from RabbitMQ
     */
    void disconnect();

    /**
     * @brief Checks if connected to RabbitMQ
     * @return True if connected
     */
    bool isConnected() const;

    /**
     * @brief Sends a command message to RabbitMQ
     * @param message JSON message to send
     * @param routingKey Routing key to use (optional)
     * @return True if message sent successfully
     */
    bool sendCommand(const QJsonObject &message,
                     const QString &routingKey = QString());

    /**
     * @brief Starts consuming messages from response queue
     */
    void startConsuming();

    /**
     * @brief Stops consuming messages from response queue
     */
    void stopConsuming();

    /**
     * @brief Sets up heartbeat mechanism
     * @param heartbeatInterval Interval in seconds between
     *                          heartbeats
     */
    void setupHeartbeat(int heartbeatInterval = 5);

    /**
     * @brief Stops the heartbeat mechanism
     */
    void stopHeartbeat();

signals:
    /**
     * @brief Emitted when a message is received
     * @param message JSON message received
     */
    void messageReceived(const QJsonObject &message);

    /**
     * @brief Emitted when connection status changes
     * @param connected True if connected, false otherwise
     */
    void connectionChanged(bool connected);

    /**
     * @brief Emitted when an error occurs
     * @param errorMessage Error message
     */
    void errorOccurred(const QString &errorMessage);

private slots:
    /**
     * @brief Consumer thread function
     */
    void workerThreadFunction();

    /**
     * @brief Heartbeat thread function
     * @param interval Heartbeat interval in seconds
     */
    void heartbeatThreadFunction(int interval);

private:
    /**
     * @brief Sets up the RabbitMQ exchange
     * @return True if setup successful
     */
    bool setupExchange();

    /**
     * @brief Sets up the command and response queues
     * @return True if setup successful
     */
    bool setupQueues();

    /**
     * @brief Binds queues to exchange with routing keys
     * @return True if binding successful
     */
    bool bindQueues();

    /**
     * @brief Processes incoming messages
     */
    void processMessages();

    /**
     * @brief Publishes a raw message to RabbitMQ
     * @param message Raw message bytes
     * @param routingKey Routing key
     * @return True if published successfully
     */
    bool publishMessage(const QByteArray &message,
                        const QString    &routingKey);

    /**
     * @brief Reconnects the sending connection
     */
    void reconnectSending();

    /**
     * @brief Reconnects the receiving connection
     */
    void reconnectReceiving();

    // RabbitMQ connection state
    amqp_connection_state_t m_sendConnection;
    amqp_connection_state_t m_receiveConnection;
    bool                    m_connected;

    // Connection parameters
    QString     m_host;
    int         m_port;
    QString     m_exchange;
    QString     m_commandQueue;
    QString     m_responseQueue;
    QString     m_sendingRoutingKey;
    QStringList m_receivingRoutingKeys;

    // Threads
    QThread          *m_consumerThread;
    QThread          *m_heartbeatThread;
    std::atomic<bool> m_threadRunning;
    std::atomic<bool> m_heartbeatActive;
    qint64            m_lastHeartbeatSent;

    // Thread safety
    mutable QMutex m_mutex;

    // Constants
    static const int MAX_RETRIES = 5;
};

} // namespace Backend
} // namespace CargoNetSim

Q_DECLARE_METATYPE(CargoNetSim::Backend::RabbitMQHandler)
Q_DECLARE_METATYPE(CargoNetSim::Backend::RabbitMQHandler *)
