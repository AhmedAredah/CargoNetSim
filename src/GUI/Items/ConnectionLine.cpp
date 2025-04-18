#include "ConnectionLine.h"
#include "../Items/GlobalTerminalItem.h"
#include "../Items/TerminalItem.h"
#include "ConnectionLabel.h"

#include "AnimationObject.h"
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>
#include <cmath>

namespace CargoNetSim
{
namespace GUI
{

// Initialize static members
int ConnectionLine::CONNECTION_LINE_ID = 0;

// Define connection type styles with valid pen styles
const QMap<QString, QMap<QString, QVariant>>
    ConnectionLine::CONNECTION_STYLES = {
        {"Truck",
         {{"color", QColor(Qt::magenta)},
          {"width", 5},
          {"style", static_cast<int>(Qt::SolidLine)},
          {"offset", 0}}},
        {"Rail",
         {{"color", QColor(Qt::darkGray)},
          {"width", 5},
          {"style", static_cast<int>(Qt::SolidLine)},
          {"offset", 0}}},
        {"Ship",
         {{"color", QColor(Qt::blue)},
          {"width", 5},
          {"style", static_cast<int>(Qt::SolidLine)},
          {"offset", 0}}}};

ConnectionLine::ConnectionLine(
    QGraphicsItem *startItem, QGraphicsItem *endItem,
    const QString                 &connectionType,
    const QMap<QString, QVariant> &properties,
    const QString &region, QGraphicsItem *parent)
    : GraphicsObjectBase(parent)
    , m_startItem(startItem)
    , m_endItem(endItem)
    , m_connectionType(connectionType)
    , m_properties(properties)
    , m_id(getNewConnectionID())
    , m_isHovered(false)
    , m_animObject(nullptr)
    , m_animation(nullptr)
{
    // Set higher z-value to ensure visibility
    setZValue(4);

    // Enable hover events
    setAcceptHoverEvents(true);

    // Set flags
    setFlags(QGraphicsItem::ItemIsSelectable);

    // Initialize properties if needed
    if (m_properties.isEmpty())
    {
        initializeProperties(region);
    }
    else
    {
        m_properties["Region"] = region;
    }

    // Create label
    m_label = new ConnectionLabel(this);
    m_label->setText(m_connectionType.at(0));

    // Set label color based on connection type
    QColor color = CONNECTION_STYLES.value(m_connectionType)
                       .value("color")
                       .value<QColor>();
    m_label->setColor(color);

    // Connect signals
    createConnections();

    // Initialize position and geometry
    updatePosition();
}

ConnectionLine::~ConnectionLine()
{
    // Label is automatically deleted as a child item
}

void ConnectionLine::setRegion(const QString &region)
{
    if (m_properties["Region"].toString() != region)
    {
        m_properties["Region"] = region;
        emit regionChanged(region);
    }
}

void ConnectionLine::setConnectionType(const QString &type)
{
    if (m_connectionType != type
        && CONNECTION_STYLES.contains(type))
    {
        m_connectionType                = type;
        m_properties["Connection type"] = type;

        // Update label
        m_label->setText(type.at(0));
        QColor color = CONNECTION_STYLES.value(type)
                           .value("color")
                           .value<QColor>();
        m_label->setColor(color);

        // Update geometry and redraw
        updatePosition();
        update();

        emit connectionTypeChanged(type);
    }
}

void ConnectionLine::setProperty(const QString  &key,
                                 const QVariant &value)
{
    if (m_properties.value(key) != value)
    {
        m_properties[key] = value;
        update(); // Redraw if necessary
        emit propertyChanged(key, value);
    }
}

void ConnectionLine::createConnections()
{
    // Connect to start and end items' position changes
    if (dynamic_cast<TerminalItem *>(m_startItem))
    {
        connect(
            dynamic_cast<TerminalItem *>(m_startItem),
            &TerminalItem::positionChanged, this,
            &ConnectionLine::onStartItemPositionChanged);
    }
    else if (dynamic_cast<GlobalTerminalItem *>(
                 m_startItem))
    {
        connect(
            dynamic_cast<GlobalTerminalItem *>(m_startItem),
            &GlobalTerminalItem::positionChanged, this,
            &ConnectionLine::onStartItemPositionChanged);
    }

    if (dynamic_cast<TerminalItem *>(m_endItem))
    {
        connect(dynamic_cast<TerminalItem *>(m_endItem),
                &TerminalItem::positionChanged, this,
                &ConnectionLine::onEndItemPositionChanged);
    }
    else if (dynamic_cast<GlobalTerminalItem *>(m_endItem))
    {
        connect(
            dynamic_cast<GlobalTerminalItem *>(m_endItem),
            &GlobalTerminalItem::positionChanged, this,
            &ConnectionLine::onEndItemPositionChanged);
    }

    // Connect label's clicked signal to our clicked signal
    connect(m_label, &ConnectionLabel::clicked,
            [this]() { emit clicked(this); });
}

void ConnectionLine::initializeProperties(QString region)
{
    m_properties = {
        {"Type", "Connection"},
        {"Connection type", m_connectionType},
        {"Region", region},
        {"cost", "0.0"},             // USD
        {"travelTime", "0.0"},       // Hours
        {"distance", "0.0"},         // Km
        {"carbonEmissions", "0.0"},  // ton CO2
        {"risk", "0.0"},             // percentage [0-100]
        {"energyConsumption", "0.0"} // kWh
    };
}

void ConnectionLine::onStartItemPositionChanged(
    const QPointF &newPos)
{
    updatePosition(newPos, true);
    emit startPositionChanged(newPos);
}

void ConnectionLine::onEndItemPositionChanged(
    const QPointF &newPos)
{
    updatePosition(newPos, false);
    emit endPositionChanged(newPos);
}

void ConnectionLine::updatePosition(const QPointF &newPos,
                                    bool           isStart)
{
    prepareGeometryChange();

    // Calculate terminal centers
    QPointF startCenter;
    QPointF endCenter;

    if (isStart && !newPos.isNull())
    {
        startCenter = newPos;
        endCenter = m_endItem->scenePos();
    }
    else if (!isStart && !newPos.isNull())
    {
        startCenter = m_startItem->scenePos();
        endCenter = newPos;
    }
    else
    {
        startCenter = m_startItem->scenePos();
        endCenter = m_endItem->scenePos();
    }

    // Create base line and apply offset
    QLineF baseLine(startCenter, endCenter);
    m_line = calculateOffsetLine(baseLine);

    // Calculate midpoint and line properties
    qreal midX       = (m_line.x1() + m_line.x2()) / 2;
    qreal midY       = (m_line.y1() + m_line.y2()) / 2;
    qreal dx         = m_line.x2() - m_line.x1();
    qreal dy         = m_line.y2() - m_line.y1();
    qreal lineLength = std::sqrt(dx * dx + dy * dy);

    // Default control point and label position
    qreal ctrlX = midX, ctrlY = midY;
    qreal labelX = midX, labelY = midY;

    if (lineLength > 0)
    {
        // Determine curve direction based on line orientation
        if (m_connectionType != "Truck")
        {
            // Choose perpendicular offset direction
            bool  isVertical = std::abs(dy) > std::abs(dx);
            qreal offset     = 30; // Control point offset

            if (isVertical)
            {
                // For vertical alignments, curve horizontally
                qreal nx = (m_connectionType == "Ship") ? 1.0 : -1.0;
                qreal ny = 0.0;

                // Set control point
                ctrlX = midX + nx * offset;
                ctrlY = midY + ny * offset;
            }
            else
            {
                // For horizontal alignments, curve vertically
                qreal nx = 0.0;
                qreal ny = (m_connectionType == "Rail") ? -1.0 : 1.0;

                // Set control point
                ctrlX = midX + nx * offset;
                ctrlY = midY + ny * offset;
            }

            // Calculate label position at t=0.5 (middle of Bezier curve)
            qreal t = 0.5;
            labelX  = (1 - t) * (1 - t) * m_line.x1()
                     + 2 * (1 - t) * t * ctrlX
                     + t * t * m_line.x2();

            labelY = (1 - t) * (1 - t) * m_line.y1()
                     + 2 * (1 - t) * t * ctrlY
                     + t * t * m_line.y2();
        }
    }

    // Store control points for painting
    m_ctrlPoint = QPointF(ctrlX, ctrlY);

    // Set label position
    m_label->setPos(labelX, labelY);

    // Update bounding rectangle
    int padding = std::max(
        5, CONNECTION_STYLES.value(m_connectionType)
               .value("width")
               .toInt());

    if (m_connectionType == "Truck")
    {
        // Simple rectangle for straight lines
        m_boundingRect = QRectF(
            std::min(m_line.x1(), m_line.x2()) - padding,
            std::min(m_line.y1(), m_line.y2()) - padding,
            std::abs(m_line.x2() - m_line.x1())
                + (2 * padding),
            std::abs(m_line.y2() - m_line.y1())
                + (2 * padding));
    }
    else
    {
        // For curves, include the control point
        QList<QPointF> points = {m_line.p1(), m_ctrlPoint,
                                 m_line.p2()};
        qreal minX = std::numeric_limits<qreal>::max();
        qreal minY = std::numeric_limits<qreal>::max();
        qreal maxX = std::numeric_limits<qreal>::min();
        qreal maxY = std::numeric_limits<qreal>::min();

        for (const QPointF &p : points)
        {
            minX = std::min(minX, p.x());
            minY = std::min(minY, p.y());
            maxX = std::max(maxX, p.x());
            maxY = std::max(maxY, p.y());
        }

        m_boundingRect =
            QRectF(minX - padding, minY - padding,
                   maxX - minX + 2 * padding,
                   maxY - minY + 2 * padding);
    }

    update();
}

QLineF ConnectionLine::calculateOffsetLine(
    const QLineF &originalLine) const
{
    // Get the offset distance for this connection type
    qreal offset = CONNECTION_STYLES.value(m_connectionType)
                       .value("offset")
                       .toDouble();

    if (offset == 0)
    {
        return originalLine;
    }

    // Calculate the perpendicular vector
    qreal dx     = originalLine.dx();
    qreal dy     = originalLine.dy();
    qreal length = std::sqrt(dx * dx + dy * dy);

    if (length == 0)
    {
        return originalLine;
    }

    // Normalize and rotate 90 degrees
    qreal perpX = -dy / length;
    qreal perpY = dx / length;

    // Create the offset line
    qreal startX = originalLine.x1() + offset * perpX;
    qreal startY = originalLine.y1() + offset * perpY;
    qreal endX   = originalLine.x2() + offset * perpX;
    qreal endY   = originalLine.y2() + offset * perpY;

    return QLineF(startX, startY, endX, endY);
}

QRectF ConnectionLine::boundingRect() const
{
    return m_boundingRect;
}

QPainterPath ConnectionLine::shape() const
{
    // For selection purposes, use a simplified selection
    // area that focuses more on the label and less on the
    // line itself
    QPainterPath path;

    // Add the label's shape (with some padding)
    QRectF labelSceneRect = m_label->sceneBoundingRect();
    QRectF labelLocalRect =
        mapFromScene(labelSceneRect).boundingRect();

    // Add padding around the label for easier selection
    int padding = 10;
    labelLocalRect.adjust(-padding, -padding, padding,
                          padding);
    path.addRect(labelLocalRect);

    // Add a narrow path along the line for selection
    qreal lineWidth = 10; // Narrow selection width

    // Get line control points
    QPointF start = m_line.p1();
    QPointF end   = m_line.p2();

    // Create a narrow rectangle along the line for
    // selection
    if (m_connectionType == "Truck")
    {
        // For straight line
        qreal angle =
            m_line.angle()
            * (M_PI / 180.0); // Convert to radians
        qreal dx = lineWidth * 0.5 * std::sin(angle);
        qreal dy = lineWidth * 0.5 * std::cos(angle);

        QPolygonF poly;
        poly.append(
            QPointF(start.x() + dx, start.y() - dy));
        poly.append(
            QPointF(start.x() - dx, start.y() + dy));
        poly.append(QPointF(end.x() - dx, end.y() + dy));
        poly.append(QPointF(end.x() + dx, end.y() - dy));
        path.addPolygon(poly);
    }
    else
    {
        // For curved lines, use a simplified approach
        qreal midX = (start.x() + end.x()) / 2;
        qreal midY = (start.y() + end.y()) / 2;

        // Add midpoint with larger radius for better
        // selection of curved lines
        path.addEllipse(QPointF(midX, midY), 15, 15);

        // Add small circles at endpoints
        path.addEllipse(start, 5, 5);
        path.addEllipse(end, 5, 5);
    }

    return path;
}

void ConnectionLine::paint(
    QPainter                       *painter,
    const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Get style for this connection type
    QMap<QString, QVariant> style =
        CONNECTION_STYLES.value(m_connectionType);

    // Set up pen with appropriate scale
    QGraphicsView *view      = scene()->views().first();
    qreal          viewScale = view->transform().m11();

    // Create pen
    QPen pen(style.value("color").value<QColor>(),
             style.value("width").toInt() / viewScale,
             static_cast<Qt::PenStyle>(
                 style.value("style").toInt()));

    painter->setPen(pen);

    // Draw the path
    QPainterPath path;
    path.moveTo(m_line.p1());

    if (m_connectionType == "Truck")
    {
        path.lineTo(m_line.p2());
    }
    else
    {
        path.quadTo(m_ctrlPoint, m_line.p2());
    }

    painter->drawPath(path);

    // Draw debug bounding rect if needed
    if (false)
    { // Set to true for debugging
        QPen debugPen(QColor(0, 255, 0), 1, Qt::DashLine);
        debugPen.setCosmetic(true);
        painter->setPen(debugPen);
        painter->drawRect(boundingRect());
    }
}

void ConnectionLine::updateProperties(
    const QMap<QString, QVariant> &newProperties)
{
    for (auto it = newProperties.constBegin();
         it != newProperties.constEnd(); ++it)
    {
        m_properties[it.key()] = it.value();
    }
    emit propertiesChanged();
}

bool ConnectionLine::isSelected() const
{
    return m_label->isSelected();
}

void ConnectionLine::setSelected(bool selected)
{
    // Set both custom and Qt selection
    QGraphicsItem::setSelected(selected);
    m_label->setSelected(selected);
    m_label->update();
}

void ConnectionLine::mousePressEvent(
    QGraphicsSceneMouseEvent *event)
{
    // Prevent selection by ignoring the event
    event->ignore();
}

void ConnectionLine::hoverEnterEvent(
    QGraphicsSceneHoverEvent *event)
{
    m_isHovered = true;
    update();
    QGraphicsObject::hoverEnterEvent(event);
}

void ConnectionLine::hoverLeaveEvent(
    QGraphicsSceneHoverEvent *event)
{
    m_isHovered = false;
    update();
    QGraphicsObject::hoverLeaveEvent(event);
}

void ConnectionLine::flash(bool          evenIfHidden,
                           const QColor &color)
{
    bool wasHidden = !isVisible();
    if (evenIfHidden && wasHidden)
    {
        setVisible(true);
    }

    // If a flash is already running, stop and clean it up
    if (m_animation)
    {
        m_animation->stop();
        m_animation->deleteLater();
        m_animation = nullptr;
    }

    if (m_animObject)
    {
        m_animObject->deleteLater();
        m_animObject = nullptr;
    }

    // Create a path item as an overlay to follow the line
    QPainterPath path;
    qreal        penWidth =
        CONNECTION_STYLES.value(m_connectionType)
            .value("width")
            .toInt()
        * 3;

    QPen pen(color, penWidth, Qt::SolidLine);
    path.moveTo(m_line.p1());

    if (m_connectionType == "Truck")
    {
        path.lineTo(m_line.p2());
    }
    else
    {
        path.quadTo(m_ctrlPoint, m_line.p2());
    }

    QGraphicsPathItem *overlay =
        new QGraphicsPathItem(path, this);
    overlay->setPen(pen);
    overlay->setZValue(100);

    // Store animation objects as instance variables
    m_animObject = new AnimationObject(this);
    static_cast<AnimationObject *>(m_animObject)
        ->setOverlay(overlay);

    m_animation = new QPropertyAnimation(m_animObject,
                                         "opacity", this);
    m_animation->setDuration(1000);
    m_animation->setLoopCount(3);
    m_animation->setStartValue(1.0);
    m_animation->setKeyValueAt(0.5, 0.0);
    m_animation->setEndValue(1.0);

    // Set up cleanup on animation completion
    connect(m_animation, &QPropertyAnimation::finished,
            [=]() {
                if (overlay && scene())
                {
                    scene()->removeItem(overlay);
                }

                if (evenIfHidden && wasHidden)
                {
                    setVisible(false);
                }

                // Clean up
                m_animation->deleteLater();
                m_animation = nullptr;
                m_animObject->deleteLater();
                m_animObject = nullptr;
            });

    m_animation->start();
}

QMap<QString, QVariant> ConnectionLine::toDict() const
{
    QMap<QString, QVariant> data;

    // Store IDs for serialization
    int     startItemId = -1;
    int     endItemId   = -1;
    QString startItemType;
    QString endItemType;

    // Extract IDs from terminal items
    if (TerminalItem *terminal =
            dynamic_cast<TerminalItem *>(m_startItem))
    {
        startItemId =
            terminal->getProperties().value("ID").toInt();
        startItemType = "TerminalItem";
    }
    else if (GlobalTerminalItem *globalTerminal =
                 dynamic_cast<GlobalTerminalItem *>(
                     m_startItem))
    {
        if (globalTerminal->getLinkedTerminalItem())
        {
            startItemId =
                globalTerminal->getLinkedTerminalItem()
                    ->getProperties()
                    .value("ID")
                    .toInt();
            startItemType = "GlobalTerminalItem";
        }
    }

    if (TerminalItem *terminal =
            dynamic_cast<TerminalItem *>(m_endItem))
    {
        endItemId =
            terminal->getProperties().value("ID").toInt();
        endItemType = "TerminalItem";
    }
    else if (GlobalTerminalItem *globalTerminal =
                 dynamic_cast<GlobalTerminalItem *>(
                     m_endItem))
    {
        if (globalTerminal->getLinkedTerminalItem())
        {
            endItemId =
                globalTerminal->getLinkedTerminalItem()
                    ->getProperties()
                    .value("ID")
                    .toInt();
            endItemType = "GlobalTerminalItem";
        }
    }

    data["id"]              = m_id;
    data["start_item_id"]   = startItemId;
    data["start_item_type"] = startItemType;
    data["end_item_id"]     = endItemId;
    data["end_item_type"]   = endItemType;
    data["connection_type"] = m_connectionType;
    data["properties"]      = m_properties;
    data["selected"]        = isSelected();
    data["z_value"]         = zValue();
    data["visible"]         = isVisible();

    // Add label data
    data["label"] = m_label->toDict();

    return data;
}

ConnectionLine *ConnectionLine::fromDict(
    const QMap<QString, QVariant>    &data,
    const QMap<int, QGraphicsItem *> &terminalsByID,
    QGraphicsScene *globalScene, QGraphicsItem *parent)
{
    // Find terminals
    int startId = data["start_item_id"].toInt();
    int endId   = data["end_item_id"].toInt();

    QGraphicsItem *startItem = terminalsByID.value(startId);
    QGraphicsItem *endItem   = terminalsByID.value(endId);

    if (!startItem)
    {
        qWarning() << "Start terminal with ID" << startId
                   << "not found";
        return nullptr;
    }

    if (!endItem)
    {
        qWarning() << "End terminal with ID" << endId
                   << "not found";
        return nullptr;
    }

    // Handle global items if needed
    if (globalScene
        && data["start_item_type"].toString()
               == "GlobalTerminalItem")
    {
        for (QGraphicsItem *item : globalScene->items())
        {
            GlobalTerminalItem *globalItem =
                dynamic_cast<GlobalTerminalItem *>(item);
            if (globalItem
                && globalItem->getLinkedTerminalItem()
                && globalItem->getLinkedTerminalItem()
                           ->getProperties()
                           .value("ID")
                           .toInt()
                       == startId)
            {
                startItem = globalItem;
                break;
            }
        }
    }

    if (globalScene
        && data["end_item_type"].toString()
               == "GlobalTerminalItem")
    {
        for (QGraphicsItem *item : globalScene->items())
        {
            GlobalTerminalItem *globalItem =
                dynamic_cast<GlobalTerminalItem *>(item);
            if (globalItem
                && globalItem->getLinkedTerminalItem()
                && globalItem->getLinkedTerminalItem()
                           ->getProperties()
                           .value("ID")
                           .toInt()
                       == endId)
            {
                endItem = globalItem;
                break;
            }
        }
    }

    // Create connection line
    ConnectionLine *connection = new ConnectionLine(
        startItem, endItem,
        data.value("connection_type", "Truck").toString(),
        data.value("properties").toMap(),
        data.value("region", "Default Region").toString(),
        parent);

    // Set ID
    connection->m_id = data["id"].toInt();

    // Set visual properties
    connection->setSelected(
        data.value("selected", false).toBool());
    connection->setZValue(
        data.value("z_value", 4).toDouble());
    connection->setVisible(
        data.value("visible", true).toBool());

    // Update position
    connection->updatePosition();

    return connection;
}

void ConnectionLine::resetClassIDs()
{
    CONNECTION_LINE_ID = 0;
}

void ConnectionLine::setClassIDs(
    const QMap<int, ConnectionLine *> &allConnectionsById)
{
    if (allConnectionsById.isEmpty())
    {
        CONNECTION_LINE_ID = 0;
        return;
    }

    // Find maximum ID
    int maxId = 0;
    for (ConnectionLine *connection :
         allConnectionsById.values())
    {
        maxId = std::max(maxId, connection->connectionId());
    }

    CONNECTION_LINE_ID = maxId;
}

int ConnectionLine::getNewConnectionID()
{
    CONNECTION_LINE_ID++;
    return CONNECTION_LINE_ID;
}

} // namespace GUI
} // namespace CargoNetSim
