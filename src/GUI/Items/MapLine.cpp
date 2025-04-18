#include "MapLine.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace CargoNetSim
{
namespace GUI
{

MapLine::MapLine(const QString &referenceNetworkID,
                 const QPointF &startPoint,
                 const QPointF &endPoint,
                 const QString &region,
                 const QMap<QString, QVariant> &properties)
    : startPoint(startPoint)
    , endPoint(endPoint)
    , m_properties(properties)
    , baseWidth(1)
    , pen(Qt::black, baseWidth)
    , m_animObject(nullptr)
    , m_animation(nullptr)
{
    this->m_properties["Network_ID"] = referenceNetworkID;
    this->m_properties["region"]     = region;

    // Set higher z-value to ensure lines are drawn above
    // the background but below points
    setZValue(3);

    // Make the line selectable
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void MapLine::setColor(const QColor &color)
{
    if (pen.color() != color)
    {
        pen.setColor(color);
        emit colorChanged(color);
        update();
    }
}

void MapLine::setPen(const QPen &newPen)
{
    if (pen != newPen)
    {
        QColor oldColor = pen.color();
        pen             = newPen;

        if (pen.color() != oldColor)
        {
            emit colorChanged(pen.color());
        }

        update();
    }
}

QRectF MapLine::boundingRect() const
{
    // Calculate bounding rect with padding
    return QRectF(qMin(startPoint.x(), endPoint.x()),
                  qMin(startPoint.y(), endPoint.y()),
                  qAbs(endPoint.x() - startPoint.x()),
                  qAbs(endPoint.y() - startPoint.y()))
        .adjusted(-2, -2, 2, 2);
}

void MapLine::paint(QPainter                       *painter,
                    const QStyleOptionGraphicsItem *option,
                    QWidget                        *widget)
{
    // Get the current view scale
    QGraphicsScene *itemScene = scene();
    if (!itemScene || itemScene->views().isEmpty())
    {
        return;
    }

    QGraphicsView *view      = itemScene->views().first();
    qreal          viewScale = view->transform().m11();

    // Scale the pen width inversely to maintain constant
    // visual thickness
    QPen scaledPen(pen);
    scaledPen.setWidth(
        qMax(1, qRound(baseWidth / viewScale)));

    // Change pen style if selected
    if (option->state & QStyle::State_Selected)
    {
        scaledPen.setColor(Qt::blue);
        scaledPen.setStyle(Qt::DashLine);
    }

    painter->setPen(scaledPen);
    painter->drawLine(startPoint, endPoint);
}

void MapLine::mousePressEvent(
    QGraphicsSceneMouseEvent *event)
{
    emit clicked(this);

    // Select all lines in the same region when this line is
    // clicked
    selectNetworkLines();

    QGraphicsObject::mousePressEvent(event);
}

void MapLine::selectNetworkLines()
{
    if (!scene())
    {
        return;
    }

    // Loop through all items and select those in the same
    // region
    for (QGraphicsItem *item : scene()->items())
    {
        MapLine *line = dynamic_cast<MapLine *>(item);
        if (line
            && line->getRegion()
                   == m_properties.value("region")
                          .toString())
        {
            line->setSelected(true);
        }
    }
}

QMap<QString, QVariant> MapLine::toDict() const
{
    QMap<QString, QVariant> data;
    data["referenced_network_ID"] =
        m_properties.value("Network_ID");

    // Convert points to dictionaries
    QMap<QString, QVariant> startPointDict;
    startPointDict["x"] = startPoint.x();
    startPointDict["y"] = startPoint.y();

    QMap<QString, QVariant> endPointDict;
    endPointDict["x"] = endPoint.x();
    endPointDict["y"] = endPoint.y();

    data["start_point"] = startPointDict;
    data["end_point"]   = endPointDict;
    data["properties"]  = m_properties;
    data["color"]       = pen.color().name();
    data["selected"]    = isSelected();
    data["z_value"]     = zValue();
    data["base_width"]  = baseWidth;

    return data;
}

MapLine *
MapLine::fromDict(const QMap<QString, QVariant> &data)
{
    // Extract start and end points
    QMap<QString, QVariant> startPointDict =
        data["start_point"].toMap();
    QMap<QString, QVariant> endPointDict =
        data["end_point"].toMap();

    qreal startX = startPointDict["x"].toDouble();
    qreal startY = startPointDict["y"].toDouble();
    qreal endX   = endPointDict["x"].toDouble();
    qreal endY   = endPointDict["y"].toDouble();

    MapLine *instance = new MapLine(
        data.value("referenced_network_ID").toString(),
        QPointF(startX, startY), QPointF(endX, endY),
        data.value("region", "default").toString(),
        data.value("properties").toMap());

    // Set color and other properties
    instance->setColor(
        QColor(data.value("color", "#000000").toString()));
    instance->baseWidth =
        data.value("base_width", 5).toInt();
    instance->setSelected(
        data.value("selected", false).toBool());
    instance->setZValue(
        data.value("z_value", 3).toDouble());

    return instance;
}

void MapLine::flash(bool evenIfHidden, const QColor &color)
{
    bool wasHidden = !isVisible();
    if (evenIfHidden && wasHidden)
    {
        setVisible(true);
    }

    // Create a path item as an overlay to follow the line
    QPainterPath path;
    QGraphicsView *view = scene()->views().isEmpty()
                              ? nullptr
                              : scene()->views().first();

    qreal viewScale = view ? view->transform().m11() : 1.0;
    qreal penWidth  = qMax(5.0, 6.0 / viewScale);

    QPen pen(color, penWidth, Qt::SolidLine);
    path.moveTo(startPoint);
    path.lineTo(endPoint);

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

} // namespace GUI
} // namespace CargoNetSim
