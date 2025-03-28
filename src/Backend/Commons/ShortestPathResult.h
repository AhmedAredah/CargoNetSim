/**
 * @file ShortestPathResult.h
 * @brief Defines common structure for shortest path results
 * @author Ahmed Aredah
 * @date 2025-03-28
 */

#pragma once

#include <QString>
#include <QVector>

namespace CargoNetSim
{
namespace Backend
{

/**
 * @struct ShortestPathResult
 * @brief Common structure to hold shortest path calculation
 * results
 *
 * This structure is shared between train and truck network
 * classes to provide consistent path finding result
 * representation.
 */
struct ShortestPathResult
{
    QVector<int> pathNodes;        // Node IDs in the path
    QVector<int> pathLinks;        // Link IDs in the path
    double       totalLength;      // Total path length
    double       minTravelTime;    // Minimum travel time
    QString optimizationCriterion; // Criterion used for
                                   // optimization

    // Default constructor with empty path and infinite
    // values
    ShortestPathResult()
        : totalLength(
              std::numeric_limits<double>::infinity())
        , minTravelTime(
              std::numeric_limits<double>::infinity())
        , optimizationCriterion("distance")
    {
    }
};

} // namespace Backend
} // namespace CargoNetSim
