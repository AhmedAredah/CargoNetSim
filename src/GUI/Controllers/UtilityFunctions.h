#pragma once

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

class UtilitiesFunctions
{
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

    static QList<CargoNetSim::GUI::MapPoint *>
    getMapPointsOfTerminal(
        GraphicsScene *scene, TerminalItem *terminal,
        const QString &region      = "*",
        const QString &networkName = "*",
        NetworkType    networkType = NetworkType::Train);

    static CargoNetSim::GUI::TerminalItem *
    getOriginTerminal(GraphicsScene *scene);

    static CargoNetSim::GUI::TerminalItem *
    getDestinationTerminal(GraphicsScene *scene);

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
};

} // namespace GUI
} // namespace CargoNetSim
