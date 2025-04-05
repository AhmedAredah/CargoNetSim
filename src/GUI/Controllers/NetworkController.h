#pragma once

#include <QGraphicsItem>
#include <QMainWindow>
#include <QPointF>
#include <QString>
#include <QToolButton>

#include "Backend/Controllers/RegionDataController.h"
#include "GUI/Commons/NetworkType.h"

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
