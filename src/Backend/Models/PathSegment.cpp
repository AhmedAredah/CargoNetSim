#include "PathSegment.h"
#include <stdexcept>

namespace CargoNetSim
{
namespace Backend
{

// PathSegment constructor
PathSegment::PathSegment(
    const QString &pathSegmentId, const QString &start,
    const QString                          &end,
    TransportationTypes::TransportationMode mode,
    const QJsonObject &attributes, QObject *parent)
    : QObject(parent)
    , m_pathSegmentId(pathSegmentId)
    , m_start(start)
    , m_end(end)
    , m_mode(mode)
    , m_attributes(attributes)
{
    // Validate input parameters for construction
    if (pathSegmentId.isEmpty() || start.isEmpty()
        || end.isEmpty())
    {
        // Throw exception if any required field is missing
        throw std::invalid_argument(
            "Path segment parameters cannot be empty");
    }
    // No additional logging here; handled by caller if
    // needed
}

PathSegment::PathSegment(const QJsonObject &json,
                         QObject           *parent)
    : QObject(parent)
    , m_pathSegmentId("")
{
    // Extract fields from JSON
    if (json.contains("from") && json["from"].isString())
    {
        m_start = json["from"].toString();
    }
    else
    {
        throw std::invalid_argument(
            "Missing or invalid 'from' field in "
            "PathSegment JSON");
    }

    if (json.contains("to") && json["to"].isString())
    {
        m_end = json["to"].toString();
    }
    else
    {
        throw std::invalid_argument(
            "Missing or invalid 'to' field in PathSegment "
            "JSON");
    }

    if (json.contains("mode") && json["mode"].isDouble())
    {
        m_mode = static_cast<
            TransportationTypes::TransportationMode>(
            json["mode"].toInt());
    }
    else
    {
        throw std::invalid_argument(
            "Missing or invalid 'mode' field in "
            "PathSegment JSON");
    }

    // Construct an ID from endpoints and mode
    m_pathSegmentId =
        QString("%1_%2_%3")
            .arg(m_start, m_end,
                 QString::number(static_cast<int>(m_mode)));

    // Extract optional attributes
    if (json.contains("attributes")
        && json["attributes"].isObject())
    {
        m_attributes = json["attributes"].toObject();
    }

    // Extract weight if available
    if (json.contains("weight")
        && json["weight"].isDouble())
    {
        m_attributes["weight"] = json["weight"].toDouble();
    }
}

// Convert PathSegment to JSON
QJsonObject PathSegment::toJson() const
{
    // Initialize JSON object for serialization
    QJsonObject json;

    // Set mandatory fields required by server
    json["route_id"]       = m_pathSegmentId;
    json["start_terminal"] = m_start;
    json["end_terminal"]   = m_end;
    json["mode"] = TransportationTypes::toInt(m_mode);

    // Include attributes only if not empty
    if (!m_attributes.isEmpty())
    {
        json["attributes"] = m_attributes;
    }

    // Return constructed JSON object
    return json;
}

PathSegment *PathSegment::fromJson(const QJsonObject &json,
                                   QObject *parent)
{
    return new PathSegment(json, parent);
}

void PathSegment::setAttributes(
    const QJsonObject &attributes)
{
    // Set the attributes of the path segment
    m_attributes = attributes;
}

} // namespace Backend
} // namespace CargoNetSim
