#pragma once

#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QMap>
#include <QJsonObject>
#include <QStringList>
#include "Backend/Commons/ClientType.h"
#include "RabbitMQHandler.h"

namespace CargoNetSim {
namespace Backend {


class SimulationClientBase : public QObject {
    Q_OBJECT

public:
    explicit SimulationClientBase(
        QObject* parent = nullptr,
        const QString& host = "localhost",
        int port = 5672,
        const QString& exchange = "simulation_exchange",
        const QString& commandQueue = "command_queue",
        const QString& responseQueue = "response_queue",
        const QString& sendingRoutingKey = QString(),
        const QStringList& receivingRoutingKeys = QStringList(),
        ClientType clientType = ClientType::PortClient);
    
    virtual ~SimulationClientBase();

    // Command methods
    bool sendCommand(const QString& command,
                     const QJsonObject& params = QJsonObject(),
                     const QString& routingKey = QString());
    bool waitForEvent(const QStringList& expectedEvents,
                      int timeoutMs = -1);
    void resetServer();

signals:
    void eventReceived(const QString& event,
                       const QJsonObject& data);
    void errorOccurred(const QString& errorMessage);
    void connectionStatusChanged(bool connected);

protected slots:
    virtual void handleMessage(const QJsonObject& message);

protected:
    // Methods to be implemented by derived classes
    virtual void onServerReset() {}
    virtual void onSimulationCreated(
        const QJsonObject& message) {}
    virtual void onSimulationEnded(
        const QJsonObject& message) {}
    virtual void onSimulationAdvanced(
        const QJsonObject& message) {}
    virtual void onSimulationProgressUpdate(
        const QJsonObject& message) {}
    virtual void onErrorOccurred(
        const QJsonObject& message) {}

    // RabbitMQ handler
    RabbitMQHandler* m_rabbitMQHandler;
    ClientType m_clientType;

    // Event handling
    QMutex m_eventMutex;
    QWaitCondition m_eventCondition;
    QMap<QString, QJsonObject> m_receivedEvents;
    
    // Connection parameters
    QString m_host;
    int m_port;
    QString m_exchange;
    QString m_commandQueue;
    QString m_responseQueue;
    QString m_sendingRoutingKey;
    QStringList m_receivingRoutingKeys;
};

} // namespace Backend
} // namespace CargoNetSim

Q_DECLARE_METATYPE(CargoNetSim::Backend::SimulationClientBase)
Q_DECLARE_METATYPE(CargoNetSim::Backend::SimulationClientBase*)

