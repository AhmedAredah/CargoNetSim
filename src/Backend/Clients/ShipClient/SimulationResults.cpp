#include "ShipSimulationClient.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QThread>
#include <QCoreApplication>
#include <QFile>
#include <QDir>

// Placeholder includes
#include "Backend/Models/ShipSystem.h"
// #include "TerminalGraphServer.h"
// #include "SimulatorTimeServer.h"
// #include "ProgressBarManager.h"
// #include "ApplicationLogger.h"

namespace CargoNetSim {
namespace Backend {
namespace ShipClient {

SimulationResults::SimulationResults()
    : m_summaryData(QList<QPair<QString, QString>>()),
      m_trajectoryFileData(),
      m_trajectoryFileName(),
      m_summaryFileName()
{
}

SimulationResults::SimulationResults(
    const QList<QPair<QString, QString>>& summaryData,
    const QByteArray& trajectoryFileData,
    const QString& trajectoryFileName,
    const QString& summaryFileName)
    : m_summaryData(summaryData),
      m_trajectoryFileData(trajectoryFileData),
      m_trajectoryFileName(trajectoryFileName),
      m_summaryFileName(summaryFileName)
{
}

SimulationResults SimulationResults::fromJson(
    const QJsonObject& jsonObj)
{
    QList<QPair<QString, QString>> summaryData;
    
    // Parse summary data from JSON
    QJsonArray summaryArray = 
        jsonObj.value("summaryData").toArray();
    for (const QJsonValue& pairValue : summaryArray) {
        QJsonObject pairObj = pairValue.toObject();
        for (auto it = pairObj.constBegin(); 
             it != pairObj.constEnd(); 
             ++it) {
            QString key = it.key().trimmed();
            QString value = it.value().toString().trimmed();
            
            if (!key.isEmpty()) {
                summaryData.append(qMakePair(key, value));
            }
        }
    }
    
    // Get trajectory file data
    QByteArray trajectoryFileData;
    if (jsonObj.value("trajectoryFileDataIncluded").toBool()) {
        QString base64Data = 
            jsonObj.value("trajectoryFileData").toString();
        trajectoryFileData = QByteArray::fromBase64(
            base64Data.toLatin1());
    }
    
    return SimulationResults(
        summaryData,
        trajectoryFileData,
        jsonObj.value("trajectoryFileName").toString(),
        jsonObj.value("summaryFileName").toString()
    );
}

QString SimulationResults::getTrajectoryFileName() const
{
    return QFileInfo(m_trajectoryFileName).fileName();
}

QString SimulationResults::getSummaryFileName() const
{
    return QFileInfo(m_summaryFileName).fileName();
}

SimulationSummaryData SimulationResults::summaryData() const
{
    return m_summaryData;
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

} // namespace ShipClient
} // namespace Backend
} // namespace CargoNetSim
