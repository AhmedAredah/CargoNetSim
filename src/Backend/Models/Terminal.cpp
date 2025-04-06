#include "Terminal.h"
#include <QJsonArray>

namespace CargoNetSim
{
namespace Backend
{

Terminal::Terminal(
    const QStringList &names, const QJsonObject &config,
    const QMap<
        TerminalTypes::TerminalInterface,
        QSet<TransportationTypes::TransportationMode>>
                  &interfaces,
    const QString &region, QObject *parent)
    : QObject(parent)
    , m_names(names)
    , m_config(config)
    , m_interfaces(interfaces)
    , m_region(region)
{
    if (names.isEmpty())
    {
        throw std::invalid_argument(
            "Names list cannot be empty");
    }
}

QJsonObject Terminal::toJson() const
{
    QJsonObject json;
    json["terminal_names"] =
        QJsonArray::fromStringList(m_names);
    json["custom_config"] = m_config;
    QJsonObject interfacesJson;
    for (auto it = m_interfaces.constBegin();
         it != m_interfaces.constEnd(); ++it)
    {
        QJsonArray modesArray;
        for (TransportationTypes::TransportationMode mode :
             it.value())
        {
            modesArray.append(static_cast<int>(mode));
        }
        interfacesJson[QString::number(
            static_cast<int>(it.key()))] = modesArray;
    }
    json["terminal_interfaces"] = interfacesJson;

    if (!m_region.isEmpty())
    {
        json["region"] = m_region;
    }
    return json;
}

} // namespace Backend
} // namespace CargoNetSim
