/**
 * @file SimulationResults.cpp
 * @brief Implements truck simulation results
 * @author Ahmed Aredah
 * @date March 21, 2025
 */

#include "SimulationResults.h"
#include <QFileInfo>
#include <QJsonArray>

namespace CargoNetSim
{
namespace Backend
{
namespace TruckClient
{

SimulationResults::SimulationResults(QObject *parent)
    : QObject(parent)
    , m_summaryData(QList<QPair<QString, QString>>())
    , m_trajectoryFileData()
    , m_trajectoryFileName()
    , m_summaryFileName()
{
}

SimulationResults::SimulationResults(
    const QList<QPair<QString, QString>> &summaryData,
    const QByteArray                     &trajectoryData,
    const QString &trajectoryFileName,
    const QString &summaryFileName, QObject *parent)
    : QObject(parent)
    , m_summaryData(summaryData)
    , m_trajectoryFileData(trajectoryData)
    , m_trajectoryFileName(trajectoryFileName)
    , m_summaryFileName(summaryFileName)
{
}

SimulationResults *
SimulationResults::fromJson(const QJsonObject &jsonObj,
                            QObject           *parent)
{
    QList<QPair<QString, QString>> summaryData;
    QJsonArray                     summaryArray =
        jsonObj["summaryData"].toArray();
    for (const QJsonValue &pairValue : summaryArray)
    {
        QJsonObject pairObj = pairValue.toObject();
        for (auto it = pairObj.constBegin();
             it != pairObj.constEnd(); ++it)
        {
            QString key   = it.key().trimmed();
            QString value = it.value().toString().trimmed();
            if (!key.isEmpty())
            {
                summaryData.append(qMakePair(key, value));
            }
        }
    }

    QByteArray trajectoryData;
    if (jsonObj["trajectoryFileDataIncluded"].toBool())
    {
        QString base64Data =
            jsonObj["trajectoryFileData"].toString();
        trajectoryData =
            QByteArray::fromBase64(base64Data.toLatin1());
    }

    return new SimulationResults(
        summaryData, trajectoryData,
        jsonObj["trajectoryFileName"].toString(),
        jsonObj["summaryFileName"].toString(), parent);
}

QString SimulationResults::getTrajectoryFileName() const
{
    return QFileInfo(m_trajectoryFileName).fileName();
}

QString SimulationResults::getSummaryFileName() const
{
    return QFileInfo(m_summaryFileName).fileName();
}

const SimulationSummaryData &
SimulationResults::summaryData() const
{
    return m_summaryData; // Return const reference
}

QByteArray SimulationResults::trajectoryFileData() const
{
    return m_trajectoryFileData;
}

QString SimulationResults::trajectoryFileName() const
{
    return m_trajectoryFileName;
}

QString SimulationResults::summaryFileName() const
{
    return m_summaryFileName;
}

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
