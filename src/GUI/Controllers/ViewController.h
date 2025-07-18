#pragma once

#include "Backend/Controllers/RegionDataController.h"
#include "GUI/Items/ConnectionLine.h"
#include "GUI/Items/MapLine.h"
#include "GUI/Items/MapPoint.h"
#include "GUI/Items/RegionCenterPoint.h"
#include <QGraphicsItem>
#include <QMainWindow>
#include <QPointF>
#include <QString>
#include <QToolButton>

#include "GUI/Commons/NetworkType.h"

namespace CargoNetSim
{
namespace GUI
{

// Forward declarations
class MainWindow;
class GraphicsScene;
class TerminalItem;

class ViewController
{
public:
    static void
    updateSceneVisibility(MainWindow *mainWindow);

    static void updateGlobalMapItem(MainWindow *main_window,
                                    TerminalItem *terminal);

    static TerminalItem *createTerminalAtPoint(
        MainWindow *main_window, const QString &region,
        const QString &terminalType, const QPointF &point);

    static void
    flashTerminalItems(QList<TerminalItem *> terminals,
                       bool evenIfHidden = false);

    static void drawNetwork(MainWindow          *mainWindow,
                            Backend::RegionData *regionData,
                            NetworkType networkType,
                            QString    &networkName);

    static void
    changeNetworkVisibility(MainWindow    *mainWindow,
                            const QString &networkName,
                            const bool     isVisible);

    static void renameRegion(MainWindow    *mainWindow,
                             const QString &oldRegionName,
                             const QString &newName);

    static void
    changeNetworkColor(MainWindow    *mainWindow,
                       const QString &networkName,
                       const QColor   newColor);

    static void
    removeNetwork(MainWindow          *mainWindow,
                  NetworkType          networkType,
                  Backend::RegionData *regionData,
                  QString             &networkName);

    /**
     * @brief Adds a background photo to the current view
     */
    static void addBackgroundPhoto(MainWindow *mainWindow);

    /**
     * @brief Checks if a connection of the same type
     * already exists between two terminals
     * @param startItem The first terminal
     * @param endItem The second terminal
     * @param connectionType The type of connection to check
     * @return True if a connection of the specified type
     * already exists, false otherwise
     */
    static bool
    checkExistingConnection(MainWindow    *mainWindow,
                            QGraphicsItem *startItem,
                            QGraphicsItem *endItem,
                            const QString &connectionType);

    static ConnectionLine *
    createConnectionLine(MainWindow    *mainWindow,
                         QGraphicsItem *startItem,
                         QGraphicsItem *endItem,
                         const QString &connectionType);

    static bool removeConnectionLine(
        MainWindow                       *mainWindow,
        CargoNetSim::GUI::ConnectionLine *connectionLine);

    static void connectVisibleTerminalsByNetworks(
        MainWindow *mainWindow);

    static void connectVisibleTerminalsByInterfaces(
        MainWindow *mainWindow);

    /**
     * @brief Creates a new region center
     * @param mainWindow The main window
     * @param regionName The name of the region
     * @param color The color of the region
     * @param pos The position of the region center
     * (optional)
     * @return Pointer to the created RegionCenterPoint
     */
    static RegionCenterPoint *
    createRegionCenter(MainWindow    *mainWindow,
                       const QString &regionName,
                       const QColor  &color,
                       const QPointF  pos = QPointF(0, 0),
                       const bool     keepVisible = false);

    static bool updateTerminalPositionByGlobalPosition(
        MainWindow *mainWindow, TerminalItem *terminal,
        QPointF globalGeoPos);

    /**
     * @brief Flashes the path lines for a selected path
     * @param mainWindow Pointer to MainWindow
     * @param pathId ID of the path to visualize
     */
    static void flashPathLines(MainWindow *mainWindow,
                               int         pathId);

    static bool linkTerminalToClosestNetworkPoint(
        MainWindow *mainWindow, TerminalItem *terminal,
        const QList<NetworkType> &networkTypes);

    static void linkAllVisibleTerminalsToNetwork(
        MainWindow               *mainWindow,
        const QList<NetworkType> &networkTypes);

    static bool unlinkTerminalFromNetworkPoints(
        MainWindow *mainWindow, TerminalItem *terminal,
        const QList<NetworkType> &networkTypes);

    static void unlinkAllVisibleTerminalsToNetwork(
        MainWindow               *mainWindow,
        const QList<NetworkType> &networkTypes);

    static void showFilteredConnections(
        MainWindow        *mainWindow,
        const QStringList &terminalNames,
        const QStringList &connectionTypes);

    static bool moveNetworkItems(MainWindow    *mainWindow,
                                 NetworkType    networkType,
                                 const QString &networkName,
                                 const QPointF &offset,
                                 const QString &regionName);

private:
    static void updateTerminalGlobalPosition(
        MainWindow        *main_window,
        RegionCenterPoint *regionCenterPoint,
        TerminalItem      *terminal);

    static void drawTrainNetwork(
        MainWindow                              *mainWindow,
        Backend::TrainClient::NeTrainSimNetwork *network,
        QString &regionName, QColor &linksColor);

    static void drawTruckNetwork(
        MainWindow *mainWindow,
        Backend::TruckClient::IntegrationSimulationConfig
                *networkConfig,
        QString &regionName, QColor &linksColor);

    static CargoNetSim::GUI::MapPoint *
    drawNode(MainWindow    *mainWindow,
             const QString &networkNodeID,
             const QString &nodeUniqueID,
             QPointF projectedPoint, QString &regionName,
             QColor                         color,
             const QMap<QString, QVariant> &properties =
                 QMap<QString, QVariant>());

    static CargoNetSim::GUI::MapLine *
    drawLink(MainWindow    *mainWindow,
             const QString &networkNodeID,
             const QString &linkUniqueID,
             QPointF        projectedStartPoint,
             QPointF projectedEndPoint, QString &regionName,
             QColor                         color,
             const QMap<QString, QVariant> &properties =
                 QMap<QString, QVariant>());
};

} // namespace GUI
} // namespace CargoNetSim
