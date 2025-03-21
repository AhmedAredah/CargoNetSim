#pragma once

#include <QObject>
#include <QMutex>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QList>
#include <QString>
#include "Backend/Models/Terminal.h"
#include "Backend/Models/Route.h"
#include "Backend/Clients/BaseClient/SimulationClientBase.h"


namespace CargoNetSim {
namespace Backend {

/**
 * @brief Client for interacting with TerminalSim server
 *
 * This class provides functionality to manage terminals, routes,
 * and containers in the TerminalSim server via RabbitMQ.
 */
class TerminalSimulationClient : public SimulationClientBase {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject
     * @param host RabbitMQ host
     * @param port RabbitMQ port
     */
    explicit TerminalSimulationClient(
        QObject* parent = nullptr,
        const QString& host = "localhost",
        int port = 5672);

    /**
     * @brief Destructor
     */
    ~TerminalSimulationClient() override;

    /**
     * @brief Reset the TerminalSim server
     * @return True if successful
     */
    bool resetServer();

    /**
     * @brief Initialize the client in its thread
     *
     * Sets up thread-specific resources after the object moves to
     * its thread. Runs automatically via QThread::started signal.
     *
     * Tasks:
     * - Calls base init for shared resources.
     * - Sets up RabbitMQ heartbeat.
     * - Logs init details.
     *
     * @note Avoid manual calls unless synchronized.
     * @warning Call only once after thread start.
     * @throws std::runtime_error If init fails.
     */
    void initializeClient(LoggerInterface* logger = nullptr) override;

    /**
     * @brief Add a terminal using a Terminal object
     * @param terminal Terminal pointer
     * @return True if successful
     */
    bool addTerminal(const Terminal* terminal);

    /**
     * @brief Add a route using a Route object
     * @param route Route pointer
     * @return True if successful
     */
    bool addRoute(const Route* route);

    /**
     * @brief Find shortest path between terminals
     * @param start Start terminal
     * @param end End terminal
     * @param mode Transportation mode
     * @return Path segments as JSON array
     */
    QJsonArray findShortestPath(
        const QString& start,
        const QString& end,
        int mode);

    /**
     * @brief Add containers to a terminal
     * @param terminalId Terminal ID
     * @param containers List of container JSON objects
     * @param addTime Time of addition (optional)
     * @return True if successful
     */
    bool addContainers(
        const QString& terminalId,
        const QList<QJsonObject>& containers,
        double addTime = -1.0);

    /**
     * @brief Get terminal status
     * @param terminalId Terminal ID (empty for all)
     * @return Terminal status as JSON
     */
    QJsonObject getTerminalStatus(const QString& terminalId);

protected:
    /**
     * @brief Process server messages
     * @param message JSON message from server
     */
    void processMessage(const QJsonObject& message) override;

private:
    /**
     * @brief Handle terminal added event
     * @param message Event data
     */
    void onTerminalAdded(const QJsonObject& message);

    /**
     * @brief Handle route added event
     * @param message Event data
     */
    void onRouteAdded(const QJsonObject& message);

    /**
     * @brief Handle path found event
     * @param message Event data
     */
    void onPathFound(const QJsonObject& message);

    /**
     * @brief Handle containers added event
     * @param message Event data
     */
    void onContainersAdded(const QJsonObject& message);

    /**
     * @brief Handle server reset event
     * @param message Event data
     */
    void onServerReset(const QJsonObject& message);

    /**
     * @brief Handle error event
     * @param message Event data
     */
    void onErrorOccurred(const QJsonObject& message);

    // Thread-safe data storage
    mutable QMutex m_dataMutex;
    QMap<QString, QJsonObject> m_terminalStatus;
    QMap<QString, QJsonArray> m_paths;
};

} // namespace Backend
} // namespace CargoNetSim

Q_DECLARE_METATYPE(CargoNetSim::Backend::TerminalSimulationClient)
Q_DECLARE_METATYPE(CargoNetSim::Backend::TerminalSimulationClient*)
