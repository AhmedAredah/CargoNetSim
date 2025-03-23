#include "ShipSimulationClient.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QThread>

#include "Backend/Models/ShipSystem.h"
// #include "TerminalGraphServer.h"
// #include "SimulatorTimeServer.h"
// #include "ProgressBarManager.h"
// #include "ApplicationLogger.h"

namespace CargoNetSim
{
namespace Backend
{
namespace ShipClient
{

SimulationSummaryData::SimulationSummaryData(
    const QList<SummaryPair> &summaryData)
    : m_rawSummaryData(summaryData)
{
    m_parsedData = _parseSummaryData();
}

QVariantMap SimulationSummaryData::_parseSummaryData()
{
    QVariantMap parsed;
    QString     currentCategory;
    QString     currentSubcategory;

    for (const auto &pair : m_rawSummaryData)
    {
        QString summaryText = pair.first.trimmed();
        QString value       = pair.second;

        // Skip separator lines
        if (summaryText.isEmpty()
            || summaryText.startsWith("~.~")
            || summaryText.startsWith("..."))
        {
            continue;
        }

        // Handle new categories
        if (summaryText.startsWith('+'))
        {
            currentCategory = summaryText.mid(1).trimmed();
            currentCategory.remove(':');
            parsed[currentCategory] = QVariantMap();
            currentSubcategory.clear();
            continue;
        }

        // Handle new subcategories
        if (summaryText.startsWith("|->"))
        {
            currentSubcategory =
                summaryText.mid(3).trimmed();
            if (!currentCategory.isEmpty())
            {
                QVariantMap categoryMap =
                    parsed[currentCategory].toMap();
                categoryMap[currentSubcategory] =
                    QVariantMap();
                parsed[currentCategory] = categoryMap;
            }
            continue;
        }

        // Handle key-value pairs
        if (summaryText.startsWith("|_"))
        {
            QString key = summaryText.mid(2).trimmed();
            if (!currentCategory.isEmpty())
            {
                QVariantMap categoryMap =
                    parsed[currentCategory].toMap();

                if (!currentSubcategory.isEmpty())
                {
                    QVariantMap subcategoryMap =
                        categoryMap[currentSubcategory]
                            .toMap();
                    subcategoryMap[key] = value;
                    categoryMap[currentSubcategory] =
                        subcategoryMap;
                }
                else
                {
                    categoryMap[key] = value;
                }

                parsed[currentCategory] = categoryMap;
            }
        }
    }

    return parsed;
}

QMap<QString, QVariant> SimulationSummaryData::getCategory(
    const QString &category) const
{
    return m_parsedData.value(category).toMap();
}

QMap<QString, QVariant>
SimulationSummaryData::getSubcategory(
    const QString &category,
    const QString &subcategory) const
{
    QVariantMap categoryData = getCategory(category);
    return categoryData.value(subcategory).toMap();
}

QVariant
SimulationSummaryData::getValue(const QString &category,
                                const QString &subcategory,
                                const QString &key) const
{
    QVariantMap subcategoryData =
        getSubcategory(category, subcategory);
    return subcategoryData.value(key);
}

QStringList SimulationSummaryData::getAllCategories() const
{
    return m_parsedData.keys();
}

QMap<QString, QStringList>
SimulationSummaryData::getAllSubcategories(
    const QString &category) const
{
    QMap<QString, QStringList> result;

    if (category == "*")
    {
        for (auto it = m_parsedData.constBegin();
             it != m_parsedData.constEnd(); ++it)
        {
            QVariantMap categoryData = it.value().toMap();
            result[it.key()]         = categoryData.keys();
        }
    }
    else
    {
        QVariantMap categoryData =
            m_parsedData.value(category).toMap();
        result[category] = categoryData.keys();
    }

    return result;
}

QVariantMap SimulationSummaryData::info() const
{
    return m_parsedData;
}

} // namespace ShipClient
} // namespace Backend
} // namespace CargoNetSim
