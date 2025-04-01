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
    auto props = terminal->getProperties();
    bool show =
        props.contains("Show on Global Map")
            ? props.value("Show on Global Map").toBool()
            : true;

    if (show)
    {
        auto regionCenterPoint =
            CargoNetSim::CargoNetSimController::
                getInstance()
                    .getRegionDataController()
                    ->getRegionData(terminal->getRegion())
                    ->getVariableAs<RegionCenterPoint *>(
                        "regionCenterPoint", nullptr);
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
        }
    }
    else
    {
        // Remove the global terminal item
        if (terminal->getGlobalTerminalItem())
        {
            main_window->globalMapView_->getScene()
                ->removeItemWithId<GlobalTerminalItem>(
                    terminal->getGlobalTerminalItem()
                        ->getID());
            terminal->getGlobalTerminalItem()
                ->deleteLater();
            terminal->setGlobalTerminalItem(nullptr);
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
    globalITem->setPos(
        main_window->globalMapView_->wgs84ToScene(QPointF(
            item_global_view_lon, item_global_view_lat)));
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

void CargoNetSim::GUI::ViewController::drawTrainNetwork(
    MainWindow                              *mainWindow,
    Backend::TrainClient::NeTrainSimNetwork *network,
    QString &regionName, QColor &linksColor)
{
    // Define node color
    QColor nodesColor = QColor(linksColor);
    nodesColor.setHsv(nodesColor.hue(),
                      nodesColor.saturation(),
                      nodesColor.value() * 0.7);

    // Draw the train network
    for (auto &node : network->getNodes())
    {

        QMap<QString, QVariant> properties = {
            {"Is_terminal", node->isTerminal()},
            {"Dwell_time", node->getDwellTime()},
            {"Description", node->getDescription()}};

        MapPoint *point =
            CargoNetSim::GUI::ViewController::drawNode(
                mainWindow, node->getInternalUniqueID(),
                QPointF(node->getX() * node->getXScale(),
                        node->getY() * node->getYScale()),
                regionName, nodesColor, properties);

        point->setReferenceNetwork(network);

        // Link terminal to point
        if (point && node->isTerminal())
        {
            auto terminal =
                ViewController::createTerminalAtPoint(
                    mainWindow, regionName,
                    "Intermodal Land Terminal",
                    QPointF(point->getX(), point->getY()));

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
        QPointF sourcePoint =
            QPointF(sourceNode->getX(), sourceNode->getY());
        QPointF destPoint =
            QPointF(destNode->getX(), destNode->getY());

        QMap<QString, QVariant> properties = {
            {"Length", link->getLength()},
            {"MaxSpeed",
             link->getMaxSpeed() * link->getSpeedScale()}};

        auto line =
            CargoNetSim::GUI::ViewController::drawLink(
                mainWindow, link->getInternalUniqueID(),
                sourcePoint, destPoint, regionName,
                linksColor, properties);

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
    // Define node color
    QColor nodesColor = QColor(linksColor);
    nodesColor.setHsv(nodesColor.hue(),
                      nodesColor.saturation(),
                      nodesColor.value() * 0.7);

    for (auto &node :
         networkConfig->getNetwork()->getNodes())
    {

        QMap<QString, QVariant> properties = {
            {"Description", node->getDescription()}};

        auto point =
            CargoNetSim::GUI::ViewController::drawNode(
                mainWindow, node->getInternalUniqueID(),
                QPointF(node->getXCoordinate()
                            * node->getXScale(),
                        node->getYCoordinate()
                            * node->getYScale()),
                regionName, nodesColor, properties);

        point->setReferenceNetwork(
            networkConfig->getNetwork());
    }

    for (auto &link :
         networkConfig->getNetwork()->getLinks())
    {
        QMap<QString, QVariant> properties = {
            {"Length", link->getLength()
                           * link->getLengthScale()
                           * 1000.0}, // km to m
            {"FreeFlowTime",
             link->getFreeSpeed() * link->getSpeedScale()},
            {"NoOfLanes", link->getLanes()}};

        auto from = networkConfig->getNetwork()->getNode(
            link->getDownstreamNodeId());
        auto to = networkConfig->getNetwork()->getNode(
            link->getUpstreamNodeId());
        auto line =
            CargoNetSim::GUI::ViewController::drawLink(
                mainWindow, link->getInternalUniqueID(),
                QPointF(from->getXCoordinate()
                            * from->getXScale(),
                        from->getYCoordinate()
                            * from->getYScale()),
                QPointF(
                    to->getXCoordinate() * to->getXScale(),
                    to->getYCoordinate() * to->getYScale()),
                regionName, linksColor, properties);

        line->setReferenceNetwork(
            networkConfig->getNetwork());
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
    MainWindow *mainWindow, const QString nodeID,
    QPointF projectedPoint, QString &regionName,
    QColor color, const QMap<QString, QVariant> &properties)
{

    QPointF geodeticPoint =
        mainWindow->regionView_->convertCoordinates(
            projectedPoint, "to_geodetic");

    // Create projected coordinate point
    MapPoint *point = new MapPoint(
        nodeID, geodeticPoint.x(), geodeticPoint.y(),
        regionName, "circle", nullptr, properties);

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

    point->setColor(color); // Set node color

    mainWindow->regionScene_->addItemWithId(
        point, point->getID()); // Add node to scene

    return point;
}

CargoNetSim::GUI::MapLine *
CargoNetSim::GUI::ViewController::drawLink(
    MainWindow *mainWindow, const QString &linkID,
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

        // Create the link
        line = new MapLine(
            QPointF(sourceGeodetic.x(), sourceGeodetic.y()),
            QPointF(destGeodetic.x(), destGeodetic.y()),
            regionName, properties);

        QObject::connect(
            line, &MapLine::clicked,
            [mainWindow](MapLine *line) {
                UtilitiesFunctions::updatePropertiesPanel(
                    mainWindow, line);
            });

        line->setColor(color); // Set link color

        mainWindow->regionScene_->addItemWithId(
            line, line->getID()); // Add link to scene
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

            // Remove the item
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
        auto line =
            new ConnectionLine(SP, EP, connectionType);
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
            auto line = new ConnectionLine(SPG, EPG,
                                           connectionType);
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
        || (terminals.empty() && isGlobalView))
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
             || (terminals.size() == 1 && isGlobalView))
    {
        QString msgHndler =
            isGlobalView ? "view" : "region";
        QString mssg = QString("There is only one terminal "
                               "in the current %1.")
                           .arg(msgHndler);
        mainWindow->showStatusBarError(mssg, 3000);
        return;
    }

    // Connect terminals based on common networks
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

            if (commonModes.contains("Rail"))
            {
                auto sourceTerminalObj =
                    dynamic_cast<TerminalItem *>(
                        sourceTerminal);
                auto targetTerminalObj =
                    dynamic_cast<TerminalItem *>(
                        targetTerminal);
                auto sourceMapPoints = UtilitiesFunctions::
                    getMapPointsOfTerminal(
                        mainWindow->regionScene_,
                        sourceTerminalObj, currentRegion,
                        "*", NetworkType::Train);

                auto tqrgetMapPoints = UtilitiesFunctions::
                    getMapPointsOfTerminal(
                        mainWindow->regionScene_,
                        targetTerminalObj, currentRegion,
                        "*", NetworkType::Train);

                bool keepProcessing = true;
                if (sourceMapPoints.empty())
                {
                    mainWindow->showStatusBarError(
                        QString("Terminal %1 has no "
                                "associated nodes.")
                            .arg(sourceTerminalObj
                                     ->getProperty("Name",
                                                   "")
                                     .toString()),
                        3000);
                    keepProcessing = false;
                }

                if (tqrgetMapPoints.empty())
                {
                    mainWindow->showStatusBarError(
                        QString("Terminal %1 has no "
                                "associated nodes.")
                            .arg(sourceTerminalObj
                                     ->getProperty("Name",
                                                   "")
                                     .toString()),
                        3000);
                    keepProcessing = false;
                }

                if (keepProcessing)
                {
                }
            }
        }
    }
}

void CargoNetSim::GUI::ViewController::
    connectVisibleTerminalsByInterfaces(
        MainWindow *mainWindow)
{
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

    // Connect terminals based on common interfaces
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

            // Create connections for each common mode
            for (const QString &mode : commonModes)
            {

                if (!mode.isEmpty())
                {
                    createConnectionLine(mainWindow,
                                         sourceItem,
                                         targetItem, mode);
                }
            }
        }
    }

    mainWindow->showStatusBarMessage(
        "Terminal connections created by common "
        "interfaces.",
        3000);
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
