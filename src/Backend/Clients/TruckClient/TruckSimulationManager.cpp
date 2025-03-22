/**
 * @file TruckSimulationManager.cpp
 * @brief Implements truck simulation manager
 * @author Ahmed Aredah
 * @date March 21, 2025
 */

#include "TruckSimulationManager.h"
#include <QThread>

namespace CargoNetSim {
namespace Backend {
namespace TruckClient {

TruckSimulationManager::TruckSimulationManager(
    QObject *parent)
    : QObject(parent) {}

TruckSimulationManager::~TruckSimulationManager() {
    for (auto *client : m_clients.values()) {
        delete client;
    }
}

void TruckSimulationManager::addClient(
    const QString         &networkName,
    TruckSimulationClient *client) {
    m_clients[networkName] = client;
}

void TruckSimulationManager::removeClient(
    const QString &networkName) {
    delete m_clients.take(networkName);
}

bool TruckSimulationManager::runSimulationSync(
    const QStringList &networkNames) {
    while (keepGoing(networkNames)) {
        syncGoOnce(networkNames);
        QThread::msleep(
            static_cast<long>(WAIT_INTERVAL * 1000));
    }
    return true;
}

bool TruckSimulationManager::runSimulationAsync(
    const QStringList &networkNames) {
    for (const QString &name : networkNames) {
        if (m_clients.contains(name)) {
            m_clients[name]->runSimulator({name});
        }
    }
    return true;
}

double TruckSimulationManager::getOverallProgress() const {
    double totalProgress = 0.0;
    int    count         = 0;
    for (const auto *client : m_clients.values()) {
        for (const QString &name : m_clients.keys()) {
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
    const QStringList &networkNames) const {
    QStringList names = networkNames.contains("*")
                            ? m_clients.keys()
                            : networkNames;
    for (const QString &name : names) {
        if (m_clients.contains(name)) {
            double progress =
                m_clients[name]->getProgressPercentage(
                    name);
            if (progress < 100.0)
                return true;
            m_clients[name]->endSimulator({name});
        }
    }
    return false;
}

void TruckSimulationManager::syncGoOnce(
    const QStringList &networkNames) {
    QStringList names   = networkNames.contains("*")
                              ? m_clients.keys()
                              : networkNames;
    double      maxTime = 0.0;
    for (const QString &name : names) {
        if (m_clients.contains(name)) {
            double time =
                m_clients[name]->getProgressPercentage(name)
                * m_clients[name]->getSimulationTime(name)
                / 100.0;
            maxTime = qMax(maxTime, time);
        }
    }

    for (const QString &name : names) {
        if (m_clients.contains(name)) {
            double current =
                m_clients[name]->getProgressPercentage(name)
                * m_clients[name]->getSimulationTime(name)
                / 100.0;
            if (current >= maxTime) {
                m_clients[name]->runSimulator({name});
            }
        }
    }
}

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
