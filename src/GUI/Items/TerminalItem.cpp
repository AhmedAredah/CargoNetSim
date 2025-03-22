#include "TerminalItem.h"

#include <QApplication>
#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>

namespace CargoNetSim {
namespace GUI {

// Initialize static variables
int                TerminalItem::TERMINAL_ID = 0;
QMap<QString, int> TerminalItem::TERMINAL_TYPES_IDs;

TerminalItem::TerminalItem(
    const QPixmap                 &pixmap,
    const QMap<QString, QVariant> &properties,
    const QString &region, QGraphicsItem *parent,
    const QString &terminalType)
    : QGraphicsObject(parent)
    , pixmap(pixmap)
    , region(region)
    , terminalType(terminalType)
    , properties(properties)
    , animObject(nullptr)
    , animation(nullptr)
    , wasSelected(false) {
    // Set a higher Z-value for terminals (will be drawn on
    // top)
    setZValue(11);

    // Initialize properties if not provided
    if (properties.isEmpty()) {
        initializeDefaultProperties();
    }

    // Enable ItemSendsGeometryChanges flag to receive
    // position change notifications
    setFlags(QGraphicsItem::ItemIsSelectable
             | QGraphicsItem::ItemIsMovable
             | QGraphicsItem::ItemSendsGeometryChanges
             | QGraphicsItem::ItemIgnoresTransformations);

    setAcceptHoverEvents(true);

    // Initialize bounding rectangle
    int pixmapWidth  = this->pixmap.width();
    int pixmapHeight = this->pixmap.height();
    boundingRectValue =
        QRectF(-pixmapWidth / 2, -pixmapHeight / 2,
               pixmapWidth, pixmapHeight);
}

TerminalItem::~TerminalItem() {
    // Clean up any active animations
    if (animation) {
        animation->stop();
        animation->deleteLater();
    }

    if (animObject) {
        animObject->deleteLater();
    }
}

void TerminalItem::initializeDefaultProperties() {
    int generalId = getNewTerminalID();
    int typeId    = getNewTerminalID(terminalType);

    if (terminalType == "Origin"
        || terminalType == "Destination") {
        QMap<QString, QVariant> interfaces;
        QStringList             landSide, seaSide;
        landSide << "Train" << "Truck";
        seaSide << "Ship";
        interfaces["land_side"] = landSide;
        interfaces["sea_side"]  = seaSide;

        this->properties["ID"] = generalId;
        this->properties["Name"] =
            QString("%1%2").arg(terminalType).arg(typeId);
        this->properties["Show on Global Map"] = "True";
        this->properties["Available Interfaces"] =
            interfaces;
    } else {
        this->properties["ID"] = generalId;
        this->properties["Name"] =
            QString("%1%2").arg(terminalType).arg(typeId);
        this->properties["Region"] = this->region;
        this->properties["Show on Global Map"] = "True";

        QMap<QString, QVariant> cost;
        cost["fixed_fees"]       = "400";
        cost["customs_fees"]     = "100";
        cost["risk_factor"]      = "0.015";
        this->properties["cost"] = cost;

        QMap<QString, QVariant> dwellTime;
        QMap<QString, QVariant> parameters;
        parameters["mean"]             = "2880";
        parameters["std_dev"]          = "720";
        dwellTime["method"]            = "normal";
        dwellTime["parameters"]        = parameters;
        this->properties["dwell_time"] = dwellTime;

        if (terminalType == "Sea Port Terminal"
            || terminalType == "Intermodal Land Terminal") {
            QMap<QString, QVariant> customs;
            customs["probability"]      = "0.08";
            customs["delay_mean"]       = "48";
            customs["delay_variance"]   = "24";
            this->properties["customs"] = customs;

            QMap<QString, QVariant> capacity;
            capacity["max_capacity"]       = 100000;
            capacity["critical_threshold"] = 0.8;
            this->properties["capacity"]   = capacity;
        }

        // Set interfaces based on terminal type
        QMap<QString, QVariant> interfaces;
        QStringList             landSide, seaSide;

        if (terminalType == "Sea Port Terminal") {
            landSide << "Truck" << "Train";
            seaSide << "Ship";
        } else if (terminalType
                   == "Intermodal Land Terminal") {
            landSide << "Truck" << "Train";
            this->properties["Show on Global Map"] =
                "False";
        } else if (terminalType == "Train Stop/Depot") {
            landSide << "Train";
            this->properties["Show on Global Map"] =
                "False";
        } else if (terminalType == "Truck Parking") {
            landSide << "Truck";
            this->properties["Show on Global Map"] =
                "False";
        } else {
            // Default case
            landSide << "Truck";
        }

        interfaces["land_side"] = landSide;
        interfaces["sea_side"]  = seaSide;
        this->properties["Available Interfaces"] =
            interfaces;
    }

    if (terminalType == "Origin") {
        this->properties["Containers"] =
            QMap<QString, QVariant>();
    }
}

void TerminalItem::setRegion(const QString &newRegion) {
    if (region != newRegion) {
        QString oldRegion    = region;
        region               = newRegion;
        properties["Region"] = newRegion;
        emit regionChanged(newRegion);
    }
}

void TerminalItem::updateProperties(
    const QMap<QString, QVariant> &newProperties) {
    for (auto it = newProperties.constBegin();
         it != newProperties.constEnd(); ++it) {
        properties[it.key()] = it.value();
    }
    emit propertiesChanged();
}

void TerminalItem::setProperty(const QString  &key,
                               const QVariant &value) {
    // Check if property exists and is different
    if (!properties.contains(key)
        || properties[key] != value) {
        properties[key] = value;
        emit propertyChanged(key, value);

        // If this is a visual property, update the item
        if (key == "Name" || key == "Show on Global Map") {
            update();
        }
    }
}

QVariant TerminalItem::getProperty(
    const QString  &key,
    const QVariant &defaultValue) const {
    return properties.value(key, defaultValue);
}

void TerminalItem::resetClassIDs() {
    TERMINAL_ID = 0;
    TERMINAL_TYPES_IDs.clear();
}

void TerminalItem::setClassIDs(
    const QMap<int, TerminalItem *> &allTerminalsById) {
    if (allTerminalsById.isEmpty()) {
        TERMINAL_ID = 1;
        TERMINAL_TYPES_IDs.clear();
        return;
    }

    // Find max ID used
    int maxId = 0;
    for (auto terminal : allTerminalsById) {
        int currentId = terminal->properties["ID"].toInt();
        if (currentId > maxId) {
            maxId = currentId;
        }
    }

    TERMINAL_ID = maxId + 1;

    // Reset TERMINAL_TYPES_IDs by iterating over all
    // terminals
    TERMINAL_TYPES_IDs.clear();
    for (auto terminal : allTerminalsById) {
        QString terminalType = terminal->terminalType;
        int terminalId = terminal->properties["ID"].toInt();

        if (!TERMINAL_TYPES_IDs.contains(terminalType)) {
            TERMINAL_TYPES_IDs[terminalType] = terminalId;
        } else {
            TERMINAL_TYPES_IDs[terminalType] =
                qMax(TERMINAL_TYPES_IDs[terminalType],
                     terminalId);
        }
    }

    // Ensure the next ID for each terminal type starts from
    // max_ID + 1
    for (auto it = TERMINAL_TYPES_IDs.begin();
         it != TERMINAL_TYPES_IDs.end(); ++it) {
        *it = *it + 1;
    }
}

int TerminalItem::getNewTerminalID(
    const QString &terminalType) {
    if (terminalType.isEmpty()) {
        TERMINAL_ID++;
        return TERMINAL_ID;
    } else {
        int value =
            TERMINAL_TYPES_IDs.value(terminalType, 0);
        value++;
        TERMINAL_TYPES_IDs[terminalType] = value;
        return value;
    }
}

QRectF TerminalItem::boundingRect() const {
    return boundingRectValue;
}

void TerminalItem::paint(
    QPainter                       *painter,
    const QStyleOptionGraphicsItem *option,
    QWidget                        *widget) {
    if (!pixmap.isNull()) {
        int pixmapWidth  = pixmap.width();
        int pixmapHeight = pixmap.height();
        painter->drawPixmap(-pixmapWidth / 2,
                            -pixmapHeight / 2, pixmap);
    }

    if (option->state & QStyle::State_Selected) {
        QPen pen(Qt::red, 2, Qt::DashLine);
        painter->setPen(pen);
        painter->drawRect(boundingRect());
    }
}

void TerminalItem::mousePressEvent(
    QGraphicsSceneMouseEvent *event) {
    // Store the initial click position relative to the
    // item's origin
    dragOffset = event->pos();

    // Emit clicked signal
    emit clicked(this);

    // Call base class to handle selection
    QGraphicsObject::mousePressEvent(event);
}

QVariant TerminalItem::itemChange(GraphicsItemChange change,
                                  const QVariant &value) {
    if (change == ItemPositionChange && scene()) {
        // If this is a position change and we have a drag
        // offset, adjust the position
        if (dragOffset != QPointF()) {
            // Get the proposed new position
            QPointF newPos = value.toPointF();

            // Get current mouse position in scene
            // coordinates
            if (scene()->mouseGrabberItem() == this) {
                QGraphicsView *view =
                    scene()->views().first();
                QPointF mousePos = view->mapToScene(
                    view->mapFromGlobal(QCursor::pos()));

                // Adjust position to keep item under the
                // mouse at the right offset
                return mousePos - dragOffset;
            }
        }
    } else if (change == ItemPositionHasChanged
               && scene()) {
        // Emit position changed signal when position has
        // been changed
        emit positionChanged(pos());
    } else if (change == ItemSelectedChange) {
        bool selected = value.toBool();
        if (selected != wasSelected) {
            wasSelected = selected;
            emit selectionChanged(selected);
        }
    }

    return QGraphicsObject::itemChange(change, value);
}

void TerminalItem::hoverEnterEvent(
    QGraphicsSceneHoverEvent *event) {
    setCursor(QCursor(Qt::PointingHandCursor));
    QGraphicsObject::hoverEnterEvent(event);
}

void TerminalItem::hoverLeaveEvent(
    QGraphicsSceneHoverEvent *event) {
    unsetCursor();
    QGraphicsObject::hoverLeaveEvent(event);
}

void TerminalItem::flash(bool          evenIfHidden,
                         const QColor &color) {
    bool wasHidden = !isVisible();
    if (evenIfHidden && wasHidden) {
        setVisible(true);
    }

    // Clean up any existing animation
    if (animation) {
        animation->stop();
        animation->deleteLater();
        animation = nullptr;
    }

    if (animObject) {
        animObject->deleteLater();
        animObject = nullptr;
    }

    // Create a rectangle item as an overlay
    QGraphicsRectItem *rect =
        new QGraphicsRectItem(boundingRect(), this);
    rect->setBrush(QBrush(color));
    rect->setPen(QPen(Qt::NoPen));
    rect->setZValue(100);

    // Create an animation object to control opacity
    class AnimationObject : public QObject {
    public:
        AnimationObject(QObject *parent = nullptr)
            : QObject(parent)
            , _opacity(1.0) {}

        qreal opacity() const {
            return _opacity;
        }
        void setOpacity(qreal opacity) {
            _opacity = opacity;
            if (_rect)
                _rect->setOpacity(opacity);
        }

        void setRect(QGraphicsRectItem *rect) {
            _rect = rect;
        }

    private:
        qreal              _opacity;
        QGraphicsRectItem *_rect = nullptr;
    };

    // Store animation object as an instance variable
    animObject = new AnimationObject(this);
    static_cast<AnimationObject *>(animObject)
        ->setRect(rect);

    // Create and configure animation
    animation =
        new QPropertyAnimation(animObject, "opacity", this);
    animation->setDuration(1000);
    animation->setLoopCount(3);
    animation->setStartValue(1.0);
    animation->setKeyValueAt(0.5, 0.0);
    animation->setEndValue(1.0);

    // Connect finished signal for cleanup
    connect(animation, &QPropertyAnimation::finished,
            [=]() {
                if (rect && scene()) {
                    scene()->removeItem(rect);
                    delete rect;
                }

                if (evenIfHidden && wasHidden) {
                    setVisible(false);
                }

                // Clean up animation resources
                animation->deleteLater();
                animation = nullptr;
                animObject->deleteLater();
                animObject = nullptr;
            });

    // Start animation
    animation->start();
}

QMap<QString, QVariant> TerminalItem::toDict() const {
    QMap<QString, QVariant> data;

    // Store position
    QMap<QString, QVariant> posMap;
    posMap["x"]      = pos().x();
    posMap["y"]      = pos().y();
    data["position"] = posMap;

    // Store other properties
    data["terminal_type"] = terminalType;
    data["region"]        = region;
    data["properties"]    = properties;
    data["selected"]      = isSelected();
    data["visible"]       = isVisible();
    data["z_value"]       = zValue();

    return data;
}

TerminalItem *
TerminalItem::fromDict(const QMap<QString, QVariant> &data,
                       const QPixmap &pixmap,
                       QGraphicsItem *parent) {
    // Create new instance with essential data
    TerminalItem *instance =
        new TerminalItem(pixmap, data["properties"].toMap(),
                         data["region"].toString(), parent,
                         data["terminal_type"].toString());

    // Set position
    QMap<QString, QVariant> posMap =
        data["position"].toMap();
    QPointF pos(posMap.value("x", 0).toDouble(),
                posMap.value("y", 0).toDouble());
    instance->setPos(pos);

    // Set other properties
    instance->setSelected(
        data.value("selected", false).toBool());
    instance->setVisible(
        data.value("visible", true).toBool());
    instance->setZValue(
        data.value("z_value", 11).toDouble());

    return instance;
}

} // namespace GUI
} // namespace CargoNetSim
