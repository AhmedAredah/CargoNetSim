#pragma once

#include <QMap>
#include <QString>
#include <QVariant>
#include <QVector>

/**
 * @file SimulationSummaryData.h
 * @brief Header file for the SimulationSummaryData class
 * @author Ahmed Aredah
 * @date March 20, 2025
 *
 * This file declares the SimulationSummaryData class, which is responsible for
 * managing and structuring simulation summary data within the CargoNetSim
 * backend. It provides a hierarchical structure for categories and
 * subcategories, parsed from a list of text-value pairs.
 */

namespace CargoNetSim {
namespace Backend {
namespace TrainClient {

/**
 * @class SimulationSummaryData
 * @brief Manages structured simulation summary data
 *
 * This class parses a list of summary text-value pairs into a hierarchical
 * structure with categories and subcategories. It provides methods to access
 * specific data points within this structure.
 *
 * The input data is expected to be a list of pairs, where each pair contains
 * a string (the summary text) and another string (the value, if applicable).
 * The summary text is used to determine the structure:
 * - Lines starting with '+' indicate a new category.
 * - Lines starting with '|->' indicate a new subcategory within
 *      the current category.
 * - Lines starting with '|_ ' indicate a key-value pair within the
 *      current subcategory or category.
 *
 * Example input:
 *   + Category1:
 *   |-> Subcategory1
 *   |_ Key1: Value1
 *   |_ Key2: Value2
 *   + Category2:
 *   |_ Key3: Value3
 *
 * This would be parsed into a structure where "Category1" has a
 * subcategory "Subcategory1" containing "Key1" and "Key2", and
 * "Category2" directly contains "Key3".
 */
class SimulationSummaryData {
public:
    /**
     * @brief Constructor
     * @param summaryData List of summary text-value pairs to
     *        initialize the structure
     *
     * Constructs an instance of SimulationSummaryData and parses the provided
     * summary data into a hierarchical structure.
     */
    explicit SimulationSummaryData(
        const QVector<QPair<QString, QString>>& summaryData = {});

    /**
     * @brief Retrieve a specific category
     * @param category The name of the category to retrieve
     * @return A map representing the category's data, or an
     *         empty map if not found
     *
     * Accesses all data stored under the specified category name.
     */
    QMap<QString, QVariant> getCategory(const QString& category) const;

    /**
     * @brief Retrieve a specific subcategory within a category
     * @param category The name of the category containing the subcategory
     * @param subcategory The name of the subcategory to retrieve
     * @return A map representing the subcategory's data, or an empty
     *         map if not found
     *
     * Accesses all data stored under the specified subcategory
     * within a category.
     */
    QMap<QString, QVariant> getSubcategory(
        const QString& category,
        const QString& subcategory) const;

    /**
     * @brief Retrieve a specific value by category, subcategory, and key
     * @param category The name of the category
     * @param subcategory The name of the subcategory
     * @param key The key associated with the desired value
     * @return The value associated with the key, or an invalid QVariant
     *         if not found
     *
     * Provides direct access to a specific value within the
     * hierarchical structure.
     */
    QVariant getValue(
        const QString& category,
        const QString& subcategory,
        const QString& key) const;

    /**
     * @brief Retrieve all category names
     * @return A list of all category names in the parsed data
     *
     * Returns a list of top-level category names for navigation or inspection.
     */
    QStringList getAllCategories() const;

    /**
     * @brief Retrieve all subcategory names for a given category
     * @param category The category name, or "*" to retrieve subcategories
     *        for all categories
     * @return A map of category names to their subcategory lists
     *
     * Provides a way to explore the structure by listing subcategories,
     * either for a specific category or across all categories.
     */
    QMap<QString, QStringList> getAllSubcategories(
        const QString& category = "*") const;

    /**
     * @brief Retrieve the entire parsed data structure
     * @return A map representing the entire summary data
     *
     * Returns the complete hierarchical structure for debugging or
     * full data access.
     */
    QMap<QString, QVariant> info() const;

private:
    /**
     * @brief Parsed summary data
     *
     * A map where keys are category names, and values are either sub-maps (for
     * subcategories) or direct key-value pairs, storing the hierarchical
     * structure.
     */
    QMap<QString, QVariant> m_parsedData;

    /**
     * @brief Parse the input summary data into the structured format
     * @param summaryData The raw summary data to parse
     *
     * Internal method to process the input text-value pairs and build the
     * hierarchical data structure stored in m_parsedData.
     */
    void parseSummaryData(
        const QVector<QPair<QString, QString>>& summaryData);
};

} // namespace TrainClient
} // namespace Backend
} // namespace CargoNetSim

Q_DECLARE_METATYPE(CargoNetSim::Backend::TrainClient::SimulationSummaryData)
Q_DECLARE_METATYPE(CargoNetSim::Backend::TrainClient::SimulationSummaryData*)

