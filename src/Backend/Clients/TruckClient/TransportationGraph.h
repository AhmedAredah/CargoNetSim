/**
 * @file TransportationGraph.h
 * @brief Enhanced directed graph for transportation
 * networks
 * @author [Your Name]
 * @date 2025-03-22
 */

#pragma once

#include "Backend/Commons/DirectedGraph.h"
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QString>
#include <QVector>
#include <functional>
#include <queue>
#include <set>

namespace CargoNetSim {
namespace Backend {
namespace TruckClient {

/**
 * @class TransportationGraph
 * @brief Specialized graph for transportation networks
 *
 * Extends the DirectedGraph with transportation-specific
 * features such as traffic modeling, vehicle routing, and
 * network metrics.
 */
template <typename T>
class TransportationGraph : public DirectedGraph<T> {
public:
    /**
     * @brief Constructor
     * @param parent The parent QObject
     */
    explicit TransportationGraph(QObject *parent = nullptr);

    /**
     * @brief Destructor
     */
    virtual ~TransportationGraph();

    /**
     * @brief Finds path with edge constraints
     * @param startNodeId Starting node identifier
     * @param endNodeId Ending node identifier
     * @param edgeFilter Function that returns true for
     * valid edges
     * @return Vector of node identifiers representing the
     * path
     */
    QVector<T> findPathWithConstraints(
        const T &startNodeId, const T &endNodeId,
        const std::function<bool(const T &, const T &)>
            &edgeFilter) const;

    /**
     * @brief Calculates traffic congestion on an edge
     * @param fromNodeId Source node identifier
     * @param toNodeId Target node identifier
     * @return Congestion factor (1.0 = no congestion)
     */
    float calculateCongestion(const T &fromNodeId,
                              const T &toNodeId) const;

    /**
     * @brief Adds traffic to an edge
     * @param fromNodeId Source node identifier
     * @param toNodeId Target node identifier
     * @param vehicleCount Number of vehicles to add
     */
    void addTraffic(const T &fromNodeId, const T &toNodeId,
                    int vehicleCount = 1);

    /**
     * @brief Removes traffic from an edge
     * @param fromNodeId Source node identifier
     * @param toNodeId Target node identifier
     * @param vehicleCount Number of vehicles to remove
     */
    void removeTraffic(const T &fromNodeId,
                       const T &toNodeId,
                       int      vehicleCount = 1);

    /**
     * @brief Calculates path metric based on edge
     * attributes
     * @param path Vector of node identifiers
     * @param metricName Name of the metric to calculate
     * @return Total metric value for the path
     */
    double
    calculatePathMetric(const QVector<T> &path,
                        const QString    &metricName) const;

    /**
     * @brief Finds the k shortest paths between nodes
     * @param startNodeId Starting node identifier
     * @param endNodeId Ending node identifier
     * @param k Maximum number of paths to find
     * @return List of paths sorted by length
     */
    QList<QVector<T>>
    findKShortestPaths(const T &startNodeId,
                       const T &endNodeId, int k) const;

    /**
     * @brief Converts a node path to a link path
     * @param nodePath Vector of node identifiers
     * @return Vector of link identifiers
     */
    QVector<int> convertNodePathToLinkPath(
        const QVector<T> &nodePath) const;

    /**
     * @brief Gets transportation mode for a link
     * @param linkId Link identifier
     * @return Transportation mode identifier
     */
    int getLinkTransportationMode(int linkId) const;

    /**
     * @brief Sets transportation mode for a link
     * @param linkId Link identifier
     * @param modeId Transportation mode identifier
     */
    void setLinkTransportationMode(int linkId, int modeId);

private:
    // Traffic data by edge (fromNode, toNode)
    QMap<QPair<T, T>, int> m_trafficMap;

    // Map of link IDs to transportation modes
    QMap<int, int> m_linkModes;

    // Implementation of Yen's algorithm for k shortest
    // paths
    QList<QVector<T>> yenKSP(const T &startNodeId,
                             const T &endNodeId,
                             int      k) const;
};

// Include implementation
template <typename T>
TransportationGraph<T>::TransportationGraph(QObject *parent)
    : DirectedGraph<T>(parent) {}

template <typename T>
TransportationGraph<T>::~TransportationGraph() {}

template <typename T>
QVector<T> TransportationGraph<T>::findPathWithConstraints(
    const T &startNodeId, const T &endNodeId,
    const std::function<bool(const T &, const T &)>
        &edgeFilter) const {
    // Validate inputs
    if (!this->hasNode(startNodeId)
        || !this->hasNode(endNodeId)) {
        return QVector<T>();
    }

    // Use Dijkstra's algorithm with edge filtering
    std::map<T, float> distances;
    std::map<T, T>     predecessors;
    std::set<T>        visited;

    // Priority queue for Dijkstra's algorithm
    std::priority_queue<std::pair<float, T>,
                        std::vector<std::pair<float, T>>,
                        std::greater<std::pair<float, T>>>
        pq;

    // Initialize distances and predecessors
    for (const T &node : this->getNodes()) {
        distances[node] =
            std::numeric_limits<float>::infinity();
        predecessors[node] =
            node; // Self-reference for unconnected
    }

    // Start with the starting node
    distances[startNodeId] = 0.0f;
    pq.push(std::make_pair(0.0f, startNodeId));

    // Process nodes in order of distance
    while (!pq.empty()) {
        float currentDist = pq.top().first;
        T     currentNode = pq.top().second;
        pq.pop();

        // Skip if already visited or not the best path
        if (visited.count(currentNode) > 0
            || currentDist > distances[currentNode]) {
            continue;
        }

        // Mark as visited
        visited.insert(currentNode);

        // Destination reached?
        if (currentNode == endNodeId) {
            break;
        }

        // Check all neighbors
        for (const auto &edge :
             this->getOutgoingEdges(currentNode)) {
            T     neighbor = edge.first;
            float weight   = edge.second;

            // Skip if already visited
            if (visited.count(neighbor) > 0) {
                continue;
            }

            // Apply edge filter
            if (!edgeFilter(currentNode, neighbor)) {
                continue;
            }

            // Calculate new distance
            float newDist = distances[currentNode] + weight;

            // Update if better path found
            if (newDist < distances[neighbor]) {
                distances[neighbor]    = newDist;
                predecessors[neighbor] = currentNode;
                pq.push(std::make_pair(newDist, neighbor));
            }
        }
    }

    // Reconstruct path
    QVector<T> path;

    // Check if a path exists
    if (predecessors[endNodeId] == endNodeId
        && endNodeId != startNodeId) {
        // No path found
        return path;
    }

    // Build path by following predecessors
    T current = endNodeId;
    while (current != startNodeId) {
        path.prepend(current);
        current = predecessors[current];
    }
    path.prepend(startNodeId);

    return path;
}

template <typename T>
float TransportationGraph<T>::calculateCongestion(
    const T &fromNodeId, const T &toNodeId) const {
    // Get traffic count for this edge
    QPair<T, T> edge    = qMakePair(fromNodeId, toNodeId);
    int         traffic = m_trafficMap.value(edge, 0);

    // Get edge attributes
    QMap<QString, QVariant> attrs =
        this->getEdgeAttributes(fromNodeId, toNodeId);

    // Get capacity (lanes * saturation flow)
    float lanes = attrs.value("lanes", 1.0f).toFloat();
    float satFlow =
        attrs.value("saturation_flow", 1800.0f).toFloat();
    float capacity = lanes * satFlow;

    // Calculate congestion factor
    if (capacity <= 0.0f) {
        return 1.0f; // Default no congestion
    }

    // Use BPR formula: 1 + 0.15 * (v/c)^4
    float vc = static_cast<float>(traffic) / capacity;
    return 1.0f + 0.15f * std::pow(vc, 4.0f);
}

template <typename T>
void TransportationGraph<T>::addTraffic(const T &fromNodeId,
                                        const T &toNodeId,
                                        int vehicleCount) {
    QPair<T, T> edge = qMakePair(fromNodeId, toNodeId);
    m_trafficMap[edge] += vehicleCount;
}

template <typename T>
void TransportationGraph<T>::removeTraffic(
    const T &fromNodeId, const T &toNodeId,
    int vehicleCount) {
    QPair<T, T> edge = qMakePair(fromNodeId, toNodeId);

    // Ensure we don't go below zero
    int current        = m_trafficMap.value(edge, 0);
    m_trafficMap[edge] = qMax(0, current - vehicleCount);

    // Remove entry if traffic is zero
    if (m_trafficMap[edge] == 0) {
        m_trafficMap.remove(edge);
    }
}

template <typename T>
double TransportationGraph<T>::calculatePathMetric(
    const QVector<T> &path,
    const QString    &metricName) const {
    double total = 0.0;

    // Empty path has zero metric
    if (path.size() <= 1) {
        return total;
    }

    // Calculate metric for each segment
    for (int i = 0; i < path.size() - 1; ++i) {
        T fromNode = path[i];
        T toNode   = path[i + 1];

        // Skip if edge doesn't exist
        if (!this->hasEdge(fromNode, toNode)) {
            continue;
        }

        if (metricName == "distance") {
            // Distance is just the edge weight
            total += this->getEdgeWeight(fromNode, toNode);
        } else if (metricName == "time") {
            // Time is distance / speed * congestion
            float distance =
                this->getEdgeWeight(fromNode, toNode);
            float speed =
                this->getEdgeAttributes(fromNode, toNode)
                    .value("free_speed", 50.0f)
                    .toFloat();
            float congestion =
                calculateCongestion(fromNode, toNode);

            if (speed > 0.0f) {
                total += (distance / speed) * congestion;
            }
        } else if (metricName == "cost") {
            // Cost model could include fuel, tolls, etc.
            float distance =
                this->getEdgeWeight(fromNode, toNode);
            float costFactor =
                this->getEdgeAttributes(fromNode, toNode)
                    .value("cost_factor", 1.0f)
                    .toFloat();
            total += distance * costFactor;
        }
    }

    return total;
}

template <typename T>
QList<QVector<T>>
TransportationGraph<T>::findKShortestPaths(
    const T &startNodeId, const T &endNodeId, int k) const {
    return yenKSP(startNodeId, endNodeId, k);
}

template <typename T>
QVector<int>
TransportationGraph<T>::convertNodePathToLinkPath(
    const QVector<T> &nodePath) const {
    QVector<int> linkPath;

    // Skip empty paths
    if (nodePath.size() <= 1) {
        return linkPath;
    }

    // Convert nodes to links
    for (int i = 0; i < nodePath.size() - 1; ++i) {
        T fromNode = nodePath[i];
        T toNode   = nodePath[i + 1];

        // Get link ID from edge attributes
        QMap<QString, QVariant> attrs =
            this->getEdgeAttributes(fromNode, toNode);

        if (attrs.contains("link_id")) {
            linkPath.append(attrs["link_id"].toInt());
        }
    }

    return linkPath;
}

template <typename T>
int TransportationGraph<T>::getLinkTransportationMode(
    int linkId) const {
    return m_linkModes.value(linkId, 0);
}

template <typename T>
void TransportationGraph<T>::setLinkTransportationMode(
    int linkId, int modeId) {
    m_linkModes[linkId] = modeId;
}

template <typename T>
QList<QVector<T>> TransportationGraph<T>::yenKSP(
    const T &startNodeId, const T &endNodeId, int k) const {
    QList<QVector<T>> results;

    // Get first shortest path using Dijkstra
    QVector<T> firstPath =
        this->findShortestPath(startNodeId, endNodeId);

    // If no path exists or k <= 0, return empty list
    if (firstPath.isEmpty() || k <= 0) {
        return results;
    }

    // Add first path to results
    results.append(firstPath);

    // Priority queue for potential paths
    std::priority_queue<
        std::pair<float, QVector<T>>,
        std::vector<std::pair<float, QVector<T>>>,
        std::greater<std::pair<float, QVector<T>>>>
        candidates;

    // Set to track paths we've already found
    std::set<QVector<T>> pathSet;
    pathSet.insert(firstPath);

    // Find k-1 more paths
    for (int i = 1; i < k; ++i) {
        // Previous path
        const QVector<T> &prevPath = results.last();

        // For each node in the previous path (except last)
        for (int j = 0; j < prevPath.size() - 1; ++j) {
            // This node is the deviation point
            T spurNode = prevPath[j];

            // Root path is path to deviation point
            QVector<T> rootPath;
            for (int m = 0; m <= j; ++m) {
                rootPath.append(prevPath[m]);
            }

            // Instead of copying the entire graph, create a
            // new one
            TransportationGraph<T> tempGraph(nullptr);

            // Copy all nodes from original graph
            for (const T &nodeId : this->getNodes()) {
                tempGraph.addNode(
                    nodeId,
                    this->getNodeAttributes(nodeId));
            }

            // Copy all edges except those that would lead
            // to a repeat path
            for (const T &fromNode : this->getNodes()) {
                for (const auto &edge :
                     this->getOutgoingEdges(fromNode)) {
                    T     toNode = edge.first;
                    float weight = edge.second;

                    // Skip edges that would lead to a
                    // repeat path
                    bool shouldSkip = false;
                    for (const QVector<T> &path : results) {
                        if (path.size() > j + 1
                            && path.mid(0, j + 1)
                                   == rootPath
                            && path[j] == fromNode
                            && path[j + 1] == toNode) {
                            shouldSkip = true;
                            break;
                        }
                    }

                    if (!shouldSkip) {
                        tempGraph.addEdge(
                            fromNode, toNode, weight,
                            this->getEdgeAttributes(
                                fromNode, toNode));
                    }
                }
            }

            // Find shortest path from spur node to target
            QVector<T> spurPath =
                tempGraph.findShortestPath(spurNode,
                                           endNodeId);

            if (spurPath.isEmpty()) {
                continue;
            }

            // Remove spurNode from the spur path
            spurPath.removeFirst();

            // Complete path: root + spur
            QVector<T> totalPath = rootPath + spurPath;

            // Calculate path cost
            float pathCost = 0.0f;
            for (int m = 0; m < totalPath.size() - 1; ++m) {
                if (this->hasEdge(totalPath[m],
                                  totalPath[m + 1])) {
                    pathCost += this->getEdgeWeight(
                        totalPath[m], totalPath[m + 1]);
                }
            }

            // Add to candidates if not already found
            if (pathSet.find(totalPath) == pathSet.end()) {
                candidates.push(
                    std::make_pair(pathCost, totalPath));
                pathSet.insert(totalPath);
            }
        }

        // No more candidates?
        if (candidates.empty()) {
            break;
        }

        // Add the best candidate to results
        results.append(candidates.top().second);
        candidates.pop();
    }

    return results;
}

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim

// Explicit instantiations for common types
namespace CargoNetSim {
namespace Backend {
extern template class TruckClient::TransportationGraph<int>;
extern template class TruckClient::TransportationGraph<
    QString>;
} // namespace Backend
} // namespace CargoNetSim
