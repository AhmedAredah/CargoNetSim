#pragma once

#include "Backend/Commons/ShortestPathResult.h"
#include "Backend/Controllers/CargoNetSimController.h"
#include "GUI/Commons/NetworkType.h"
#include "GUI/Items/RegionCenterPoint.h"
#include "GUI/Items/TerminalItem.h"
#include "GUI/Widgets/GraphicsScene.h"
#include <QGraphicsItem>
#include <QMainWindow>
#include <QPointF>
#include <QString>
#include <QToolButton>

namespace CargoNetSim
{
namespace GUI
{

class MapPoint;
class MainWindow;
class PathFindingWorker;

class UtilitiesFunctions
{
    friend class PathFindingWorker;

public:
    enum class ConnectionType
    {
        Any,
        Connected,
        NotConnected
    };

    enum class LinkType
    {
        Any,
        Linked,
        NotLinked
    };

    static QList<CargoNetSim::GUI::TerminalItem *>
    getTerminalItems(
        GraphicsScene *scene, const QString &region = "*",
        const QString &terminalType   = "*",
        ConnectionType connectionType = ConnectionType::Any,
        LinkType       linkType       = LinkType::Any);

    static QList<CargoNetSim::GUI::GlobalTerminalItem *>
    getGlobalTerminalItems(GraphicsScene *scene,
                           const QString &region,
                           const QString &terminalType,
                           ConnectionType connectionType,
                           LinkType       linkType);

    static QList<CargoNetSim::GUI::MapPoint *>
    getMapPointsOfTerminal(
        GraphicsScene *scene, TerminalItem *terminal,
        const QString &region      = "*",
        const QString &networkName = "*",
        NetworkType    networkType = NetworkType::Train);

    static CargoNetSim::GUI::TerminalItem *
    getOriginTerminal(MainWindow *mainWindow);

    static CargoNetSim::GUI::TerminalItem *
    getDestinationTerminal(MainWindow *mainWindow);

    /**
     * @brief Updates the properties panel with the selected
     * item's properties
     * @ param mainWindow The main window
     * @param item The selected graphics item
     */
    static void
    updatePropertiesPanel(MainWindow    *mainWindow,
                          QGraphicsItem *item);

    /**
     * @brief Hides the properties panel
     * @ param mainWindow The main window
     */
    static void hidePropertiesPanel(MainWindow *mainWindow);

    /**
     * @brief Updates global map items for a region
     * @param mainWindow The main window
     * @param regionName The name of the region to update
     */
    static void
    updateGlobalMapForRegion(MainWindow    *mainWindow,
                             const QString &regionName);

    static QList<QString>
    getCommonModes(QGraphicsItem *sourceItem,
                   QGraphicsItem *targetItem);

    static QList<QPair<MapPoint *, MapPoint *>>
    getCommonNetworks(QList<MapPoint *> firstEntries,
                      QList<MapPoint *> secondEntries);

    static QList<QPair<CargoNetSim::GUI::MapPoint *,
                       CargoNetSim::GUI::MapPoint *>>
    getCommonNetworksOfNetworkType(
        QList<CargoNetSim::GUI::MapPoint *> firstEntries,
        QList<CargoNetSim::GUI::MapPoint *> secondEntries,
        NetworkType                         networkType);

    static double
    getApproximateGeoDistance(const QPointF &point1,
                              const QPointF &point2);

    static void getTopShortestPaths(MainWindow *mainWindow,
                                    int         PathsCount);

    static bool setConnectionProperties(
        MainWindow                       *mainWindow,
        CargoNetSim::GUI::ConnectionLine *connection,
        const CargoNetSim::Backend::ShortestPathResult
                                      &pathResult,
        CargoNetSim::GUI::NetworkType &networkType);

    static bool processNetworkModeConnection(
        MainWindow                     *mainWindow,
        CargoNetSim::GUI::TerminalItem *sourceTerminal,
        CargoNetSim::GUI::TerminalItem *targetTerminal,
        CargoNetSim::GUI::NetworkType   networkType);

    static void
    linkMapPointToTerminal(MainWindow   *mainWindow,
                           MapPoint     *mapPoint,
                           TerminalItem *terminal);

    static void
    validateSelectedSimulation(MainWindow *mainWindow);
};

} // namespace GUI
} // namespace CargoNetSim
