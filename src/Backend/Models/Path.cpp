#include "Path.h"

namespace CargoNetSim {
namespace Backend {

// Path constructor
Path::Path(int id, double totalCost, double edgeCost,
           double                      termCost,
           const QList<QJsonObject>   &terminals,
           const QList<PathSegment *> &segments,
           QObject                    *parent)
    : QObject(parent)
    , m_pathId(id)
    , m_totalPathCost(totalCost)
    , m_totalEdgeCosts(edgeCost)
    , m_totalTerminalCosts(termCost)
    , m_terminalsInPath(terminals)
    , m_segments(segments) {
    // Validate input parameters
    if (id <= 0) {
        // Ensure path ID is positive
        throw std::invalid_argument(
            "Path ID must be positive");
    }
    if (totalCost < 0 || edgeCost < 0 || termCost < 0) {
        // Ensure costs are non-negative
        throw std::invalid_argument(
            "Costs must be non-negative");
    }
    if (segments.isEmpty()) {
        // Ensure at least one segment exists
        throw std::invalid_argument(
            "Path must have segments");
    }

    // Verify terminal list consistency with segments
    int expectedTerminals = segments.size() + 1;
    if (terminals.size() != expectedTerminals) {
        qWarning() << "Terminal count" << terminals.size()
                   << "does not match expected"
                   << expectedTerminals;
    }

    // No ownership transfer; segments managed by this
    // instance
}

// Path destructor
Path::~Path() {
    // Iterate over all segments in the path
    for (PathSegment *segment : m_segments) {
        // Delete each segment to free memory
        delete segment;
        // Nullify pointer for safety (not required but
        // explicit)
        segment = nullptr;
    }
    // Clear the list to ensure no dangling pointers
    m_segments.clear();
}

} // namespace Backend
} // namespace CargoNetSim
