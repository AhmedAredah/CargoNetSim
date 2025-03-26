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

class HeartbeatController : public QObject
{
    Q_OBJECT

public:
    explicit HeartbeatController(MainWindow *mainWindow);
    ~HeartbeatController();

    void initialize();
    void updateServerHeartbeat(const QString &serverId,
                               float          timestamp);
    bool isServerActive(const QString &serverId) const;

private slots:
    void checkServerHeartbeats();

private:
    void setupHeartbeatMonitor();
    void heartbeatMonitorThread();
    void checkQueueConsumers();
    void updateServerStatus(const QString &serverId,
                            bool connected = false);

    MainWindow                            *mainWindow;
    QMap<QString, QMap<QString, QVariant>> serverIndicators;
    QTimer                                *heartbeatTimer;
    QThread                               *monitorThread;
    bool                                   isRunning;

    // Last check timestamps
    qint64 lastConsumerCheck;
    int    consumerCheckInterval;
};

} // namespace GUI
} // namespace CargoNetSim
