#include "ConnectionLabel.h"

#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace CargoNetSim
{
namespace GUI
{

ConnectionLabel::ConnectionLabel(QGraphicsItem *parent)
    : GraphicsObjectBase(parent)
    , m_text("")
    , m_color(Qt::black)
    , m_isHovered(false)
    , m_isSelected(false)
    , m_boundingRect(-16, -16, 32, 32) // Fixed size 32x32
{
    // Set flags
    setFlag(QGraphicsItem::ItemIgnoresTransformations);
    setFlag(QGraphicsItem::ItemIsSelectable);

    // Enable hover events
    setAcceptHoverEvents(true);

    // Set Z-value slightly higher than parent connection
    // line
    setZValue(5);
}

void ConnectionLabel::setText(const QString &text)
{
    if (m_text != text)
    {
        m_text = text;
        update();
        emit textChanged(text);
    }
}

void ConnectionLabel::setColor(const QColor &color)
{
    if (m_color != color)
    {
        m_color = color;
        update();
        emit colorChanged(color);
    }
}

void ConnectionLabel::setSelected(bool selected)
{
    if (m_isSelected != selected)
    {
        m_isSelected = selected;
        update();
        emit selectionChanged(selected);
    }
}

QRectF ConnectionLabel::boundingRect() const
{
    return m_boundingRect;
}

void ConnectionLabel::paint(
    QPainter                       *painter,
    const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Draw label background
    if (m_isHovered)
    {
        painter->setBrush(QBrush(QColor(
            255, 255, 0))); // Light yellow when hovered
    }
    else
    {
        painter->setBrush(QBrush(Qt::white));
    }

    painter->setPen(QPen(Qt::black, 1));
    painter->drawRect(boundingRect());

    // Draw the text
    QFont font = painter->font();
    font.setPointSize(15);
    painter->setFont(font);
    painter->setPen(QPen(m_color));
    painter->drawText(boundingRect(), Qt::AlignCenter,
                      m_text);

    // Draw selection indicator if selected
    if (m_isSelected)
    {
        painter->setPen(QPen(Qt::red, 2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(
            boundingRect().adjusted(-2, -2, 2, 2));
    }
}

void ConnectionLabel::mousePressEvent(
    QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        // Clear previous selections
        if (scene())
        {
            scene()->clearSelection();
        }

        // Select this label
        setSelected(true);
        update();

        // Emit clicked signal
        emit clicked();

        event->accept();
    }
    else
    {
        QGraphicsObject::mousePressEvent(event);
    }
}

void ConnectionLabel::hoverEnterEvent(
    QGraphicsSceneHoverEvent *event)
{
    m_isHovered = true;
    setCursor(QCursor(Qt::PointingHandCursor));
    update();
    QGraphicsObject::hoverEnterEvent(event);
}

void ConnectionLabel::hoverLeaveEvent(
    QGraphicsSceneHoverEvent *event)
{
    m_isHovered = false;
    unsetCursor();
    update();
    QGraphicsObject::hoverLeaveEvent(event);
}

QMap<QString, QVariant> ConnectionLabel::toDict() const
{
    QMap<QString, QVariant> data;

    data["text"]  = m_text;
    data["color"] = m_color.name();

    // Store position
    QMap<QString, QVariant> posMap;
    posMap["x"]      = pos().x();
    posMap["y"]      = pos().y();
    data["position"] = posMap;

    data["z_value"] = zValue();
    data["visible"] = isVisible();

    return data;
}

ConnectionLabel *ConnectionLabel::fromDict(
    const QMap<QString, QVariant> &data,
    QGraphicsItem                 *parent)
{
    ConnectionLabel *instance = new ConnectionLabel(parent);

    // Set properties
    instance->setText(data.value("text", "").toString());
    instance->setColor(
        QColor(data.value("color", "#000000").toString()));

    // Set position
    QMap<QString, QVariant> posMap =
        data.value("position").toMap();
    QPointF pos(posMap.value("x", 0).toDouble(),
                posMap.value("y", 0).toDouble());
    instance->setPos(pos);

    // Set other properties
    instance->setZValue(
        data.value("z_value", 5).toDouble());
    instance->setVisible(
        data.value("visible", true).toBool());

    return instance;
}

} // namespace GUI
} // namespace CargoNetSim
