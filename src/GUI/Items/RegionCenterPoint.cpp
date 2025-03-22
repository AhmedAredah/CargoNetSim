#include "RegionCenterPoint.h"

#include <QApplication>
#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QMap>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace CargoNetSim {
namespace GUI {

RegionCenterPoint::RegionCenterPoint(
    const QColor                  &color,
    const QMap<QString, QVariant> &properties,
    QGraphicsItem                 *parent)
    : QGraphicsObject(parent)
    , color(color)
    , properties(properties) {
    // Set high Z-value to ensure visibility
    setZValue(100);

    // Initialize properties if none were provided
    if (this->properties.isEmpty()) {
        this->properties = {
            {"Type", QString("Region Center")},
            {"Latitude", "0.0000000"},
            {"Longitude", "0.0000000"},
            {"Shared Latitude", "0.0000000"},
            {"Shared Longitude", "0.0000000"}};
    }

    // Enable needed flags
    setFlags(QGraphicsItem::ItemIsSelectable
             | QGraphicsItem::ItemIsMovable
             | QGraphicsItem::ItemSendsGeometryChanges
             | QGraphicsItem::ItemIgnoresTransformations);

    setAcceptHoverEvents(true);
}

void RegionCenterPoint::updateCoordinates(double lat,
                                          double lon) {
    QString oldLat = properties["Latitude"].toString();
    QString oldLon = properties["Longitude"].toString();

    // Format with 6 decimal places
    properties["Latitude"]  = QString::number(lat, 'f', 6);
    properties["Longitude"] = QString::number(lon, 'f', 6);

    // Only emit signals if values actually changed
    if (properties["Latitude"].toString() != oldLat
        || properties["Longitude"].toString() != oldLon) {
        emit coordinatesChanged(lat, lon);
        emit propertyChanged("Latitude",
                             properties["Latitude"]);
        emit propertyChanged("Longitude",
                             properties["Longitude"]);
    }

    update();
}

void RegionCenterPoint::updateSharedCoordinates(
    double lat, double lon) {
    QString oldLat =
        properties["Shared Latitude"].toString();
    QString oldLon =
        properties["Shared Longitude"].toString();

    // Format with 6 decimal places
    properties["Shared Latitude"] =
        QString::number(lat, 'f', 6);
    properties["Shared Longitude"] =
        QString::number(lon, 'f', 6);

    // Only emit signals if values actually changed
    if (properties["Shared Latitude"].toString() != oldLat
        || properties["Shared Longitude"].toString()
               != oldLon) {
        emit sharedCoordinatesChanged(lat, lon);
        emit propertyChanged("Shared Latitude",
                             properties["Shared Latitude"]);
        emit propertyChanged(
            "Shared Longitude",
            properties["Shared Longitude"]);
    }

    update();
}

void RegionCenterPoint::setColor(const QColor &newColor) {
    if (color != newColor) {
        color = newColor;
        emit colorChanged(color);
        update();
    }
}

void RegionCenterPoint::updateProperties(
    const QMap<QString, QVariant> &newProperties) {
    for (auto it = newProperties.constBegin();
         it != newProperties.constEnd(); ++it) {
        properties[it.key()] = it.value();
    }
    emit propertiesChanged();
}

void RegionCenterPoint::updateCoordinatesFromPosition() {
    QGraphicsScene *graphicsScene = scene();
    if (!graphicsScene
        || graphicsScene->views().isEmpty()) {
        return;
    }

    QGraphicsView *view = graphicsScene->views().first();
    if (!view) {
        return;
    }

    // Get the view that contains this item
    QObject *viewObj = view->parent();
    while (viewObj
           && !viewObj->inherits(
               "CargoNetSim::GUI::GraphicsView")) {
        viewObj = viewObj->parent();
    }

    if (!viewObj) {
        return;
    }

    // Then try the call with the correct type name as
    // string
    QPair<double, double> result;

    // First, ensure the type is properly registered with
    // the Qt meta-type system
    qRegisterMetaType<QPair<double, double>>(
        "QPair<double,double>");

    using Coordinates = QPair<double, double>;

    bool invoked = QMetaObject::invokeMethod(
        viewObj, "sceneToWGS84", Qt::AutoConnection,
        Q_RETURN_ARG(Coordinates, result),
        Q_ARG(QPointF, pos()));

    if (invoked) {
        updateCoordinates(result.first, result.second);
    } else {
        // Handle the case where invokeMethod fails
    }
}

QRectF RegionCenterPoint::boundingRect() const {
    return QRectF(-10, -10, 20, 20);
}

void RegionCenterPoint::paint(
    QPainter                       *painter,
    const QStyleOptionGraphicsItem *option,
    QWidget                        *widget) {
    // Draw outer circle
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(color));
    painter->drawEllipse(-8, -8, 16, 16);

    // Draw center cross
    painter->setPen(QPen(Qt::black, 1));
    painter->drawLine(-4, 0, 4, 0);
    painter->drawLine(0, -4, 0, 4);

    // Draw selection indicator if selected
    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(Qt::red, 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

void RegionCenterPoint::mousePressEvent(
    QGraphicsSceneMouseEvent *event) {
    dragOffset = event->pos();
    emit clicked(this);
    QGraphicsObject::mousePressEvent(event);
}

QVariant
RegionCenterPoint::itemChange(GraphicsItemChange change,
                              const QVariant    &value) {
    if (change == ItemPositionChange && scene()) {
        // If drag offset is set, adjust position during
        // drag
        if (dragOffset != QPointF()) {
            QGraphicsView *view = scene()->views().first();
            QPointF        mousePos = view->mapToScene(
                view->mapFromGlobal(QCursor::pos()));
            return mousePos - dragOffset;
        }
    } else if (change == ItemPositionHasChanged
               && scene()) {
        // Update coordinates when position changes
        updateCoordinatesFromPosition();

        // Emit position changed signal
        emit positionChanged(pos());
    }

    return QGraphicsObject::itemChange(change, value);
}

void RegionCenterPoint::hoverEnterEvent(
    QGraphicsSceneHoverEvent *event) {
    setCursor(QCursor(Qt::PointingHandCursor));
    QGraphicsObject::hoverEnterEvent(event);
}

void RegionCenterPoint::hoverLeaveEvent(
    QGraphicsSceneHoverEvent *event) {
    unsetCursor();
    QGraphicsObject::hoverLeaveEvent(event);
}

QMap<QString, QVariant> RegionCenterPoint::toDict() const {
    QMap<QString, QVariant> data;

    // Create position map
    QMap<QString, QVariant> posMap;
    posMap["x"] = pos().x();
    posMap["y"] = pos().y();

    // Add all data to the map
    data["position"]   = posMap;
    data["color"]      = color.name();
    data["properties"] = properties;
    data["selected"]   = isSelected();
    data["visible"]    = isVisible();
    data["z_value"]    = zValue();

    return data;
}

RegionCenterPoint *RegionCenterPoint::fromDict(
    const QMap<QString, QVariant> &data) {
    // Parse color from the hex string
    QColor color(data.value("color", "#000000").toString());

    // Create new instance
    RegionCenterPoint *instance = new RegionCenterPoint(
        color, data.value("properties").toMap());

    // Set position
    if (data.contains("position")) {
        QMap<QString, QVariant> posMap =
            data["position"].toMap();
        QPointF pos(posMap.value("x", 0).toDouble(),
                    posMap.value("y", 0).toDouble());
        instance->setPos(pos);
    }

    // Set other properties
    instance->setSelected(
        data.value("selected", false).toBool());
    instance->setVisible(
        data.value("visible", true).toBool());
    instance->setZValue(
        data.value("z_value", 2).toDouble());

    return instance;
}

} // namespace GUI
} // namespace CargoNetSim
