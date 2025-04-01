/**
 * @file TruckSimulationManager.h
 * @brief Manages multiple truck simulation clients
 * @author Ahmed Aredah
 * @date March 21, 2025
 */

#pragma once

#include "Backend/Clients/BaseClient/RabbitMQHandler.h"
#include "TruckSimulationClient.h"
#include <QMap>
#include <QObject>

namespace CargoNetSim
{
namespace Backend
{
namespace TruckClient
{

class TruckSimulationManager : public QObject
{
    Q_OBJECT

public:
    explicit TruckSimulationManager(
        QObject *parent = nullptr);
    ~TruckSimulationManager() override;

    void addClient(const QString         &networkName,
                   TruckSimulationClient *client,
                   LoggerInterface       *logger = nullptr);
    void removeClient(const QString &networkName);

    bool runSimulationSync(const QStringList &networkNames);
    bool
    runSimulationAsync(const QStringList &networkNames);

    double getOverallProgress() const;

    /**
     * @brief Checks if any of the truck clients are
     * connected
     * @return True if at least one client is connected,
     * false otherwise
     */
    bool isConnected() const;

    /**
     * @brief Checks if any of the truck clients have active
     * command queue consumers
     * @return True if at least one client has an active
     * consumer, false otherwise
     */
    bool hasCommandQueueConsumers() const;

    /**
     * @brief Gets the RabbitMQ handler of the first
     * connected client
     * @return Pointer to the RabbitMQ handler, or nullptr
     * if no clients are connected
     */
    RabbitMQHandler *getRabbitMQHandler() const;

private:
    bool keepGoing(const QStringList &networkNames) const;
    void syncGoOnce(const QStringList &networkNames);

    QMap<QString, TruckSimulationClient *> m_clients;
    static constexpr double WAIT_INTERVAL = 0.1; // Seconds

signals:
    void progressUpdated(double percentage) const;
};

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
