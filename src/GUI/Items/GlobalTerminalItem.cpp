#include "GlobalTerminalItem.h"
#include "TerminalItem.h"

#include <QCursor>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace CargoNetSim
{
namespace GUI
{

GlobalTerminalItem::GlobalTerminalItem(
    const QPixmap &pixmap, TerminalItem *terminalItem,
    QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , originalPixmap(pixmap)
    , linkedTerminalItem(terminalItem)
{
    // Enable hover events
    setAcceptHoverEvents(true);

    // Enable position change notification
    setFlags(QGraphicsItem::ItemSendsGeometryChanges
             | QGraphicsItem::ItemIgnoresTransformations
             | QGraphicsItem::ItemIsSelectable);

    // Scale down the pixmap for global map display (24x24)
    scaledPixmap =
        originalPixmap.scaled(24, 24, Qt::KeepAspectRatio,
                              Qt::SmoothTransformation);

    // Set tooltip with terminal name if available
    updateFromLinkedTerminal();
}

void GlobalTerminalItem::setLinkedTerminalItem(
    TerminalItem *terminalItem)
{
    if (linkedTerminalItem != terminalItem)
    {
        TerminalItem *oldTerminal = linkedTerminalItem;
        linkedTerminalItem        = terminalItem;

        updateFromLinkedTerminal();

        // Emit signal about terminal change
        emit linkedTerminalChanged(oldTerminal,
                                   linkedTerminalItem);
    }
}

void GlobalTerminalItem::updateFromLinkedTerminal()
{
    if (linkedTerminalItem)
    {
        // Get terminal name for tooltip
        QString terminalName =
            linkedTerminalItem->property("Name").toString();
        if (terminalName.isEmpty())
        {
            // Try to get name from properties if not
            // available as property
            QMap<QString, QVariant> props =
                linkedTerminalItem->property("properties")
                    .toMap();
            terminalName =
                props.value("Name", "Terminal").toString();
        }

        // Update tooltip
        setToolTip(terminalName);

        // Update pixmap if needed
        QPixmap terminalPixmap =
            linkedTerminalItem->property("pixmap")
                .value<QPixmap>();
        if (!terminalPixmap.isNull()
            && terminalPixmap.cacheKey()
                   != originalPixmap.cacheKey())
        {
            originalPixmap = terminalPixmap;
            scaledPixmap   = originalPixmap.scaled(
                24, 24, Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
            update(); // Trigger repaint
        }
    }
    else
    {
        setToolTip("Terminal");
    }
}

QRectF GlobalTerminalItem::boundingRect() const
{
    return QRectF(0, 0, scaledPixmap.width(),
                  scaledPixmap.height());
}

void GlobalTerminalItem::paint(
    QPainter                       *painter,
    const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Draw the scaled pixmap
    painter->drawPixmap(0, 0, scaledPixmap);

    // Draw selection outline if selected
    if (option->state & QStyle::State_Selected)
    {
        QPen pen(Qt::red, 1, Qt::DashLine);
        painter->setPen(pen);
        painter->drawRect(boundingRect());
    }
}

QVariant
GlobalTerminalItem::itemChange(GraphicsItemChange change,
                               const QVariant    &value)
{
    // Emit position changed signal when position has
    // changed
    if (change == ItemPositionHasChanged)
    {
        emit positionChanged(pos());
    }

    return QGraphicsObject::itemChange(change, value);
}

void GlobalTerminalItem::hoverEnterEvent(
    QGraphicsSceneHoverEvent *event)
{
    // Change cursor to hand pointer on hover
    setCursor(QCursor(Qt::PointingHandCursor));
    QGraphicsObject::hoverEnterEvent(event);
}

void GlobalTerminalItem::hoverLeaveEvent(
    QGraphicsSceneHoverEvent *event)
{
    // Reset cursor when leaving
    unsetCursor();
    QGraphicsObject::hoverLeaveEvent(event);
}

void GlobalTerminalItem::mousePressEvent(
    QGraphicsSceneMouseEvent *event)
{
    // Emit clicked signal
    emit itemClicked(this);

    // Call base class implementation to handle selection
    // etc.
    QGraphicsObject::mousePressEvent(event);
}

QMap<QString, QVariant> GlobalTerminalItem::toDict() const
{
    QMap<QString, QVariant> data;

    // Store position
    QMap<QString, QVariant> posMap;
    posMap["x"]      = pos().x();
    posMap["y"]      = pos().y();
    data["position"] = posMap;

    // Store z-value, visibility and selection state
    data["z_value"]  = zValue();
    data["visible"]  = isVisible();
    data["selected"] = isSelected();
    data["tooltip"]  = toolTip();

    // Store linked terminal ID if available
    if (linkedTerminalItem)
    {
        QMap<QString, QVariant> props =
            linkedTerminalItem->property("properties")
                .toMap();
        data["linked_terminal_id"] =
            props.value("ID", QVariant());
    }

    return data;
}

GlobalTerminalItem *GlobalTerminalItem::fromDict(
    const QMap<QString, QVariant> &data,
    const QPixmap &pixmap, QGraphicsItem *parent)
{
    // Create new instance with pixmap
    GlobalTerminalItem *instance =
        new GlobalTerminalItem(pixmap, nullptr, parent);

    // Set position if available
    if (data.contains("position"))
    {
        QMap<QString, QVariant> posMap =
            data["position"].toMap();
        QPointF pos(posMap.value("x", 0).toDouble(),
                    posMap.value("y", 0).toDouble());
        instance->setPos(pos);
    }

    // Set other properties
    instance->setZValue(
        data.value("z_value", 0).toDouble());
    instance->setVisible(
        data.value("visible", true).toBool());
    instance->setSelected(
        data.value("selected", false).toBool());
    instance->setToolTip(
        data.value("tooltip", "Terminal").toString());

    // Note: The linked terminal needs to be set separately
    // after all terminals have been created, using the
    // "linked_terminal_id" stored in the data

    return instance;
}

} // namespace GUI
} // namespace CargoNetSim
