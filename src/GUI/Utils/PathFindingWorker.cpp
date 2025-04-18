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

    auto handler = terminalClient->getRabbitMQHandler();
    if (!handler)
    {
        emit error("RabbitMQ handler not found");
        emit finished();
        return;
    }
    if (!handler->isConnected()
        || !handler->hasCommandQueueConsumers())
    {
        emit error("TerminalSim is not connected");
        emit finished();
        return;
    }

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
        auto originTerminal =
            UtilitiesFunctions::getOriginTerminal(
                mainWindow);
        auto destinationTerminal =
            UtilitiesFunctions::getDestinationTerminal(
                mainWindow);

        if (!originTerminal || !destinationTerminal)
        {
            emit error("No Origin or Destination terminals "
                       "found.");
            emit finished();
            return;
        }

        // Keep track of terminals we've already collected
        // to avoid duplicates
        QSet<QString>         terminalIds;
        QList<TerminalItem *> terminalsToAdd;

        // First process region scene connections to collect
        // terminals
        auto regionConnections =
            mainWindow->regionScene_
                ->getItemsByType<ConnectionLine>();

        // Then process global scene connections to collect
        // terminals
        auto globalConnections =
            mainWindow->globalMapScene_
                ->getItemsByType<ConnectionLine>();

        if (regionConnections.isEmpty()
            && globalConnections.isEmpty())
        {
            emit error(
                "No connections found in the scenes.");
            emit finished();
            return;
        }

        // Collect all terminals from connections
        for (auto connection : regionConnections)
        {
            if (!collectTerminals(connection,
                                  terminalsToAdd,
                                  terminalIds))
            {
                emit error("Failed to process region "
                           "connection terminals.");
                emit finished();
                return;
            }
        }

        for (auto connection : globalConnections)
        {
            if (!collectTerminals(connection,
                                  terminalsToAdd,
                                  terminalIds))
            {
                emit error("Failed to process global "
                           "connection terminals.");
                emit finished();
                return;
            }
        }

        // Convert TerminalItems to Terminal objects for
        // bulk addition
        QList<Backend::Terminal *> terminals;
        for (auto terminal : terminalsToAdd)
        {
            Backend::Terminal *terminalObj =
                createTerminalObject(terminal);
            terminals.append(terminalObj);
        }

        // Add all terminals at once
        bool terminalsAdded =
            terminalClient->addTerminals(terminals);

        // Clean up Terminal objects
        for (auto terminal : terminals)
        {
            delete terminal;
        }

        if (!terminalsAdded)
        {
            emit error(
                "Failed to add terminals to server.");
            emit finished();
            return;
        }

        // Now collect all route segments for bulk addition
        QList<Backend::PathSegment *> routes;
        QSet<QString> processedConnectionIds;

        // Process region connections to collect routes
        if (!processConnections(regionConnections, routes,
                                processedConnectionIds))
        {
            emit error("Failed to process region "
                       "connection routes.");
            emit finished();
            return;
        }

        // Process global connections to collect routes
        if (!processConnections(globalConnections, routes,
                                processedConnectionIds))
        {
            emit error("Failed to process global "
                       "connection routes.");
            emit finished();
            return;
        }

        // Add all routes at once
        bool routesAdded =
            terminalClient->addRoutes(routes);

        // Clean up PathSegment objects
        for (auto route : routes)
        {
            delete route;
        }

        if (!routesAdded)
        {
            emit error("Failed to add routes to server.");
            emit finished();
            return;
        }

        // Get terminal IDs for origin and destination
        QString originId = originTerminal->getID();
        QString destId   = destinationTerminal->getID();

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
                true);

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

bool PathFindingWorker::collectTerminals(
    ConnectionLine        *connection,
    QList<TerminalItem *> &terminals,
    QSet<QString>         &terminalIds)
{
    // Extract terminal items from the connection
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
        return false;
    }

    // Add terminals to the list if they haven't been added
    // yet
    if (!terminalIds.contains(startId))
    {
        terminals.append(startTerminal);
        terminalIds.insert(startId);
    }

    if (!terminalIds.contains(endId))
    {
        terminals.append(endTerminal);
        terminalIds.insert(endId);
    }

    return true;
}

Backend::Terminal *PathFindingWorker::createTerminalObject(
    TerminalItem *terminal)
{
    auto    props      = terminal->getProperties();
    QString terminalId = terminal->getID();
    QString terminalName =
        terminal->getProperties().value("Name").toString();
    QString regionName = terminal->getRegion();

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
    return new Backend::Terminal(
        QStringList{terminalId}, // Include both ID and name
        terminalName, config, interfaces, regionName,
        nullptr);
}

bool PathFindingWorker::processConnections(
    const QList<ConnectionLine *> &connections,
    QList<Backend::PathSegment *> &routes,
    QSet<QString>                 &processedConnectionIds)
{
    for (auto connection : connections)
    {
        // Skip if we've already processed this connection
        QString connectionId = connection->getID();
        if (processedConnectionIds.contains(connectionId))
        {
            continue;
        }

        // Get the connection type
        QString connType = connection->connectionType();
        Backend::TransportationTypes::TransportationMode
            mode;

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
            return false;
        }

        // Extract terminal items and IDs from the
        // connection
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
            return false;
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

        // Create a PathSegment object for the route
        Backend::PathSegment *segment =
            new Backend::PathSegment(connectionId, startId,
                                     endId, mode,
                                     attributes, nullptr);

        // Add to routes list
        routes.append(segment);

        // Mark as processed
        processedConnectionIds.insert(connectionId);
    }

    return true;
}

} // namespace GUI
} // namespace CargoNetSim
