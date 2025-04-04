/**
 * @file TruckSimulationManager.cpp
 * @brief Implements truck simulation manager
 * @author Ahmed Aredah
 * @date March 21, 2025
 */

#include "TruckSimulationManager.h"
#include <QThread>

namespace CargoNetSim
{
namespace Backend
{
namespace TruckClient
{

TruckSimulationManager::TruckSimulationManager(
    QObject *parent)
    : QObject(parent)
{
}

TruckSimulationManager::~TruckSimulationManager()
{
    Commons::ScopedWriteLock locker(m_mutex);
    for (auto *client : m_clients.values())
    {
        delete client;
    }
}

void TruckSimulationManager::initializeManager(
    LoggerInterface *logger)
{
    m_defaultLogger = logger;
}

void TruckSimulationManager::addClient(
    const QString         &networkName,
    TruckSimulationClient *client)
{
    Commons::ScopedWriteLock locker(m_mutex);
    m_clients[networkName] = client;
    moveClientToThread(networkName,
                       QThread::currentThread(),
                       m_defaultLogger);
}

QList<TruckSimulationClient *>
TruckSimulationManager::getAllClients()
{
    Commons::ScopedWriteLock locker(m_mutex);
    return m_clients.values();
}

void TruckSimulationManager::removeClient(
    const QString &networkName)
{
    Commons::ScopedWriteLock locker(m_mutex);
    delete m_clients.take(networkName);
}

bool TruckSimulationManager::runSimulationSync(
    const QStringList &networkNames)
{
    while (keepGoing(networkNames))
    {
        syncGoOnce(networkNames);
        QThread::msleep(
            static_cast<long>(WAIT_INTERVAL * 1000));
    }
    return true;
}

bool TruckSimulationManager::runSimulationAsync(
    const QStringList &networkNames)
{
    Commons::ScopedReadLock locker(m_mutex);
    for (const QString &name : networkNames)
    {
        if (m_clients.contains(name))
        {
            m_clients[name]->runSimulator({name});
        }
    }
    return true;
}

void TruckSimulationManager::moveClientToThread(
    const QString &networkName, QThread *thread,
    LoggerInterface *logger)
{
    Commons::ScopedReadLock locker(m_mutex);
    if (m_clients.contains(networkName))
    {
        m_clients[networkName]->moveToThread(thread);
        m_clients[networkName]->initializeClient(logger);
        m_clients[networkName]->connectToServer();
    }
}

double TruckSimulationManager::getOverallProgress() const
{
    Commons::ScopedReadLock locker(m_mutex);
    double                  totalProgress = 0.0;
    int                     count         = 0;
    for (const auto *client : m_clients.values())
    {
        for (const QString &name : m_clients.keys())
        {
            totalProgress +=
                client->getProgressPercentage(name);
            count++;
        }
    }
    double progress = count ? totalProgress / count : 0.0;
    emit progressUpdated(progress);
    return progress;
}

bool TruckSimulationManager::keepGoing(
    const QStringList &networkNames) const
{
    // First check with a read lock
    {
        Commons::ScopedReadLock locker(m_mutex);
        QStringList names = networkNames.contains("*")
                                ? m_clients.keys()
                                : networkNames;
        for (const QString &name : names)
        {
            if (m_clients.contains(name))
            {
                double progress =
                    m_clients[name]->getProgressPercentage(
                        name);
                if (progress < 100.0)
                {
                    return true;
                }

                // Store the client pointer to use after
                // releasing the lock
                TruckSimulationClient *client =
                    m_clients[name];

                // The lock will be automatically released
                // when we exit this scope
            }
        }

        // If we get here, no more simulations are running
        return false;
    }

    // Call endSimulator outside the lock scope to avoid
    // potential deadlocks This is only reached if we found
    // a simulation at 100% progress
    for (const QString &name : networkNames.contains("*")
                                   ? m_clients.keys()
                                   : networkNames)
    {
        TruckSimulationClient *client = nullptr;

        // Briefly acquire the lock again to check and get
        // the client
        {
            Commons::ScopedReadLock locker(m_mutex);
            if (m_clients.contains(name))
            {
                client = m_clients[name];
            }
        }

        // Call endSimulator outside the lock scope
        if (client)
        {
            client->endSimulator({name});
        }
    }

    // Recursively check again after ending simulations
    return keepGoing(networkNames);
}

void TruckSimulationManager::syncGoOnce(
    const QStringList &networkNames)
{
    double                                 maxTime = 0.0;
    QStringList                            names;
    QMap<QString, double>                  currentTimes;
    QMap<QString, TruckSimulationClient *> relevantClients;

    // Collect all necessary data with the lock held
    {
        Commons::ScopedReadLock locker(m_mutex);
        names = networkNames.contains("*")
                    ? m_clients.keys()
                    : networkNames;

        // First pass: calculate maxTime
        for (const QString &name : names)
        {
            if (m_clients.contains(name))
            {
                double time =
                    m_clients[name]->getProgressPercentage(
                        name)
                    * m_clients[name]->getSimulationTime(
                        name)
                    / 100.0;
                maxTime               = qMax(maxTime, time);
                currentTimes[name]    = time;
                relevantClients[name] = m_clients[name];
            }
        }
    }

    // Now process without holding the lock
    for (const QString &name : names)
    {
        if (relevantClients.contains(name))
        {
            double current = currentTimes[name];
            if (current >= maxTime)
            {
                relevantClients[name]->runSimulator({name});
                return; // Exit early after sending command
            }
        }
    }
}

bool TruckSimulationManager::isConnected() const
{
    Commons::ScopedReadLock locker(m_mutex);
    // Check if any client is connected
    for (const auto *client : m_clients.values())
    {
        if (client && client->isConnected())
        {
            return true;
        }
    }
    return false;
}

bool TruckSimulationManager::hasCommandQueueConsumers()
    const
{
    Commons::ScopedReadLock locker(m_mutex);
    // Check if any client has command queue consumers
    for (const auto *client : m_clients.values())
    {
        if (client && client->isConnected())
        {
            auto *handler = client->getRabbitMQHandler();
            if (handler
                && handler->hasCommandQueueConsumers())
            {
                return true;
            }
        }
    }
    return false;
}

RabbitMQHandler *
TruckSimulationManager::getRabbitMQHandler() const
{
    Commons::ScopedReadLock locker(m_mutex);
    // Return the first connected client's RabbitMQ handler
    for (const auto *client : m_clients.values())
    {
        if (client && client->isConnected())
        {
            return client->getRabbitMQHandler();
        }
    }
    return nullptr;
}

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
