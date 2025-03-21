#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QString>
#include "SimulationSummaryData.h"

/**
 * @file SimulationResults.h
 * @brief Header for SimulationResults class
 * @author Ahmed Aredah
 * @date March 20, 2025
 *
 * Declares the SimulationResults class, which manages simulation
 * results including summary data, trajectory files, and metadata
 * in the CargoNetSim backend.
 */

namespace CargoNetSim {
namespace Backend {
namespace TrainClient {

/**
 * @class SimulationResults
 * @brief Holds simulation results including summary and files
 *
 * Stores simulation summary data, trajectory file data, and file
 * names. Provides access methods and a static method to create
 * an instance from JSON data. This class is used to encapsulate
 * the output of a simulation run.
 */
class SimulationResults {
public:
    /**
     * @brief Constructor
     * @param summaryData List of summary text-value pairs
     * @param trajectoryFileData Raw data of the trajectory file
     * @param trajectoryFileName Name of the trajectory file
     * @param summaryFileName Name of the summary file
     *
     * Initializes a SimulationResults object with provided data.
     */
    explicit SimulationResults(
        const QVector<QPair<QString, QString>>& summaryData = {},
        const QByteArray& trajectoryFileData = {},
        const QString& trajectoryFileName = {},
        const QString& summaryFileName = {});

    /**
     * @brief Create instance from JSON
     * @param jsonObj JSON object containing simulation results
     * @return A new SimulationResults instance
     *
     * Static method to parse a JSON object into a SimulationResults
     * object, handling summary data and file information.
     */
    static SimulationResults fromJson(const QJsonObject& jsonObj);

    /**
     * @brief Get summary data
     * @return The SimulationSummaryData object
     *
     * Returns the structured summary data of the simulation.
     */
    SimulationSummaryData summaryData() const;

    /**
     * @brief Get trajectory file data
     * @return Raw byte data of the trajectory file
     *
     * Returns the binary content of the trajectory file.
     */
    QByteArray trajectoryFileData() const;

    /**
     * @brief Get trajectory file name
     * @return Full trajectory file name with path
     *
     * Returns the complete name (including path) of the trajectory file.
     */
    QString trajectoryFileName() const;

    /**
     * @brief Get summary file name
     * @return Full summary file name with path
     *
     * Returns the complete name (including path) of the summary file.
     */
    QString summaryFileName() const;

    /**
     * @brief Get trajectory file base name
     * @return Base name of the trajectory file
     *
     * Extracts and returns the file name without the path.
     */
    QString getTrajectoryFileName() const;

    /**
     * @brief Get summary file base name
     * @return Base name of the summary file
     *
     * Extracts and returns the file name without the path.
     */
    QString getSummaryFileName() const;

private:
    /**
     * @brief Structured summary data
     *
     * Holds the parsed summary data in a hierarchical structure.
     */
    SimulationSummaryData m_summaryData;

    /**
     * @brief Trajectory file raw data
     *
     * Stores the binary content of the trajectory file.
     */
    QByteArray m_trajectoryFileData;

    /**
     * @brief Trajectory file name with path
     *
     * Contains the full name (including path) of the trajectory file.
     */
    QString m_trajectoryFileName;

    /**
     * @brief Summary file name with path
     *
     * Contains the full name (including path) of the summary file.
     */
    QString m_summaryFileName;
};

} // namespace TrainClient
} // namespace Backend
} // namespace CargoNetSim

Q_DECLARE_METATYPE(CargoNetSim::Backend::TrainClient::SimulationResults)
Q_DECLARE_METATYPE(CargoNetSim::Backend::TrainClient::SimulationResults*)