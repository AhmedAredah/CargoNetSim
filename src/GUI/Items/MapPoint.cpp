#include "MapPoint.h"
#include "GUI/Controllers/UtilityFunctions.h"
#include "GUI/Controllers/ViewController.h"
#include "GUI/MainWindow.h"
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

namespace CargoNetSim
{
namespace GUI
{

int MapPoint::POINT_ID = 0;
MapPoint::MapPoint(
    const QString &referencedNetworkID,
    QPointF sceneCoordinates, const QString &region,
    const QString &shape, TerminalItem *terminal,
    const QMap<QString, QVariant> &properties)
    : m_id(POINT_ID++)
    , m_sceneCoordinate(sceneCoordinates)
    , m_shape(shape)
    , m_terminal(terminal)
    , m_color(Qt::black)
    , m_properties(properties)
{
    // Override properties if they were given differently
    // or initialize them if they are missing
    this->m_properties["x"] = m_sceneCoordinate.x();
    this->m_properties["y"] = m_sceneCoordinate.y();
    this->m_properties["Network_ID"] = referencedNetworkID;
    this->m_properties["region"]     = region;

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

void MapPoint::setLinkedTerminal(TerminalItem *newTerminal)
{
    TerminalItem *oldTerminal = m_terminal;
    m_terminal                  = newTerminal;

    if (m_terminal)
    {
        m_properties["LinkedTerminal"] =
            m_terminal->getProperties()["ID"];
    }
    else
    {
        m_properties.remove("LinkedTerminal");
    }

    // Emit signal about the terminal change
    emit terminalChanged(oldTerminal, m_terminal);

    // Trigger a repaint
    update();
}

void MapPoint::setColor(const QColor &newColor)
{
    if (m_color != newColor)
    {
        m_color = newColor;
        emit colorChanged(m_color);
        update();
    }
}

void MapPoint::updateProperties(
    const QMap<QString, QVariant> &newProperties)
{
    for (auto it = newProperties.constBegin();
         it != newProperties.constEnd(); ++it)
    {
        m_properties[it.key()] = it.value();
    }
    emit propertiesChanged();
}

void MapPoint::setSceneCoordinate(const QPointF &newPos)
{
    m_sceneCoordinate = newPos;
    setPos(newPos);
    emit positionChanged(newPos);
}

QRectF MapPoint::boundingRect() const
{
    if (m_terminal)
    {
        const QPixmap &pixmap = m_terminal->getPixmap();
        if (!pixmap.isNull())
        {
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
                     QWidget                        *widget)
{
    if (m_terminal)
    {
        // If linked to a m_terminal, draw m_terminal icon at
        // reduced opacity
        const QPixmap &pixmap = m_terminal->getPixmap();
        if (!pixmap.isNull())
        {
            painter->setOpacity(
                0.7); // Slightly transparent to show it's a
                      // link
            int pixmapWidth  = pixmap.width();
            int pixmapHeight = pixmap.height();
            painter->drawPixmap(-pixmapWidth / 2,
                                -pixmapHeight / 2, pixmap);
            painter->setOpacity(1.0);
        }
    }
    else
    {
        // Draw a shape if no terminal
        painter->setPen(QPen(Qt::black, 2));
        painter->setBrush(QBrush(m_color));

        if (m_shape == "circle")
        {
            painter->drawEllipse(-7.0, -7.0, 14.0, 14.0);
        }
        else if (m_shape == "rectangle")
        {
            painter->drawRect(-7.0, -7.0, 14, 14);
        }
        else if (m_shape == "triangle")
        {
            QPainterPath path;
            path.moveTo(0, -7);
            path.lineTo(7, 7);
            path.lineTo(-7, 7);
            path.lineTo(0, -7);
            painter->drawPath(path);
        }
    }

    setPos(m_sceneCoordinate);

    // Draw selection indicator
    if (option->state & QStyle::State_Selected)
    {
        painter->setPen(QPen(Qt::blue, 2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

void MapPoint::mousePressEvent(
    QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        // Forward to context menu event handler
        QGraphicsSceneContextMenuEvent contextEvent(
            QEvent::GraphicsSceneContextMenu);
        contextEvent.setScenePos(event->scenePos());
        contextEvent.setScreenPos(event->screenPos());
        contextMenuEvent(&contextEvent);
    }
    else
    {
        // Emit clicked signal
        emit clicked(this);
        QGraphicsObject::mousePressEvent(event);
    }
}

void MapPoint::contextMenuEvent(
    QGraphicsSceneContextMenuEvent *event)
{
    showContextMenu(event);
    event->accept();
}

void MapPoint::showContextMenu(
    QGraphicsSceneContextMenuEvent *event)
{
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
    unlinkAction->setEnabled(m_terminal != nullptr);

    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction == originAction)
    {
        createTerminalAtPosition("Origin");
    }
    else if (selectedAction == destinationAction)
    {
        createTerminalAtPosition("Destination");
    }
    else if (selectedAction == seaTerminalAction)
    {
        createTerminalAtPosition("Sea Port Terminal");
    }
    else if (selectedAction == intermodalTerminalAction)
    {
        createTerminalAtPosition(
            "Intermodal Land Terminal");
    }
    else if (selectedAction == trainDepotAction)
    {
        createTerminalAtPosition("Train Stop/Depot");
    }
    else if (selectedAction == parkingAction)
    {
        createTerminalAtPosition("Truck Parking");
    }
    else if (selectedAction == unlinkAction)
    {
        setLinkedTerminal(nullptr);
    }
}

void MapPoint::createTerminalAtPosition(
    const QString &terminalType)
{
    MainWindow *mainWindow =
        qobject_cast<MainWindow *>(scene()->parent());

    if (!mainWindow)
    {
        return;
    }

    // Create terminal using ViewController
    TerminalItem *newTerminal =
        ViewController::createTerminalAtPoint(
            mainWindow,
            m_properties.value("region", "Default Region")
                .toString(),
            terminalType, pos());

    // Link the newly created terminal to this map point
    if (newTerminal)
    {
        UtilitiesFunctions::linkMapPointToTerminal(
            mainWindow, this, newTerminal);
    }
}

QMap<QString, QVariant> MapPoint::toDict() const
{
    QMap<QString, QVariant> data;

    data["referenced_network_ID"] =
        m_properties.value("Network_ID");
    data["x"]          = m_sceneCoordinate.x();
    data["y"]          = m_sceneCoordinate.y();
    data["shape"]      = m_shape;
    data["properties"] = m_properties;
    data["color"]      = m_color.name();
    data["selected"]   = isSelected();
    data["z_value"]    = zValue();

    // Add terminal ID if there's a linked terminal
    if (m_terminal)
    {
        data["terminal_id"] =
            m_terminal->getProperties().value("ID");
    }

    return data;
}

MapPoint *MapPoint::fromDict(
    const QMap<QString, QVariant>   &data,
    const QMap<int, TerminalItem *> &terminalsById)
{
    // Find linked terminal by ID if terminals dictionary is
    // provided
    TerminalItem *terminal = nullptr;
    if (data.contains("terminal_id")
        && !terminalsById.isEmpty())
    {
        int terminalId = data["terminal_id"].toInt();
        terminal       = terminalsById.value(terminalId);
    }

    MapPoint *instance = new MapPoint(
        data.value("referenced_network_ID").toString(),
        QPointF(data.value("x").toDouble(),
                data.value("y").toDouble()),
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
