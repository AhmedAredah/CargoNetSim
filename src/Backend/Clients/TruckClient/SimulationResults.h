/**
 * @file SimulationResults.h
 * @brief Stores truck simulation results
 * @author Ahmed Aredah
 * @date March 21, 2025
 */

#pragma once

#include "Backend/Clients/TruckClient/SimulationSummaryData.h"
#include <QByteArray>
#include <QJsonObject>
#include <QObject>

namespace CargoNetSim {
namespace Backend {
namespace TruckClient {

class SimulationResults : public QObject {
    Q_OBJECT

public:
    explicit SimulationResults(QObject *parent = nullptr);
    SimulationResults(
        const QList<QPair<QString, QString>> &summaryData,
        const QByteArray &trajectoryData,
        const QString    &trajectoryFileName,
        const QString    &summaryFileName,
        QObject          *parent = nullptr);
    ~SimulationResults() override = default;

    static SimulationResults *
    fromJson(const QJsonObject &jsonObj,
             QObject           *parent = nullptr);

    QString getTrajectoryFileName() const;
    QString getSummaryFileName() const;

    const SimulationSummaryData            &
    summaryData() const; // Changed to const ref
    QByteArray trajectoryFileData() const;
    QString    trajectoryFileName() const;
    QString    summaryFileName() const;

private:
    SimulationSummaryData m_summaryData;
    QByteArray            m_trajectoryFileData;
    QString               m_trajectoryFileName;
    QString               m_summaryFileName;
};

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim

Q_DECLARE_METATYPE(
    CargoNetSim::Backend::TruckClient::SimulationResults)
Q_DECLARE_METATYPE(
    CargoNetSim::Backend::TruckClient::SimulationResults *)
