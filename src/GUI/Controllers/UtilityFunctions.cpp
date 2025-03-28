#include "UtilityFunctions.h"
#include "../MainWindow.h"
#include "GUI/Controllers/ViewController.h"
#include "GUI/Items/ConnectionLine.h"
#include "GUI/Items/MapPoint.h"

#include "Backend/Controllers/CargoNetSimController.h"
#include "GUI/Widgets/PropertiesPanel.h"

QList<CargoNetSim::GUI::TerminalItem *>
CargoNetSim::GUI::UtilitiesFunctions::getTerminalItems(
    GraphicsScene *scene, const QString &region,
    const QString &terminalType,
    ConnectionType connectionType, LinkType linkType)
{
    // Get all terminals upfront
    QList<TerminalItem *> allTerminals =
        scene->getItemsByType<TerminalItem>();

    // Pre-calculate the connection and link sets only if
    // needed
    QSet<TerminalItem *> connectionLineTerminalItems;
    QSet<TerminalItem *> mapPointsLinkedTerminals;

    if (connectionType != ConnectionType::Any)
    {
        QList<ConnectionLine *> connectionLines =
            scene->getItemsByType<ConnectionLine>();
        for (auto connectionLine : connectionLines)
        {
            if (auto startTerminal =
                    dynamic_cast<TerminalItem *>(
                        connectionLine->startItem()))
                connectionLineTerminalItems.insert(
                    startTerminal);
            if (auto endTerminal =
                    dynamic_cast<TerminalItem *>(
                        connectionLine->endItem()))
                connectionLineTerminalItems.insert(
                    endTerminal);
        }
    }

    if (linkType != LinkType::Any)
    {
        QList<MapPoint *> mapPoints =
            scene->getItemsByType<MapPoint>();
        for (auto mapPoint : mapPoints)
        {
            if (auto linkedTerminal =
                    mapPoint->getLinkedTerminal())
                mapPointsLinkedTerminals.insert(
                    linkedTerminal);
        }
    }

    // Create result container with initial capacity to
    // avoid reallocations
    QList<TerminalItem *> result;
    result.reserve(allTerminals.size());

    const bool anyRegion = region == "*";
    const bool anyType   = terminalType == "*";

    // Filter terminals
    for (auto terminal : allTerminals)
    {
        // Region check
        if (!anyRegion && terminal->getRegion() != region)
        {
            continue;
        }

        // Type check
        if (!anyType
            && terminal->getTerminalType() != terminalType)
        {
            continue;
        }

        // Connection check
        if (connectionType == ConnectionType::Connected
            && !connectionLineTerminalItems.contains(
                terminal))
        {
            continue;
        }
        else if (connectionType
                     == ConnectionType::NotConnected
                 && connectionLineTerminalItems.contains(
                     terminal))
        {
            continue;
        }

        // Link check
        if (linkType == LinkType::Linked
            && !mapPointsLinkedTerminals.contains(terminal))
        {
            continue;
        }
        else if (linkType == LinkType::NotLinked
                 && mapPointsLinkedTerminals.contains(
                     terminal))
        {
            continue;
        }

        result.append(terminal);
    }

    return result;
}

QList<CargoNetSim::GUI::GlobalTerminalItem *> CargoNetSim::
    GUI::UtilitiesFunctions::getGlobalTerminalItems(
        GraphicsScene *scene, const QString &region,
        const QString &terminalType,
        ConnectionType connectionType, LinkType linkType)
{
    // Get all terminals upfront
    QList<GlobalTerminalItem *> allTerminals =
        scene->getItemsByType<GlobalTerminalItem>();

    // Pre-calculate the connection and link sets only if
    // needed
    QSet<GlobalTerminalItem *> connectionLineTerminalItems;
    QSet<TerminalItem *>       mapPointsLinkedTerminals;

    if (connectionType != ConnectionType::Any)
    {
        QList<ConnectionLine *> connectionLines =
            scene->getItemsByType<ConnectionLine>();
        for (auto connectionLine : connectionLines)
        {
            if (auto startTerminal =
                    dynamic_cast<GlobalTerminalItem *>(
                        connectionLine->startItem()))
                connectionLineTerminalItems.insert(
                    startTerminal);
            if (auto endTerminal =
                    dynamic_cast<GlobalTerminalItem *>(
                        connectionLine->endItem()))
                connectionLineTerminalItems.insert(
                    endTerminal);
        }
    }

    if (linkType != LinkType::Any)
    {
        QList<MapPoint *> mapPoints =
            scene->getItemsByType<MapPoint>();
        for (auto mapPoint : mapPoints)
        {
            if (auto linkedTerminal =
                    mapPoint->getLinkedTerminal())
                mapPointsLinkedTerminals.insert(
                    linkedTerminal);
        }
    }

    // Create result container with initial capacity to
    // avoid reallocations
    QList<GlobalTerminalItem *> result;
    result.reserve(allTerminals.size());

    const bool anyRegion = region == "*";
    const bool anyType   = terminalType == "*";

    // Filter terminals
    for (auto terminal : allTerminals)
    {
        // Region check
        if (!anyRegion && terminal->getLinkedTerminalItem()
            && terminal->getLinkedTerminalItem()
                       ->getRegion()
                   != region)
        {
            continue;
        }

        // Type check
        if (!anyType && terminal->getLinkedTerminalItem()
            && terminal->getLinkedTerminalItem()
                       ->getTerminalType()
                   != terminalType)
        {
            continue;
        }

        // Connection check
        if (connectionType == ConnectionType::Connected
            && !connectionLineTerminalItems.contains(
                terminal))
        {
            continue;
        }
        else if (connectionType
                     == ConnectionType::NotConnected
                 && connectionLineTerminalItems.contains(
                     terminal))
        {
            continue;
        }

        // Link check
        if (linkType == LinkType::Linked
            && !mapPointsLinkedTerminals.contains(
                terminal->getLinkedTerminalItem()))
        {
            continue;
        }
        else if (linkType == LinkType::NotLinked
                 && mapPointsLinkedTerminals.contains(
                     terminal->getLinkedTerminalItem()))
        {
            continue;
        }

        result.append(terminal);
    }

    return result;
}

QList<CargoNetSim::GUI::MapPoint *> CargoNetSim::GUI::
    UtilitiesFunctions::getMapPointsOfTerminal(
        GraphicsScene *scene, TerminalItem *terminal,
        const QString &region, const QString &networkName,
        NetworkType networkType)
{
    QList<MapPoint *> result;

    // Early return if terminal is null
    if (!terminal)
    {
        return result;
    }

    QList<MapPoint *> allMapPoints =
        scene->getItemsByType<MapPoint>();

    const bool anyRegion      = region == "*";
    const bool anyNetworkName = networkName == "*";

    for (const auto &mapPoint : allMapPoints)
    {
        // Skip immediately if not linked to the target
        // terminal
        if (mapPoint->getLinkedTerminal() != terminal)
        {
            continue;
        }

        // Skip if region doesn't match (when not wildcard)
        if (!anyRegion && mapPoint->getRegion() != region)
        {
            continue;
        }

        // Check network type regardless of network name
        bool    networkTypeMatches = false;
        QString refNetworkName;

        if (networkType == NetworkType::Train)
        {
            auto network = dynamic_cast<
                Backend::TrainClient::NeTrainSimNetwork *>(
                mapPoint->getReferenceNetwork());

            if (network)
            {
                networkTypeMatches = true;
                refNetworkName = network->getNetworkName();
            }
        }
        else if (networkType == NetworkType::Truck)
        {
            auto network = dynamic_cast<
                Backend::TruckClient::IntegrationNetwork *>(
                mapPoint->getReferenceNetwork());

            if (network)
            {
                networkTypeMatches = true;
                refNetworkName = network->getNetworkName();
            }
        }

        // Skip if network type doesn't match
        if (!networkTypeMatches)
        {
            continue;
        }

        // Check network name only if it's not wildcard
        if (!anyNetworkName
            && refNetworkName != networkName)
        {
            continue;
        }

        // If we got here, all conditions are met
        result.append(mapPoint);
    }

    return result;
}

CargoNetSim::GUI::TerminalItem *
CargoNetSim::GUI::UtilitiesFunctions::getOriginTerminal(
    GraphicsScene *scene)
{

    CargoNetSim::GUI::TerminalItem *result;

    for (auto &terminal :
         scene->getItemsByType<TerminalItem>())
    {
        if (terminal->getTerminalType() == "Origin")
        {
            result = terminal;
            break;
        }
    }

    return result;
}

CargoNetSim::GUI::TerminalItem *
CargoNetSim::GUI::UtilitiesFunctions::
    getDestinationTerminal(GraphicsScene *scene)
{
    CargoNetSim::GUI::TerminalItem *result;

    for (auto &terminal :
         scene->getItemsByType<TerminalItem>())
    {
        if (terminal->getTerminalType() == "Destination")
        {
            result = terminal;
            break;
        }
    }

    return result;
}

void CargoNetSim::GUI::UtilitiesFunctions::
    updatePropertiesPanel(MainWindow    *mainWindow,
                          QGraphicsItem *item)
{
    if (!item)
    {
        // If no item is selected, hide the properties dock
        CargoNetSim::GUI::UtilitiesFunctions::
            hidePropertiesPanel(mainWindow);
    }
    else
    {
        // Show properties dock and update its contents
        mainWindow->propertiesDock_->show();
        mainWindow->propertiesPanel_->displayProperties(
            item);
    }
}

void CargoNetSim::GUI::UtilitiesFunctions::
    hidePropertiesPanel(MainWindow *mainWindow)
{
    // Show map properties only if on main view or global
    // map tabs
    int currentTab = mainWindow->tabWidget_->currentIndex();
    if (currentTab
        == mainWindow->tabWidget_->indexOf(
            mainWindow->tabWidget_->widget(0)))
    { // Main view tab
        mainWindow->propertiesPanel_->displayProperties(
            nullptr);
        mainWindow->propertiesDock_->show();
        mainWindow->propertiesPanel_
            ->displayMapProperties();
    }
    else
    {
        mainWindow->propertiesDock_->hide();
        mainWindow->propertiesPanel_->displayProperties(
            nullptr);
    }
}

void CargoNetSim::GUI::UtilitiesFunctions::
    updateGlobalMapForRegion(MainWindow    *mainWindow,
                             const QString &regionName)
{
    auto regionTerminals = CargoNetSim::GUI::
        UtilitiesFunctions::getTerminalItems(
            mainWindow->regionScene_, regionName, "*");
    for (TerminalItem *item : regionTerminals)
    {
        ViewController::updateGlobalMapItem(mainWindow,
                                            item);
    }
}

QList<QString>
CargoNetSim::GUI::UtilitiesFunctions::getCommonModes(
    QGraphicsItem *sourceItem, QGraphicsItem *targetItem)
{
    // Early null check to avoid segmentation faults
    if (!sourceItem || !targetItem)
    {
        return {};
    }

    // Get modes from each terminal
    QSet<QString> sourceModes;
    QSet<QString> targetModes;

    // Handle different terminal types
    if (auto *sourceGlobal = dynamic_cast<
            CargoNetSim::GUI::GlobalTerminalItem *>(
            sourceItem))
    {
        if (auto *linkedTerminal =
                sourceGlobal->getLinkedTerminalItem())
        {
            const QMap<QString, QVariant> &interfaces =
                linkedTerminal->getProperties()
                    .value("Available Interfaces")
                    .toMap();
            const QStringList &landModes =
                interfaces.value("land_side")
                    .toStringList();
            const QStringList &seaModes =
                interfaces.value("sea_side").toStringList();

            sourceModes.reserve(landModes.size()
                                + seaModes.size());
            for (const auto &mode : landModes)
                sourceModes.insert(mode);
            for (const auto &mode : seaModes)
                sourceModes.insert(mode);
        }
    }
    else if (auto *sourceTerminal = dynamic_cast<
                 CargoNetSim::GUI::TerminalItem *>(
                 sourceItem))
    {
        const QMap<QString, QVariant> &interfaces =
            sourceTerminal->getProperties()
                .value("Available Interfaces")
                .toMap();
        const QStringList &landModes =
            interfaces.value("land_side").toStringList();
        const QStringList &seaModes =
            interfaces.value("sea_side").toStringList();

        sourceModes.reserve(landModes.size()
                            + seaModes.size());
        for (const auto &mode : landModes)
            sourceModes.insert(mode);
        for (const auto &mode : seaModes)
            sourceModes.insert(mode);
    }

    // Skip processing target if source is empty
    if (sourceModes.isEmpty())
    {
        return {};
    }

    // Build target modes set and find common modes directly
    QSet<QString> commonModes;

    if (auto *targetGlobal = dynamic_cast<
            CargoNetSim::GUI::GlobalTerminalItem *>(
            targetItem))
    {
        if (auto *linkedTerminal =
                targetGlobal->getLinkedTerminalItem())
        {
            const QMap<QString, QVariant> &interfaces =
                linkedTerminal->getProperties()
                    .value("Available Interfaces")
                    .toMap();
            const QStringList &landModes =
                interfaces.value("land_side")
                    .toStringList();
            const QStringList &seaModes =
                interfaces.value("sea_side").toStringList();

            commonModes.reserve(
                qMin(sourceModes.size(),
                     landModes.size() + seaModes.size()));
            for (const auto &mode : landModes)
            {
                if (sourceModes.contains(mode))
                {
                    commonModes.insert(mode);
                }
            }
            for (const auto &mode : seaModes)
            {
                if (sourceModes.contains(mode))
                {
                    commonModes.insert(mode);
                }
            }
        }
    }
    else if (auto *targetTerminal = dynamic_cast<
                 CargoNetSim::GUI::TerminalItem *>(
                 targetItem))
    {
        const QMap<QString, QVariant> &interfaces =
            targetTerminal->getProperties()
                .value("Available Interfaces")
                .toMap();
        const QStringList &landModes =
            interfaces.value("land_side").toStringList();
        const QStringList &seaModes =
            interfaces.value("sea_side").toStringList();

        commonModes.reserve(
            qMin(sourceModes.size(),
                 landModes.size() + seaModes.size()));
        for (const auto &mode : landModes)
        {
            if (sourceModes.contains(mode))
            {
                commonModes.insert(mode);
            }
        }
        for (const auto &mode : seaModes)
        {
            if (sourceModes.contains(mode))
            {
                commonModes.insert(mode);
            }
        }
    }

    return QList<QString>(commonModes.begin(),
                          commonModes.end());
}

double CargoNetSim::GUI::UtilitiesFunctions::
    getApproximateGeoDistance(const QPointF &point1,
                              const QPointF &point2)
{
    // Earth's approximate radius in meters
    const double earthRadius = 6371000.0;

    // Get coordinates (QPointF stores as x=longitude,
    // y=latitude)
    double lon1 = point1.x();
    double lat1 = point1.y();
    double lon2 = point2.x();
    double lat2 = point2.y();

    // Convert degrees to radians
    double lat1Rad = lat1 * M_PI / 180.0;
    double lon1Rad = lon1 * M_PI / 180.0;
    double lat2Rad = lat2 * M_PI / 180.0;
    double lon2Rad = lon2 * M_PI / 180.0;

    // Simplified haversine formula for performance
    double dLat = lat2Rad - lat1Rad;
    double dLon = lon2Rad - lon1Rad;

    // Approximate using equirectangular projection
    double x = dLon * cos((lat1Rad + lat2Rad) / 2.0);
    double y = dLat;

    // Calculate distance
    return earthRadius * sqrt(x * x + y * y);
}

CargoNetSim::GUI::UtilitiesFunctions::PathResult
findShortestPath(const QString                &regionName,
                 const QString                &networkName,
                 CargoNetSim::GUI::NetworkType networkType,
                 int startNodeId, int endNodeId)
{
    CargoNetSim::GUI::UtilitiesFunctions::PathResult result;
    try
    {
        if (networkType
            == CargoNetSim::GUI::NetworkType::Train)
        {
            auto network =
                CargoNetSim::CargoNetSimController::
                    getInstance()
                        .getRegionDataController()
                        ->getRegionData(regionName)
                        ->getTrainNetwork(networkName);

            // Find the shortest path
            auto path = network->findShortestPath(
                startNodeId, endNodeId);

            result.path_nodes    = path.pathNodes;
            result.totalLength   = path.totalLength;
            result.minTravelTime = path.minTravelTime;
        }
        else if (networkType
                 == CargoNetSim::GUI::NetworkType::Truck)
        {
            auto network =
                CargoNetSim::CargoNetSimController::
                    getInstance()
                        .getRegionDataController()
                        ->getRegionData(regionName)
                        ->getTruckNetwork(networkName);

            auto path = network->findShortestPath(
                startNodeId, endNodeId);

            result.path_nodes    = path.pathNodes;
            result.totalLength   = path.totalLength;
            result.minTravelTime = path.minTravelTime;
        }
    }
    catch (const std::exception &e)
    {
        qWarning() << "Error finding shortest path:"
                   << e.what();
    }

    return result;
}
