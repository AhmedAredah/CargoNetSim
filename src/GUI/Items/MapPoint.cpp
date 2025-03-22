#include "MapPoint.h"
#include "TerminalItem.h"

#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QMenu>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

namespace CargoNetSim {
namespace GUI {

int MapPoint::POINT_ID = 0;

MapPoint::MapPoint(
    const QString &referencedNetworkID, qreal x, qreal y,
    const QString &shape, const QString &region,
    TerminalItem                  *terminal,
    const QMap<QString, QVariant> &properties)
    : id(POINT_ID++)
    , x(x)
    , y(y)
    , shape(shape)
    , region(region)
    , terminal(terminal)
    , color(Qt::black)
    , properties(properties) {
    // Initialize properties if none provided
    if (this->properties.isEmpty()) {
        this->properties["x"] = x;
        this->properties["y"] = y;
        this->properties["Network_ID"] =
            referencedNetworkID;
        this->properties["region"] = region;
    }

    // Set higher z-value to ensure points are drawn above
    // lines
    setZValue(10);

    // Update terminal link state
    setLinkedTerminal(terminal);

    // Enable selection and hover events
    setFlags(QGraphicsItem::ItemIsSelectable
             | QGraphicsItem::ItemIgnoresTransformations);
    setAcceptHoverEvents(true);
}

void MapPoint::setLinkedTerminal(
    TerminalItem *newTerminal) {
    TerminalItem *oldTerminal = terminal;
    terminal                  = newTerminal;

    if (terminal) {
        properties["LinkedTerminal"] =
            terminal->getProperties()["ID"];
    } else {
        properties.remove("LinkedTerminal");
    }

    // Emit signal about the terminal change
    emit terminalChanged(oldTerminal, terminal);

    // Trigger a repaint
    update();
}

void MapPoint::setColor(const QColor &newColor) {
    if (color != newColor) {
        color = newColor;
        emit colorChanged(color);
        update();
    }
}

void MapPoint::updateProperties(
    const QMap<QString, QVariant> &newProperties) {
    for (auto it = newProperties.constBegin();
         it != newProperties.constEnd(); ++it) {
        properties[it.key()] = it.value();
    }
    emit propertiesChanged();
}

QRectF MapPoint::boundingRect() const {
    if (terminal) {
        const QPixmap &pixmap = terminal->getPixmap();
        if (!pixmap.isNull()) {
            int pixmapWidth  = pixmap.width();
            int pixmapHeight = pixmap.height();
            return QRectF(-pixmapWidth / 2,
                          -pixmapHeight / 2, pixmapWidth,
                          pixmapHeight);
        }
    }

    // Default size for shapes
    return QRectF(-7, -7, 14, 14);
}

void MapPoint::paint(QPainter *painter,
                     const QStyleOptionGraphicsItem *option,
                     QWidget *widget) {
    if (terminal) {
        // If linked to a terminal, draw terminal icon at
        // reduced opacity
        const QPixmap &pixmap = terminal->getPixmap();
        if (!pixmap.isNull()) {
            painter->setOpacity(
                0.7); // Slightly transparent to show it's a
                      // link
            int pixmapWidth  = pixmap.width();
            int pixmapHeight = pixmap.height();
            painter->drawPixmap(-pixmapWidth / 2,
                                -pixmapHeight / 2, pixmap);
            painter->setOpacity(1.0);
        }
    } else {
        // Draw a shape if no terminal
        painter->setPen(QPen(Qt::black, 2));
        painter->setBrush(QBrush(color));

        if (shape == "circle") {
            painter->drawEllipse(-7, -7, 14, 14);
        } else if (shape == "rectangle") {
            painter->drawRect(-7, -7, 14, 14);
        } else if (shape == "triangle") {
            QPainterPath path;
            path.moveTo(0, -7);
            path.lineTo(7, 7);
            path.lineTo(-7, 7);
            path.lineTo(0, -7);
            painter->drawPath(path);
        }
    }

    // Draw selection indicator
    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(Qt::blue, 2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

void MapPoint::mousePressEvent(
    QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        // Forward to context menu event handler
        QGraphicsSceneContextMenuEvent contextEvent(
            QEvent::GraphicsSceneContextMenu);
        contextEvent.setScenePos(event->scenePos());
        contextEvent.setScreenPos(event->screenPos());
        contextMenuEvent(&contextEvent);
    } else {
        // Emit clicked signal
        emit clicked(this);
        QGraphicsObject::mousePressEvent(event);
    }
}

void MapPoint::contextMenuEvent(
    QGraphicsSceneContextMenuEvent *event) {
    showContextMenu(event);
    event->accept();
}

void MapPoint::showContextMenu(
    QGraphicsSceneContextMenuEvent *event) {
    QMenu menu;

    // Create sub-actions for terminal creation
    QMenu *createTerminalMenu =
        menu.addMenu("Create Terminal at Node");

    QAction *originAction =
        menu.addAction("Mark as Origin");
    QAction *destinationAction =
        menu.addAction("Mark as Destination");

    QAction *seaTerminalAction =
        createTerminalMenu->addAction("Sea Terminal");
    QAction *intermodalTerminalAction =
        createTerminalMenu->addAction(
            "Intermodal Terminal");
    QAction *trainDepotAction =
        createTerminalMenu->addAction("Train Depot");
    QAction *parkingAction =
        createTerminalMenu->addAction("Parking");

    QAction *unlinkAction =
        menu.addAction("Unlink Terminal");
    unlinkAction->setEnabled(terminal != nullptr);

    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction == originAction) {
        createTerminalAtPosition("Origin");
    } else if (selectedAction == destinationAction) {
        createTerminalAtPosition("Destination");
    } else if (selectedAction == seaTerminalAction) {
        createTerminalAtPosition("Sea Port Terminal");
    } else if (selectedAction == intermodalTerminalAction) {
        createTerminalAtPosition(
            "Intermodal Land Terminal");
    } else if (selectedAction == trainDepotAction) {
        createTerminalAtPosition("Train Stop/Depot");
    } else if (selectedAction == parkingAction) {
        createTerminalAtPosition("Truck Parking");
    } else if (selectedAction == unlinkAction) {
        setLinkedTerminal(nullptr);
    }
}

void MapPoint::createTerminalAtPosition(
    const QString &terminalType) {
    // This will be implemented to call the ViewController's
    // terminal creation method Since we need to avoid
    // circular dependencies, this should be handled by
    // connecting to the clicked signal and using that to
    // create terminals

    // For now, just emit clicked - the actual creation will
    // be handled by MainWindow
    emit clicked(this);

    // Note: In a full implementation, this would call a
    // controller method to create the terminal and then
    // link it, but that requires access to MainWindow
}

QMap<QString, QVariant> MapPoint::toDict() const {
    QMap<QString, QVariant> data;

    data["referenced_network_ID"] =
        properties.value("Network_ID");
    data["x"]          = x;
    data["y"]          = y;
    data["shape"]      = shape;
    data["region"]     = region;
    data["properties"] = properties;
    data["color"]      = color.name();
    data["selected"]   = isSelected();
    data["z_value"]    = zValue();

    // Add terminal ID if there's a linked terminal
    if (terminal) {
        data["terminal_id"] =
            terminal->getProperties().value("ID");
    }

    return data;
}

MapPoint *MapPoint::fromDict(
    const QMap<QString, QVariant>   &data,
    const QMap<int, TerminalItem *> &terminalsById) {
    // Find linked terminal by ID if terminals dictionary is
    // provided
    TerminalItem *terminal = nullptr;
    if (data.contains("terminal_id")
        && !terminalsById.isEmpty()) {
        int terminalId = data["terminal_id"].toInt();
        terminal       = terminalsById.value(terminalId);
    }

    MapPoint *instance = new MapPoint(
        data.value("referenced_network_ID").toString(),
        data.value("x").toDouble(),
        data.value("y").toDouble(),
        data.value("shape", "circle").toString(),
        data.value("region", "default").toString(),
        terminal, data.value("properties").toMap());

    // Set color
    instance->setColor(
        QColor(data.value("color", "#000000").toString()));

    // Set other properties
    instance->setSelected(
        data.value("selected", false).toBool());
    instance->setZValue(
        data.value("z_value", 10).toDouble());

    return instance;
}

} // namespace GUI
} // namespace CargoNetSim
