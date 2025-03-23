/**
 * @file SimulationSummaryData.h
 * @brief Parses and stores truck simulation summary
 * @author Ahmed Aredah
 * @date March 21, 2025
 */

#pragma once

#include <QMap>
#include <QObject>
#include <QStringList>
#include <QVariant>

namespace CargoNetSim
{
namespace Backend
{
namespace TruckClient
{

/**
 * @class SimulationSummaryData
 * @brief Manages truck simulation summary data
 */
class SimulationSummaryData : public QObject
{
    Q_OBJECT

public:
    using SummaryPair = QPair<QString, QString>;

    explicit SimulationSummaryData(
        const QList<SummaryPair> &summaryData = {},
        QObject                  *parent      = nullptr);
    ~SimulationSummaryData() override = default;

    QMap<QString, QVariant>
    getCategory(const QString &category) const;
    QMap<QString, QVariant>
             getSubcategory(const QString &category,
                            const QString &subcategory) const;
    QVariant getValue(const QString &category,
                      const QString &subcategory,
                      const QString &key) const;

    QStringList                getAllCategories() const;
    QMap<QString, QStringList> getAllSubcategories(
        const QString &category = "*") const;

    QVariantMap info() const;

private:
    QVariantMap parseSummaryData();

    QList<SummaryPair> m_rawSummaryData;
    QVariantMap        m_parsedData;
};

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim

Q_DECLARE_METATYPE(CargoNetSim::Backend::TruckClient::
                       SimulationSummaryData)
Q_DECLARE_METATYPE(
    CargoNetSim::Backend::TruckClient::SimulationSummaryData
        *)
