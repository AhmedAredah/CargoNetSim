#include "PathSegment.h"
#include <stdexcept>

namespace CargoNetSim {
namespace Backend {

// PathSegment constructor
PathSegment::PathSegment(
    const QString& pathSegmentId,
    const QString& start,
    const QString& end,
    int mode,
    const QJsonObject& attributes,
    QObject* parent)
    : QObject(parent),
    m_pathSegmentId(pathSegmentId),
    m_start(start),
    m_end(end),
    m_mode(mode),
    m_attributes(attributes)
{
    // Validate input parameters for construction
    if (pathSegmentId.isEmpty() || start.isEmpty() || end.isEmpty()) {
        // Throw exception if any required field is missing
        throw std::invalid_argument(
            "Path segment parameters cannot be empty");
    }
    // No additional logging here; handled by caller if needed
}

// Convert PathSegment to JSON
QJsonObject PathSegment::toJson() const
{
    // Initialize JSON object for serialization
    QJsonObject json;

    // Set mandatory fields required by server
    json["route_id"] = m_pathSegmentId;
    json["start_terminal"] = m_start;
    json["end_terminal"] = m_end;
    json["mode"] = m_mode;

    // Include attributes only if not empty
    if (!m_attributes.isEmpty()) {
        json["attributes"] = m_attributes;
    }

    // Return constructed JSON object
    return json;
}

} // namespace Backend
} // namespace CargoNetSim
