#include "Route.h"

namespace CargoNetSim {
namespace Backend {

Route::Route(
    const QString& routeId,
    const QString& start,
    const QString& end,
    int mode,
    const QJsonObject& attributes,
    QObject* parent)
    : QObject(parent),
      m_routeId(routeId),
      m_start(start),
      m_end(end),
      m_mode(mode),
      m_attributes(attributes)
{
    if (routeId.isEmpty() || start.isEmpty() || end.isEmpty()) {
        throw std::invalid_argument("Route parameters cannot be empty");
    }
}

QJsonObject Route::toJson() const
{
    QJsonObject json;
    json["route_id"] = m_routeId;
    json["start_terminal"] = m_start;
    json["end_terminal"] = m_end;
    json["mode"] = m_mode;
    if (!m_attributes.isEmpty()) {
        json["attributes"] = m_attributes;
    }
    return json;
}

} // namespace Backend
} // namespace CargoNetSim
