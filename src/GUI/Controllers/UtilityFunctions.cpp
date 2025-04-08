#include "UtilityFunctions.h"
#include "../MainWindow.h"
#include "GUI/Controllers/NetworkController.h"
#include "GUI/Controllers/ViewController.h"
#include "GUI/Items/ConnectionLine.h"
#include "GUI/Items/MapPoint.h"

#include "GUI/Widgets/PropertiesPanel.h"
#include "GUI/Widgets/ShortestPathTable.h"

#include "GUI/Utils/PathFindingWorker.h"

QList<CargoNetSim::GUI::TerminalItem *>
CargoNetSim::GUI::UtilitiesFunctions::getTerminalItems(
    GraphicsScene *scene, const QString &region,
    const QString &terminalType,
    ConnectionType connectionType, LinkType linkType)
{
    // Early check
    if (!scene)
    {
        return QList<CargoNetSim::GUI::TerminalItem *>();
    }

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
    // Early check
    if (!scene)
    {
        return QList<
            CargoNetSim::GUI::GlobalTerminalItem *>();
    }

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
    if (!terminal || !scene)
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

    CargoNetSim::GUI::TerminalItem *result = nullptr;

    if (!scene)
    {
        return result;
    }

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
    CargoNetSim::GUI::TerminalItem *result = nullptr;

    if (!scene)
    {
        return result;
    }

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
    // Early check
    if (!mainWindow)
    {
        return;
    }

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
        mainWindow->propertiesDock_->raise();
        mainWindow->propertiesPanel_->displayProperties(
            item);
    }
}

void CargoNetSim::GUI::UtilitiesFunctions::
    hidePropertiesPanel(MainWindow *mainWindow)
{
    // Early check
    if (!mainWindow)
    {
        return;
    }

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
    // Early check
    if (!mainWindow)
    {
        return;
    }

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

void CargoNetSim::GUI::UtilitiesFunctions::
    getTopShortestPaths(MainWindow *mainWindow,
                        int         PathsCount)
{
    if (!mainWindow)
    {
        return;
    }

    // Create a worker and a thread
    QThread           *thread = new QThread();
    PathFindingWorker *worker = new PathFindingWorker();

    // Set up connections
    QObject::connect(thread, &QThread::started, worker,
                     [mainWindow, PathsCount, worker]() {
                         worker->initialize(mainWindow,
                                            PathsCount);
                         worker->process();
                     });
    // &PathFindingWorker::process);
    QObject::connect(worker, &PathFindingWorker::finished,
                     thread, &QThread::quit);
    QObject::connect(worker, &PathFindingWorker::finished,
                     worker,
                     &PathFindingWorker::deleteLater);
    QObject::connect(thread, &QThread::finished, thread,
                     &QThread::deleteLater);

    // Handle results
    QObject::connect(
        worker, &PathFindingWorker::resultReady, mainWindow,
        [mainWindow](const QList<Backend::Path *> &paths) {
            // Display results in the shortest paths table
            mainWindow->shortestPathTable_->clear();
            mainWindow->shortestPathTable_->addPaths(paths);
            // Show the table
            mainWindow->shortestPathTableDock_->show();
            mainWindow->findShortestPathButton_->setEnabled(
                true);
        },
        Qt::QueuedConnection);

    // Handle errors
    QObject::connect(
        worker, &PathFindingWorker::error, mainWindow,
        [mainWindow](const QString &message) {
            mainWindow->showStatusBarError(message, 3000);
            mainWindow->findShortestPathButton_->setEnabled(
                true);
        },
        Qt::QueuedConnection);

    // Move worker to thread and start
    worker->moveToThread(thread);
    // turn off the find shortest path button
    mainWindow->findShortestPathButton_->setEnabled(false);
    thread->start();
}

void CargoNetSim::GUI::UtilitiesFunctions::
    setConnectionProperties(
        CargoNetSim::GUI::ConnectionLine *connection,
        const CargoNetSim::Backend::ShortestPathResult
                                      &pathResult,
        CargoNetSim::GUI::NetworkType &networkType)
{
    // Early check
    if (!connection)
    {
        return;
    }

    // Convert distance from meters to kilometers
    double totalDistanceKm =
        pathResult.totalLength / 1000.0;

    // Set basic properties
    connection->setProperty(
        "distance",
        QString::number(totalDistanceKm, 'f', 2));

    // Get transport mode properties
    auto configController =
        CargoNetSim::CargoNetSimController::getInstance()
            .getConfigController();
    auto transportModes =
        configController->getTransportModes();

    QVariantMap modeProperties;
    if (networkType == CargoNetSim::GUI::NetworkType::Train)
    {
        modeProperties =
            transportModes.value("Rail").toMap();
    }
    else if (networkType
             == CargoNetSim::GUI::NetworkType::Truck)
    {
        modeProperties =
            transportModes.value("Truck").toMap();
    }
    else
    {
        return;
    }

    // Calculate travel time
    bool useNetwork =
        modeProperties.value("use_network", false).toBool();
    if (!useNetwork)
    {
        double averageSpeed =
            modeProperties.value("average_speed", 60.0)
                .toDouble();
        double travelTime =
            totalDistanceKm / qMax(averageSpeed, 0.01);
        connection->setProperty(
            "travelTime",
            QString::number(travelTime, 'f', 2));
    }
    else
    {
        // Use network-calculated travel time (convert from
        // seconds to hours)
        double travelTimeHours =
            pathResult.minTravelTime / (60.0 * 60.0);
        connection->setProperty(
            "travelTime",
            QString::number(travelTimeHours, 'f', 2));
    }

    // Set risk factor
    connection->setProperty(
        "risk", modeProperties.value("risk_factor"));

    // Calculate energy consumption and carbon emissions
    double fuelConsumption =
        modeProperties
            .value("average_fuel_consumption", 0.0)
            .toDouble();
    QString fuelType =
        modeProperties
            .value("fuel_type",
                   networkType
                           == CargoNetSim::GUI::
                               NetworkType::Train
                       ? "diesel_1"
                       : "diesel_2")
            .toString();

    // Get fuel properties
    double calorificValue =
        configController->getFuelEnergy()
            .value(fuelType, 10.0)
            .toDouble();
    double carbonContent =
        configController->getFuelCarbonContent()
            .value(fuelType, 2.68)
            .toDouble();

    // Calculate energy consumption
    double energyConsumption =
        fuelConsumption * totalDistanceKm * calorificValue;
    connection->setProperty(
        "energyConsumption",
        QString::number(energyConsumption, 'f', 2));

    // Calculate carbon emissions (convert to tons)
    double carbonEmissions = fuelConsumption
                             * totalDistanceKm
                             * carbonContent / 1000.0;
    connection->setProperty(
        "carbonEmissions",
        QString::number(carbonEmissions, 'f', 2));
}

bool CargoNetSim::GUI::UtilitiesFunctions::
    processNetworkModeConnection(
        MainWindow                     *mainWindow,
        CargoNetSim::GUI::TerminalItem *sourceTerminal,
        CargoNetSim::GUI::TerminalItem *targetTerminal,
        CargoNetSim::GUI::NetworkType   networkType)
{

    QString networkTypeStr =
        networkType == CargoNetSim::GUI::NetworkType::Train
            ? "Rail"
            : "Truck";

    QString regionName;

    if (sourceTerminal && targetTerminal)
    {
        if (sourceTerminal == targetTerminal)
        {
            return false;
        }

        if (sourceTerminal->getRegion()
            != targetTerminal->getRegion())
        {
            mainWindow->showStatusBarError(
                "Terminals are in different regions.",
                3000);
            return false;
        }
        else
        {
            regionName = sourceTerminal->getRegion();
        }
    }
    else
    {
        return false;
    }

    // Get map points for both terminals
    QList<CargoNetSim::GUI::MapPoint *> sourcePoints =
        getMapPointsOfTerminal(mainWindow->regionScene_,
                               sourceTerminal, regionName,
                               "*", networkType);

    QList<CargoNetSim::GUI::MapPoint *> targetPoints =
        getMapPointsOfTerminal(mainWindow->regionScene_,
                               targetTerminal, regionName,
                               "*", networkType);

    bool continueProcess = true;
    if (sourcePoints.empty())
    {
        mainWindow->showStatusBarError(
            QString("Terminal %1 has no associated nodes.")
                .arg(sourceTerminal->getProperty("Name", "")
                         .toString()),
            3000);
        continueProcess = false;
    }

    if (targetPoints.empty())
    {
        mainWindow->showStatusBarError(
            QString("Terminal %1 has no associated nodes.")
                .arg(targetTerminal->getProperty("Name", "")
                         .toString()),
            3000);
        continueProcess = false;
    }

    if (!continueProcess)
        return false;

    // Group map points by network
    QMap<QString, QList<CargoNetSim::GUI::MapPoint *>>
        sourceNetworks;
    QMap<QString, QList<CargoNetSim::GUI::MapPoint *>>
        targetNetworks;

    auto getNetworkName = [](GUI::MapPoint *point) {
        QString networkName;
        auto    netObj = point->getReferenceNetwork();
        if (!netObj)
        {
            return QString();
        }
        Backend::TrainClient::NeTrainSimNetwork
            *trainNetObj = qobject_cast<
                Backend::TrainClient::NeTrainSimNetwork *>(
                netObj);
        if (trainNetObj)
        {
            networkName = trainNetObj->getNetworkName();
        }
        else
        {
            Backend::TruckClient::IntegrationNetwork
                *truckNetObj = qobject_cast<
                    Backend::TruckClient::IntegrationNetwork
                        *>(netObj);

            if (truckNetObj)
            {
                networkName = truckNetObj->getNetworkName();
            }
        }
        return networkName;
    };

    for (auto it = sourcePoints.constBegin();
         it != sourcePoints.constEnd(); ++it)
    {
        QString network = getNetworkName(*it);
        if (!network.isEmpty())
            sourceNetworks[network].append(*it);
    }

    for (auto it = targetPoints.constBegin();
         it != targetPoints.constEnd(); ++it)
    {
        QString network = getNetworkName(*it);
        if (!network.isEmpty())
            targetNetworks[network].append(*it);
    }

    // Find common networks
    QSet<QString> sourceNetworkNames;
    for (auto it = sourceNetworks.constBegin();
         it != sourceNetworks.constEnd(); ++it)
    {
        sourceNetworkNames.insert(it.key());
    }

    QSet<QString> targetNetworkNames;
    for (auto it = targetNetworks.constBegin();
         it != targetNetworks.constEnd(); ++it)
    {
        targetNetworkNames.insert(it.key());
    }

    QSet<QString> commonNetworks = sourceNetworkNames;
    commonNetworks.intersect(targetNetworkNames);

    // Check each common network for valid paths
    for (const QString &network : commonNetworks)
    {
        auto sourcePointsForNetwork =
            sourceNetworks[network];
        auto targetPointsForNetwork =
            targetNetworks[network];

        for (auto sourcePoint : sourcePointsForNetwork)
        {
            for (auto targetPoint : targetPointsForNetwork)
            {
                try
                {
                    QString sourceNodeId =
                        sourcePoint
                            ->getReferencedNetworkNodeID();
                    QString targetNodeId =
                        targetPoint
                            ->getReferencedNetworkNodeID();

                    bool validSourceID, validTargetID;
                    int  sourceID =
                        sourceNodeId.toInt(&validSourceID);
                    int targetID =
                        targetNodeId.toInt(&validTargetID);

                    if (!validSourceID || !validTargetID)
                    {
                        qWarning() << "Invalid source or "
                                      "target node ID:"
                                   << sourceNodeId << "or"
                                   << targetNodeId;
                        continue;
                    }

                    // Find shortest path
                    CargoNetSim::Backend::ShortestPathResult
                        result = NetworkController::
                            findNetworkShortestPath(
                                regionName, network,
                                networkType, sourceID,
                                targetID);

                    if (!result.pathNodes.empty()
                        && result.pathNodes.size() > 1)
                    {
                        // Create connection
                        CargoNetSim::GUI::ConnectionLine
                            *connection = CargoNetSim::GUI::
                                ViewController::
                                    createConnectionLine(
                                        mainWindow,
                                        sourceTerminal,
                                        targetTerminal,
                                        networkTypeStr);

                        if (!connection)
                        {
                            continue;
                        }

                        // Set connection properties
                        setConnectionProperties(
                            connection, result,
                            networkType);

                        // We found a valid path, so break
                        // out of the inner loops
                        break;
                    }
                }
                catch (const std::exception &e)
                {
                    qWarning()
                        << "Error processing network path:"
                        << e.what();
                }
            }
        }
    }

    return true;
}

void CargoNetSim::GUI::UtilitiesFunctions::
    linkMapPointToTerminal(MainWindow   *mainWindow,
                           MapPoint     *mapPoint,
                           TerminalItem *terminal)
{
    if (!mainWindow || !mapPoint || !terminal)
    {
        return;
    }

    // Link the terminal to the node
    mapPoint->setLinkedTerminal(terminal);

    // Update the properties panel if this item is currently
    // selected
    if (mainWindow->propertiesPanel_->getCurrentItem()
        == mapPoint)
    {
        mainWindow->propertiesPanel_->displayProperties(
            mapPoint);
    }

    // Force a redraw of the MapPoint to show the terminal
    // icon
    mapPoint->update();

    // Show status message
    mainWindow->showStatusBarMessage(
        "Terminal linked to node successfully", 2000);
}
