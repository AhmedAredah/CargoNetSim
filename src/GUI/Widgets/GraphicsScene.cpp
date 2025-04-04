#include "GraphicsScene.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QMessageBox>
#include <QtGui/qevent.h>

#include "../Controllers/UtilityFunctions.h"
#include "../Controllers/ViewController.h"
#include "../Items/ConnectionLine.h"
#include "../Items/DistanceMeasurementTool.h"
#include "../Items/GlobalTerminalItem.h"
#include "../Items/TerminalItem.h"
#include "../MainWindow.h"
#include "Backend/Controllers/CargoNetSimController.h"
#include "GUI/Controllers/BasicButtonController.h"
#include "GUI/Widgets/GraphicsView.h"

namespace CargoNetSim
{
namespace GUI
{

GraphicsScene::GraphicsScene(QObject *parent)
    : QGraphicsScene(parent)
    , m_connectMode(false)
    , m_linkTerminalMode(false)
    , m_unlinkTerminalMode(false)
    , m_measureMode(false)
    , m_globalPositionMode(false)
    , m_connectFirstItem(QVariant())
    , m_measurementTool(nullptr)
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
        if (!parent())
        {
            return;
        }

        MainWindow *mainWindowObj =
            dynamic_cast<MainWindow *>(parent());

        if (!mainWindowObj)
        {
            return;
            qDebug() << "Could not extract "
                        "MainWindow object from "
                        "view parent";
        }

        // Handle global position setting mode
        if (m_globalPositionMode)
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

                    // Call the set global position
                    // method through the controller
                    BasicButtonController::
                        setTerminalGlobalPosition(
                            mainWindowObj,
                            globalTerminal
                                ->getLinkedTerminalItem());

                    terminalFound = true;
                    break;
                }
            }

            // Exit the mode if we successfully processed a
            // terminal
            if (terminalFound)
            {
                m_globalPositionMode = false;

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
        else if (m_measureMode)
        {
            QPointF scenePos = event->scenePos();

            if (!m_measurementTool)
            {
                // First click - create measurement tool and
                // set start point
                if (!views().isEmpty())
                {
                    m_measurementTool =
                        new DistanceMeasurementTool(
                            dynamic_cast<GraphicsView *>(
                                views().first()));
                    addItemWithId(m_measurementTool,
                                  m_measurementTool->getID());
                    m_measurementTool->setStartPoint(
                        scenePos);

                    // Show status message if we can find
                    // the main window
                    if (!views().isEmpty())
                    {
                        QObject *mainWindowObj =
                            views().first()->parent();
                        if (mainWindowObj)
                        {
                            MainWindow *mainWin =
                                dynamic_cast<MainWindow *>(
                                    mainWindowObj);
                            if (mainWin)
                            {
                                mainWin
                                    ->showStatusBarMessage(
                                        "Click again to "
                                        "complete "
                                        "measurement",
                                        2000);
                            }
                        }
                    }
                }
            }
            else
            {
                // Second click - complete measurement and
                // reset for next measurement
                m_measurementTool->setEndPoint(scenePos);
                m_measurementTool = nullptr;
                m_measureMode     = false;

                // Get main window through the view's parent
                // to uncheck the button and show message
                if (!views().isEmpty())
                {

                    QObject *button =
                        mainWindowObj->findChild<QObject *>(
                            "measure_action");
                    if (button)
                    {
                        button->setProperty("checked",
                                            false);
                    }

                    QGraphicsView *view = views().first();
                    view->unsetCursor();

                    QObject *statusBar =
                        mainWindowObj->findChild<QObject *>(
                            "statusBar");
                    if (statusBar)
                    {
                        QMetaObject::invokeMethod(
                            statusBar, "showMessage",
                            Q_ARG(QString,
                                  "Measurement complete"),
                            Q_ARG(int, 2000));
                    }
                }
                return;
            }
        }
        // Handle connection mode
        else if (m_connectMode)
        {
            // Get the current connection
            // type and region
            QString currentConnectionType =
                mainWindowObj->getConnectionType();

            QString currentRegion =
                CargoNetSim::CargoNetSimController::
                    getInstance()
                        .getRegionDataController()
                        ->getCurrentRegion();

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
                if (m_connectFirstItem.isNull())
                {
                    // First terminal selected
                    m_connectFirstItem = terminalVariant;

                    // Show status message

                    mainWindowObj->showStatusBarMessage(
                        QString("Selected first terminal. "
                                "Click another terminal to "
                                "create a %1 connection.")
                            .arg(currentConnectionType),
                        3000);
                }
                else
                {
                    // Compare against first item
                    bool isSameItem = false;

                    if (m_connectFirstItem
                            .canConvert<TerminalItem *>()
                        && terminalVariant.canConvert<
                            TerminalItem *>())
                    {
                        isSameItem =
                            m_connectFirstItem
                                .value<TerminalItem *>()
                            == terminalVariant
                                   .value<TerminalItem *>();
                    }
                    else if (m_connectFirstItem.canConvert<
                                 GlobalTerminalItem *>()
                             && terminalVariant.canConvert<
                                 GlobalTerminalItem *>())
                    {
                        isSameItem =
                            m_connectFirstItem.value<
                                GlobalTerminalItem *>()
                            == terminalVariant.value<
                                GlobalTerminalItem *>();
                    }

                    if (isSameItem)
                    {
                        m_connectFirstItem =
                            QVariant(); // Reset to null

                        // Show error message
                        if (!views().isEmpty())
                        {
                            mainWindowObj
                                ->showStatusBarMessage(
                                    "Cannot connect "
                                    "terminal to "
                                    "itself.",
                                    2000);
                        }
                        return;
                    }

                    // Create connection through utility
                    // function

                    // Get first terminal
                    QGraphicsItem *firstItem = nullptr;
                    if (m_connectFirstItem
                            .canConvert<TerminalItem *>())
                    {
                        firstItem =
                            m_connectFirstItem
                                .value<TerminalItem *>();
                    }
                    else if (m_connectFirstItem.canConvert<
                                 GlobalTerminalItem *>())
                    {
                        firstItem = m_connectFirstItem.value<
                            GlobalTerminalItem *>();
                    }

                    // Get second terminal
                    QGraphicsItem *secondItem = nullptr;
                    if (terminalVariant
                            .canConvert<TerminalItem *>())
                    {
                        secondItem =
                            terminalVariant
                                .value<TerminalItem *>();
                    }
                    else if (terminalVariant.canConvert<
                                 GlobalTerminalItem *>())
                    {
                        secondItem = terminalVariant.value<
                            GlobalTerminalItem *>();
                    }

                    // Call createConnectionLine utility
                    // function
                    ConnectionLine *connection =
                        ViewController::
                            createConnectionLine(
                                mainWindowObj, firstItem,
                                secondItem,
                                currentConnectionType);

                    if (connection)
                    {
                        mainWindowObj->showStatusBarMessage(
                            "Connection created. "
                            "Click another "
                            "terminal to "
                            "continue connecting.",
                            2000);

                        // Update scene visibility
                        ViewController::
                            updateSceneVisibility(
                                mainWindowObj);

                        // Set the second terminal
                        // as the first for the next
                        // connection
                        m_connectFirstItem = terminalVariant;
                    }
                    else
                    {
                        // If connection failed,
                        // reset first item
                        m_connectFirstItem = QVariant();
                    }

                    return;
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
                // Clear selection and hide properties
                // panel
                clearSelection();
                UtilitiesFunctions::hidePropertiesPanel(
                    mainWindowObj);
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

void GraphicsScene::keyPressEvent(QKeyEvent *event)
{
    // For Delete key, pass it up to MainWindow
    if (event->key() == Qt::Key_Delete
        || event->key() == Qt::Key_Backspace)
    {
        event->ignore();
        return;
    }

    QGraphicsScene::keyPressEvent(event);
}

} // namespace GUI
} // namespace CargoNetSim
