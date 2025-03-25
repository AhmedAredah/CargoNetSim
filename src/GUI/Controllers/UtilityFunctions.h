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

    QList<CargoNetSim::GUI::MapPoint *>
    getMapPointsOfTerminal(
        GraphicsScene *scene, TerminalItem *terminal,
        const QString &region      = "*",
        const QString &networkName = "*",
        NetworkType    networkType = NetworkType::Train);
};

} // namespace GUI
} // namespace CargoNetSim
