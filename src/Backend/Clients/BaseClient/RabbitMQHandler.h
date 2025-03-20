#pragma once

#include <QObject>
#include <QMutex>
#include <QThread>
#include <QString>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <atomic>
#include <rabbitmq-c/amqp.h>

namespace CargoNetSim {
namespace Backend {


class RabbitMQHandler : public QObject {
    Q_OBJECT

public:
    explicit RabbitMQHandler(
        QObject* parent = nullptr,
        const QString& host = "localhost",
        int port = 5672,
        const QString& exchange = "simulation_exchange",
        const QString& commandQueue = "command_queue",
        const QString& responseQueue = "response_queue",
        const QString& sendingRoutingKey = "default_key",
        const QStringList& receivingRoutingKeys = {"default_key"});
    
    virtual ~RabbitMQHandler();

    bool establishConnection();
    void disconnect();
    bool isConnected() const;
    bool sendCommand(const QJsonObject& message,
                     const QString& routingKey = QString());
    void startConsuming();
    void stopConsuming();
    void setupHeartbeat(int heartbeatInterval = 5);
    void stopHeartbeat();

signals:
    void messageReceived(const QJsonObject& message);
    void connectionChanged(bool connected);
    void errorOccurred(const QString& errorMessage);

private slots:
    void workerThreadFunction();
    void heartbeatThreadFunction(int interval);

private:
    bool setupExchange();
    bool setupQueues();
    bool bindQueues();
    void processMessages();
    bool publishMessage(const QByteArray& message,
                        const QString& routingKey);
    void reconnectSending();
    void reconnectReceiving();

    // RabbitMQ connection state
    amqp_connection_state_t m_sendConnection;
    amqp_connection_state_t m_receiveConnection;
    bool m_connected;

    // Connection parameters
    QString m_host;
    int m_port;
    QString m_exchange;
    QString m_commandQueue;
    QString m_responseQueue;
    QString m_sendingRoutingKey;
    QStringList m_receivingRoutingKeys;

    // Threads
    QThread* m_consumerThread;
    QThread* m_heartbeatThread;
    std::atomic<bool> m_threadRunning;
    std::atomic<bool> m_heartbeatActive;
    qint64 m_lastHeartbeatSent;

    // Thread safety
    mutable QMutex m_mutex;
    static const int MAX_RETRIES = 5;
};

} // namespace Backend
} // namespace CargoNetSim

Q_DECLARE_METATYPE(CargoNetSim::Backend::RabbitMQHandler)
Q_DECLARE_METATYPE(CargoNetSim::Backend::RabbitMQHandler*)

