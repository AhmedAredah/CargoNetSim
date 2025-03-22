#include "Terminal.h"
#include <QJsonArray>

namespace CargoNetSim {
namespace Backend {

Terminal::Terminal(const QStringList          &names,
                   const QJsonObject          &config,
                   const QMap<int, QSet<int>> &interfaces,
                   const QString &region, QObject *parent)
    : QObject(parent)
    , m_names(names)
    , m_config(config)
    , m_interfaces(interfaces)
    , m_region(region) {
    if (names.isEmpty()) {
        throw std::invalid_argument(
            "Names list cannot be empty");
    }
}

QJsonObject Terminal::toJson() const {
    QJsonObject json;
    json["terminal_names"] =
        QJsonArray::fromStringList(m_names);
    json["custom_config"] = m_config;
    QJsonObject interfacesObj;
    for (auto it = m_interfaces.constBegin();
         it != m_interfaces.constEnd(); ++it) {
        QJsonArray modes;
        for (int mode : it.value()) {
            modes.append(mode);
        }
        interfacesObj[QString::number(it.key())] = modes;
    }
    json["terminal_interfaces"] = interfacesObj;
    if (!m_region.isEmpty()) {
        json["region"] = m_region;
    }
    return json;
}

} // namespace Backend
} // namespace CargoNetSim
