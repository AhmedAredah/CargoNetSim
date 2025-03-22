#include "BackgroundPhotoItem.h"

#include <QBuffer>
#include <QByteArray>
#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace CargoNetSim {
namespace GUI {

BackgroundPhotoItem::BackgroundPhotoItem(
    const QPixmap &pixmap, const QString &regionName,
    QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , pixmap(pixmap)
    , regionName(regionName)
    , locked(false)
    , currentOpacity(1.0) {
    // Set a low Z-value to stay below other items
    setZValue(-1);

    // Initialize properties
    properties["Type"] =
        QString("Background - %1").arg(regionName);
    properties["Region"]    = regionName;
    properties["Scale"]     = "1.0";
    properties["Latitude"]  = "0.0";
    properties["Longitude"] = "0.0";
    properties["Locked"]    = "False";
    properties["Opacity"]   = "1.0";

    // Configure flags for interaction
    setFlags(QGraphicsItem::ItemIsSelectable
             | QGraphicsItem::ItemIsMovable
             | QGraphicsItem::ItemSendsGeometryChanges);
}

void BackgroundPhotoItem::setLocked(bool newLocked) {
    if (locked != newLocked) {
        locked = newLocked;

        // Update the locked property
        updateProperty("Locked", locked ? "True" : "False");

        // Set appropriate flags based on locked state
        if (locked) {
            setFlags(QGraphicsItem::ItemIsSelectable);
        } else {
            setFlags(
                QGraphicsItem::ItemIsSelectable
                | QGraphicsItem::ItemIsMovable
                | QGraphicsItem::ItemSendsGeometryChanges);
        }

        // Notify about lock state change
        emit lockStateChanged(locked);
    }
}

bool BackgroundPhotoItem::isLocked() const {
    return locked;
}

void BackgroundPhotoItem::updateCoordinates() {
    // Get scene and view
    QGraphicsScene *graphicsScene = scene();
    if (!graphicsScene
        || graphicsScene->views().isEmpty()) {
        return;
    }

    QGraphicsView *view = graphicsScene->views().first();

    // Get parent class to access scene_to_wgs84 method
    QObject *parentObj = view->parent();
    if (!parentObj) {
        return;
    }

    // Convert to coordinates - we need to call a method on
    // the main window This would typically be implemented
    // by calling a controller or utility method
    // TODO: we're using a placeholder implementation
    double lat = 0.0, lon = 0.0;

    // Using Qt's meta-object system to call the method
    // dynamically
    QMetaObject::invokeMethod(parentObj, "sceneToWgs84",
                              Q_RETURN_ARG(double, lat),
                              Q_ARG(QPointF, pos()),
                              Q_ARG(double, lon));

    // Update properties
    updateProperty("Latitude",
                   QString::number(lat, 'f', 6));
    updateProperty("Longitude",
                   QString::number(lon, 'f', 6));
}

void BackgroundPhotoItem::setFromWGS84(double lat,
                                       double lon) {
    // Get scene and view
    QGraphicsScene *graphicsScene = scene();
    if (!graphicsScene
        || graphicsScene->views().isEmpty()) {
        return;
    }

    QGraphicsView *view = graphicsScene->views().first();

    // Get parent class to access wgs84_to_scene method
    QObject *parentObj = view->parent();
    if (!parentObj) {
        return;
    }

    // Convert from coordinates - call a method on the main
    // window
    QPointF scenePos;

    // Using Qt's meta-object system to call the method
    // dynamically
    QMetaObject::invokeMethod(
        parentObj, "wgs84ToScene",
        Q_RETURN_ARG(QPointF, scenePos), Q_ARG(double, lat),
        Q_ARG(double, lon));

    // Set position
    setPos(scenePos);

    // Update properties
    updateProperty("Latitude",
                   QString::number(lat, 'f', 6));
    updateProperty("Longitude",
                   QString::number(lon, 'f', 6));
}

QRectF BackgroundPhotoItem::boundingRect() const {
    float scale = getScale();
    return QRectF(0, 0, pixmap.width() * scale,
                  pixmap.height() * scale);
}

void BackgroundPhotoItem::updateScale() {
    prepareGeometryChange();
    update();

    // Notify about scale change
    emit scaleChanged(getScale());
}

void BackgroundPhotoItem::setRegion(const QString &region) {
    if (regionName != region) {
        regionName = region;
        emit regionChanged(region);
    }
}

float BackgroundPhotoItem::getScale() const {
    bool  ok    = false;
    float scale = properties.value("Scale", "1.0")
                      .toString()
                      .toFloat(&ok);
    return ok ? scale : 1.0f;
}

void BackgroundPhotoItem::setScale(float scale) {
    if (scale <= 0.0f) {
        scale = 0.1f; // Minimum scale
    }

    if (qAbs(scale - getScale()) > 0.001f) {
        updateProperty("Scale",
                       QString::number(scale, 'f', 2));
        updateScale();
    }
}

qreal BackgroundPhotoItem::opacity() const {
    return currentOpacity;
}

void BackgroundPhotoItem::setOpacity(qreal opacity) {
    opacity = qBound(0.0, opacity, 1.0);

    if (qAbs(opacity - currentOpacity) > 0.01) {
        currentOpacity = opacity;
        updateProperty("Opacity",
                       QString::number(opacity, 'f', 2));

        // Must call QGraphicsItem's setOpacity which will
        // trigger a redraw
        QGraphicsItem::setOpacity(opacity);

        // Notify about opacity change
        emit opacityChanged(opacity);
    }
}

void BackgroundPhotoItem::paint(
    QPainter                       *painter,
    const QStyleOptionGraphicsItem *option,
    QWidget                        *widget) {
    // Get current scale from properties
    float scale = getScale();

    // Calculate scaled dimensions
    float scaledWidth  = pixmap.width() * scale;
    float scaledHeight = pixmap.height() * scale;

    // Draw the scaled pixmap
    painter->drawPixmap(
        QRectF(0, 0, scaledWidth, scaledHeight), pixmap,
        QRectF(pixmap.rect()));

    // Draw selection rectangle if selected
    if (option->state & QStyle::State_Selected) {
        QPen pen(Qt::red, 2, Qt::DashLine);
        painter->setPen(pen);
        painter->drawRect(
            QRectF(0, 0, scaledWidth, scaledHeight));
    }
}

void BackgroundPhotoItem::mousePressEvent(
    QGraphicsSceneMouseEvent *event) {
    if (!locked) {
        // Store drag offset for position adjustment
        dragOffset = event->pos();

        // Emit clicked signal
        emit clicked(this);

        // Call parent implementation to handle selection
        QGraphicsObject::mousePressEvent(event);
    } else {
        // Still emit clicked signal when locked, but don't
        // allow movement
        emit clicked(this);
        event->accept();
    }
}

QVariant
BackgroundPhotoItem::itemChange(GraphicsItemChange change,
                                const QVariant    &value) {
    if (change == ItemPositionChange && scene()) {
        // If locked, prevent movement
        if (locked) {
            return pos();
        }

        // If dragging, adjust position based on drag offset
        if (dragOffset != QPointF()) {
            QPointF newPos = value.toPointF();

            // If mouse grabber is this item, adjust by
            // cursor position
            if (scene()->mouseGrabberItem() == this) {
                QGraphicsView *view =
                    scene()->views().first();
                QPointF mousePos = view->mapToScene(
                    view->mapFromGlobal(QCursor::pos()));
                return mousePos - dragOffset;
            }
        }

        return value;
    } else if (change == ItemPositionHasChanged
               && scene()) {
        // Update coordinates and notify about position
        // change
        updateCoordinates();
        emit positionChanged(pos());
    }

    return QGraphicsObject::itemChange(change, value);
}

void BackgroundPhotoItem::updateProperties(
    const QMap<QString, QVariant> &newProperties) {
    for (auto it = newProperties.constBegin();
         it != newProperties.constEnd(); ++it) {
        properties[it.key()] = it.value();
    }
    emit propertiesChanged();
}

void BackgroundPhotoItem::updateProperty(
    const QString &key, const QVariant &value) {
    // Only update if value actually changes
    if (properties.value(key) != value) {
        properties[key] = value;
        emit propertyChanged(key, value);
    }
}

QMap<QString, QVariant>
BackgroundPhotoItem::toDict() const {
    QMap<QString, QVariant> data;

    // Store position
    QMap<QString, QVariant> posMap;
    posMap["x"]      = pos().x();
    posMap["y"]      = pos().y();
    data["position"] = posMap;

    // Store basic properties
    data["region_name"] = regionName;
    data["properties"]  = properties;
    data["locked"]      = locked;
    data["selected"]    = isSelected();
    data["z_value"]     = zValue();
    data["visible"]     = isVisible();
    data["opacity"]     = currentOpacity;

    // Convert pixmap to base64 for serialization
    QByteArray byteArray;
    QBuffer    buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    data["image_data"] = QString(byteArray.toBase64());

    return data;
}

BackgroundPhotoItem *BackgroundPhotoItem::fromDict(
    const QMap<QString, QVariant> &data,
    QGraphicsItem                 *parent) {
    // Convert base64 back to pixmap
    QPixmap    pixmap;
    QByteArray imageData = QByteArray::fromBase64(
        data["image_data"].toString().toLatin1());
    pixmap.loadFromData(imageData);

    // Create new instance
    BackgroundPhotoItem *instance = new BackgroundPhotoItem(
        pixmap, data["region_name"].toString(), parent);

    // Set position
    QMap<QString, QVariant> posMap =
        data["position"].toMap();
    QPointF pos(posMap.value("x", 0).toDouble(),
                posMap.value("y", 0).toDouble());
    instance->setPos(pos);

    // Set properties
    instance->properties = data["properties"].toMap();

    // Set other attributes
    instance->locked = data["locked"].toBool();
    instance->setLocked(
        instance->locked); // Will update flags accordingly

    instance->setSelected(data["selected"].toBool());
    instance->setZValue(data["z_value"].toDouble());
    instance->setVisible(data["visible"].toBool());

    // Set opacity if present
    if (data.contains("opacity")) {
        instance->setOpacity(data["opacity"].toDouble());
    }

    return instance;
}

} // namespace GUI
} // namespace CargoNetSim
