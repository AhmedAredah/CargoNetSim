#pragma once

#include "GraphicsObjectBase.h"

#include <QGraphicsObject>
#include <QPixmap>
#include <QPointF>
#include <QString>

namespace CargoNetSim
{
namespace GUI
{

class TerminalItem;

/**
 * @brief Represents a terminal on the global map view
 *
 * A global map representation using the same pixmap as
 * TerminalItem but scaled down. Links to the original
 * terminal item and represents it on the global map.
 */
class GlobalTerminalItem : public GraphicsObjectBase
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a GlobalTerminalItem
     * @param pixmap The pixmap to display
     * @param terminalItem The linked terminal item
     * @param parent The parent item
     */
    GlobalTerminalItem(const QPixmap &pixmap,
                       TerminalItem *terminalItem = nullptr,
                       QGraphicsItem *parent = nullptr);

    /**
     * @brief Destructor
     */
    virtual ~GlobalTerminalItem() = default;

    /**
     * @brief Get the linked terminal item
     * @return Pointer to the linked terminal item
     */
    TerminalItem *getLinkedTerminalItem() const
    {
        return linkedTerminalItem;
    }

    /**
     * @brief Set the linked terminal item
     * @param terminalItem The terminal item to link
     */
    void setLinkedTerminalItem(TerminalItem *terminalItem);

    /**
     * @brief Updates the item's appearance based on
     * properties from linked terminal
     */
    void updateFromLinkedTerminal();

    /**
     * @brief Serializes the GlobalTerminalItem to a
     * dictionary
     * @return Dictionary containing serialized data
     */
    QMap<QString, QVariant> toDict() const;

    /**
     * @brief Creates a GlobalTerminalItem from serialized
     * data
     * @param data The serialized data dictionary
     * @param pixmap The pixmap to display
     * @param parent The parent item
     * @return Pointer to new GlobalTerminalItem instance
     */
    static GlobalTerminalItem *
    fromDict(const QMap<QString, QVariant> &data,
             const QPixmap                 &pixmap,
             QGraphicsItem *parent = nullptr);

signals:
    /**
     * @brief Signal emitted when item position has changed
     * @param newPos The new position
     */
    void positionChanged(const QPointF &newPos);

    /**
     * @brief Signal emitted when item is clicked
     * @param item The item that was clicked
     */
    void itemClicked(GlobalTerminalItem *item);

    /**
     * @brief Signal emitted when linked terminal item
     * changes
     * @param oldTerminal Previous terminal
     * @param newTerminal New terminal
     */
    void linkedTerminalChanged(TerminalItem *oldTerminal,
                               TerminalItem *newTerminal);

protected:
    /**
     * @brief Returns the bounding rectangle of this item
     * @return The bounding rectangle
     */
    QRectF boundingRect() const override;

    /**
     * @brief Paints the item
     * @param painter The painter to use
     * @param option Style options
     * @param widget Widget being painted on
     */
    void paint(QPainter                       *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    /**
     * @brief Handles item position changes
     * @param change The type of change
     * @param value The new value
     * @return The adjusted value
     */
    QVariant itemChange(GraphicsItemChange change,
                        const QVariant    &value) override;

    /**
     * @brief Handles mouse hover enter events
     * @param event The hover event
     */
    void hoverEnterEvent(
        QGraphicsSceneHoverEvent *event) override;

    /**
     * @brief Handles mouse hover leave events
     * @param event The hover event
     */
    void hoverLeaveEvent(
        QGraphicsSceneHoverEvent *event) override;

    /**
     * @brief Handles mouse press events
     * @param event The mouse event
     */
    void mousePressEvent(
        QGraphicsSceneMouseEvent *event) override;

private:
    QPixmap
        originalPixmap;   ///< The original terminal pixmap
    QPixmap scaledPixmap; ///< The scaled version for global
                          ///< view
    TerminalItem
        *linkedTerminalItem; ///< The linked terminal item
};

} // namespace GUI
} // namespace CargoNetSim

// Register the type for QVariant
Q_DECLARE_METATYPE(CargoNetSim::GUI::GlobalTerminalItem)
Q_DECLARE_METATYPE(CargoNetSim::GUI::GlobalTerminalItem *)
