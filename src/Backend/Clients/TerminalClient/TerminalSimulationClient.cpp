#include "TerminalSimulationClient.h"
#include <QDebug>
#include <QJsonDocument>
#include <QThread>

namespace CargoNetSim {
namespace Backend {

/**
 * Constructor initializes the terminal client
 */
TerminalSimulationClient::TerminalSimulationClient(
    QObject* parent,
    const QString& host,
    int port)
    : SimulationClientBase(
          parent,
          host,
          port,
          "CargoNetSim.Exchange",
          "CargoNetSim.CommandQueue.TerminalSim",
          "CargoNetSim.ResponseQueue.TerminalSim",
          "CargoNetSim.Command.TerminalSim",
          QStringList{"CargoNetSim.Response.TerminalSim"},
          ClientType::TerminalClient)
{
    qDebug() << "TerminalSimulationClient initialized";
}

/**
 * Destructor cleans up resources
 */
TerminalSimulationClient::~TerminalSimulationClient()
{
    QMutexLocker locker(&m_dataMutex);
    m_terminalStatus.clear();
    m_paths.clear();
    qDebug() << "TerminalSimulationClient destroyed";
}

/**
 * Reset the TerminalSim server
 */
bool TerminalSimulationClient::resetServer()
{
    return executeSerializedCommand([this]() {
        return sendCommandAndWait(
            "resetServer",
            QJsonObject(),
            {"serverReset"});
    });
}

/**
 * Initialize client in its thread
 */
void TerminalSimulationClient::initializeClient(LoggerInterface *logger)
{
    SimulationClientBase::initializeClient(logger);
    if (!m_rabbitMQHandler) {
        throw std::runtime_error("RabbitMQ handler not set");
    }
    m_rabbitMQHandler->setupHeartbeat(5);
    qDebug() << "TerminalSimulationClient init in thread:"
             << QThread::currentThreadId();
}

/**
 * Add a terminal to the server
 */
bool TerminalSimulationClient::addTerminal(const Terminal* terminal)
{
    return executeSerializedCommand([&]() {
        if (!terminal) {
            qCritical() << "Null terminal pointer";
            return false;
        }
        return sendCommandAndWait(
            "add_terminal",
            terminal->toJson(),
            {"terminalAdded"});
    });
}

/**
 * Add a route between terminals
 */
bool TerminalSimulationClient::addRoute(const Route* route)
{
    return executeSerializedCommand([&]() {
        if (!route) {
            qCritical() << "Null route pointer";
            return false;
        }
        return sendCommandAndWait(
            "add_route",
            route->toJson(),
            {"routeAdded"});
    });
}

/**
 * Find shortest path between terminals
 */
QJsonArray TerminalSimulationClient::findShortestPath(
    const QString& start,
    const QString& end,
    int mode)
{
    executeSerializedCommand([&]() {
        QJsonObject params;
        params["start_terminal"] = start;
        params["end_terminal"] = end;
        params["mode"] = mode;
        return sendCommandAndWait(
            "find_shortest_path",
            params,
            {"pathFound"});
    });
    QMutexLocker locker(&m_dataMutex);
    QString key = start + "-" + end + "-" + QString::number(mode);
    return m_paths.value(key, QJsonArray());
}

/**
 * Add containers to a terminal
 */
bool TerminalSimulationClient::addContainers(
    const QString& terminalId,
    const QList<QJsonObject>& containers,
    double addTime)
{
    return executeSerializedCommand([&]() {
        QJsonObject params;
        params["terminal_id"] = terminalId;
        QJsonArray containersArray;
        for (const auto& container : containers) {
            containersArray.append(container);
        }
        params["containers"] = containersArray;
        if (addTime >= 0.0) {
            params["adding_time"] = addTime;
        }
        return sendCommandAndWait(
            "add_containers",
            params,
            {"containersAdded"});
    });
}

/**
 * Get terminal status
 */
QJsonObject TerminalSimulationClient::getTerminalStatus(
    const QString& terminalId)
{
    executeSerializedCommand([&]() {
        QJsonObject params;
        if (!terminalId.isEmpty()) {
            params["terminal_name"] = terminalId;
        }
        return sendCommandAndWait(
            "get_terminal",
            params,
            {"terminalStatus"});
    });
    QMutexLocker locker(&m_dataMutex);
    return m_terminalStatus.value(terminalId, QJsonObject());
}

/**
 * Process server messages
 */
void TerminalSimulationClient::processMessage(
    const QJsonObject& message)
{
    SimulationClientBase::processMessage(message);
    if (!message.contains("event")) {
        return;
    }
    QString event = message["event"].toString();
    QString normEvent = normalizeEventName(event);
    if (normEvent == "terminaladded") {
        onTerminalAdded(message);
    } else if (normEvent == "routeadded") {
        onRouteAdded(message);
    } else if (normEvent == "pathfound") {
        onPathFound(message);
    } else if (normEvent == "containersadded") {
        onContainersAdded(message);
    } else if (normEvent == "serverreset") {
        onServerReset(message);
    } else if (normEvent == "erroroccurred") {
        onErrorOccurred(message);
    } else {
        qWarning() << "Unknown event:" << event;
    }
}

/**
 * Handle terminal added event
 */
void TerminalSimulationClient::onTerminalAdded(
    const QJsonObject& message)
{
    QJsonObject result = message["result"].toObject();
    QString name = result["terminal_name"].toString();
    QMutexLocker locker(&m_dataMutex);
    m_terminalStatus[name] = result;
    qDebug() << "Terminal added:" << name;
}

/**
 * Handle route added event
 */
void TerminalSimulationClient::onRouteAdded(
    const QJsonObject& message)
{
    QJsonObject result = message["result"].toObject();
    QString routeId = result["route_id"].toString();
    qDebug() << "Route added:" << routeId;
}

/**
 * Handle path found event
 */
void TerminalSimulationClient::onPathFound(
    const QJsonObject& message)
{
    QJsonArray path = message["result"].toArray();
    QJsonObject params = message["params"].toObject();
    QString start = params["start_terminal"].toString();
    QString end = params["end_terminal"].toString();
    int mode = params["mode"].toInt();
    QString key = start + "-" + end + "-" + QString::number(mode);
    QMutexLocker locker(&m_dataMutex);
    m_paths[key] = path;
    qDebug() << "Path found from" << start << "to" << end;
}

/**
 * Handle containers added event
 */
void TerminalSimulationClient::onContainersAdded(
    const QJsonObject& message)
{
    QJsonObject params = message["params"].toObject();
    QString terminalId = params["terminal_id"].toString();
    qDebug() << "Containers added to terminal:" << terminalId;
}

/**
 * Handle server reset event
 */
void TerminalSimulationClient::onServerReset(
    const QJsonObject& message)
{
    QMutexLocker locker(&m_dataMutex);
    m_terminalStatus.clear();
    m_paths.clear();
    qDebug() << "Server reset successfully";
}

/**
 * Handle error event
 */
void TerminalSimulationClient::onErrorOccurred(
    const QJsonObject& message)
{
    QString error = message["error"].toString();
    qCritical() << "Error occurred:" << error;
}

} // namespace Backend
} // namespace CargoNetSim
