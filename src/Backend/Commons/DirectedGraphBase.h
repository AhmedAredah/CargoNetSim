/**
 * @file DirectedGraphBase.h
 * @brief Base class for directed graph implementations in
 * the CargoNetSim backend.
 * @author Ahmed Aredah
 */

#pragma once
#include <QObject>
#include <QVariant>

namespace CargoNetSim
{
namespace Backend
{

/**
 * @class DirectedGraphBase
 * @brief Base class that provides a Qt-based implementation
 * of a directed graph.
 *
 * DirectedGraphBase serves as an abstract base class for
 * all directed graph implementations within the CargoNetSim
 * backend. It provides a signal-based interface for graph
 * modification events, allowing for reactive UI updates and
 * event-driven programming patterns.
 *
 * Derived classes should implement the actual graph data
 * structure and operations, emitting the appropriate
 * signals when the graph structure changes.
 *
 * @note This class uses QVariant for node and edge
 * identifiers to provide flexibility in the types of
 * identifiers that can be used. Implementing classes should
 * document the expected format of these identifiers.
 */
class DirectedGraphBase : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructs a DirectedGraphBase instance.
     * @param parent The parent QObject (default: nullptr).
     */
    explicit DirectedGraphBase(QObject *parent = nullptr);

    /**
     * @brief Virtual destructor to ensure proper cleanup of
     * derived classes.
     */
    virtual ~DirectedGraphBase();

signals:
    /**
     * @brief Emitted when any change occurs in the graph
     * structure.
     *
     * This is a general signal that indicates some change
     * has occurred in the graph. For more specific
     * information about what changed, listen to the other
     * signals.
     */
    void graphChanged();

    /**
     * @brief Emitted when a new node is added to the graph.
     * @param nodeId The identifier of the newly added node.
     */
    void nodeAdded(QVariant nodeId);

    /**
     * @brief Emitted when a node is removed from the graph.
     * @param nodeId The identifier of the removed node.
     */
    void nodeRemoved(QVariant nodeId);

    /**
     * @brief Emitted when a node's properties are modified.
     * @param nodeId The identifier of the modified node.
     */
    void nodeModified(QVariant nodeId);

    /**
     * @brief Emitted when a new edge is added to the graph.
     * @param fromNodeId The identifier of the source node.
     * @param toNodeId The identifier of the target node.
     */
    void edgeAdded(QVariant fromNodeId, QVariant toNodeId);

    /**
     * @brief Emitted when an edge is removed from the
     * graph.
     * @param fromNodeId The identifier of the source node.
     * @param toNodeId The identifier of the target node.
     */
    void edgeRemoved(QVariant fromNodeId,
                     QVariant toNodeId);

    /**
     * @brief Emitted when an edge's properties are
     * modified.
     * @param fromNodeId The identifier of the source node.
     * @param toNodeId The identifier of the target node.
     */
    void edgeModified(QVariant fromNodeId,
                      QVariant toNodeId);
};

} // namespace Backend
} // namespace CargoNetSim

// Register custom types with Qt's meta-object system
Q_DECLARE_METATYPE(CargoNetSim::Backend::DirectedGraphBase)
Q_DECLARE_METATYPE(
    CargoNetSim::Backend::DirectedGraphBase *)
