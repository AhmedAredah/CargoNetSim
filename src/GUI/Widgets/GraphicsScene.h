#pragma once

#include <QGraphicsScene>
#include <QPointF>
#include <QVariant>

namespace CargoNetSim {
namespace GUI {

// Forward declarations
class TerminalItem;
class ConnectionLine;
class DistanceMeasurementTool;

/**
 * @brief Custom graphics scene for the CargoNetSim application
 * 
 * Extends QGraphicsScene to handle special interaction modes like connection
 * creation, terminal linking, and measurement tools.
 */
class GraphicsScene : public QGraphicsScene {
    Q_OBJECT

public:
    /**
     * @brief Constructs a GraphicsScene
     * @param parent Parent QObject
     */
    explicit GraphicsScene(QObject* parent = nullptr);
    
    /**
     * @brief Checks if a connection of the same type already exists between two terminals
     * @param startItem The first terminal
     * @param endItem The second terminal
     * @param connectionType The type of connection to check
     * @return True if a connection of the specified type already exists, false otherwise
     */
    bool checkExistingConnection(TerminalItem* startItem, TerminalItem* endItem, const QString& connectionType);

    // Mode flags
    bool connectMode;            ///< Flag indicating if connection creation mode is active
    bool linkTerminalMode;       ///< Flag indicating if terminal-node linking mode is active
    bool unlinkTerminalMode;     ///< Flag indicating if terminal-node unlinking mode is active
    bool measureMode;            ///< Flag indicating if distance measurement mode is active
    bool setGlobalPositionMode;  ///< Flag indicating if setting global position mode is active
    
    // Objects used for connection and measurement modes
    QVariant connectFirstItem;  ///< First terminal selected in connect mode (can be TerminalItem* or GlobalTerminalItem*)
    DistanceMeasurementTool* measurementTool; ///< Current measurement tool being used

protected:
    /**
     * @brief Handles mouse press events in the scene
     * @param event The mouse event
     */
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
};

} // namespace GUI
} // namespace CargoNetSim
