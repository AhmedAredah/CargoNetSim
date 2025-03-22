/**
 * @file SimulationSummaryData.cpp
 * @brief Implements truck simulation summary data
 * @author Ahmed Aredah
 * @date March 21, 2025
 */

#include "SimulationSummaryData.h"

namespace CargoNetSim {
namespace Backend {
namespace TruckClient {

SimulationSummaryData::SimulationSummaryData(
    const QList<SummaryPair> &summaryData, QObject *parent)
    : QObject(parent)
    , m_rawSummaryData(summaryData) {
    m_parsedData = parseSummaryData();
}

QMap<QString, QVariant> SimulationSummaryData::getCategory(
    const QString &category) const {
    return m_parsedData.value(category).toMap();
}

QMap<QString, QVariant>
SimulationSummaryData::getSubcategory(
    const QString &category,
    const QString &subcategory) const {
    QVariantMap categoryData = getCategory(category);
    return categoryData.value(subcategory).toMap();
}

QVariant
SimulationSummaryData::getValue(const QString &category,
                                const QString &subcategory,
                                const QString &key) const {
    QVariantMap subcategoryData =
        getSubcategory(category, subcategory);
    return subcategoryData.value(key);
}

QStringList
SimulationSummaryData::getAllCategories() const {
    return m_parsedData.keys();
}

QMap<QString, QStringList>
SimulationSummaryData::getAllSubcategories(
    const QString &category) const {
    QMap<QString, QStringList> result;
    if (category == "*") {
        for (auto it = m_parsedData.constBegin();
             it != m_parsedData.constEnd(); ++it) {
            QVariantMap categoryData = it.value().toMap();
            result[it.key()]         = categoryData.keys();
        }
    } else {
        QVariantMap categoryData =
            m_parsedData.value(category).toMap();
        result[category] = categoryData.keys();
    }
    return result;
}

QVariantMap SimulationSummaryData::info() const {
    return m_parsedData;
}

QVariantMap SimulationSummaryData::parseSummaryData() {
    QVariantMap parsed;
    QString     currentCategory;
    QString     currentSubcategory;

    for (const auto &pair : m_rawSummaryData) {
        QString summaryText = pair.first.trimmed();
        QString value       = pair.second;

        if (summaryText.isEmpty()
            || summaryText.startsWith("~.~")
            || summaryText.startsWith("...")) {
            continue;
        }

        if (summaryText.startsWith('+')) {
            currentCategory = summaryText.mid(1).trimmed();
            currentCategory.remove(':');
            parsed[currentCategory] = QVariantMap();
            currentSubcategory.clear();
            continue;
        }

        if (summaryText.startsWith("|->")) {
            currentSubcategory =
                summaryText.mid(3).trimmed();
            if (!currentCategory.isEmpty()) {
                QVariantMap categoryMap =
                    parsed[currentCategory].toMap();
                categoryMap[currentSubcategory] =
                    QVariantMap();
                parsed[currentCategory] = categoryMap;
            }
            continue;
        }

        if (summaryText.startsWith("|_")) {
            QString key = summaryText.mid(2).trimmed();
            if (!currentCategory.isEmpty()) {
                QVariantMap categoryMap =
                    parsed[currentCategory].toMap();
                if (!currentSubcategory.isEmpty()) {
                    QVariantMap subMap =
                        categoryMap[currentSubcategory]
                            .toMap();
                    subMap[key] = value;
                    categoryMap[currentSubcategory] =
                        subMap;
                } else {
                    categoryMap[key] = value;
                }
                parsed[currentCategory] = categoryMap;
            }
        }
    }

    return parsed;
}

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
