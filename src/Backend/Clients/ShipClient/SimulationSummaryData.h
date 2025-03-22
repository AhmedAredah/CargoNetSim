#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QThread>
#include <QWaitCondition>

#include "Backend/Clients/BaseClient/SimulationClientBase.h"
#include "Backend/Commons/ClientType.h"

namespace CargoNetSim {
namespace Backend {
namespace ShipClient {

/**
 * @brief Stores and parses simulation summary data
 */
class SimulationSummaryData {
public:
    using SummaryPair = QPair<QString, QString>;

    /**
     * @brief Constructor with raw summary data
     * @param summaryData List of text/value pairs
     */
    explicit SimulationSummaryData(
        const QList<SummaryPair> &summaryData = {});

    /**
     * @brief Get data for a specific category
     * @param category Category name
     * @return Map of key-value pairs for that category
     */
    QMap<QString, QVariant>
    getCategory(const QString &category) const;

    /**
     * @brief Get data for a specific subcategory
     * @param category Main category name
     * @param subcategory Subcategory name
     * @return Map of key-value pairs for that subcategory
     */
    QMap<QString, QVariant>
    getSubcategory(const QString &category,
                   const QString &subcategory) const;

    /**
     * @brief Get a specific value
     * @param category Main category
     * @param subcategory Subcategory
     * @param key Key name
     * @return Value if found, empty QVariant otherwise
     */
    QVariant getValue(const QString &category,
                      const QString &subcategory,
                      const QString &key) const;

    /**
     * @brief Get all category names
     * @return List of category names
     */
    QStringList getAllCategories() const;

    /**
     * @brief Get all subcategory names
     * @param category Category to get subcategories for,
     *                 or "*" for all
     * @return Map of categories to subcategory lists
     */
    QMap<QString, QStringList> getAllSubcategories(
        const QString &category = "*") const;

    /**
     * @brief Get all parsed data
     * @return Structured data as nested maps
     */
    QVariantMap info() const;

private:
    /**
     * @brief Parse raw summary data into structured format
     * @return Nested map of parsed data
     */
    QVariantMap _parseSummaryData();

    QList<SummaryPair> m_rawSummaryData;
    QVariantMap        m_parsedData;
};

} // namespace ShipClient
} // namespace Backend
} // namespace CargoNetSim

Q_DECLARE_METATYPE(
    CargoNetSim::Backend::ShipClient::SimulationSummaryData)
Q_DECLARE_METATYPE(
    CargoNetSim::Backend::ShipClient::SimulationSummaryData
        *)
