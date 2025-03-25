#include "GraphicsScene.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QMessageBox>

#include "../Controllers/UtilityFunctions.h"
#include "../Controllers/ViewController.h"
#include "../Items/ConnectionLine.h"
#include "../Items/DistanceMeasurementTool.h"
#include "../Items/GlobalTerminalItem.h"
#include "../Items/TerminalItem.h"
#include "../MainWindow.h"
#include "GUI/Widgets/GraphicsView.h"

namespace CargoNetSim
{
namespace GUI
{

GraphicsScene::GraphicsScene(QObject *parent)
    : QGraphicsScene(parent)
    , connectMode(false)
    , linkTerminalMode(false)
    , unlinkTerminalMode(false)
    , measureMode(false)
    , setGlobalPositionMode(false)
    , connectFirstItem(QVariant())
    , measurementTool(nullptr)
{
}

void GraphicsScene::addItemWithId(GraphicsObjectBase *item,
                                  const QString      &id)
{
    // First add the item to the scene using the base class
    // method
    QGraphicsScene::addItem(item);

    // Take ownership
    item->setParent(this);

    // Get the class key and store in type map
    QString className = QString(typeid(*item).name());

    // Initialize the inner map if needed
    if (!itemsByType.contains(className))
    {
        itemsByType[className] =
            QMap<QString, QGraphicsItem *>();
    }

    // Add to type-specific map
    itemsByType[className][id] = item;
}

void GraphicsScene::mousePressEvent(
    QGraphicsSceneMouseEvent *event)
{
    try
    {
        // Handle global position setting mode
        if (setGlobalPositionMode)
        {
            QList<QGraphicsItem *> clickedItems =
                items(event->scenePos());
            bool terminalFound = false;

            for (QGraphicsItem *item : clickedItems)
            {
                GlobalTerminalItem *globalTerminal =
                    dynamic_cast<GlobalTerminalItem *>(
                        item);
                if (globalTerminal
                    && globalTerminal
                           ->getLinkedTerminalItem())
                {
                    if (views().isEmpty())
                    {
                        return;
                    }

                    // Get main window through the view's
                    // parent
                    QObject *mainWindowObj =
                        views().first()->parent();

                    if (mainWindowObj)
                    {
                        // Call the set global position
                        // method through the controller
                        // Controllers::BasicButtonController::setTerminalGlobalPosition(
                        //     mainWindowObj,
                        //     globalTerminal->getLinkedTerminalItem()
                        // );

                        terminalFound = true;
                        break;
                    }
                }
            }

            // Exit the mode if we successfully processed a
            // terminal
            if (terminalFound)
            {
                setGlobalPositionMode = false;

                // Get main window through the view's parent
                // to uncheck the button
                if (!views().isEmpty())
                {
                    QObject *mainWindowObj =
                        views().first()->parent();
                    if (mainWindowObj)
                    {
                        QObject *button =
                            mainWindowObj
                                ->findChild<QObject *>(
                                    "set_global_position_"
                                    "button");
                        if (button)
                        {
                            button->setProperty("checked",
                                                false);
                        }
                    }
                }
                return;
            }
        }
        // Handle measurement mode
        else if (measureMode)
        {
            QPointF scenePos = event->scenePos();

            if (!measurementTool)
            {
                // First click - create measurement tool and
                // set start point
                if (!views().isEmpty())
                {
                    measurementTool =
                        new DistanceMeasurementTool(
                            dynamic_cast<GraphicsView *>(
                                views().first()));
                    addItemWithId(measurementTool,
                                  measurementTool->getID());
                    measurementTool->setStartPoint(
                        scenePos);

                    // Show status message if we can find
                    // the main window
                    if (!views().isEmpty())
                    {
                        QObject *mainWindowObj =
                            views().first()->parent();
                        if (mainWindowObj)
                        {
                            QObject *statusBar =
                                mainWindowObj
                                    ->findChild<QObject *>(
                                        "statusBar");
                            if (statusBar)
                            {
                                QMetaObject::invokeMethod(
                                    statusBar,
                                    "showMessage",
                                    Q_ARG(QString,
                                          "Click again to "
                                          "complete "
                                          "measurement"),
                                    Q_ARG(int, 2000));
                            }
                        }
                    }
                }
            }
            else
            {
                // Second click - complete measurement and
                // reset for next measurement
                measurementTool->setEndPoint(scenePos);
                measurementTool = nullptr;
                measureMode     = false;

                // Get main window through the view's parent
                // to uncheck the button and show message
                if (!views().isEmpty())
                {
                    QObject *mainWindowObj =
                        views().first()->parent();
                    if (mainWindowObj)
                    {
                        QObject *button =
                            mainWindowObj
                                ->findChild<QObject *>(
                                    "measure_action");
                        if (button)
                        {
                            button->setProperty("checked",
                                                false);
                        }

                        QObject *view = views().first();
                        QMetaObject::invokeMethod(
                            view, "unsetCursor");

                        QObject *statusBar =
                            mainWindowObj
                                ->findChild<QObject *>(
                                    "statusBar");
                        if (statusBar)
                        {
                            QMetaObject::invokeMethod(
                                statusBar, "showMessage",
                                Q_ARG(
                                    QString,
                                    "Measurement complete"),
                                Q_ARG(int, 2000));
                        }
                    }
                }
                return;
            }
        }
        // Handle connection mode
        else if (connectMode)
        {
            QList<QGraphicsItem *> clickedItems =
                items(event->scenePos());
            QVariant terminalVariant;

            // Find a terminal item among clicked items
            for (QGraphicsItem *item : clickedItems)
            {
                TerminalItem *terminal =
                    dynamic_cast<TerminalItem *>(item);
                if (terminal)
                {
                    terminalVariant =
                        QVariant::fromValue(terminal);
                    break;
                }

                GlobalTerminalItem *globalTerminal =
                    dynamic_cast<GlobalTerminalItem *>(
                        item);
                if (globalTerminal)
                {
                    terminalVariant =
                        QVariant::fromValue(globalTerminal);
                    break;
                }
            }

            if (!terminalVariant.isNull())
            {
                if (connectFirstItem.isNull())
                {
                    // First terminal selected
                    connectFirstItem = terminalVariant;

                    // Show status message
                    if (!views().isEmpty())
                    {
                        QObject *mainWindowObj =
                            views().first()->parent();
                        if (mainWindowObj)
                        {
                            // Get the current connection
                            // type
                            QString currentConnectionType =
                                mainWindowObj
                                    ->property("currentConn"
                                               "ectionType")
                                    .toString();

                            QObject *statusBar =
                                mainWindowObj
                                    ->findChild<QObject *>(
                                        "statusBar");
                            if (statusBar)
                            {
                                QMetaObject::invokeMethod(
                                    statusBar,
                                    "showMessage",
                                    Q_ARG(
                                        QString,
                                        QString(
                                            "Selected "
                                            "first "
                                            "terminal. "
                                            "Click another "
                                            "terminal to "
                                            "create a %1 "
                                            "connection.")
                                            .arg(
                                                currentConnectionType)),
                                    Q_ARG(int, 3000));
                            }
                        }
                    }
                }
                else
                {
                    // Compare against first item
                    bool isSameItem = false;

                    if (connectFirstItem
                            .canConvert<TerminalItem *>()
                        && terminalVariant.canConvert<
                            TerminalItem *>())
                    {
                        isSameItem =
                            connectFirstItem
                                .value<TerminalItem *>()
                            == terminalVariant
                                   .value<TerminalItem *>();
                    }
                    else if (connectFirstItem.canConvert<
                                 GlobalTerminalItem *>()
                             && terminalVariant.canConvert<
                                 GlobalTerminalItem *>())
                    {
                        isSameItem =
                            connectFirstItem.value<
                                GlobalTerminalItem *>()
                            == terminalVariant.value<
                                GlobalTerminalItem *>();
                    }

                    if (isSameItem)
                    {
                        connectFirstItem =
                            QVariant(); // Reset to null

                        // Show error message
                        if (!views().isEmpty())
                        {
                            QObject *mainWindowObj =
                                views().first()->parent();
                            if (mainWindowObj)
                            {
                                QObject *statusBar =
                                    mainWindowObj
                                        ->findChild<
                                            QObject *>(
                                            "statusBar");
                                if (statusBar)
                                {
                                    QMetaObject::
                                        invokeMethod(
                                            statusBar,
                                            "showMessage",
                                            Q_ARG(
                                                QString,
                                                "Cannot "
                                                "connect "
                                                "terminal "
                                                "to "
                                                "itself."),
                                            Q_ARG(int,
                                                  2000));
                                }
                            }
                        }
                        return;
                    }

                    // Create connection through utility
                    // function
                    if (!views().isEmpty())
                    {
                        QObject *mainWindowObj =
                            views().first()->parent();

                        if (mainWindowObj)
                        {
                            // Get the current connection
                            // type and region
                            QString currentConnectionType =
                                mainWindowObj
                                    ->property("currentConn"
                                               "ectionType")
                                    .toString();
                            QString currentRegion =
                                mainWindowObj
                                    ->property(
                                        "currentRegion")
                                    .toString();

                            // Get first terminal
                            QGraphicsItem *firstItem =
                                nullptr;
                            if (connectFirstItem.canConvert<
                                    TerminalItem *>())
                            {
                                firstItem =
                                    connectFirstItem.value<
                                        TerminalItem *>();
                            }
                            else if (
                                connectFirstItem.canConvert<
                                    GlobalTerminalItem *>())
                            {
                                firstItem =
                                    connectFirstItem.value<
                                        GlobalTerminalItem
                                            *>();
                            }

                            // Get second terminal
                            QGraphicsItem *secondItem =
                                nullptr;
                            if (terminalVariant.canConvert<
                                    TerminalItem *>())
                            {
                                secondItem =
                                    terminalVariant.value<
                                        TerminalItem *>();
                            }
                            else if (
                                terminalVariant.canConvert<
                                    GlobalTerminalItem *>())
                            {
                                secondItem =
                                    terminalVariant.value<
                                        GlobalTerminalItem
                                            *>();
                            }

                            // Call createConnectionLine
                            // utility function
                            ConnectionLine
                                *connection; //= // TODO
                            //     Controllers::UtilityFunctions::createConnectionLine(
                            //     mainWindowObj,
                            //     connectFirstItem,
                            //     terminalItem,
                            //     this,
                            //     currentConnectionType,
                            //     currentRegion
                            // );

                            if (connection)
                            {
                                QObject *statusBar =
                                    mainWindowObj
                                        ->findChild<
                                            QObject *>(
                                            "statusBar");
                                if (statusBar)
                                {
                                    QMetaObject::
                                        invokeMethod(
                                            statusBar,
                                            "showMessage",
                                            Q_ARG(
                                                QString,
                                                "Connection"
                                                " created. "
                                                "Click "
                                                "another "
                                                "terminal "
                                                "to "
                                                "continue "
                                                "connecting"
                                                "."),
                                            Q_ARG(int,
                                                  2000));
                                }

                                // Update scene visibility
                                // Controllers::ViewController::updateSceneVisibility(mainWindowObj);

                                // Set the second terminal
                                // as the first for the next
                                // connection
                                connectFirstItem =
                                    terminalVariant;
                            }
                            else
                            {
                                // If connection failed,
                                // reset first item
                                connectFirstItem =
                                    QVariant();
                            }

                            return;
                        }
                    }
                }
            }
        }
        else
        {
            // Check if clicked on empty area
            QList<QGraphicsItem *> clickedItems =
                items(event->scenePos());
            if (clickedItems.isEmpty()
                && !views().isEmpty())
            {
                CargoNetSim::GUI::MainWindow
                    *mainWindowObj = dynamic_cast<
                        CargoNetSim::GUI::MainWindow *>(
                        parent());
                if (mainWindowObj)
                {
                    // Clear selection and hide properties
                    // panel
                    clearSelection();
                    UtilitiesFunctions::hidePropertiesPanel(
                        mainWindowObj);
                }
                else
                {
                    qDebug() << "Could not extract "
                                "MainWindow object from "
                                "view parent";
                }
            }

            // Pass the event to the base class for normal
            // handling
            QGraphicsScene::mousePressEvent(event);
        }
    }
    catch (const std::exception &e)
    {
        qWarning() << "Exception in "
                      "GraphicsScene::mousePressEvent:"
                   << e.what();
    }
    catch (...)
    {
        qWarning() << "Unknown exception in "
                      "GraphicsScene::mousePressEvent";
    }
}

} // namespace GUI
} // namespace CargoNetSim
