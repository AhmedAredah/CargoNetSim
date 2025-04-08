#pragma once

/**
 * @file Path.h
 * @brief Defines the Path class for complete paths
 * @author Ahmed Aredah
 * @date March 21, 2025
 *
 * This file declares the Path class, representing a
 * complete transportation path with multiple segments in
 * the CargoNetSim simulation framework.
 *
 * @note Part of the CargoNetSim::Backend namespace.
 * @warning Manages PathSegment pointers; ensure proper
 * deletion.
 */

#include "PathSegment.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>

namespace CargoNetSim
{
namespace Backend
{

/**
 * @class Path
 * @brief Represents a complete transportation path
 *
 * This class encapsulates a full path, including its ID,
 * costs, terminals, and segments, used for path-finding
 * results in the simulation.
 *
 * @note Owns PathSegment pointers and deletes them on
 * destruction.
 */
class Path : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructs a Path instance
     * @param id Unique path identifier
     * @param totalCost Total cost of the path
     * @param edgeCost Sum of edge costs
     * @param termCost Sum of terminal costs
     * @param terminals List of terminal details
     * @param segments List of PathSegment pointers
     * @param parent Parent QObject, defaults to nullptr
     *
     * Creates a Path with specified properties and
     * segments.
     */
    explicit Path(int id, double totalCost, double edgeCost,
                  double                      termCost,
                  const QList<QJsonObject>   &terminals,
                  const QList<PathSegment *> &segments,
                  QObject *parent = nullptr);

    /**
     * @brief Constructs a Path from a JSON object
     * @param json JSON object containing path data
     * @param parent Parent QObject, defaults to nullptr
     * @throws std::invalid_argument If required fields are
     * missing
     *
     * Creates a Path by parsing a JSON object from the
     * server.
     */
    explicit Path(const QJsonObject &json,
                  QObject           *parent = nullptr);

    /**
     * @brief Creates a Path from a JSON object
     * @param json JSON object containing path data
     * @param parent Parent QObject, defaults to nullptr
     * @return New Path instance
     * @throws std::invalid_argument If required fields are
     * missing
     *
     * Static factory method to create a Path from JSON.
     */
    static Path *fromJson(const QJsonObject &json,
                          QObject *parent = nullptr);

    /**
     * @brief Destroys the Path, freeing segments
     *
     * Deletes all owned PathSegment pointers.
     */
    ~Path();

    /**
     * @brief Retrieves the path identifier
     * @return Unique path ID as integer
     *
     * Returns the ID assigned to this path.
     */
    int getPathId() const
    {
        return m_pathId;
    }

    /**
     * @brief Retrieves the total path cost
     * @return Total cost as double
     *
     * Returns the sum of edge and terminal costs.
     */
    double getTotalPathCost() const
    {
        return m_totalPathCost;
    }

    /**
     * @brief Sets the total path cost
     * @param cost New total cost to set
     * @throws std::invalid_argument If cost is negative
     *
     * Updates the total path cost with validation.
     */
    void setTotalPathCost(double cost);

    /**
     * @brief Retrieves the total edge costs
     * @return Sum of edge costs as double
     *
     * Returns the cumulative cost of path segments.
     */
    double getTotalEdgeCosts() const
    {
        return m_totalEdgeCosts;
    }

    /**
     * @brief Sets the total edge costs
     * @param cost New edge cost to set
     * @throws std::invalid_argument If cost is negative
     *
     * Updates the total edge costs with validation.
     */
    void setTotalEdgeCosts(double cost);

    /**
     * @brief Retrieves the total terminal costs
     * @return Sum of terminal costs as double
     *
     * Returns the cumulative cost at terminals.
     */
    double getTotalTerminalCosts() const
    {
        return m_totalTerminalCosts;
    }

    /**
     * @brief Sets the total terminal costs
     * @param cost New terminal cost to set
     * @throws std::invalid_argument If cost is negative
     *
     * Updates the total terminal costs with validation.
     */
    void setTotalTerminalCosts(double cost);

    /**
     * @brief Retrieves terminals in the path
     * @return List of terminal details as JSON objects
     *
     * Returns information about terminals in the path.
     */
    QList<QJsonObject> getTerminalsInPath() const
    {
        return m_terminalsInPath;
    }

    /**
     * @brief Retrieves path segments
     * @return List of PathSegment pointers
     *
     * Returns the segments composing this path.
     */
    QList<PathSegment *> getSegments() const
    {
        return m_segments;
    }

    /**
     * @brief Retrieves the starting terminal of the path
     * @return Starting terminal identifier as QString
     * @throws std::runtime_error If path has no segments
     *      * Returns the ID of the first terminal in the
     * path.
     */
    QString getStartTerminal() const;

    /**
     * @brief Retrieves the ending terminal of the path
     * @return Ending terminal identifier as QString
     * @throws std::runtime_error If path has no segments
     *      * Returns the ID of the last terminal in the
     * path.
     */
    QString getEndTerminal() const;

    /**
     * @brief Converts the path to JSON format
     * @return QJsonObject representing the path
     *
     * Serializes the path for server communication.
     */
    QJsonObject toJson() const;

private:
    /**
     * @brief Unique identifier for the path
     */
    int m_pathId;

    /**
     * @brief Total cost of the path
     */
    double m_totalPathCost;

    /**
     * @brief Total cost of path edges
     */
    double m_totalEdgeCosts;

    /**
     * @brief Total cost at terminals
     */
    double m_totalTerminalCosts;

    /**
     * @brief List of terminal details in the path
     */
    QList<QJsonObject> m_terminalsInPath;

    /**
     * @brief List of segments composing the path
     */
    QList<PathSegment *> m_segments;
};

} // namespace Backend
} // namespace CargoNetSim
