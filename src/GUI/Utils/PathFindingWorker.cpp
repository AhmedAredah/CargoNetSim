#include "GUI/Utils/PathFindingWorker.h"
#include "Backend/Clients/TerminalClient/TerminalSimulationClient.h"
#include "GUI/Controllers/UtilityFunctions.h"
#include "GUI/Items/ConnectionLine.h"
#include <QSet>
#include <exception>

namespace CargoNetSim
{
namespace GUI
{

PathFindingWorker::PathFindingWorker()
    : QObject(nullptr)
{
}

void PathFindingWorker::initialize(MainWindow *window,
                                   int         count)
{
    mainWindow = window;
    pathsCount = count;
}

void PathFindingWorker::process()
{
    // Get controller instance
    auto &controller = CargoNetSimController::getInstance();
    auto  terminalClient = controller.getTerminalClient();

    try
    {
        // Reset the terminal server to start with a clean
        // state
        bool resetSuccess = terminalClient->resetServer();
        if (!resetSuccess)
        {
            emit error("Failed to reset terminal server");
            emit finished();
            return;
        }

        // Get the Origin and Destination terminals from the
        // region scene
        auto originTerminals =
            UtilitiesFunctions::getTerminalItems(
                mainWindow->regionScene_, "*", "Origin");
        auto destinationTerminals =
            UtilitiesFunctions::getTerminalItems(
                mainWindow->regionScene_, "*",
                "Destination");

        if (originTerminals.isEmpty()
            || destinationTerminals.isEmpty())
        {
            emit error("No Origin or Destination terminals "
                       "found.");
            emit finished();
            return;
        }

        // Keep track of terminals we've already added to
        // avoid duplicates
        QSet<QString> addedTerminalIds;

        // Add connections and their terminals to the server
        // First process region scene connections
        auto regionConnections =
            mainWindow->regionScene_
                ->getItemsByType<ConnectionLine>();
        for (auto connection : regionConnections)
        {
            if (!processConnectionAndTerminals(
                    connection, terminalClient,
                    addedTerminalIds, mainWindow))
            {
                emit error("Failed to add region "
                           "connection to server.");
                emit finished();
                return;
            }
        }

        // Then process global scene connections
        auto globalConnections =
            mainWindow->globalMapScene_
                ->getItemsByType<ConnectionLine>();
        for (auto connection : globalConnections)
        {
            if (!processConnectionAndTerminals(
                    connection, terminalClient,
                    addedTerminalIds, mainWindow))
            {
                emit error("Failed to add global "
                           "connection to server.");
                emit finished();
                return;
            }
        }

        if (regionConnections.isEmpty()
            && globalConnections.isEmpty())
        {
            emit error("No connections found in the region "
                       "scene.");
            emit finished();
            return;
        }

        // Get terminal IDs for origin and destination
        QString originId = originTerminals.first()->getID();
        QString destId =
            destinationTerminals.first()->getID();

        // Check if origin and destination terminals exist
        // in the server
        if (!terminalClient->getTerminalStatus(originId)
            || !terminalClient->getTerminalStatus(destId))
        {
            emit error("Origin or Destination terminal not "
                       "found in the graph server.");
            emit finished();
            return;
        }

        // Find the top N shortest paths
        QList<Backend::Path *> paths =
            terminalClient->findTopPaths(
                originId, destId, pathsCount,
                Backend::TransportationTypes::
                    TransportationMode::Any,
                false);

        if (paths.isEmpty())
        {
            emit error("No valid paths found between "
                       "Origin and Destination.");
            emit finished();
            return;
        }

        // Signal that we have paths
        emit resultReady(paths);
    }
    catch (const std::exception &e)
    {
        emit error(QString("Error finding paths: %1")
                       .arg(e.what()));
    }

    // Signal that we're done
    emit finished();
}

bool PathFindingWorker::addTerminalToServer(
    TerminalItem *terminal,
    CargoNetSim::Backend::TerminalSimulationClient
        *terminalClient)
{
    auto    props      = terminal->getProperties();
    QString terminalId = terminal->getID();
    QString terminalName =
        terminal->getProperties().value("Name").toString();
    QString regionName   = terminal->getRegion();

    // Create interfaces map for the Terminal object
    QMap<Backend::TerminalTypes::TerminalInterface,
         QSet<Backend::TransportationTypes::
                  TransportationMode>>
        interfaces;

    // Add terminal interfaces
    QMap<QString, QVariant> interfaceProps =
        props.value("Available Interfaces").toMap();

    // Process land-side interfaces
    if (interfaceProps.contains("land_side"))
    {
        QStringList landSide =
            interfaceProps.value("land_side")
                .toStringList();
        for (const QString &mode : landSide)
        {
            if (mode == "Truck")
            {
                if (!interfaces.contains(
                        Backend::TerminalTypes::
                            TerminalInterface::LAND_SIDE))
                {
                    interfaces[Backend::TerminalTypes::
                                   TerminalInterface::
                                       LAND_SIDE] =
                        QSet<Backend::TransportationTypes::
                                 TransportationMode>();
                }
                interfaces[Backend::TerminalTypes::
                               TerminalInterface::LAND_SIDE]
                    .insert(Backend::TransportationTypes::
                                TransportationMode::Truck);
            }
            if (mode == "Rail")
            {
                if (!interfaces.contains(
                        Backend::TerminalTypes::
                            TerminalInterface::LAND_SIDE))
                {
                    interfaces[Backend::TerminalTypes::
                                   TerminalInterface::
                                       LAND_SIDE] =
                        QSet<Backend::TransportationTypes::
                                 TransportationMode>();
                }
                interfaces[Backend::TerminalTypes::
                               TerminalInterface::LAND_SIDE]
                    .insert(Backend::TransportationTypes::
                                TransportationMode::Train);
            }
        }
    }

    // Process sea-side interfaces
    if (interfaceProps.contains("sea_side"))
    {
        QStringList seaSide =
            interfaceProps.value("sea_side").toStringList();
        for (const QString &mode : seaSide)
        {
            if (mode == "Ship")
            {
                if (!interfaces.contains(
                        Backend::TerminalTypes::
                            TerminalInterface::SEA_SIDE))
                {
                    interfaces[Backend::TerminalTypes::
                                   TerminalInterface::
                                       SEA_SIDE] =
                        QSet<Backend::TransportationTypes::
                                 TransportationMode>();
                }
                interfaces[Backend::TerminalTypes::
                               TerminalInterface::SEA_SIDE]
                    .insert(Backend::TransportationTypes::
                                TransportationMode::Ship);
            }
        }
    }

    // Create config JSON object for terminal
    QJsonObject config;

    // If terminal has type-specific properties, add them to
    // config
    if (props.contains("cost"))
    {
        QVariant costVar = props.value("cost");
        if (costVar.canConvert<QMap<QString, QVariant>>())
        {
            QMap<QString, QVariant> costMap =
                costVar.toMap();
            QJsonObject costObj;
            for (auto it = costMap.constBegin();
                 it != costMap.constEnd(); ++it)
            {
                costObj[it.key()] =
                    QJsonValue::fromVariant(it.value());
            }
            config["cost"] = costObj;
        }
    }

    if (props.contains("dwell_time"))
    {
        QVariant dwellVar = props.value("dwell_time");
        if (dwellVar.canConvert<QMap<QString, QVariant>>())
        {
            QMap<QString, QVariant> dwellMap =
                dwellVar.toMap();
            QJsonObject dwellObj;
            for (auto it = dwellMap.constBegin();
                 it != dwellMap.constEnd(); ++it)
            {
                if (it.key() == "parameters"
                    && it.value()
                           .canConvert<
                               QMap<QString, QVariant>>())
                {
                    QMap<QString, QVariant> paramMap =
                        it.value().toMap();
                    QJsonObject paramObj;
                    for (auto paramIt =
                             paramMap.constBegin();
                         paramIt != paramMap.constEnd();
                         ++paramIt)
                    {
                        paramObj[paramIt.key()] =
                            QJsonValue::fromVariant(
                                paramIt.value());
                    }
                    dwellObj["parameters"] = paramObj;
                }
                else
                {
                    dwellObj[it.key()] =
                        QJsonValue::fromVariant(it.value());
                }
            }
            config["dwell_time"] = dwellObj;
        }
    }

    if (props.contains("capacity"))
    {
        QVariant capVar = props.value("capacity");
        if (capVar.canConvert<QMap<QString, QVariant>>())
        {
            QMap<QString, QVariant> capMap = capVar.toMap();
            QJsonObject             capObj;
            for (auto it = capMap.constBegin();
                 it != capMap.constEnd(); ++it)
            {
                capObj[it.key()] =
                    QJsonValue::fromVariant(it.value());
            }
            config["capacity"] = capObj;
        }
    }

    if (props.contains("customs"))
    {
        QVariant custVar = props.value("customs");
        if (custVar.canConvert<QMap<QString, QVariant>>())
        {
            QMap<QString, QVariant> custMap =
                custVar.toMap();
            QJsonObject custObj;
            for (auto it = custMap.constBegin();
                 it != custMap.constEnd(); ++it)
            {
                custObj[it.key()] =
                    QJsonValue::fromVariant(it.value());
            }
            config["customs"] = custObj;
        }
    }

    // Create Terminal object using the full constructor
    Backend::Terminal *terminalObj = new Backend::Terminal(
        QStringList{terminalId}, // Include both ID and name
        terminalName, config, interfaces, regionName,
        nullptr);

    // Add terminal to server
    bool success = terminalClient->addTerminal(terminalObj);
    delete terminalObj; // Client makes a copy

    return success;
}

bool PathFindingWorker::processConnectionAndTerminals(
    ConnectionLine *connection,
    CargoNetSim::Backend::TerminalSimulationClient
                  *terminalClient,
    QSet<QString> &addedTerminalIds, MainWindow *mainWindow)
{
    // Get the connection type
    QString connType = connection->connectionType();
    Backend::TransportationTypes::TransportationMode mode;

    // Convert string connection type to numeric mode ID
    if (connType == "Truck")
        mode = Backend::TransportationTypes::
            TransportationMode::Truck;
    else if (connType == "Rail")
        mode = Backend::TransportationTypes::
            TransportationMode::Train;
    else if (connType == "Ship")
        mode = Backend::TransportationTypes::
            TransportationMode::Ship;
    else
    {
        mainWindow->showStatusBarError(
            QString("Unknown connection type: %1")
                .arg(connType),
            3000);
        return false;
    }

    // Extract terminal items and IDs from the connection
    TerminalItem *startTerminal = nullptr;
    TerminalItem *endTerminal   = nullptr;
    QString       startId, endId;

    // Handle different item types (TerminalItem or
    // GlobalTerminalItem)
    if (auto terminal = dynamic_cast<TerminalItem *>(
            connection->startItem()))
    {
        startTerminal = terminal;
        startId       = terminal->getID();
    }
    else if (auto globalTerminal =
                 dynamic_cast<GlobalTerminalItem *>(
                     connection->startItem()))
    {
        if (globalTerminal->getLinkedTerminalItem())
        {
            startTerminal =
                globalTerminal->getLinkedTerminalItem();
            startId = startTerminal->getID();
        }
    }

    if (auto terminal = dynamic_cast<TerminalItem *>(
            connection->endItem()))
    {
        endTerminal = terminal;
        endId       = terminal->getID();
    }
    else if (auto globalTerminal =
                 dynamic_cast<GlobalTerminalItem *>(
                     connection->endItem()))
    {
        if (globalTerminal->getLinkedTerminalItem())
        {
            endTerminal =
                globalTerminal->getLinkedTerminalItem();
            endId = endTerminal->getID();
        }
    }

    if (startId.isEmpty() || endId.isEmpty()
        || !startTerminal || !endTerminal)
    {
        mainWindow->showStatusBarError(
            "Connection has invalid terminal endpoints",
            3000);
        return false;
    }

    // Add terminals to server if they haven't been added
    // yet
    if (!addedTerminalIds.contains(startId))
    {
        if (!addTerminalToServer(startTerminal,
                                 terminalClient))
        {
            mainWindow->showStatusBarError(
                QString(
                    "Failed to add terminal %1 to server")
                    .arg(startTerminal->getProperties()
                             .value("Name")
                             .toString()),
                3000);
            return false;
        }
        addedTerminalIds.insert(startId);
    }

    if (!addedTerminalIds.contains(endId))
    {
        if (!addTerminalToServer(endTerminal,
                                 terminalClient))
        {
            mainWindow->showStatusBarError(
                QString(
                    "Failed to add terminal %1 to server")
                    .arg(endTerminal->getProperties()
                             .value("Name")
                             .toString()),
                3000);
            return false;
        }
        addedTerminalIds.insert(endId);
    }

    // Create attributes for the connection
    QJsonObject attributes;
    auto        props = connection->getProperties();

    // Convert properties to JSON attributes
    if (props.contains("distance"))
    {
        attributes["distance"] =
            props["distance"].toDouble();
    }

    if (props.contains("travelTime"))
    {
        attributes["travelTime"] =
            props["travelTime"].toDouble();
    }

    if (props.contains("cost"))
    {
        attributes["cost"] = props["cost"].toDouble();
    }

    if (props.contains("carbonEmissions"))
    {
        attributes["carbonEmissions"] =
            props["carbonEmissions"].toDouble();
    }

    if (props.contains("energyConsumption"))
    {
        attributes["energyConsumption"] =
            props["energyConsumption"].toDouble();
    }

    if (props.contains("risk"))
    {
        attributes["risk"] = props["risk"].toDouble();
    }

    // Create a unique ID for the route segment
    QString segmentId = connection->getID();

    // Create and add the route
    Backend::PathSegment *segment =
        new Backend::PathSegment(segmentId, startId, endId,
                                 mode, attributes, nullptr);

    bool success = terminalClient->addRoute(segment);
    delete segment; // Client makes a copy

    if (!success)
    {
        mainWindow->showStatusBarError(
            QString("Failed to add route from %1 to %2")
                .arg(startTerminal->getProperties()
                         .value("Name")
                         .toString())
                .arg(endTerminal->getProperties()
                         .value("Name")
                         .toString()),
            3000);
        return false;
    }

    return true;
}

} // namespace GUI
} // namespace CargoNetSim
