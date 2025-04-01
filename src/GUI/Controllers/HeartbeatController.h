#pragma once

#include <QLabel>
#include <QMap>
#include <QObject>
#include <QString>
#include <QThread>
#include <QTimer>
#include <memory>

namespace CargoNetSim
{
namespace GUI
{

class MainWindow;

/**
 * @brief Controller that monitors server availability by
 * checking for queue consumers
 */
class HeartbeatController : public QObject
{
    Q_OBJECT

public:
    explicit HeartbeatController(MainWindow *mainWindow);
    ~HeartbeatController();

    /**
     * @brief Initialize the controller and start the
     * consumer check timer
     */
    void initialize();

private slots:
    /**
     * @brief Check queue consumers for all servers
     */
    void checkQueueConsumers();

private:
    /**
     * @brief Update the status indicator for a server in
     * the UI
     * @param serverId The ID of the server to update
     * @param connected Whether the server is connected (has
     * consumers)
     */
    void updateServerStatus(const QString &serverId,
                            bool connected = false);

    /**
     * @brief Check all queues directly using RabbitMQ
     * connections
     */
    void checkQueuesDirectly();

    MainWindow                            *mainWindow;
    QMap<QString, QMap<QString, QVariant>> serverIndicators;
    QMap<QString, bool>
        activeConsumers; // Tracks servers with active
                         // consumers
    QTimer  *consumerCheckTimer;
    QThread *monitorThread;
    bool     isRunning;

    // Consumer check timestamp and interval
    qint64 lastConsumerCheck;
    int    consumerCheckInterval; // in seconds
};

} // namespace GUI
} // namespace CargoNetSim
