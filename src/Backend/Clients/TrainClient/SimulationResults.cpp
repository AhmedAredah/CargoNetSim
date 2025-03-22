#include "SimulationResults.h"
#include <QJsonArray>

/**
 * @file SimulationResults.cpp
 * @brief Implementation of SimulationResults class
 * @author Ahmed Aredah
 * @date March 20, 2025
 *
 * Implements the SimulationResults class, managing
 * simulation results with extensive comments for auditing
 * and review.
 */

namespace CargoNetSim {
namespace Backend {
namespace TrainClient {

SimulationResults::SimulationResults(
    const QVector<QPair<QString, QString>> &summaryData,
    const QByteArray &trajectoryFileData,
    const QString    &trajectoryFileName,
    const QString    &summaryFileName)
    : m_summaryData(summaryData)
    , // Init summary data
    m_trajectoryFileData(trajectoryFileData)
    , // Init file data
    m_trajectoryFileName(trajectoryFileName)
    ,                                  // Init file name
    m_summaryFileName(summaryFileName) // Init summary name
{
    // Constructor initializes all member variables directly
    // using the provided parameters, no additional logic
    // needed
}

SimulationResults
SimulationResults::fromJson(const QJsonObject &jsonObj) {
    // Vector to store summary data as text-value pairs
    QVector<QPair<QString, QString>> summaryData;

    // Extract summary data array from JSON object
    QJsonArray summaryArray =
        jsonObj["summaryData"].toArray();

    // Iterate over each element in the summary array
    for (const QJsonValue &pairVal : summaryArray) {
        // Convert value to object (expected format:
        // key-value pairs)
        QJsonObject pairObj = pairVal.toObject();

        // Process each key-value pair in the object
        for (auto it = pairObj.begin(); it != pairObj.end();
             ++it) {
            // Clean the key by removing whitespace
            QString key = it.key().trimmed();

            // Skip if key is empty to avoid invalid entries
            if (!key.isEmpty()) {
                // Get value, trim if it exists, empty
                // string if not
                QString value =
                    it.value().toString().trimmed();

                // Add cleaned key-value pair to summary
                // data
                summaryData.append(qMakePair(key, value));
            }
        }
    }

    // Initialize trajectory file data as empty by default
    QByteArray trajectoryFileData;

    // Check if trajectory data is included in JSON
    if (jsonObj["trajectoryFileDataIncluded"].toBool()) {
        // Get base64-encoded string from JSON
        QString base64Data =
            jsonObj["trajectoryFileData"].toString();

        // Decode base64 string to raw byte data
        trajectoryFileData =
            QByteArray::fromBase64(base64Data.toUtf8());
    }

    // Extract trajectory file name from JSON, default to
    // empty
    QString trajectoryFileName =
        jsonObj["trajectoryFileName"].toString();

    // Extract summary file name from JSON, default to empty
    QString summaryFileName =
        jsonObj["summaryFileName"].toString();

    // Return a new instance with parsed data
    return SimulationResults(
        summaryData,        // Parsed summary data
        trajectoryFileData, // Decoded file data
        trajectoryFileName, // Trajectory file name
        summaryFileName);   // Summary file name
}

SimulationSummaryData
SimulationResults::summaryData() const {
    // Return a copy of the stored summary data
    return m_summaryData;
}

QByteArray SimulationResults::trajectoryFileData() const {
    // Return a copy of the stored trajectory file data
    return m_trajectoryFileData;
}

QString SimulationResults::trajectoryFileName() const {
    // Return the full trajectory file name with path
    return m_trajectoryFileName;
}

QString SimulationResults::summaryFileName() const {
    // Return the full summary file name with path
    return m_summaryFileName;
}

QString SimulationResults::getTrajectoryFileName() const {
    // Split the file name by '/' to separate path
    // components
    QStringList parts = m_trajectoryFileName.split("/");

    // Return the last part (base name), or empty if none
    return parts.isEmpty() ? "" : parts.last();
}

QString SimulationResults::getSummaryFileName() const {
    // Split the file name by '/' to separate path
    // components
    QStringList parts = m_summaryFileName.split("/");

    // Return the last part (base name), or empty if none
    return parts.isEmpty() ? "" : parts.last();
}

} // namespace TrainClient
} // namespace Backend
} // namespace CargoNetSim
