#pragma once

#include <QObject>
#include <QMutex>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <QMap>
#include <QList>
#include <QString>
#include <QThread>
#include <QDateTime>
#include <QWaitCondition>

#include "Backend/Clients/ShipClient/SimulationSummaryData.h"


namespace CargoNetSim {
namespace Backend {

/**
     * @brief Stores simulation results data
     */
class SimulationResults {
public:
    /**
     * @brief Default constructor
     */
    SimulationResults();

    /**
     * @brief Constructor with all data
     * @param summaryData Summary data pairs
     * @param trajectoryFileData Binary trajectory file data
     * @param trajectoryFileName Name of trajectory file
     * @param summaryFileName Name of summary file
     */
    SimulationResults(
        const QList<QPair<QString, QString>>& summaryData,
        const QByteArray& trajectoryFileData,
        const QString& trajectoryFileName,
        const QString& summaryFileName);

    /**
     * @brief Create from JSON object
     * @param jsonObj JSON object with results data
     * @return SimulationResults instance
     */
    static SimulationResults fromJson(
        const QJsonObject& jsonObj);

    /**
     * @brief Get trajectory filename without path
     * @return Filename only
     */
    QString getTrajectoryFileName() const;

    /**
     * @brief Get summary filename without path
     * @return Filename only
     */
    QString getSummaryFileName() const;

    // Accessors
    SimulationSummaryData summaryData() const;
    QByteArray trajectoryFileData() const;
    QString trajectoryFileName() const;
    QString summaryFileName() const;
    
private:
    SimulationSummaryData m_summaryData;
    QByteArray m_trajectoryFileData;
    QString m_trajectoryFileName;
    QString m_summaryFileName;
};

} // namespace Backend
} // namespace CargoNetSim


Q_DECLARE_METATYPE(CargoNetSim::Backend::SimulationResults)
Q_DECLARE_METATYPE(CargoNetSim::Backend::SimulationResults*)

