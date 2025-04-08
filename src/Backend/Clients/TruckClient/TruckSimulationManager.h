/**
 * @file TruckSimulationManager.h
 * @brief Manages multiple truck simulation clients
 * @author Ahmed Aredah
 * @date March 21, 2025
 */

#pragma once

#include "Backend/Clients/BaseClient/RabbitMQHandler.h"
#include "Backend/Commons/ThreadSafetyUtils.h"
#include "TruckSimulationClient.h"
#include <QMap>
#include <QObject>
#include <QReadWriteLock>

namespace CargoNetSim
{
namespace Backend
{
namespace TruckClient
{

/**
 * @class TruckSimulationManager
 * @brief Manages multiple truck simulation clients
 *
 * Provides a unified interface for controlling and
 * monitoring multiple truck simulation instances.
 *
 * Thread Safety Implementation:
 * - Uses ThreadSafetyUtils' ScopedReadLock for read-only operations
 * - Uses ThreadSafetyUtils' ScopedWriteLock for write operations
 * - Ensures all access to shared data structures is properly protected
 */
class TruckSimulationManager : public QObject
{
    Q_OBJECT

public:
    explicit TruckSimulationManager(
        QObject *parent = nullptr);
    ~TruckSimulationManager() override;

    void
    initializeManager(SimulationTime  *simulationTime,
                      LoggerInterface *logger = nullptr);

    void addClient(const QString         &networkName,
                   TruckSimulationClient *client);
    QList<TruckSimulationClient *> getAllClients();
    void removeClient(const QString &networkName);

    bool runSimulationSync(const QStringList &networkNames);
    bool
    runSimulationAsync(const QStringList &networkNames);

    void moveClientToThread(const QString   &networkName,
                            QThread         *thread,
                            LoggerInterface *logger);

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

    /**
     * @brief Map of network names to simulation clients
     * 
     * Protected by m_mutex for thread-safe access.
     */
    QMap<QString, TruckSimulationClient *> m_clients;
    
    /**
     * @brief Read-write lock for thread synchronization
     * 
     * Protects m_clients from concurrent access.
     * Access to this lock should be managed using:
     * - Commons::ScopedReadLock for read-only operations
     * - Commons::ScopedWriteLock for write operations
     */
    mutable QReadWriteLock m_mutex;

    SimulationTime  *m_defaultSimulationTime = nullptr;
    LoggerInterface *m_defaultLogger = nullptr;

    /** @brief Wait interval between simulation steps in
     * seconds */
    static constexpr double WAIT_INTERVAL = 0.1;

signals:
    void progressUpdated(double percentage) const;
};

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
