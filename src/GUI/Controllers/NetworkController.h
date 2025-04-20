#pragma once

#include <QGraphicsItem>
#include <QMainWindow>
#include <QPointF>
#include <QString>
#include <QToolButton>

#include "Backend/Controllers/RegionDataController.h"
#include "GUI/Commons/NetworkType.h"
#include "GUI/Items/MapLine.h"

namespace CargoNetSim
{
namespace GUI
{

// Forward declarations
class MainWindow;
class GraphicsScene;
class TerminalItem;

class NetworkController
{
public:
    static QString
    importNetwork(MainWindow          *mainWindow,
                  NetworkType          networkType,
                  Backend::RegionData *regionData);

    static bool
    removeNetwork(MainWindow          *mainWindow,
                  NetworkType          networkType,
                  QString             &networkName,
                  Backend::RegionData *regionData);

    static bool renameNetwork(
        MainWindow *mainWindow, NetworkType networkType,
        const QString &oldName, const QString &newName,
        Backend::RegionData *regionData);

    static bool changeNetworkColor(
        MainWindow *mainWindow, NetworkType networkType,
        const QString &networkName, const QColor &newColor,
        Backend::RegionData *regionData);

    static CargoNetSim::Backend::ShortestPathResult
    findNetworkShortestPath(const QString &regionName,
                            const QString &networkName,
                            NetworkType    networkType,
                            int startNodeId, int endNodeId);

    static bool clearAllNetworks(MainWindow *mainWindow);

    /**
     * @brief Finds the MapLines that correspond to a
     * shortest path between two nodes
     * @param mainWindow Pointer to MainWindow
     * @param regionName Name of the region
     * @param networkName Name of the network
     * @param networkType Type of the network (Train, Truck,
     * Ship)
     * @param startNodeId Starting node ID
     * @param endNodeId Ending node ID
     * @return List of MapLines that form the shortest path
     */
    static QList<MapLine *> getShortestPathMapLines(
        MainWindow *mainWindow, const QString &regionName,
        const QString &networkName, NetworkType networkType,
        int startNodeId, int endNodeId);

protected:
    static bool
    importTrainNetwork(MainWindow          *mainWindow,
                       Backend::RegionData *regionData,
                       QString             &networkName);

    static bool
    importTruckNetwork(MainWindow          *mainWindow,
                       Backend::RegionData *regionData,
                       QString             &networkName);

private:
    static QString getNetworkTypeString(NetworkType type);
};

} // namespace GUI
} // namespace CargoNetSim
