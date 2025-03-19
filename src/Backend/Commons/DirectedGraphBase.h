#pragma once

#include <QObject>
#include <QVariant>

namespace CargoNetSim {
namespace Backend {

class DirectedGraphBase : public QObject {
    Q_OBJECT

public:
    explicit DirectedGraphBase(QObject* parent = nullptr);
    virtual ~DirectedGraphBase();

signals:
    void graphChanged();
    void nodeAdded(QVariant nodeId);
    void nodeRemoved(QVariant nodeId);
    void nodeModified(QVariant nodeId);
    void edgeAdded(QVariant fromNodeId, QVariant toNodeId);
    void edgeRemoved(QVariant fromNodeId, QVariant toNodeId);
    void edgeModified(QVariant fromNodeId, QVariant toNodeId);
};

} // namespace Backend
} // namespace CargoNetSim