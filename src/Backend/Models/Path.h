#pragma once

/**
 * @file Path.h
 * @brief Defines the Path class for complete paths
 * @author [Your Name]
 * @date March 21, 2025
 *
 * This file declares the Path class, representing a complete
 * transportation path with multiple segments in the
 * CargoNetSim simulation framework.
 *
 * @note Part of the CargoNetSim::Backend namespace.
 * @warning Manages PathSegment pointers; ensure proper deletion.
 */

#include <QObject>
#include <QJsonObject>
#include <QString>
#include "PathSegment.h"

namespace CargoNetSim {
namespace Backend {

/**
 * @class Path
 * @brief Represents a complete transportation path
 *
 * This class encapsulates a full path, including its ID, costs,
 * terminals, and segments, used for path-finding results in
 * the simulation.
 *
 * @note Owns PathSegment pointers and deletes them on destruction.
 */
class Path : public QObject {
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
     * Creates a Path with specified properties and segments.
     */
    explicit Path(
        int id,
        double totalCost,
        double edgeCost,
        double termCost,
        const QList<QJsonObject>& terminals,
        const QList<PathSegment*>& segments,
        QObject* parent = nullptr);

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
    int getPathId() const { return m_pathId; }

    /**
     * @brief Retrieves the total path cost
     * @return Total cost as double
     *
     * Returns the sum of edge and terminal costs.
     */
    double getTotalPathCost() const { return m_totalPathCost; }

    /**
     * @brief Retrieves the total edge costs
     * @return Sum of edge costs as double
     *
     * Returns the cumulative cost of path segments.
     */
    double getTotalEdgeCosts() const { return m_totalEdgeCosts; }

    /**
     * @brief Retrieves the total terminal costs
     * @return Sum of terminal costs as double
     *
     * Returns the cumulative cost at terminals.
     */
    double getTotalTerminalCosts() const { return m_totalTerminalCosts; }

    /**
     * @brief Retrieves terminals in the path
     * @return List of terminal details as JSON objects
     *
     * Returns information about terminals in the path.
     */
    QList<QJsonObject> getTerminalsInPath() const { return m_terminalsInPath; }

    /**
     * @brief Retrieves path segments
     * @return List of PathSegment pointers
     *
     * Returns the segments composing this path.
     */
    QList<PathSegment*> getSegments() const { return m_segments; }

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
    QList<PathSegment*> m_segments;
};

} // namespace Backend
} // namespace CargoNetSim
