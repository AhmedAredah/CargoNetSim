/**
 * @file TruckSimulationManager.h
 * @brief Manages multiple truck simulation clients
 * @author Ahmed Aredah
 * @date March 21, 2025
 */

#pragma once

#include "TruckSimulationClient.h"
#include <QMap>
#include <QObject>

namespace CargoNetSim {
namespace Backend {
namespace TruckClient {

class TruckSimulationManager : public QObject {
    Q_OBJECT

public:
    explicit TruckSimulationManager(
        QObject *parent = nullptr);
    ~TruckSimulationManager() override;

    void addClient(const QString         &networkName,
                   TruckSimulationClient *client);
    void removeClient(const QString &networkName);

    bool runSimulationSync(const QStringList &networkNames);
    bool
    runSimulationAsync(const QStringList &networkNames);

    double getOverallProgress() const;

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
