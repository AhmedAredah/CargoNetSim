#pragma once

/**
 * @file PathSegment.h
 * @brief Defines the PathSegment class for path segments
 * @author Ahmed Aredah
 * @date March 21, 2025
 *
 * This file declares the PathSegment class, representing a
 * segment of a transportation path between two terminals in
 * the CargoNetSim simulation framework.
 *
 * @note Part of the CargoNetSim::Backend namespace.
 * @warning Instances should be managed by Path or caller.
 */

#include "Backend/Commons/TransportationMode.h"
#include <QJsonObject>
#include <QObject>
#include <QString>

namespace CargoNetSim
{
namespace Backend
{

/**
 * @class PathSegment
 * @brief Represents a single segment in a transportation
 * path
 *
 * This class encapsulates details of a route between two
 * terminals, including its identifier, endpoints, mode, and
 * attributes, used for defining static routes or dynamic
 * path segments in the simulation.
 *
 * @note Inherits QObject for signal-slot functionality.
 */
class PathSegment : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a PathSegment instance
     * @param pathSegmentId Unique identifier for the
     * segment
     * @param start Starting terminal identifier
     * @param end Ending terminal identifier
     * @param mode Transportation mode as integer
     * @param attributes Optional attributes as JSON object
     * @param parent Parent QObject, defaults to nullptr
     * @throws std::invalid_argument If required fields are
     * empty
     *
     * Creates a PathSegment with specified properties.
     */
    explicit PathSegment(
        const QString &pathSegmentId, const QString &start,
        const QString                          &end,
        TransportationTypes::TransportationMode mode,
        const QJsonObject &attributes = QJsonObject(),
        QObject           *parent     = nullptr);

    /**
     * @brief Retrieves the path segment identifier
     * @return Unique identifier as QString
     *
     * Returns the unique ID assigned to this segment.
     */
    QString getPathSegmentId() const
    {
        return m_pathSegmentId;
    }

    /**
     * @brief Retrieves the starting terminal
     * @return Starting terminal identifier as QString
     *
     * Returns the ID of the segment's starting terminal.
     */
    QString getStart() const
    {
        return m_start;
    }

    /**
     * @brief Retrieves the ending terminal
     * @return Ending terminal identifier as QString
     *
     * Returns the ID of the segment's ending terminal.
     */
    QString getEnd() const
    {
        return m_end;
    }

    /**
     * @brief Retrieves the transportation mode
     * @return Mode as integer (enum value)
     *
     * Returns the transportation mode used in this segment.
     */
    TransportationTypes::TransportationMode getMode() const
    {
        return m_mode;
    }

    /**
     * @brief Retrieves the segment attributes
     * @return Attributes as QJsonObject
     *
     * Returns additional properties of the segment.
     */
    QJsonObject getAttributes() const
    {
        return m_attributes;
    }

    /**
     * @brief Converts the segment to JSON format
     * @return QJsonObject representing the segment
     *
     * Serializes the segment for server communication.
     */
    QJsonObject toJson() const;

private:
    /**
     * @brief Unique identifier for the path segment
     */
    QString m_pathSegmentId;

    /**
     * @brief Identifier of the starting terminal
     */
    QString m_start;

    /**
     * @brief Identifier of the ending terminal
     */
    QString m_end;

    /**
     * @brief Transportation mode for the segment
     */
    TransportationTypes::TransportationMode m_mode;

    /**
     * @brief Additional attributes of the segment
     */
    QJsonObject m_attributes;
};

} // namespace Backend
} // namespace CargoNetSim
