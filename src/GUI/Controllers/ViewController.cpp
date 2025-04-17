#include "GUI/Controllers/ViewController.h"
#include "Backend/Controllers/CargoNetSimController.h"
#include "GUI/Items/BackgroundPhotoItem.h"
#include "GUI/Items/ConnectionLine.h"
#include "GUI/Items/MapLine.h"
#include "GUI/Items/MapPoint.h"
#include "GUI/MainWindow.h"
#include "GUI/Utils/ColorUtils.h"
#include "GUI/Utils/IconCreator.h"
#include "GUI/Widgets/GraphicsScene.h"
#include "GUI/Widgets/GraphicsView.h"
#include "GUI/Widgets/InterfaceSelectionDialog.h"
#include "GUI/Widgets/PropertiesPanel.h"
#include "UtilityFunctions.h"
#include <QtWidgets/qfiledialog.h>

void CargoNetSim::GUI::ViewController::
    updateSceneVisibility(MainWindow *mainWindow)
{
    QString currentRegion =
        CargoNetSim::CargoNetSimController::getInstance()
            .getRegionDataController()
            ->getCurrentRegion();
    // Update scene visibility based on settings
    GraphicsScene *scene = mainWindow->regionScene_;

    for (auto item : scene->items())
    {
        // Check if item is a terminal
        TerminalItem *terminal =
            dynamic_cast<TerminalItem *>(item);
        if (terminal)
        {
            terminal->setVisible(terminal->getRegion()
                                 == currentRegion);
        }

        // Check if item is a connection line
        ConnectionLine *connectionLine =
            dynamic_cast<ConnectionLine *>(item);
        if (connectionLine)
        {
            connectionLine->setVisible(
                connectionLine->getRegion()
                == currentRegion);
        }

        // Check if item is a region center
        RegionCenterPoint *regionCenter =
            dynamic_cast<RegionCenterPoint *>(item);
        if (regionCenter)
        {
            regionCenter->setVisible(
                regionCenter->getRegion() == currentRegion);
        }

        // Check if item is a map point
        MapPoint *mapPoint = dynamic_cast<MapPoint *>(item);
        if (mapPoint)
        {
            mapPoint->setVisible(mapPoint->getRegion()
                                 == currentRegion);
        }

        // Check if item is a map line
        MapLine *mapLine = dynamic_cast<MapLine *>(item);
        if (mapLine)
        {
            mapLine->setVisible(mapLine->getRegion()
                                == currentRegion);
        }

        BackgroundPhotoItem *backgroundPhoto =
            dynamic_cast<BackgroundPhotoItem *>(item);
        if (backgroundPhoto)
        {
            backgroundPhoto->setVisible(
                backgroundPhoto->getRegion()
                == currentRegion);
        }
    }
}

void CargoNetSim::GUI::ViewController::updateGlobalMapItem(
    MainWindow *main_window, TerminalItem *terminal)
{
    if (!terminal)
    {
        return;
    }
    auto props = terminal->getProperties();
    bool show =
        props.contains("Show on Global Map")
            ? props.value("Show on Global Map").toBool()
            : true;

    if (show)
    {
        auto regionData =
            CargoNetSim::CargoNetSimController::
                getInstance()
                    .getRegionDataController()
                    ->getRegionData(terminal->getRegion());

        if (!regionData)
        {
            return;
        }

        auto regionCenterPoint =
            regionData->getVariableAs<RegionCenterPoint *>(
                "regionCenterPoint", nullptr);

        if (!regionCenterPoint)
        {
            return;
        }

        if (terminal->getGlobalTerminalItem())
        {
            // Update the global terminal item position
            CargoNetSim::GUI::ViewController::
                updateTerminalGlobalPosition(
                    main_window, regionCenterPoint,
                    terminal);
        }
        else
        {
            // Create the global terminal item
            QPixmap pixmap       = terminal->getPixmap();
            auto global_terminal = new GlobalTerminalItem(
                pixmap, terminal, nullptr);

            // Add to the view
            main_window->globalMapView_->getScene()
                ->addItemWithId(global_terminal,
                                global_terminal->getID());
            terminal->setGlobalTerminalItem(
                global_terminal);

            QObject::connect(
                terminal, &TerminalItem::positionChanged,
                [main_window, regionCenterPoint,
                 terminal]() {
                    if (!regionCenterPoint)
                    {
                        return;
                    }
                    CargoNetSim::GUI::ViewController::
                        updateTerminalGlobalPosition(
                            main_window, regionCenterPoint,
                            terminal);
                });

            // Explicitly set its position
            updateTerminalGlobalPosition(
                main_window, regionCenterPoint, terminal);
        }
    }
    else
    {
        // Remove the global terminal item
        if (terminal->getGlobalTerminalItem())
        {
            GlobalTerminalItem *item =
                terminal->getGlobalTerminalItem();
            terminal->setGlobalTerminalItem(
                nullptr); // First detach from terminal
            main_window->globalMapView_->getScene()
                ->removeItemWithId<GlobalTerminalItem>(
                    item->getID()); // Then remove from
                                    // scene
        }
    }
}

void CargoNetSim::GUI::ViewController::
    updateTerminalGlobalPosition(
        MainWindow        *main_window,
        RegionCenterPoint *regionCenterPoint,
        TerminalItem      *terminal)
{
    // Check if the regionCenterPoint is not nullptr
    if (!regionCenterPoint || !terminal || !main_window)
    {
        return;
    }

    auto props = regionCenterPoint->getProperties();
    auto center_shared_lat =
        props.contains("Shared Latitude")
            ? props.value("Shared Latitude").toDouble()
            : 0.0;
    auto center_shared_lon =
        props.contains("Shared Longitude")
            ? props.value("Shared Longitude").toDouble()
            : 0;

    auto center_lon =
        props.contains("Longitude")
            ? props.value("Longitude").toDouble()
            : 0;
    auto center_lat =
        props.contains("Latitude")
            ? props.value("Latitude").toDouble()
            : 0;

    // Get terminal's coordinates in region view
    auto out = main_window->regionView_->sceneToWGS84(
        terminal->pos());

    double terminal_lon = out.x();
    double terminal_lat = out.y();

    // Calculate terminal's offset from region
    // center Calculate the deltas (terminal
    // relative to center)
    double delta_lat = terminal_lat - center_lat;
    double delta_lon = terminal_lon - center_lon;

    // Apply these deltas to the shared coordinates
    double item_global_view_lon =
        center_shared_lon + delta_lon;
    double item_global_view_lat =
        center_shared_lat + delta_lat;

    // Update the global terminal item
    GlobalTerminalItem *globalITem =
        terminal->getGlobalTerminalItem();
    if (!globalITem)
    {
        return;
    }

    // Important: Ensure we're using the correct coordinate
    // transformation
    QPointF globalPos =
        main_window->globalMapView_->wgs84ToScene(QPointF(
            item_global_view_lon, item_global_view_lat));
    // Set position directly to avoid any signal/slot
    // cascading issues
    globalITem->setPos(globalPos);
}

bool CargoNetSim::GUI::ViewController::
    updateTerminalPositionByGlobalPosition(
        MainWindow *mainWindow, TerminalItem *terminal,
        QPointF globalGeoPos)
{
    if (!mainWindow || !terminal
        || !terminal->getGlobalTerminalItem())
    {
        return false;
    }

    QString currentRegion = terminal->getRegion();

    // Get the region center point
    auto regionCenterPoint =
        CargoNetSim::CargoNetSimController::getInstance()
            .getRegionDataController()
            ->getRegionData(currentRegion)
            ->getVariableAs<RegionCenterPoint *>(
                "regionCenterPoint", nullptr);

    if (!regionCenterPoint)
    {
        return false;
    }

    auto props = regionCenterPoint->getProperties();

    auto center_lon =
        props.contains("Longitude")
            ? props.value("Longitude").toDouble()
            : 0;
    auto center_lat =
        props.contains("Latitude")
            ? props.value("Latitude").toDouble()
            : 0;

    QPointF terminalGeoPos =
        mainWindow->regionView_->sceneToWGS84(
            terminal->pos());

    // Calculate the delta (how far the terminal is from its
    // region center)
    double delta_lat = terminalGeoPos.y() - center_lat;
    double delta_lon = terminalGeoPos.x() - center_lon;

    // Calculate what the shared coordinates need to be to
    // position the terminal at the target location
    double new_shared_lat = globalGeoPos.y() - delta_lat;
    double new_shared_lon = globalGeoPos.x() - delta_lon;

    regionCenterPoint->setProperty("Shared Latitude",
                                   new_shared_lat);
    regionCenterPoint->setProperty("Shared Longitude",
                                   new_shared_lon);

    // Update the terminal position
    CargoNetSim::GUI::UtilitiesFunctions::
        updateGlobalMapForRegion(mainWindow, currentRegion);

    return true;
}

void CargoNetSim::GUI::ViewController::flashTerminalItems(
    QList<TerminalItem *> terminals, bool evenIfHidden)
{
    for (auto terminal : terminals)
    {
        terminal->flash(evenIfHidden);
    }
}

CargoNetSim::GUI::TerminalItem *
CargoNetSim::GUI::ViewController::createTerminalAtPoint(
    MainWindow *mainWindow, const QString &region,
    const QString &terminalType, const QPointF &point)
{
    // Create a new terminal item
    QMap<QString, QPixmap> terminalIcons =
        IconFactory::createTerminalIcons();
    QPixmap pixmap   = terminalIcons.value(terminalType);
    auto    terminal = new TerminalItem(pixmap, {}, region,
                                        nullptr, terminalType);
    terminal->setPos(point);
    mainWindow->regionScene_->addItemWithId(
        terminal, terminal->getID());

    // update the TerminalItem visibility
    terminal->setVisible(
        CargoNetSim::CargoNetSimController::getInstance()
            .getRegionDataController()
            ->getCurrentRegion()
        == region);

    // Update the Global Map Item visibility
    updateGlobalMapItem(mainWindow, terminal);

    // Connections
    QObject::connect(
        terminal, &TerminalItem::positionChanged,
        [mainWindow, terminal]() {
            CargoNetSim::GUI::ViewController::
                updateGlobalMapItem(mainWindow, terminal);
        });
    QObject::connect(
        terminal, &TerminalItem::clicked,
        [mainWindow](TerminalItem *terminal) {
            UtilitiesFunctions::updatePropertiesPanel(
                mainWindow, terminal);
        });
    QObject::connect(
        terminal, &TerminalItem::clicked, mainWindow,
        &MainWindow::handleTerminalNodeLinking);
    QObject::connect(
        terminal, &TerminalItem::clicked, mainWindow,
        &MainWindow::handleTerminalNodeUnlinking);

    return terminal;
}

void CargoNetSim::GUI::ViewController::drawNetwork(
    MainWindow *mainWindow, Backend::RegionData *regionData,
    NetworkType networkType, QString &networkName)
{
    QString regionName = regionData->getRegion();
    QColor  linksColor = ColorUtils::getRandomColor();

    // Get the network data
    if (networkType == NetworkType::Train)
    {
        auto network =
            regionData->getTrainNetwork(networkName);
        CargoNetSim::GUI::ViewController::drawTrainNetwork(
            mainWindow, network, regionName, linksColor);
    }
    else if (networkType == NetworkType::Truck)
    {
        auto network =
            regionData->getTruckNetworkConfig(networkName);
        CargoNetSim::GUI::ViewController::drawTruckNetwork(
            mainWindow, network, regionName, linksColor);
    }
}

void CargoNetSim::GUI::ViewController::
    changeNetworkVisibility(MainWindow    *mainWindow,
                            const QString &networkName,
                            const bool     isVisible)
{
    if (!mainWindow->regionScene_)
        return;

    // Common function to check network name and set
    // visibility
    auto checkNetworkAndSetVisibility =
        [&networkName,
         isVisible](const QString &itemNetworkName,
                    QGraphicsItem *graphicsItem) {
            if (itemNetworkName == networkName)
            {
                graphicsItem->setVisible(isVisible);
            }
        };

    auto mapPoints = mainWindow->regionScene_
                         ->getItemsByType<MapPoint>();
    auto mapLines =
        mainWindow->regionScene_->getItemsByType<MapLine>();

    for (auto mapPoint : mapPoints)
    {
        QObject *referencedNet =
            mapPoint->getReferenceNetwork();
        if (!referencedNet)
            continue;

        // Try to cast to train network
        if (auto *trainNet = dynamic_cast<
                Backend::TrainClient::NeTrainSimNetwork *>(
                referencedNet))
        {
            checkNetworkAndSetVisibility(
                trainNet->getNetworkName(), mapPoint);
        }
        // Try to cast to truck network
        else if (auto *truckNet =
                     dynamic_cast<Backend::TruckClient::
                                      IntegrationNetwork *>(
                         referencedNet))
        {
            checkNetworkAndSetVisibility(
                truckNet->getNetworkName(), mapPoint);
        }
    }

    for (auto mapLine : mapLines)
    {
        QObject *referencedNet =
            mapLine->getReferenceNetwork();
        if (!referencedNet)
            continue;

        // Try to cast to train network
        if (auto *trainNet = dynamic_cast<
                Backend::TrainClient::NeTrainSimNetwork *>(
                referencedNet))
        {
            checkNetworkAndSetVisibility(
                trainNet->getNetworkName(), mapLine);
        }
        // Try to cast to truck network
        else if (auto *truckNet =
                     dynamic_cast<Backend::TruckClient::
                                      IntegrationNetwork *>(
                         referencedNet))
        {
            checkNetworkAndSetVisibility(
                truckNet->getNetworkName(), mapLine);
        }
    }
}

void CargoNetSim::GUI::ViewController::renameRegion(
    MainWindow *mainWindow, const QString &oldRegionName,
    const QString &newName)
{
    if (!mainWindow || !mainWindow->regionScene_)
    {
        return;
    }

    QGraphicsScene *scene = mainWindow->regionScene_;
    for (QGraphicsItem *item : scene->items())
    {
        if (MapPoint *mapPoint =
                dynamic_cast<MapPoint *>(item))
        {
            if (mapPoint->getRegion() == oldRegionName)
            {
                mapPoint->setRegion(newName);
            }
        }
        else if (MapLine *mapLine =
                     dynamic_cast<MapLine *>(item))
        {
            if (mapLine->getRegion() == oldRegionName)
            {
                mapLine->setRegion(newName);
            }
        }
        else if (RegionCenterPoint *regionCenter =
                     dynamic_cast<RegionCenterPoint *>(
                         item))
        {
            if (regionCenter->getRegion() == oldRegionName)
            {
                regionCenter->setRegion(newName);
            }
        }
        else if (TerminalItem *terminal =
                     dynamic_cast<TerminalItem *>(item))
        {
            if (terminal->getRegion() == oldRegionName)
            {
                terminal->setRegion(newName);
            }
        }
        else if (ConnectionLine *connectionLine =
                     dynamic_cast<ConnectionLine *>(item))
        {
            if (connectionLine->getRegion()
                == oldRegionName)
            {
                connectionLine->setRegion(newName);
            }
        }
        else if (BackgroundPhotoItem *backgroundPhoto =
                     dynamic_cast<BackgroundPhotoItem *>(
                         item))
        {
            if (backgroundPhoto->getRegion()
                == oldRegionName)
            {
                backgroundPhoto->setRegion(newName);
            }
        }
    }
    updateSceneVisibility(mainWindow);
}

void CargoNetSim::GUI::ViewController::changeNetworkColor(
    MainWindow *mainWindow, const QString &networkName,
    const QColor newColor)
{
    if (!mainWindow)
    {
        return;
    }

    QColor newDarkerColor =
        newColor.darker(150); // 150% darker for MapPoints

    auto mapPoints = mainWindow->regionScene_
                         ->getItemsByType<MapPoint>();
    auto mapLines =
        mainWindow->regionScene_->getItemsByType<MapLine>();

    for (auto mapPoint : mapPoints)
    {
        Backend::TrainClient::NeTrainSimNetwork *trainNet =
            dynamic_cast<
                Backend::TrainClient::NeTrainSimNetwork *>(
                mapPoint->getReferenceNetwork());
        Backend::TruckClient::IntegrationNetwork *truckNet =
            dynamic_cast<
                Backend::TruckClient::IntegrationNetwork *>(
                mapPoint->getReferenceNetwork());

        if (trainNet)
        {
            if (trainNet->getNetworkName() == networkName)
            {
                mapPoint->setColor(newDarkerColor);
            }
        }
        else if (truckNet)
        {
            if (truckNet->getNetworkName() == networkName)
            {
                mapPoint->setColor(newDarkerColor);
            }
        }
    }

    for (auto mapLine : mapLines)
    {
        Backend::TrainClient::NeTrainSimNetwork *trainNet =
            dynamic_cast<
                Backend::TrainClient::NeTrainSimNetwork *>(
                mapLine->getReferenceNetwork());
        Backend::TruckClient::IntegrationNetwork *truckNet =
            dynamic_cast<
                Backend::TruckClient::IntegrationNetwork *>(
                mapLine->getReferenceNetwork());

        if (trainNet)
        {
            if (trainNet->getNetworkName() == networkName)
            {
                mapLine->setColor(newColor);
            }
        }
        else if (truckNet)
        {
            if (truckNet->getNetworkName() == networkName)
            {
                mapLine->setColor(newColor);
            }
        }
    }
}

void CargoNetSim::GUI::ViewController::drawTrainNetwork(
    MainWindow                              *mainWindow,
    Backend::TrainClient::NeTrainSimNetwork *network,
    QString &regionName, QColor &linksColor)
{
    mainWindow->regionView_->setUsingProjectedCoords(true);
    mainWindow->updateAllCoordinates();

    // Define node color
    QColor nodesColor = QColor(linksColor);
    nodesColor.setHsv(nodesColor.hue(),
                      nodesColor.saturation(),
                      nodesColor.value() * 0.7);

    // set the network Color
    network->setVariable("color", linksColor);

    // Draw the train network
    for (auto &node : network->getNodes())
    {

        QMap<QString, QVariant> properties = {
            {"Is_terminal", node->isTerminal()},
            {"Dwell_time", node->getDwellTime()},
            {"Description", node->getDescription()}};

        QPointF projectedPoint =
            QPointF(node->getX() * node->getXScale(),
                    node->getY() * node->getYScale());

        MapPoint *point =
            CargoNetSim::GUI::ViewController::drawNode(
                mainWindow,
                QString::number(node->getUserId()),
                node->getInternalUniqueID(), projectedPoint,
                regionName, nodesColor, properties);

        point->setReferenceNetwork(network);

        // Link terminal to point
        if (point && node->isTerminal())
        {
            auto terminal =
                ViewController::createTerminalAtPoint(
                    mainWindow, regionName,
                    "Intermodal Land Terminal",
                    point->getSceneCoordinate());

            point->setLinkedTerminal(terminal);
        }
    }

    // Draw the train network links
    for (auto &link : network->getLinks())
    {
        // Get the source and destination nodes
        auto sourceNode = link->getFromNode();
        auto destNode   = link->getToNode();

        // Create the source and destination points
        QPointF projectedSourcePoint = QPointF(
            sourceNode->getX() * sourceNode->getXScale(),
            sourceNode->getY() * sourceNode->getYScale());

        QPointF projectedDestPoint = QPointF(
            destNode->getX() * destNode->getXScale(),
            destNode->getY() * destNode->getYScale());

        QMap<QString, QVariant> properties = {
            {"Length", link->getLength()},
            {"MaxSpeed",
             link->getMaxSpeed() * link->getSpeedScale()}};

        auto line =
            CargoNetSim::GUI::ViewController::drawLink(
                mainWindow,
                QString::number(link->getUserId()),
                link->getInternalUniqueID(),
                projectedSourcePoint, projectedDestPoint,
                regionName, linksColor, properties);

        line->setReferenceNetwork(network);
    }

    // Fit the view to the scene
    mainWindow->regionView_->fitInView(
        mainWindow->regionScene_->itemsBoundingRect(),
        Qt::KeepAspectRatio);

    mainWindow->showStatusBarMessage(
        QString("Train network imported successfully."));
}

void CargoNetSim::GUI::ViewController::drawTruckNetwork(
    MainWindow *mainWindow,
    Backend::TruckClient::IntegrationSimulationConfig
            *networkConfig,
    QString &regionName, QColor &linksColor)
{
    mainWindow->regionView_->setUsingProjectedCoords(true);
    mainWindow->updateAllCoordinates();

    // Get the network object
    auto network = networkConfig->getNetwork();

    // Define node color
    QColor nodesColor = QColor(linksColor);
    nodesColor.setHsv(nodesColor.hue(),
                      nodesColor.saturation(),
                      nodesColor.value() * 0.7);

    // set the network Color
    network->setVariable("color", linksColor);

    for (auto &node : network->getNodes())
    {

        QMap<QString, QVariant> properties = {
            {"Description", node->getDescription()}};

        auto point =
            CargoNetSim::GUI::ViewController::drawNode(
                mainWindow,
                QString::number(node->getNodeId()),
                node->getInternalUniqueID(),
                QPointF(node->getXCoordinate()
                            * node->getXScale()
                            * 1000.0, // km to m
                        node->getYCoordinate()
                            * node->getYScale()
                            * 1000.0), // km to m
                regionName, nodesColor, properties);

        point->setReferenceNetwork(network);
    }

    for (auto &link : network->getLinks())
    {
        QMap<QString, QVariant> properties = {
            {"ReferenceNetworkID", link->getLinkId()},
            {"Length", link->getLength()
                           * link->getLengthScale()
                           * 1000.0}, // km to m
            {"FreeFlowTime",
             link->getFreeSpeed() * link->getSpeedScale()},
            {"NoOfLanes", link->getLanes()}};

        // Get the source and destination nodes
        auto to =
            network->getNode(link->getDownstreamNodeId());
        auto from =
            network->getNode(link->getUpstreamNodeId());
        if (!to || !from)
        {
            continue;
        }

        // Create the source and destination projected
        // points
        QPointF projectedSourcePoint = QPointF(
            from->getXCoordinate() * from->getXScale()
                * 1000.0, // km to m
            from->getYCoordinate() * from->getYScale()
                * 1000.0); // km to m
        QPointF projectedDestPoint =
            QPointF(to->getXCoordinate() * to->getXScale()
                        * 1000.0, // km to m
                    to->getYCoordinate() * to->getYScale()
                        * 1000.0); // km to m

        auto line =
            CargoNetSim::GUI::ViewController::drawLink(
                mainWindow,
                QString::number(link->getLinkId()),
                link->getInternalUniqueID(),
                projectedSourcePoint, projectedDestPoint,
                regionName, linksColor, properties);

        line->setReferenceNetwork(network);
    }

    // Fit the view to the scene
    mainWindow->regionView_->fitInView(
        mainWindow->regionScene_->itemsBoundingRect(),
        Qt::KeepAspectRatio);

    mainWindow->showStatusBarMessage(
        QString("Truck network imported successfully."));
}

CargoNetSim::GUI::MapPoint *
CargoNetSim::GUI::ViewController::drawNode(
    MainWindow *mainWindow, const QString &networkNodeID,
    const QString &nodeUniqueID, QPointF projectedPoint,
    QString &regionName, QColor color,
    const QMap<QString, QVariant> &properties)
{

    QPointF geodeticPoint =
        mainWindow->regionView_->convertCoordinates(
            projectedPoint, "to_geodetic");
    QPointF scenePoint =
        mainWindow->regionView_->wgs84ToScene(
            geodeticPoint);

    // Create projected coordinate point
    MapPoint *point =
        new MapPoint(networkNodeID, scenePoint, regionName,
                     "circle", nullptr, properties);

    QObject::connect(
        point, &MapPoint::clicked,
        [mainWindow](MapPoint *point) {
            UtilitiesFunctions::updatePropertiesPanel(
                mainWindow, point);
        });

    QObject::connect(
        point, &MapPoint::clicked, mainWindow,
        &MainWindow::handleTerminalNodeLinking);

    QObject::connect(
        point, &MapPoint::clicked, mainWindow,
        &MainWindow::handleTerminalNodeUnlinking);

    point->setProperty("NodeID", nodeUniqueID);
    point->setColor(color); // Set node color

    mainWindow->regionScene_->addItemWithId(
        point, nodeUniqueID); // Add node to scene

    return point;
}

CargoNetSim::GUI::MapLine *
CargoNetSim::GUI::ViewController::drawLink(
    MainWindow *mainWindow, const QString &networkNodeID,
    const QString &linkUniqueID,
    QPointF projectedStartPoint, QPointF projectedEndPoint,
    QString &regionName, QColor color,
    const QMap<QString, QVariant> &properties)
{
    MapLine *line = nullptr;
    try
    {
        // Convert the points to geodetic coordinates
        QPointF sourceGeodetic =
            mainWindow->regionView_->convertCoordinates(
                projectedStartPoint, "to_geodetic");
        QPointF destGeodetic =
            mainWindow->regionView_->convertCoordinates(
                projectedEndPoint, "to_geodetic");
        QPointF sourceScenePoint =
            mainWindow->regionView_->wgs84ToScene(
                sourceGeodetic);
        QPointF destScenePoint =
            mainWindow->regionView_->wgs84ToScene(
                destGeodetic);

        // Create the link
        line = new MapLine(networkNodeID, sourceScenePoint,
                           destScenePoint, regionName,
                           properties);

        QObject::connect(
            line, &MapLine::clicked,
            [mainWindow](MapLine *line) {
                UtilitiesFunctions::updatePropertiesPanel(
                    mainWindow, line);
            });

        line->setProperty("LinkID", linkUniqueID);

        line->setColor(color); // Set link color

        mainWindow->regionScene_->addItemWithId(
            line, linkUniqueID); // Add link to scene
    }
    catch (const std::exception &e)
    {
        qWarning() << "Error in drawLink:" << e.what();

        QMessageBox::warning(
            mainWindow, "Error",
            QString("Failed to draw link: %1")
                .arg(e.what()));
    }

    return line;
}

void CargoNetSim::GUI::ViewController::removeNetwork(
    MainWindow *mainWindow, NetworkType networkType,
    Backend::RegionData *regionData, QString &networkName)
{
    QString regionName = regionData->getRegion();

    if (networkType == NetworkType::Train)
    {
        Backend::TrainClient::NeTrainSimNetwork *network =
            regionData->getTrainNetwork(networkName);
        if (!network)
        {
            return;
        }
        for (auto &node : network->getNodes())
        {
            if (!node)
            {
                continue;
            }
            // Get the point
            MapPoint *point =
                mainWindow->regionScene_
                    ->getItemById<MapPoint>(
                        node->getInternalUniqueID());
            // Check if we got the correct point
            if (!point)
            {
                continue;
            }

            // Get the associated terminal of the point
            TerminalItem *terminal =
                point->getLinkedTerminal();

            // Set it to not appear on global map
            point->setProperty("Show on Global Map", false);
            // Update the Global Map to delete the global
            // terminal item from the global map
            CargoNetSim::GUI::ViewController::
                updateGlobalMapItem(mainWindow, terminal);

            // Remove the item
            mainWindow->regionScene_
                ->removeItemWithId<MapPoint>(
                    node->getInternalUniqueID());
        }
        for (auto &link : network->getLinks())
        {
            if (!link)
            {
                continue;
            }
            mainWindow->regionScene_
                ->removeItemWithId<MapLine>(
                    link->getInternalUniqueID());
        }
    }
    else if (networkType == NetworkType::Truck)
    {
        Backend::TruckClient::IntegrationNetwork *network =
            regionData->getTruckNetwork(networkName);
        for (auto &node : network->getNodes())
        {
            // Get the point
            MapPoint *point =
                mainWindow->regionScene_
                    ->getItemById<MapPoint>(
                        node->getInternalUniqueID());

            // Get the associated terminal of the point
            TerminalItem *terminal =
                point->getLinkedTerminal();

            // Set it to not appear on global map
            point->setProperty("Show on Global Map", false);
            // Update the Global Map to delete the global
            // terminal item from the global map
            CargoNetSim::GUI::ViewController::
                updateGlobalMapItem(mainWindow, terminal);

            mainWindow->regionScene_
                ->removeItemWithId<MapPoint>(
                    node->getInternalUniqueID());
        }
        for (auto &link : network->getLinks())
        {
            mainWindow->regionScene_
                ->removeItemWithId<MapLine>(
                    link->getInternalUniqueID());
        }
    }
}

void CargoNetSim::GUI::ViewController::addBackgroundPhoto(
    CargoNetSim::GUI::MainWindow *mainWindow)
{
    try
    {
        // Open file dialog (using non-native dialog for
        // consistency)
        QString fileName = QFileDialog::getOpenFileName(
            nullptr, "Select Background Photo", "",
            "Images (*.png *.jpg *.bmp)", nullptr,
            QFileDialog::DontUseNativeDialog);

        if (fileName.isEmpty())
        {
            return;
        }

        QPixmap pixmap(fileName);
        if (pixmap.isNull())
        {
            QMessageBox::warning(mainWindow, "Error",
                                 "Failed to load image.");
            return;
        }

        // Check which tab is currently active
        if (mainWindow->tabWidget_->currentWidget()
            == mainWindow->tabWidget_->widget(0))
        { // Main view tab
            // Create a new BackgroundPhotoItem for the main
            // view
            QString currentRegion =
                CargoNetSim::CargoNetSimController::
                    getInstance()
                        .getRegionDataController()
                        ->getCurrentRegion();
            BackgroundPhotoItem *background =
                new BackgroundPhotoItem(pixmap,
                                        currentRegion);
            QObject::connect(
                background, &BackgroundPhotoItem::clicked,
                [mainWindow](BackgroundPhotoItem *item) {
                    UtilitiesFunctions::
                        updatePropertiesPanel(mainWindow,
                                              item);
                });
            QObject::connect(
                background,
                &BackgroundPhotoItem::positionChanged,
                [background,
                 mainWindow](const QPointF &pos) {
                    if (mainWindow->propertiesPanel_
                            ->getCurrentItem()
                        == background)
                    {
                        mainWindow->propertiesPanel_
                            ->updatePositionFields(pos);
                    }
                });

            // Place the photo at the center of the main
            // view
            QPointF viewCenter =
                mainWindow->regionView_->mapToScene(
                    mainWindow->regionView_->viewport()
                        ->rect()
                        .center());

            double lat, lon;
            auto   wgsPoint =
                mainWindow->regionView_->sceneToWGS84(
                    viewCenter);
            lat = wgsPoint.x();
            lon = wgsPoint.y();
            background->getProperties()["Latitude"] =
                QString::number(lat, 'f', 6);
            background->getProperties()["Longitude"] =
                QString::number(lon, 'f', 6);
            background->setPos(viewCenter);

            mainWindow->regionScene_->addItemWithId(
                background, background->getID());

            CargoNetSim::CargoNetSimController::
                getInstance()
                    .getRegionDataController()
                    ->setRegionVariable(
                        currentRegion,
                        "backgroundPhotoItem",
                        QVariant::fromValue(background));
        }
        else
        { // Global map tab
            // Create a new BackgroundPhotoItem for the
            // global map
            BackgroundPhotoItem *background =
                new BackgroundPhotoItem(pixmap, "global");
            QObject::connect(
                background, &BackgroundPhotoItem::clicked,
                [mainWindow](BackgroundPhotoItem *item) {
                    UtilitiesFunctions::
                        updatePropertiesPanel(mainWindow,
                                              item);
                });
            QObject::connect(
                background,
                &BackgroundPhotoItem::positionChanged,
                [background,
                 &mainWindow](const QPointF &pos) {
                    if (mainWindow->propertiesPanel_
                            ->getCurrentItem()
                        == background)
                    {
                        mainWindow->propertiesPanel_
                            ->updatePositionFields(pos);
                    }
                });

            // Place the photo at the center of the global
            // map view
            QPointF viewCenter =
                mainWindow->globalMapView_->mapToScene(
                    mainWindow->globalMapView_->viewport()
                        ->rect()
                        .center());

            double lat, lon;
            auto   wgsPoint =
                mainWindow->regionView_->sceneToWGS84(
                    viewCenter);
            lon = wgsPoint.x();
            lat = wgsPoint.y();
            background->getProperties()["Latitude"] =
                QString::number(lat, 'f', 6);
            background->getProperties()["Longitude"] =
                QString::number(lon, 'f', 6);
            background->setPos(viewCenter);

            mainWindow->globalMapScene_->addItemWithId(
                background, background->getID());
            CargoNetSim::CargoNetSimController::
                getInstance()
                    .getRegionDataController()
                    ->setGlobalVariable(
                        "globalBackgroundPhotoItem",
                        QVariant::fromValue(background));
        }
    }
    catch (const std::exception &e)
    {
        qWarning() << "Error in addBackgroundPhoto:"
                   << e.what();
        QMessageBox::warning(
            mainWindow, "Error",
            QString("Failed to add background photo: %1")
                .arg(e.what()));
    }
}

bool CargoNetSim::GUI::ViewController::
    checkExistingConnection(MainWindow    *mainWindow,
                            QGraphicsItem *startItem,
                            QGraphicsItem *endItem,
                            const QString &connectionType)
{
    // Early validation check
    if (!startItem || !endItem || !mainWindow)
    {
        return false;
    }

    // Determine which scene to use based on item types
    QList<ConnectionLine *> viewConnectionLines;

    const bool isRegionStart =
        dynamic_cast<TerminalItem *>(startItem) != nullptr;
    const bool isRegionEnd =
        dynamic_cast<TerminalItem *>(endItem) != nullptr;

    if (isRegionStart && isRegionEnd)
    {
        // Both are region items
        viewConnectionLines =
            mainWindow->regionScene_
                ->getItemsByType<ConnectionLine>();
    }
    else
    {
        const bool isGlobalStart =
            dynamic_cast<GlobalTerminalItem *>(startItem)
            != nullptr;
        const bool isGlobalEnd =
            dynamic_cast<GlobalTerminalItem *>(endItem)
            != nullptr;

        if (isGlobalStart && isGlobalEnd)
        {
            // Both are global items
            viewConnectionLines =
                mainWindow->globalMapView_->getScene()
                    ->getItemsByType<ConnectionLine>();
        }
        else
        {
            // Mixed types or unrecognized types
            return false;
        }
    }

    if (viewConnectionLines.empty())
    {
        return false;
    }

    // Look for matching connection
    for (const ConnectionLine *line : viewConnectionLines)
    {
        // Check connection type first as it's likely faster
        // than pointer comparison
        if (line->connectionType() != connectionType)
        {
            continue;
        }

        // Then check if terminals match (in either
        // direction)
        if ((line->startItem() == startItem
             && line->endItem() == endItem)
            || (line->startItem() == endItem
                && line->endItem() == startItem))
        {
            return true;
        }
    }

    return false;
}

CargoNetSim::GUI::ConnectionLine *
CargoNetSim::GUI::ViewController::createConnectionLine(
    MainWindow *mainWindow, QGraphicsItem *startItem,
    QGraphicsItem *endItem, const QString &connectionType)
{
    if (!mainWindow || !startItem || !endItem)
    {
        return nullptr; // Early validation check
    }

    // Check if a connection of the same type already
    // exists between the terminals
    if (CargoNetSim::GUI::ViewController::
            checkExistingConnection(mainWindow, startItem,
                                    endItem,
                                    connectionType))
    {
        mainWindow->showStatusBarError(
            "A connection of this type already exists.",
            3000);
        return nullptr;
    }

    auto SP = dynamic_cast<TerminalItem *>(startItem);
    auto EP = dynamic_cast<TerminalItem *>(endItem);

    if (SP && EP && SP->getRegion() == EP->getRegion())
    {
        auto line = new ConnectionLine(
            SP, EP, connectionType, {}, SP->getRegion());
        mainWindow->regionScene_->addItemWithId(
            line, line->getID());

        // Connect the clicked signal to update properties
        // panel
        QObject::connect(
            line, &ConnectionLine::clicked,
            [mainWindow](ConnectionLine *line) {
                UtilitiesFunctions::updatePropertiesPanel(
                    mainWindow, line);
            });

        return line;
    }
    else if (SP && EP && SP->getRegion() != EP->getRegion())
    {
        mainWindow->showStatusBarError(
            "Cannot create a connection between two "
            "different regions in region view.",
            3000);
        return nullptr;
    }
    else if (!SP && !EP)
    {
        auto SPG =
            dynamic_cast<GlobalTerminalItem *>(startItem);
        auto EPG =
            dynamic_cast<GlobalTerminalItem *>(endItem);

        if (SPG && EPG && SPG != EPG)
        {
            if (SPG->getLinkedTerminalItem()->getRegion()
                == EPG->getLinkedTerminalItem()
                       ->getRegion())
            {
                mainWindow->showStatusBarError(
                    "Cannot link terminals in the same "
                    "region in global map.",
                    3000);
                return nullptr;
            }

            // Create the connection line
            auto line = new ConnectionLine(
                SPG, EPG, connectionType, {}, "Global");
            mainWindow->globalMapView_->getScene()
                ->addItemWithId(line, line->getID());

            // Connect the clicked signal to update
            // properties panel
            QObject::connect(
                line, &ConnectionLine::clicked,
                [mainWindow](ConnectionLine *line) {
                    UtilitiesFunctions::
                        updatePropertiesPanel(mainWindow,
                                              line);
                });

            return line;
        }
        else if (SPG && EPG && SPG == EPG)
        {
            mainWindow->showStatusBarError(
                "Cannot link a terminal to itself.", 3000);
        }
    }

    return nullptr;
}

void CargoNetSim::GUI::ViewController::
    connectVisibleTerminalsByNetworks(
        MainWindow *mainWindow)
{
    bool isGlobalView = mainWindow->isGlobalViewActive();
    GraphicsScene *currentScene =
        isGlobalView ? mainWindow->globalMapScene_
                     : mainWindow->regionScene_;

    QString                     currentRegion = "";
    QList<TerminalItem *>       terminals;
    QList<GlobalTerminalItem *> globalTerminals;
    bool anyConnectionCreated = false;

    TerminalItem *originTerminal =
        UtilitiesFunctions::getOriginTerminal(mainWindow);
    if (!originTerminal)
    {
        mainWindow->showStatusBarError(
            "Origin is not present in the region view!",
            3000);
        return;
    }

    QVariant containersVar =
        originTerminal->getProperty("Containers");
    if (containersVar.canConvert<
            QList<ContainerCore::Container *>>())
    {
        QList<ContainerCore::Container *> containers =
            containersVar
                .value<QList<ContainerCore::Container *>>();
        if (containers.empty())
        {
            mainWindow->showStatusBarError(
                "No containers at origin!", 3000);
        }
    }

    if (isGlobalView)
    {
        globalTerminals = CargoNetSim::GUI::
            UtilitiesFunctions::getGlobalTerminalItems(
                mainWindow->globalMapScene_, "*", "*",
                UtilitiesFunctions::ConnectionType::Any,
                UtilitiesFunctions::LinkType::Any);
    }
    else
    {

        currentRegion = CargoNetSim::CargoNetSimController::
                            getInstance()
                                .getRegionDataController()
                                ->getCurrentRegion();
        terminals = CargoNetSim::GUI::UtilitiesFunctions::
            getTerminalItems(
                mainWindow->regionScene_, currentRegion,
                "*",
                UtilitiesFunctions::ConnectionType::Any,
                UtilitiesFunctions::LinkType::Any);
    }

    if ((terminals.empty() && !isGlobalView)
        || (globalTerminals.empty() && isGlobalView))
    {
        QString msgHndler =
            isGlobalView ? "view" : "region";
        QString mssg = QString("There is no terminal "
                               "in the current %1")
                           .arg(msgHndler);
        mainWindow->showStatusBarError(mssg, 3000);
        return;
    }
    else if ((terminals.size() == 1 && !isGlobalView)
             || (globalTerminals.size() == 1
                 && isGlobalView))
    {
        QString msgHndler =
            isGlobalView ? "view" : "region";
        QString mssg = QString("There is only one terminal "
                               "in the current %1.")
                           .arg(msgHndler);
        mainWindow->showStatusBarError(mssg, 3000);
        return;
    }

    // Connect terminals based on common networks in the
    // region view
    for (auto &sourceTerminal : terminals)
    {
        for (auto &targetTerminal : terminals)
        {
            if (sourceTerminal == targetTerminal)
            {
                continue;
            }

            QList<QString> commonModes =
                UtilitiesFunctions::getCommonModes(
                    sourceTerminal, targetTerminal);

            // Process Rail connections in region view
            if (commonModes.contains("Rail"))
            {
                bool isConnected = UtilitiesFunctions::
                    processNetworkModeConnection(
                        mainWindow, sourceTerminal,
                        targetTerminal, NetworkType::Train);
                if (isConnected)
                {
                    anyConnectionCreated = true;
                }
            }

            // Process Truck connections in region view
            if (commonModes.contains("Truck")
                && !isGlobalView)
            {
                bool isConnected = UtilitiesFunctions::
                    processNetworkModeConnection(
                        mainWindow, sourceTerminal,
                        targetTerminal, NetworkType::Truck);
                if (isConnected)
                {
                    anyConnectionCreated = true;
                }
            }

            // Process Ship connections (if needed)
            if (commonModes.contains("Ship"))
            {
                // TODO
                // Similar implementation as Rail and Truck
                // Skip for now so we do not allow skips
                // connections between ports in the same
                // region
            }
        }
    }

    // Connect terminals based on common networks in the
    // global view
    for (auto &sourceTerminal : globalTerminals)
    {
        for (auto &targetTerminal : globalTerminals)
        {
            if (sourceTerminal == targetTerminal)
            {
                continue;
            }

            QList<QString> commonModes =
                UtilitiesFunctions::getCommonModes(
                    sourceTerminal, targetTerminal);
            for (const QString &mode : commonModes)
            {

                QString connectionType;
                if (mode.toLower() == "ship")
                    connectionType = "Ship";

                if (!connectionType.isEmpty())
                {
                    auto connectionLine = ViewController::
                        createConnectionLine(
                            mainWindow, sourceTerminal,
                            targetTerminal, connectionType);
                    if (connectionLine)
                    {
                        // Set connection properties
                        // calculate them on the fly as if
                        // there is a shortest path between
                        // the terminals
                        CargoNetSim::Backend::
                            ShortestPathResult result;

                        // Get geographic coordinates for
                        // both terminals
                        QPointF sourceGeoPoint =
                            mainWindow->globalMapView_
                                ->sceneToWGS84(
                                    sourceTerminal->pos());
                        QPointF targetGeoPoint =
                            mainWindow->globalMapView_
                                ->sceneToWGS84(
                                    targetTerminal->pos());

                        // Calculate the distance using
                        // geographic coordinates
                        result.totalLength =
                            UtilitiesFunctions::
                                getApproximateGeoDistance(
                                    sourceGeoPoint,
                                    targetGeoPoint);
                        result.optimizationCriterion =
                            "distance";

                        NetworkType networkType =
                            NetworkType::Ship;

                        UtilitiesFunctions::
                            setConnectionProperties(
                                connectionLine, result,
                                networkType);

                        anyConnectionCreated = true;
                    }
                }
            }
        }
    }

    if (anyConnectionCreated)
    {
        mainWindow->showStatusBarMessage(
            "Terminal connections created based on "
            "networks.");
    }
}

void CargoNetSim::GUI::ViewController::
    connectVisibleTerminalsByInterfaces(
        MainWindow *mainWindow)
{
    if (!mainWindow)
    {
        return;
    }

    // Get the current view and scene based on which tab is
    // active
    bool isGlobalView =
        mainWindow->tabWidget_->currentIndex() == 0 ? false
                                                    : true;
    GraphicsScene *currentScene =
        isGlobalView ? mainWindow->globalMapScene_
                     : mainWindow->regionScene_;

    // Get all visible terminals in the current view
    QList<QGraphicsItem *> visibleTerminals;

    // Track which terminal types are present in the current
    // view
    QSet<QString> visibleTerminalTypes;

    if (isGlobalView)
    {
        QList<GlobalTerminalItem *> allTerminals =
            currentScene
                ->getItemsByType<GlobalTerminalItem>();
        // Filter visible GlobalTerminalItems
        for (auto terminal : allTerminals)
        {
            if (terminal && terminal->isVisible())
            {
                visibleTerminals.append(terminal);

                // Get the terminal type
                TerminalItem *linkedTerminal =
                    terminal->getLinkedTerminalItem();
                if (linkedTerminal)
                {
                    visibleTerminalTypes.insert(
                        linkedTerminal->getTerminalType());
                }
            }
        }
    }
    else
    {
        QList<TerminalItem *> allTerminals =
            currentScene->getItemsByType<TerminalItem>();
        // Filter visible TerminalItems in current region
        QString currentRegion =
            CargoNetSim::CargoNetSimController::
                getInstance()
                    .getRegionDataController()
                    ->getCurrentRegion();

        for (auto terminal : allTerminals)
        {
            if (terminal && terminal->isVisible()
                && terminal->getRegion() == currentRegion)
            {
                visibleTerminals.append(terminal);
                visibleTerminalTypes.insert(
                    terminal->getTerminalType());
            }
        }
    }

    if (visibleTerminals.empty())
    {
        mainWindow->showStatusBarError(
            "No visible terminals found in the current "
            "view.",
            3000);
        return;
    }

    // Find all available common interfaces
    QSet<QString> availableInterfaces;
    for (int i = 0; i < visibleTerminals.size(); ++i)
    {
        for (int j = i + 1; j < visibleTerminals.size();
             ++j)
        {
            // Get the source and target terminals
            QGraphicsItem *sourceItem = visibleTerminals[i];
            QGraphicsItem *targetItem = visibleTerminals[j];

            // Get common modes
            QList<QString> commonModes =
                UtilitiesFunctions::getCommonModes(
                    sourceItem, targetItem);
            for (const QString &mode : commonModes)
            {
                if (!mode.isEmpty())
                {
                    availableInterfaces.insert(mode);
                }
            }
        }
    }

    // If no common interfaces, show error and return
    if (availableInterfaces.isEmpty())
    {
        mainWindow->showStatusBarError(
            "No common interfaces found between terminals.",
            3000);
        return;
    }

    // Show interface selection dialog with only visible
    // terminal types
    InterfaceSelectionDialog dialog(availableInterfaces,
                                    visibleTerminalTypes,
                                    mainWindow);
    if (dialog.exec() != QDialog::Accepted)
    {
        return; // User cancelled
    }

    // Get selected interfaces
    QList<QString> selectedInterfaces =
        dialog.getSelectedInterfaces();

    // Get included terminal types
    QMap<QString, bool> includedTerminalTypes =
        dialog.getIncludedTerminalTypes();

    if (selectedInterfaces.isEmpty())
    {
        mainWindow->showStatusBarMessage(
            "No interfaces selected for connection.", 3000);
        return;
    }

    // Check if any terminal types are selected
    bool anyTerminalTypeSelected = false;
    for (auto it = includedTerminalTypes.begin();
         it != includedTerminalTypes.end(); ++it)
    {
        if (it.value())
        {
            anyTerminalTypeSelected = true;
            break;
        }
    }

    if (!anyTerminalTypeSelected)
    {
        mainWindow->showStatusBarMessage(
            "No terminal types selected for connection.",
            3000);
        return;
    }

    // Connect terminals based on selected interfaces
    int connectionsCreated = 0;
    for (int i = 0; i < visibleTerminals.size(); ++i)
    {
        for (int j = i + 1; j < visibleTerminals.size();
             ++j)
        {
            // Skip if the same terminal
            if (i == j)
            {
                continue;
            }

            // Get the source and target terminals
            QGraphicsItem *sourceItem = visibleTerminals[i];
            QGraphicsItem *targetItem = visibleTerminals[j];

            QList<QString> commonModes =
                UtilitiesFunctions::getCommonModes(
                    sourceItem, targetItem);

            // Check if the terminal types are included
            bool    skipConnection = false;
            QString sourceType;
            QString targetType;

            // Get source terminal type
            if (TerminalItem *source =
                    qgraphicsitem_cast<TerminalItem *>(
                        sourceItem))
            {
                sourceType = source->getTerminalType();
            }
            else if (GlobalTerminalItem *globalSource =
                         qgraphicsitem_cast<
                             GlobalTerminalItem *>(
                             sourceItem))
            {
                TerminalItem *linkedSource =
                    globalSource->getLinkedTerminalItem();
                if (linkedSource)
                {
                    sourceType =
                        linkedSource->getTerminalType();
                }
            }

            // Get target terminal type
            if (TerminalItem *target =
                    qgraphicsitem_cast<TerminalItem *>(
                        targetItem))
            {
                targetType = target->getTerminalType();
            }
            else if (GlobalTerminalItem *globalTarget =
                         qgraphicsitem_cast<
                             GlobalTerminalItem *>(
                             targetItem))
            {
                TerminalItem *linkedTarget =
                    globalTarget->getLinkedTerminalItem();
                if (linkedTarget)
                {
                    targetType =
                        linkedTarget->getTerminalType();
                }
            }

            // Skip if either terminal type is not included
            if ((!sourceType.isEmpty()
                 && !includedTerminalTypes.value(sourceType,
                                                 true))
                || (!targetType.isEmpty()
                    && !includedTerminalTypes.value(
                        targetType, true)))
            {
                skipConnection = true;
            }

            if (!skipConnection)
            {
                // Create connections for each selected
                // common mode
                for (const QString &mode : commonModes)
                {
                    if (!mode.isEmpty()
                        && selectedInterfaces.contains(
                            mode))
                    {
                        ConnectionLine *connection =
                            ViewController::
                                createConnectionLine(
                                    mainWindow, sourceItem,
                                    targetItem, mode);
                        if (connection)
                        {
                            connectionsCreated++;
                        }
                    }
                }
            }
        }
    }

    if (connectionsCreated > 0)
    {
        mainWindow->showStatusBarMessage(
            QString("Created %1 terminal connections based "
                    "on selected interfaces.")
                .arg(connectionsCreated),
            3000);
    }
    else
    {
        mainWindow->showStatusBarMessage(
            "No new connections were created.", 3000);
    }
}

CargoNetSim::GUI::RegionCenterPoint *
CargoNetSim::GUI::ViewController::createRegionCenter(
    MainWindow *mainWindow, const QString &regionName,
    const QColor &color, const QPointF pos,
    const bool keepVisible)
{
    RegionCenterPoint *centerPoint =
        new RegionCenterPoint(regionName, color);
    QObject::connect(
        centerPoint, &RegionCenterPoint::clicked,
        [mainWindow](RegionCenterPoint *item) {
            UtilitiesFunctions::updatePropertiesPanel(
                mainWindow, item);
        });

    // Add position change connection
    QObject::connect(
        centerPoint, &RegionCenterPoint::coordinatesChanged,
        [regionName, mainWindow](QPointF newGeopoint) {
            PropertiesPanel *propertiesPanel =
                mainWindow->propertiesPanel_;
            UtilitiesFunctions::updateGlobalMapForRegion(
                mainWindow, regionName);
            propertiesPanel->updateCoordinateFields(
                newGeopoint);
        });

    QObject::connect(
        centerPoint, &RegionCenterPoint::propertiesChanged,
        [regionName, mainWindow]() {
            UtilitiesFunctions::updateGlobalMapForRegion(
                mainWindow, regionName);
        });

    centerPoint->setPos(pos);
    mainWindow->regionScene_->addItemWithId(
        centerPoint, centerPoint->getID());
    CargoNetSim::CargoNetSimController::getInstance()
        .getRegionDataController()
        ->setRegionVariable(
            regionName, "regionCenterPoint",
            QVariant::fromValue(centerPoint));

    // update visibility
    centerPoint->setVisible(keepVisible);
    return centerPoint;
}
