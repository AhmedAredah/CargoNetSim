#pragma once

#include "GUI/Items/GraphicsObjectBase.h"
#include <QGraphicsScene>
#include <QPointF>
#include <QVariant>

namespace CargoNetSim
{
namespace GUI
{

// Forward declarations
class TerminalItem;
class ConnectionLine;
class DistanceMeasurementTool;

/**
 * @brief Custom graphics scene for the CargoNetSim
 * application
 *
 * Extends QGraphicsScene to handle special interaction
 * modes like connection creation, terminal linking, and
 * measurement tools.
 */
class GraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a GraphicsScene
     * @param parent Parent QObject
     */
    explicit GraphicsScene(QObject *parent = nullptr);

    void addItemWithId(GraphicsObjectBase *item,
                       const QString      &id);

    // Get item by type and ID
    template <typename T> T *getItemById(const QString &id)
    {
        // Get the class key
        QString className = QString(typeid(T).name());

        // Check if this type and ID exist
        if (itemsByType.contains(className)
            && itemsByType[className].contains(id))
        {
            // Return the item if it's of the correct type
            return qgraphicsitem_cast<T *>(
                itemsByType[className][id]);
        }
        return nullptr;
    }

    // Get all items of a specific type
    template <typename T> QList<T *> getItemsByType()
    {
        QList<T *> result;
        QString    className = QString(typeid(T).name());

        // Check if this type exists
        if (itemsByType.contains(className))
        {
            // Iterate through all items of this type and
            // them to result
            for (auto item :
                 itemsByType[className].values())
            {
                // Check if the item is of the correct type
                T *typedItem =
                    qgraphicsitem_cast<T *>(item);
                if (typedItem)
                {
                    // Append to the result list
                    result.append(typedItem);
                }
            }
        }
        return result;
    }

    // Handle item removal
    template <typename T>
    bool removeItemWithId(const QString &id)
    {
        QString className = QString(typeid(T).name());

        // Check if this type and ID exist
        if (!itemsByType.contains(className)
            || !itemsByType[className].contains(id))
        {
            return false;
        }

        // Get the item
        QGraphicsItem *item = itemsByType[className][id];

        // Remove from type map
        itemsByType[className].remove(id);

        // Remove from scene
        QGraphicsScene::removeItem(item);

        // Delete the item
        delete item;
        item = nullptr;

        return true;
    }

    // Mode flags
    bool connectMode; ///< Flag indicating if connection
                      ///< creation mode is active
    bool linkTerminalMode;   ///< Flag indicating if
                             ///< terminal-node linking mode
                             ///< is active
    bool unlinkTerminalMode; ///< Flag indicating if
                             ///< terminal-node unlinking
                             ///< mode is active
    bool measureMode; ///< Flag indicating if distance
                      ///< measurement mode is active
    bool setGlobalPositionMode; ///< Flag indicating if
                                ///< setting global position
                                ///< mode is active

    // Objects used for connection and measurement modes
    QVariant connectFirstItem; ///< First terminal selected
                               ///< in connect mode (can be
                               ///< TerminalItem* or
                               ///< GlobalTerminalItem*)
    DistanceMeasurementTool
        *measurementTool; ///< Current measurement tool
                          ///< being used

protected:
    /**
     * @brief Handles mouse press events in the scene
     * @param event The mouse event
     */
    void mousePressEvent(
        QGraphicsSceneMouseEvent *event) override;

private:
    // Nested map structure: outer key is class name, inner
    // key is item ID
    QMap<QString, QMap<QString, QGraphicsItem *>>
        itemsByType;
};

} // namespace GUI
} // namespace CargoNetSim
