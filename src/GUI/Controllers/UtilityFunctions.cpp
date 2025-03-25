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
