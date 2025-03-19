/**
* @file DirectedGraph.h
* @brief Template implementation of a directed graph with attributes and weights.
* @author Ahmed Aredah
*/


#pragma once

#include "DirectedGraphBase.h"
#include <QMap>
#include <QVector>
#include <QPair>
#include <QSet>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutex>
#include <limits>
#include <algorithm>

namespace CargoNetSim {
namespace Backend {

/**
* @struct PriorityQueueEntry
* @brief Custom priority queue entry for the shortest path algorithm.
* @tparam T The type of node identifier.
*/template <typename T>
struct PriorityQueueEntry {
    /** @brief Cost to reach this node */
    float cost;
    /** @brief Node identifier */
    T nodeId;

    /**
    * @brief Comparison operator for priority queue ordering.
    * @param other The entry to compare with.
    * @return True if this entry has a higher cost than the other.
    */
    bool operator>(const PriorityQueueEntry& other) const {
        return cost > other.cost;
    }
};

/**
* @class DirectedGraph
* @brief A template implementation of a directed graph with node and
*        edge attributes.
*
* This class provides a comprehensive implementation of a directed
* graph with support for:
* - Node and edge attributes stored as key-value pairs
* - Edge weights for path calculations
* - Thread-safe operations via mutex locking
* - Dijkstra's shortest path algorithm with customizable cost functions
* - Serialization to and from JSON
*
* @tparam T The type of node identifier, which must be storable in QVariant.
*/
template <typename T>
class DirectedGraph : public DirectedGraphBase {
public:

    /**
    * @brief Constructs a DirectedGraph instance.
    * @param parent The parent QObject (default: nullptr).
    */
    explicit DirectedGraph(QObject* parent = nullptr);

    /**
    * @brief Destructor that cleans up the graph.
    */
    virtual ~DirectedGraph();

    /**
    * @brief Adds a node with optional attributes.
    * @param nodeId The identifier for the node.
    * @param attributes Key-value pairs of node attributes (default: empty).
    */
    void addNode(const T& nodeId, const QMap<QString, QVariant>& attributes =
                                  QMap<QString, QVariant>());

    /**
    * @brief Adds a directed edge with weight and optional attributes.
    * @param fromNodeId The source node identifier.
    * @param toNodeId The target node identifier.
    * @param weight The edge weight (used for path calculations).
    * @param attributes Key-value pairs of edge attributes (default: empty).
    */
    void addEdge(const T& fromNodeId, const T& toNodeId, float weight,
                 const QMap<QString, QVariant>& attributes =
                 QMap<QString, QVariant>());

    /**
    * @brief Removes a node and all its connected edges.
    * @param nodeId The identifier of the node to remove.
    */
    void removeNode(const T& nodeId);

    /**
    * @brief Removes an edge between two nodes.
    * @param fromNodeId The source node identifier.
    * @param toNodeId The target node identifier.
    */
    void removeEdge(const T& fromNodeId, const T& toNodeId);

    /**
    * @brief Checks if a node exists in the graph.
    * @param nodeId The node identifier to check.
    * @return True if the node exists, false otherwise.
    */
    bool hasNode(const T& nodeId) const;

    /**
    * @brief Checks if an edge exists between two nodes.
    * @param fromNodeId The source node identifier.
    * @param toNodeId The target node identifier.
    * @return True if the edge exists, false otherwise.
    */
    bool hasEdge(const T& fromNodeId, const T& toNodeId) const;

    /**
    * @brief Gets the attributes of a node.
    * @param nodeId The node identifier.
    * @return A map of attribute name to value.
    */
    QMap<QString, QVariant> getNodeAttributes(const T& nodeId) const;

    /**
    * @brief Sets or updates the attributes of a node.
    * @param nodeId The node identifier.
    * @param attributes The new attributes to set.
    */
    void setNodeAttributes(const T& nodeId, const QMap<QString, QVariant>& attributes);

    /**
    * @brief Gets the attributes of an edge.
    * @param fromNodeId The source node identifier.
    * @param toNodeId The target node identifier.
    * @return A map of attribute name to value.
    */
    QMap<QString, QVariant> getEdgeAttributes(const T& fromNodeId, const T& toNodeId) const;

    /**
    * @brief Sets or updates the attributes of an edge.
    * @param fromNodeId The source node identifier.
    * @param toNodeId The target node identifier.
    * @param attributes The new attributes to set.
    */
    void setEdgeAttributes(const T& fromNodeId, const T& toNodeId, const QMap<QString, QVariant>& attributes);

    /**
    * @brief Gets the weight of an edge.
    * @param fromNodeId The source node identifier.
    * @param toNodeId The target node identifier.
    * @return The edge weight, or infinity if the edge doesn't exist.
    */
    float getEdgeWeight(const T& fromNodeId, const T& toNodeId) const;

    /**
    * @brief Sets the weight of an edge.
    * @param fromNodeId The source node identifier.
    * @param toNodeId The target node identifier.
    * @param weight The new weight to set.
    */
    void setEdgeWeight(const T& fromNodeId, const T& toNodeId, float weight);

    /**
    * @brief Gets all nodes in the graph.
    * @return A vector of all node identifiers.
    */
    QVector<T> getNodes() const;

    /**
    * @brief Gets all outgoing edges from a node.
    * @param nodeId The source node identifier.
    * @return A vector of pairs containing target node identifiers
    *         and edge weights.
    */
    QVector<QPair<T, float>> getOutgoingEdges(const T& nodeId) const;

    /**
    * @brief Gets all incoming edges to a node.
    * @param nodeId The target node identifier.
    * @return A vector of pairs containing source node identifiers
    *         and edge weights.
    */
    QVector<QPair<T, float>> getIncomingEdges(const T& nodeId) const;

    /**
    * @brief Gets the out-degree of a node (number of outgoing edges).
    * @param nodeId The node identifier.
    * @return The number of outgoing edges.
    */
    int getOutDegree(const T& nodeId) const;

    /**
    * @brief Gets the in-degree of a node (number of incoming edges).
    * @param nodeId The node identifier.
    * @return The number of incoming edges.
    */
    int getInDegree(const T& nodeId) const;

    /**
    * @brief Finds the shortest path between two nodes using
    *        Dijkstra's algorithm.
    * @param startNodeId The starting node identifier.
    * @param endNodeId The destination node identifier.
    * @param optimizeFor The criterion to optimize for (default: "distance").
    * @return A vector of node identifiers representing the shortest path,
    *         or empty if no path exists.
    */
    QVector<T> findShortestPath(const T& startNodeId, const T& endNodeId,
                                const QString& optimizeFor = "distance") const;

    /**
    * @brief Clears all nodes and edges from the graph.
    */
    void clear();

    /**
    * @brief Exports the graph to a JSON object.
    * @return A JSON object representing the graph structure.
    */
    QJsonObject toJson() const;

    /**
    * @brief Imports a graph structure from a JSON object.
    * @param json The JSON object to import.
    */
    void fromJson(const QJsonObject& json);

private:
    /**
    * @brief Calculates the cost of an edge based on the optimization criterion.
    * @param fromNodeId The source node identifier.
    * @param toNodeId The target node identifier.
    * @param optimizeFor The criterion to optimize for
    *        (e.g., "distance", "time").
    * @return The calculated edge cost.
    */
    float calculateEdgeCost(const T& fromNodeId,
                            const T& toNodeId,
                            const QString& optimizeFor) const;

    /** @brief Maps nodes to their attributes */
    QMap<T, QMap<QString, QVariant>> m_nodeAttributes;

    /** @brief Maps edges to their attributes */
    QMap<T, QMap<T, QMap<QString, QVariant>>> m_edgeAttributes;

    /** @brief Maps edges to their weights */
    QMap<T, QMap<T, float>> m_edgeWeights;

    /** @brief Mutex for thread synchronization */
    mutable QMutex m_mutex;
};

// Include the implementation

template <typename T>
DirectedGraph<T>::DirectedGraph(QObject* parent)
    : DirectedGraphBase(parent)
{
    // Static assertion to ensure T can be stored in QVariant
    static_assert(
        std::is_same<T, int>::value ||
            std::is_same<T, uint>::value ||
            std::is_same<T, qlonglong>::value ||
            std::is_same<T, qulonglong>::value ||
            std::is_same<T, bool>::value ||
            std::is_same<T, double>::value ||
            std::is_same<T, QString>::value ||
            std::is_same<T, QByteArray>::value ||
            std::is_same<T, QStringList>::value ||
            std::is_same<T, QList<QVariant>>::value ||
            std::is_same<T, QMap<QString, QVariant>>::value,
        "Type T must be storable in QVariant for signal/slot usage"
        );
}

template <typename T>
DirectedGraph<T>::~DirectedGraph()
{
    clear();
}

template <typename T>
void DirectedGraph<T>::addNode(const T& nodeId, const QMap<QString, QVariant>& attributes)
{
    QMutexLocker locker(&m_mutex);
    
    bool nodeExists = m_nodeAttributes.contains(nodeId);
    m_nodeAttributes[nodeId] = attributes;
    
    if (!nodeExists) {
        emit nodeAdded(QVariant::fromValue(nodeId));
        emit graphChanged();
    } else {
        emit nodeModified(QVariant::fromValue(nodeId));
        emit graphChanged();
    }
}

template <typename T>
void DirectedGraph<T>::addEdge(const T& fromNodeId, const T& toNodeId, float weight, 
                               const QMap<QString, QVariant>& attributes)
{
    QMutexLocker locker(&m_mutex);
    
    // Ensure nodes exist
    if (!m_nodeAttributes.contains(fromNodeId)) {
        addNode(fromNodeId);
    }
    
    if (!m_nodeAttributes.contains(toNodeId)) {
        addNode(toNodeId);
    }
    
    // Check if edge already exists
    bool edgeExists = m_edgeWeights.contains(fromNodeId) && 
                      m_edgeWeights[fromNodeId].contains(toNodeId);
    
    // Add edge
    m_edgeAttributes[fromNodeId][toNodeId] = attributes;
    m_edgeWeights[fromNodeId][toNodeId] = weight;
    
    if (!edgeExists) {
        emit edgeAdded(QVariant::fromValue(fromNodeId), QVariant::fromValue(toNodeId));
        emit graphChanged();
    } else {
        emit edgeModified(QVariant::fromValue(fromNodeId), QVariant::fromValue(toNodeId));
        emit graphChanged();
    }
}

template <typename T>
void DirectedGraph<T>::removeNode(const T& nodeId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_nodeAttributes.contains(nodeId)) {
        return;
    }
    
    // Remove all edges to and from this node
    for (auto it = m_edgeWeights.begin(); it != m_edgeWeights.end(); ++it) {
        T fromNode = it.key();
        
        // Remove edges to the node
        if (it.value().contains(nodeId)) {
            it.value().remove(nodeId);
            m_edgeAttributes[fromNode].remove(nodeId);
            emit edgeRemoved(QVariant::fromValue(fromNode), QVariant::fromValue(nodeId));
        }
        
        // If we're removing the current fromNode
        if (fromNode == nodeId) {
            // Notify about each removed edge
            for (auto toNodeIt = it.value().begin(); toNodeIt != it.value().end(); ++toNodeIt) {
                emit edgeRemoved(QVariant::fromValue(nodeId), QVariant::fromValue(toNodeIt.key()));
            }
        }
    }
    
    // Remove all outgoing edges
    m_edgeWeights.remove(nodeId);
    m_edgeAttributes.remove(nodeId);
    
    // Remove the node
    m_nodeAttributes.remove(nodeId);
    
    emit nodeRemoved(QVariant::fromValue(nodeId));
    emit graphChanged();
}

template <typename T>
void DirectedGraph<T>::removeEdge(const T& fromNodeId, const T& toNodeId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!hasEdge(fromNodeId, toNodeId)) {
        return;
    }
    
    m_edgeWeights[fromNodeId].remove(toNodeId);
    m_edgeAttributes[fromNodeId].remove(toNodeId);
    
    emit edgeRemoved(QVariant::fromValue(fromNodeId), QVariant::fromValue(toNodeId));
    emit graphChanged();
}

template <typename T>
bool DirectedGraph<T>::hasNode(const T& nodeId) const
{
    QMutexLocker locker(&m_mutex);
    return m_nodeAttributes.contains(nodeId);
}

template <typename T>
bool DirectedGraph<T>::hasEdge(const T& fromNodeId, const T& toNodeId) const
{
    QMutexLocker locker(&m_mutex);
    return m_edgeWeights.contains(fromNodeId) && m_edgeWeights[fromNodeId].contains(toNodeId);
}

template <typename T>
QMap<QString, QVariant> DirectedGraph<T>::getNodeAttributes(const T& nodeId) const
{
    QMutexLocker locker(&m_mutex);
    return m_nodeAttributes.value(nodeId);
}

template <typename T>
void DirectedGraph<T>::setNodeAttributes(const T& nodeId, const QMap<QString, QVariant>& attributes)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_nodeAttributes.contains(nodeId)) {
        addNode(nodeId, attributes);
        return;
    }
    
    m_nodeAttributes[nodeId] = attributes;
    emit nodeModified(QVariant::fromValue(nodeId));
    emit graphChanged();
}

template <typename T>
QMap<QString, QVariant> DirectedGraph<T>::getEdgeAttributes(const T& fromNodeId, const T& toNodeId) const
{
    QMutexLocker locker(&m_mutex);
    
    if (hasEdge(fromNodeId, toNodeId)) {
        return m_edgeAttributes[fromNodeId][toNodeId];
    }
    return QMap<QString, QVariant>();
}

template <typename T>
void DirectedGraph<T>::setEdgeAttributes(const T& fromNodeId, const T& toNodeId, 
                                         const QMap<QString, QVariant>& attributes)
{
    QMutexLocker locker(&m_mutex);
    
    if (!hasEdge(fromNodeId, toNodeId)) {
        // Default weight if edge doesn't exist
        addEdge(fromNodeId, toNodeId, 1.0f, attributes);
        return;
    }
    
    m_edgeAttributes[fromNodeId][toNodeId] = attributes;
    emit edgeModified(QVariant::fromValue(fromNodeId), QVariant::fromValue(toNodeId));
    emit graphChanged();
}

template <typename T>
float DirectedGraph<T>::getEdgeWeight(const T& fromNodeId, const T& toNodeId) const
{
    QMutexLocker locker(&m_mutex);
    
    if (hasEdge(fromNodeId, toNodeId)) {
        return m_edgeWeights[fromNodeId][toNodeId];
    }
    return std::numeric_limits<float>::infinity();
}

template <typename T>
void DirectedGraph<T>::setEdgeWeight(const T& fromNodeId, const T& toNodeId, float weight)
{
    QMutexLocker locker(&m_mutex);
    
    if (!hasEdge(fromNodeId, toNodeId)) {
        addEdge(fromNodeId, toNodeId, weight);
        return;
    }
    
    m_edgeWeights[fromNodeId][toNodeId] = weight;
    emit edgeModified(QVariant::fromValue(fromNodeId), QVariant::fromValue(toNodeId));
    emit graphChanged();
}

template <typename T>
QVector<T> DirectedGraph<T>::getNodes() const
{
    QMutexLocker locker(&m_mutex);
    return m_nodeAttributes.keys().toVector();
}

template <typename T>
QVector<QPair<T, float>> DirectedGraph<T>::getOutgoingEdges(const T& nodeId) const
{
    QMutexLocker locker(&m_mutex);
    QVector<QPair<T, float>> edges;
    
    if (m_edgeWeights.contains(nodeId)) {
        for (auto it = m_edgeWeights[nodeId].constBegin(); it != m_edgeWeights[nodeId].constEnd(); ++it) {
            edges.append(qMakePair(it.key(), it.value()));
        }
    }
    
    return edges;
}

template <typename T>
QVector<QPair<T, float>> DirectedGraph<T>::getIncomingEdges(const T& nodeId) const
{
    QMutexLocker locker(&m_mutex);
    QVector<QPair<T, float>> edges;
    
    for (auto sourceIt = m_edgeWeights.constBegin(); sourceIt != m_edgeWeights.constEnd(); ++sourceIt) {
        if (sourceIt.value().contains(nodeId)) {
            edges.append(qMakePair(sourceIt.key(), sourceIt.value()[nodeId]));
        }
    }
    
    return edges;
}

template <typename T>
int DirectedGraph<T>::getOutDegree(const T& nodeId) const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_edgeWeights.contains(nodeId)) {
        return m_edgeWeights[nodeId].size();
    }
    return 0;
}

template <typename T>
int DirectedGraph<T>::getInDegree(const T& nodeId) const
{
    QMutexLocker locker(&m_mutex);
    int count = 0;
    
    for (auto sourceIt = m_edgeWeights.constBegin(); sourceIt != m_edgeWeights.constEnd(); ++sourceIt) {
        if (sourceIt.value().contains(nodeId)) {
            count++;
        }
    }
    
    return count;
}

template <typename T>
float DirectedGraph<T>::calculateEdgeCost(const T& fromNodeId, const T& toNodeId, 
                                          const QString& optimizeFor) const
{
    if (!hasEdge(fromNodeId, toNodeId)) {
        return std::numeric_limits<float>::infinity();
    }
    
    float weight = m_edgeWeights[fromNodeId][toNodeId];
    
    if (optimizeFor == "distance") {
        return weight;
    } else if (optimizeFor == "time") {
        // If time optimization is requested, check for speed attribute
        QMap<QString, QVariant> attrs = m_edgeAttributes[fromNodeId][toNodeId];
        
        if (attrs.contains("max_speed") && attrs["max_speed"].toFloat() > 0) {
            return weight / attrs["max_speed"].toFloat();
        } else if (attrs.contains("free_speed") && attrs["free_speed"].toFloat() > 0) {
            return weight / attrs["free_speed"].toFloat();
        }
    }
    
    // Default to distance
    return weight;
}

template <typename T>
QVector<T> DirectedGraph<T>::findShortestPath(const T& startNodeId, const T& endNodeId, 
                                              const QString& optimizeFor) const
{
    QMutexLocker locker(&m_mutex);
    
    // Check if nodes exist
    if (!hasNode(startNodeId) || !hasNode(endNodeId)) {
        return QVector<T>();
    }
    
    // Initialize distances and predecessors
    QMap<T, float> costs;
    QMap<T, T> predecessors;
    QSet<T> visited;
    
    // Initialize costs for all nodes
    for (const T& nodeId : getNodes()) {
        costs[nodeId] = std::numeric_limits<float>::infinity();
        // Use a sentinel value for no predecessor
        // We need to initialize with a valid value of type T
        predecessors[nodeId] = nodeId; // Temporarily point to self
    }
    
    // Special marking for the start node
    costs[startNodeId] = 0.0f;
    
    // Initialize priority queue with starting node
    QVector<PriorityQueueEntry<T>> pq;
    pq.append({0.0f, startNodeId});
    
    while (!pq.isEmpty()) {
        // Find minimum cost node in queue
        auto it = std::min_element(pq.begin(), pq.end(),
                                   [](const PriorityQueueEntry<T>& a, const PriorityQueueEntry<T>& b) {
                                       return a.cost < b.cost;
                                   });
        
        float currentCost = it->cost;
        T currentNode = it->nodeId;
        
        // Remove the processed node
        pq.erase(it);
        
        // If we've reached the destination
        if (currentNode == endNodeId) {
            break;
        }
        
        // Skip if already visited or if new path is worse
        if (visited.contains(currentNode) || currentCost > costs[currentNode]) {
            continue;
        }
        
        // Mark as visited
        visited.insert(currentNode);
        
        // Process neighbors
        for (const QPair<T, float>& edge : getOutgoingEdges(currentNode)) {
            T neighborId = edge.first;
            
            // Skip already visited nodes
            if (visited.contains(neighborId)) {
                continue;
            }
            
            // Calculate cost based on optimization criterion
            float edgeCost = calculateEdgeCost(currentNode, neighborId, optimizeFor);
            float totalCost = costs[currentNode] + edgeCost;
            
            // If new path is better
            if (totalCost < costs[neighborId]) {
                costs[neighborId] = totalCost;
                predecessors[neighborId] = currentNode;
                
                // Add to priority queue
                pq.append({totalCost, neighborId});
            }
        }
    }
    
    // Reconstruct the path
    QVector<T> path;
    T current = endNodeId;
    
    // Check if a path was found - if the end node still points to itself, no path was found
    if (predecessors[endNodeId] == endNodeId && endNodeId != startNodeId) {
        return path;  // No path found
    }
    
    // Build the path by following predecessors
    while (current != startNodeId) {
        path.prepend(current);
        current = predecessors[current];
    }
    
    // Add the start node
    path.prepend(startNodeId);
    
    return path;
}

template <typename T>
void DirectedGraph<T>::clear()
{
    QMutexLocker locker(&m_mutex);
    
    m_nodeAttributes.clear();
    m_edgeAttributes.clear();
    m_edgeWeights.clear();
    
    emit graphChanged();
}

template <typename T>
QJsonObject DirectedGraph<T>::toJson() const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject result;
    
    // Export nodes
    QJsonArray nodesArray;
    for (auto it = m_nodeAttributes.constBegin(); it != m_nodeAttributes.constEnd(); ++it) {
        QJsonObject nodeObj;
        nodeObj["id"] = QJsonValue::fromVariant(QVariant::fromValue(it.key()));
        
        // Convert attributes to JSON
        QJsonObject attributesObj;
        for (auto attrIt = it.value().constBegin(); attrIt != it.value().constEnd(); ++attrIt) {
            attributesObj[attrIt.key()] = QJsonValue::fromVariant(attrIt.value());
        }
        
        nodeObj["attributes"] = attributesObj;
        nodesArray.append(nodeObj);
    }
    
    // Export edges
    QJsonArray edgesArray;
    for (auto fromIt = m_edgeWeights.constBegin(); fromIt != m_edgeWeights.constEnd(); ++fromIt) {
        T fromNodeId = fromIt.key();
        
        for (auto toIt = fromIt.value().constBegin(); toIt != fromIt.value().constEnd(); ++toIt) {
            T toNodeId = toIt.key();
            float weight = toIt.value();
            
            QJsonObject edgeObj;
            edgeObj["from"] = QJsonValue::fromVariant(QVariant::fromValue(fromNodeId));
            edgeObj["to"] = QJsonValue::fromVariant(QVariant::fromValue(toNodeId));
            edgeObj["weight"] = weight;
            
            // Convert attributes to JSON
            QJsonObject attributesObj;
            const QMap<QString, QVariant>& attrs = m_edgeAttributes[fromNodeId][toNodeId];
            for (auto attrIt = attrs.constBegin(); attrIt != attrs.constEnd(); ++attrIt) {
                attributesObj[attrIt.key()] = QJsonValue::fromVariant(attrIt.value());
            }
            
            edgeObj["attributes"] = attributesObj;
            edgesArray.append(edgeObj);
        }
    }
    
    result["nodes"] = nodesArray;
    result["edges"] = edgesArray;
    
    return result;
}

template <typename T>
void DirectedGraph<T>::fromJson(const QJsonObject& json)
{
    QMutexLocker locker(&m_mutex);
    
    // Clear existing data
    clear();
    
    // Import nodes
    QJsonArray nodesArray = json["nodes"].toArray();
    for (const QJsonValue& nodeValue : nodesArray) {
        QJsonObject nodeObj = nodeValue.toObject();
        T nodeId = nodeObj["id"].toVariant().value<T>();
        
        // Convert attributes from JSON
        QMap<QString, QVariant> attributes;
        QJsonObject attributesObj = nodeObj["attributes"].toObject();
        for (auto it = attributesObj.constBegin(); it != attributesObj.constEnd(); ++it) {
            attributes[it.key()] = it.value().toVariant();
        }
        
        // Add node (without emitting signals yet)
        m_nodeAttributes[nodeId] = attributes;
    }
    
    // Import edges
    QJsonArray edgesArray = json["edges"].toArray();
    for (const QJsonValue& edgeValue : edgesArray) {
        QJsonObject edgeObj = edgeValue.toObject();
        T fromNodeId = edgeObj["from"].toVariant().value<T>();
        T toNodeId = edgeObj["to"].toVariant().value<T>();
        float weight = edgeObj["weight"].toDouble();
        
        // Convert attributes from JSON
        QMap<QString, QVariant> attributes;
        QJsonObject attributesObj = edgeObj["attributes"].toObject();
        for (auto it = attributesObj.constBegin(); it != attributesObj.constEnd(); ++it) {
            attributes[it.key()] = it.value().toVariant();
        }
        
        // Add edge (without emitting signals yet)
        m_edgeWeights[fromNodeId][toNodeId] = weight;
        m_edgeAttributes[fromNodeId][toNodeId] = attributes;
    }
    
    // Emit a single graphChanged signal
    emit graphChanged();
}

// Explicit instantiations for common types
extern template class DirectedGraph<int>;
extern template class DirectedGraph<QString>;

} // namespace Backend
} // namespace CargoNetSim

// Ensure Q_DECLARE_METATYPE is called for common types
Q_DECLARE_METATYPE(QVector<int>)
Q_DECLARE_METATYPE(QVector<QString>)
Q_DECLARE_METATYPE(CargoNetSim::Backend::DirectedGraph<int>)
Q_DECLARE_METATYPE(CargoNetSim::Backend::DirectedGraph<int>*)
Q_DECLARE_METATYPE(CargoNetSim::Backend::DirectedGraph<QString>)
Q_DECLARE_METATYPE(CargoNetSim::Backend::DirectedGraph<QString>*)
