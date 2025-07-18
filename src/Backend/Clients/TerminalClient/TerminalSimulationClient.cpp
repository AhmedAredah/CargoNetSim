#include "TerminalSimulationClient.h"
#include <QDebug>
#include <QJsonDocument>
#include <QThread>
#include <stdexcept>

namespace CargoNetSim
{
namespace Backend
{

// Constructor implementation
TerminalSimulationClient::TerminalSimulationClient(
    QObject *parent, const QString &host, int port)
    : SimulationClientBase(
          parent, host, port, "CargoNetSim.Exchange",
          "CargoNetSim.CommandQueue.TerminalSim",
          "CargoNetSim.ResponseQueue.TerminalSim",
          "CargoNetSim.Command.TerminalSim",
          QStringList{"CargoNetSim.Response.TerminalSim"},
          ClientType::TerminalClient)
{
    // Log initialization for debugging and auditing
    qDebug() << "TerminalSimulationClient initialized";
}

// Destructor implementation
TerminalSimulationClient::~TerminalSimulationClient()
{
    // Lock mutex to ensure thread-safe cleanup
    Commons::ScopedWriteLock locker(m_dataMutex);

    // Clean up terminal status objects
    for (Terminal *terminal : m_terminalStatus.values())
    {
        delete terminal;
    }
    m_terminalStatus.clear();

    // Clear alias map
    m_terminalAliases.clear();

    // Clean up shortest path segments
    for (const QList<PathSegment *> &segments :
         m_shortestPaths.values())
    {
        for (PathSegment *segment : segments)
        {
            delete segment;
        }
    }
    m_shortestPaths.clear();

    // Clean up top paths (Path destructor handles segments)
    for (const QList<Path *> &paths : m_topPaths.values())
    {
        for (Path *path : paths)
        {
            delete path;
        }
    }
    m_topPaths.clear();

    // Clean up only dequeued containers
    for (const QList<ContainerCore::Container *>
             &containers : m_containers.values())
    {
        for (ContainerCore::Container *container :
             containers)
        {
            delete container; // Only if ownership
                              // transferred
        }
    }
    m_containers.clear();

    // Clear capacities and terminal count
    m_capacities.clear();
    m_serializedGraph = QJsonObject();
    m_pingResponse    = QJsonObject();
    m_terminalCount   = 0;

    // Log destruction for tracking
    qDebug() << "TerminalSimulationClient destroyed";
}

// Reset server state
bool TerminalSimulationClient::resetServer()
{
    // Execute reset command in serialized manner
    return executeSerializedCommand([this]() {
        // Send reset command with no parameters
        return sendCommandAndWait(
            "resetServer", QJsonObject(), {"serverReset"});
    });
}

// Initialize client in thread
void TerminalSimulationClient::initializeClient(
    SimulationTime           *simulationTime,
    TerminalSimulationClient *terminalClient,
    LoggerInterface          *logger)
{
    // Call base class initialization first
    SimulationClientBase::initializeClient(
        simulationTime, terminalClient, logger);

    // Validate RabbitMQ handler presence
    if (!m_rabbitMQHandler)
    {
        throw std::runtime_error(
            "RabbitMQ handler not set");
    }

    // Configure heartbeat for connection health
    m_rabbitMQHandler->setupHeartbeat(5);

    // Log initialization details for audit
    qDebug() << "Client initialized in thread:"
             << QThread::currentThreadId();
}

// Set cost function parameters
bool TerminalSimulationClient::setCostFunctionParameters(
    const QVariantMap &parameters)
{
    // Execute command with serialization
    return executeSerializedCommand([&]() {
        // Create a complete parameter map with defaults
        QVariantMap completeParams = parameters;

        // Required mode entries
        QStringList requiredModes = {
            "default",
            QString::number(static_cast<int>(
                TransportationTypes::TransportationMode::
                    Ship)),
            QString::number(static_cast<int>(
                TransportationTypes::TransportationMode::
                    Train)),
            QString::number(static_cast<int>(
                TransportationTypes::TransportationMode::
                    Truck))};

        // Required attributes for each mode
        QStringList requiredAttrs = {
            "cost",           "travelTime",
            "distance",       "carbonEmissions",
            "risk",           "energyConsumption",
            "terminal_delay", "terminal_cost"};

        // Ensure all modes exist with default values
        for (const QString &mode : requiredModes)
        {
            if (!completeParams.contains(mode)
                || !completeParams[mode]
                        .canConvert<QVariantMap>())
            {
                // Create mode with all default values
                QVariantMap defaultModeParams;
                for (const QString &attr : requiredAttrs)
                {
                    defaultModeParams[attr] = 1.0;
                }
                completeParams[mode] = defaultModeParams;
                qDebug() << "Created default parameters "
                            "for mode:"
                         << mode;
            }
            else
            {
                // Mode exists, ensure all attributes exist
                QVariantMap modeParams =
                    completeParams[mode].toMap();
                bool modeUpdated = false;

                for (const QString &attr : requiredAttrs)
                {
                    if (!modeParams.contains(attr)
                        || !modeParams[attr]
                                .canConvert<double>())
                    {
                        modeParams[attr] = 1.0;
                        modeUpdated      = true;
                        qDebug()
                            << "Added default value for"
                            << attr << "in mode" << mode;
                    }
                }

                if (modeUpdated)
                {
                    completeParams[mode] = modeParams;
                }
            }
        }

        // Parameters are now complete with defaults,
        // prepare request
        QJsonObject params;
        params["parameters"] =
            QJsonObject::fromVariantMap(completeParams);

        // Send command to server
        return sendCommandAndWait(
            "set_cost_function_parameters", params,
            {"costFunctionUpdated"});
    });
}

// Add terminal
bool TerminalSimulationClient::addTerminal(
    const Terminal *terminal)
{
    // Execute command with serialization
    return executeSerializedCommand([&]() {
        // Validate terminal pointer
        if (!terminal)
        {
            qCritical() << "Null terminal pointer";
            return false;
        }
        // Send terminal addition command
        return sendCommandAndWait("add_terminal",
                                  terminal->toJson(),
                                  {"terminalAdded"});
    });
}

bool TerminalSimulationClient::addTerminals(
    const QList<Terminal *> &terminals)
{
    // Execute command with serialization
    return executeSerializedCommand([&]() {
        // Validate input
        if (terminals.isEmpty())
        {
            qCritical() << "Empty terminals list";
            return false;
        }

        // Prepare parameters for bulk addition
        QJsonObject params;
        QJsonArray  terminalsArray;

        // Convert each terminal to JSON
        for (const Terminal *terminal : terminals)
        {
            if (!terminal)
            {
                qWarning()
                    << "Skipping null terminal pointer";
                continue;
            }
            terminalsArray.append(terminal->toJson());
        }

        // Skip if no valid terminals
        if (terminalsArray.isEmpty())
        {
            qCritical() << "No valid terminals to add";
            return false;
        }

        params["terminals"] = terminalsArray;

        // Send command to server
        return sendCommandAndWait("add_terminals", params,
                                  {"terminalsAdded"});
    });
}

// Add terminal alias
bool TerminalSimulationClient::addTerminalAlias(
    const QString &terminalId, const QString &alias)
{
    // Execute alias addition serially
    return executeSerializedCommand([&]() {
        // Prepare parameters for alias addition
        QJsonObject params;
        params["terminal_name"] = terminalId;
        params["alias"]         = alias;
        // Send alias command to server
        return sendCommandAndWait("add_alias_to_terminal",
                                  params,
                                  {"terminalAdded"});
    });
}

// Get terminal aliases
QStringList TerminalSimulationClient::getTerminalAliases(
    const QString &terminalId)
{
    // Execute alias fetch command serially
    executeSerializedCommand([&]() {
        // Prepare parameters for alias retrieval
        QJsonObject params;
        params["terminal_name"] = terminalId;
        // Send command and wait for response
        return sendCommandAndWait("get_aliases_of_terminal",
                                  params,
                                  {"terminalAliases"});
    });

    // Access aliases with thread safety
    Commons::ScopedReadLock locker(m_dataMutex);

    // Retrieve aliases from dedicated map
    return m_terminalAliases.value(terminalId,
                                   QStringList());
}

// Remove terminal
bool TerminalSimulationClient::removeTerminal(
    const QString &terminalId)
{
    // Execute removal command serially
    return executeSerializedCommand([&]() {
        // Prepare parameters for removal
        QJsonObject params;
        params["terminal_name"] = terminalId;
        // Send removal command to server
        return sendCommandAndWait("remove_terminal", params,
                                  {"terminalRemoved"});
    });
}

// Get terminal count
int TerminalSimulationClient::getTerminalCount()
{
    // Execute count fetch serially
    executeSerializedCommand([&]() {
        // Send count command with no parameters
        return sendCommandAndWait("get_terminal_count",
                                  QJsonObject(),
                                  {"terminalCount"});
    });
    // Access count thread-safely
    Commons::ScopedReadLock locker(m_dataMutex);
    return m_terminalCount;
}

// Get terminal status
Terminal *TerminalSimulationClient::getTerminalStatus(
    const QString &terminalId)
{
    // Execute status fetch serially
    executeSerializedCommand([&]() {
        // Validate input parameter
        if (terminalId.isEmpty())
        {
            qWarning() << "Empty terminalId not supported";
            return false;
        }
        // Prepare parameters for status query
        QJsonObject params;
        params["terminal_name"] = terminalId;
        // Send status command to server
        return sendCommandAndWait("get_terminal", params,
                                  {"terminalStatus"});
    });
    // Access status thread-safely
    Commons::ScopedReadLock locker(m_dataMutex);
    return m_terminalStatus.value(terminalId, nullptr);
}

// Add route
bool TerminalSimulationClient::addRoute(
    const PathSegment *route)
{
    // Execute route addition serially
    return executeSerializedCommand([&]() {
        // Validate route pointer
        if (!route)
        {
            qCritical() << "Null PathSegment pointer";
            return false;
        }
        // Send route addition command
        return sendCommandAndWait(
            "add_route", route->toJson(), {"routeAdded"});
    });
}

bool TerminalSimulationClient::addRoutes(
    const QList<PathSegment *> &routes)
{
    // Execute command with serialization
    return executeSerializedCommand([&]() {
        // Validate input
        if (routes.isEmpty())
        {
            qCritical() << "Empty routes list";
            return false;
        }

        // Prepare parameters for bulk addition
        QJsonObject params;
        QJsonArray  routesArray;

        // Convert each route to JSON
        for (const PathSegment *route : routes)
        {
            if (!route)
            {
                qWarning() << "Skipping null route pointer";
                continue;
            }
            routesArray.append(route->toJson());
        }

        // Skip if no valid routes
        if (routesArray.isEmpty())
        {
            qCritical() << "No valid routes to add";
            return false;
        }

        params["routes"] = routesArray;

        // Send command to server
        return sendCommandAndWait("add_routes", params,
                                  {"routesAdded"});
    });
}

// Change route weight
bool TerminalSimulationClient::changeRouteWeight(
    const QString &start, const QString &end, int mode,
    const QJsonObject &attributes)
{
    // Execute weight update serially
    return executeSerializedCommand([&]() {
        // Prepare parameters for weight update
        QJsonObject params;
        params["start_terminal"] = start;
        params["end_terminal"]   = end;
        params["mode"]           = mode;
        params["attributes"]     = attributes;
        // Send weight update command
        return sendCommandAndWait("change_route_weight",
                                  params, {"routeAdded"});
    });
}

// Connect terminals by interface modes
bool TerminalSimulationClient::
    connectTerminalsByInterfaceModes()
{
    // Execute connection command serially
    return executeSerializedCommand([&]() {
        // Send command with no parameters
        return sendCommandAndWait(
            "connect_terminals_by_interface_modes",
            QJsonObject(), {"routeAdded"});
    });
}

// Connect terminals in region by mode
bool TerminalSimulationClient::
    connectTerminalsInRegionByMode(const QString &region)
{
    // Execute region connection serially
    return executeSerializedCommand([&]() {
        // Prepare parameters for region connection
        QJsonObject params;
        params["region"] = region;
        // Send region connection command
        return sendCommandAndWait(
            "connect_terminals_in_region_by_mode", params,
            {"routeAdded"});
    });
}

// Connect regions by mode
bool TerminalSimulationClient::connectRegionsByMode(
    int mode)
{
    // Execute mode connection serially
    return executeSerializedCommand([&]() {
        // Prepare parameters for mode connection
        QJsonObject params;
        params["mode"] = mode;
        // Send mode connection command
        return sendCommandAndWait("connect_regions_by_mode",
                                  params, {"routeAdded"});
    });
}

// Find shortest path
QList<PathSegment *>
TerminalSimulationClient::findShortestPath(
    const QString &start, const QString &end, int mode)
{
    // Execute path finding serially
    executeSerializedCommand([&]() {
        // Prepare parameters for shortest path
        QJsonObject params;
        params["start_terminal"] = start;
        params["end_terminal"]   = end;
        params["mode"]           = mode;
        // Send path finding command
        return sendCommandAndWait("find_shortest_path",
                                  params, {"pathFound"});
    });
    // Access path thread-safely
    Commons::ScopedReadLock locker(m_dataMutex);
    QString                 key =
        start + "-" + end + "-" + QString::number(mode);
    return m_shortestPaths.value(key,
                                 QList<PathSegment *>());
}

// Find top paths
QList<Path *> TerminalSimulationClient::findTopPaths(
    const QString &start, const QString &end, int n,
    TransportationTypes::TransportationMode mode,
    bool                                    skipDelays)
{
    int modeInt = TransportationTypes::toInt(mode);
    // Execute top paths finding serially
    executeSerializedCommand([&]() {
        // Prepare parameters for top paths
        QJsonObject params;
        params["start_terminal"] = start;
        params["end_terminal"]   = end;
        params["n"]              = n;
        params["mode"]           = modeInt;
        params["skip_same_mode_terminal_delays_and_costs"] =
            skipDelays;
        // Send top paths command
        return sendCommandAndWait("find_top_paths", params,
                                  {"pathFound"});
    });
    // Access paths thread-safely
    Commons::ScopedReadLock locker(m_dataMutex);
    QString                 key = start + "-" + end;
    return m_topPaths.value(key, QList<Path *>());
}

// Add single container
bool TerminalSimulationClient::addContainer(
    const QString                  &terminalId,
    const ContainerCore::Container *container,
    double                          addTime)
{
    // Execute container addition serially
    return executeSerializedCommand([&]() {
        // Prepare parameters for container addition
        QJsonObject params;
        params["terminal_id"] = terminalId;
        params["container"]   = container->toJson();
        if (addTime >= 0.0)
        {
            params["adding_time"] = addTime;
        }
        // Send container addition command
        return sendCommandAndWait("add_container", params,
                                  {"containersAdded"});
    });
}

bool TerminalSimulationClient::addContainers(
    const QString &terminalId, QString &containers,
    double addTime)
{
    // Execute containers addition serially
    return executeSerializedCommand([&]() {
        // Prepare parameters for containers addition
        QJsonObject params;
        params["terminal_id"] = terminalId;

        params["containers"] = containers;
        if (addTime >= 0.0)
        {
            params["adding_time"] = addTime;
        }
        // Send containers addition command
        return sendCommandAndWait("add_containers", params,
                                  {"containersAdded"});
    });
}

// Add multiple containers
bool TerminalSimulationClient::addContainers(
    const QString                     &terminalId,
    QList<ContainerCore::Container *> &containers,
    double                             addTime)
{
    // Execute containers addition serially
    return executeSerializedCommand([&]() {
        // Prepare parameters for containers addition
        QJsonObject params;
        params["terminal_id"] = terminalId;
        QJsonArray containersArray;
        for (const auto *container : containers)
        {
            if (container)
            {
                containersArray.append(container->toJson());
            }
        }
        params["containers"] = containersArray;
        if (addTime >= 0.0)
        {
            params["adding_time"] = addTime;
        }
        // Send containers addition command
        return sendCommandAndWait("add_containers", params,
                                  {"containersAdded"});
    });
}

// Add containers from JSON
bool TerminalSimulationClient::addContainersFromJson(
    const QString &terminalId, const QString &json,
    double addTime)
{
    // Execute JSON addition serially
    return executeSerializedCommand([&]() {
        // Prepare parameters for JSON addition
        QJsonObject params;
        params["terminal_id"]     = terminalId;
        params["containers_json"] = json;
        if (addTime >= 0.0)
        {
            params["adding_time"] = addTime;
        }
        // Send JSON containers command
        return sendCommandAndWait(
            "add_containers_from_json", params,
            {"containersAdded"});
    });
}

// Get containers by departing time
QList<ContainerCore::Container *>
TerminalSimulationClient::getContainersByDepartingTime(
    const QString &terminalId, double time,
    const QString &condition)
{
    // Execute fetch by departing time serially
    executeSerializedCommand([&]() {
        // Prepare parameters for fetch
        QJsonObject params;
        params["terminal_id"]    = terminalId;
        params["departing_time"] = time;
        params["condition"]      = condition;
        // Send fetch command
        return sendCommandAndWait(
            "get_containers_by_departing_time", params,
            {"containersFetched"});
    });
    // Access containers thread-safely
    Commons::ScopedReadLock locker(m_dataMutex);
    return m_containers.value(
        terminalId, QList<ContainerCore::Container *>());
}

// Get containers by added time
QList<ContainerCore::Container *>
TerminalSimulationClient::getContainersByAddedTime(
    const QString &terminalId, double time,
    const QString &condition)
{
    // Execute fetch by added time serially
    executeSerializedCommand([&]() {
        // Prepare parameters for fetch
        QJsonObject params;
        params["terminal_id"] = terminalId;
        params["added_time"]  = time;
        params["condition"]   = condition;
        // Send fetch command
        return sendCommandAndWait(
            "get_containers_by_added_time", params,
            {"containersFetched"});
    });
    // Access containers thread-safely
    Commons::ScopedReadLock locker(m_dataMutex);
    return m_containers.value(
        terminalId, QList<ContainerCore::Container *>());
}

// Get containers by next destination
QList<ContainerCore::Container *>
TerminalSimulationClient::getContainersByNextDestination(
    const QString &terminalId, const QString &destination)
{
    // Execute fetch by destination serially
    executeSerializedCommand([&]() {
        // Prepare parameters for fetch
        QJsonObject params;
        params["terminal_id"] = terminalId;
        params["destination"] = destination;
        // Send fetch command
        return sendCommandAndWait(
            "get_containers_by_next_destination", params,
            {"containersFetched"});
    });
    // Access containers thread-safely
    Commons::ScopedReadLock locker(m_dataMutex);
    return m_containers.value(
        terminalId, QList<ContainerCore::Container *>());
}

// Dequeue containers by next destination
QList<ContainerCore::Container *> TerminalSimulationClient::
    dequeueContainersByNextDestination(
        const QString &terminalId,
        const QString &destination)
{
    // Execute dequeue serially
    executeSerializedCommand([&]() {
        // Prepare parameters for dequeue
        QJsonObject params;
        params["terminal_id"] = terminalId;
        params["destination"] = destination;
        // Send dequeue command
        return sendCommandAndWait(
            "dequeue_containers_by_next_destination",
            params, {"containersFetched"});
    });
    // Access dequeued containers thread-safely
    Commons::ScopedReadLock locker(m_dataMutex);
    return m_containers.value(
        terminalId, QList<ContainerCore::Container *>());
}

// Get container count
int TerminalSimulationClient::getContainerCount(
    const QString &terminalId)
{
    // Execute count fetch serially
    executeSerializedCommand([&]() {
        // Prepare parameters for count fetch
        QJsonObject params;
        params["terminal_id"] = terminalId;
        // Send count fetch command
        return sendCommandAndWait("get_container_count",
                                  params,
                                  {"capacityFetched"});
    });
    // Access count thread-safely and cast
    Commons::ScopedReadLock locker(m_dataMutex);
    double count = m_capacities.value(terminalId, 0.0);
    return static_cast<int>(count);
}

// Get available capacity
double TerminalSimulationClient::getAvailableCapacity(
    const QString &terminalId)
{
    // Execute capacity fetch serially
    executeSerializedCommand([&]() {
        // Prepare parameters for capacity fetch
        QJsonObject params;
        params["terminal_id"] = terminalId;
        // Send capacity fetch command
        return sendCommandAndWait("get_available_capacity",
                                  params,
                                  {"capacityFetched"});
    });
    // Access capacity thread-safely
    Commons::ScopedReadLock locker(m_dataMutex);
    return m_capacities.value(terminalId, 0.0);
}

// Get maximum capacity
double TerminalSimulationClient::getMaxCapacity(
    const QString &terminalId)
{
    // Execute max capacity fetch serially
    executeSerializedCommand([&]() {
        // Prepare parameters for max capacity fetch
        QJsonObject params;
        params["terminal_id"] = terminalId;
        // Send max capacity fetch command
        return sendCommandAndWait("get_max_capacity",
                                  params,
                                  {"capacityFetched"});
    });
    // Access max capacity thread-safely
    Commons::ScopedReadLock locker(m_dataMutex);
    return m_capacities.value(terminalId, 0.0);
}

// Clear terminal containers
bool TerminalSimulationClient::clearTerminal(
    const QString &terminalId)
{
    // Execute clear command serially
    return executeSerializedCommand([&]() {
        // Prepare parameters for clear command
        QJsonObject params;
        params["terminal_id"] = terminalId;
        // Send clear command to server
        return sendCommandAndWait("clear_terminal", params,
                                  {"containersAdded"});
    });
}

// Serialize server graph
QJsonObject TerminalSimulationClient::serializeGraph()
{
    // Execute serialize command serially
    executeSerializedCommand([&]() {
        // Send serialize command with no parameters
        return sendCommandAndWait("serialize_graph",
                                  QJsonObject(),
                                  {"graphSerialized"});
    });

    // Access serialized graph thread-safely
    Commons::ScopedReadLock locker(m_dataMutex);
    return m_serializedGraph;
}

// Deserialize server graph
bool TerminalSimulationClient::deserializeGraph(
    const QJsonObject &graphData)
{
    // Execute deserialize command serially
    return executeSerializedCommand([&]() {
        // Prepare parameters for deserialize
        QJsonObject params;
        params["graph_data"] = graphData;
        // Send deserialize command
        return sendCommandAndWait("deserialize_graph",
                                  params,
                                  {"graphDeserialized"});
    });
}

// Ping server
QJsonObject
TerminalSimulationClient::ping(const QString &echo)
{
    // Execute ping command serially
    executeSerializedCommand([&]() {
        // Prepare parameters for ping
        QJsonObject params;
        if (!echo.isEmpty())
        {
            params["echo"] = echo;
        }
        // Send ping command to server
        return sendCommandAndWait("ping", params,
                                  {"pingResponse"});
    });
    // Access ping response thread-safely
    Commons::ScopedReadLock locker(m_dataMutex);
    return m_pingResponse;
}

// Process incoming server messages
void TerminalSimulationClient::processMessage(
    const QJsonObject &message)
{
    // Delegate to base class for initial processing
    SimulationClientBase::processMessage(message);

    // Check for event field presence
    if (!message.contains("event"))
    {
        return;
    }

    // Normalize event name for consistent handling
    QString event     = message["event"].toString();
    QString normEvent = normalizeEventName(event);

    // Dispatch event to appropriate handler
    if (normEvent == "terminaladded")
    {
        onTerminalAdded(message);
    }
    else if (normEvent == "terminalsadded")
    {
        onTerminalsAdded(message);
    }
    else if (normEvent == "routeadded")
    {
        onRouteAdded(message);
    }
    else if (normEvent == "routesadded")
    {
        onRoutesAdded(message);
    }
    else if (normEvent == "pathfound")
    {
        onPathsFound(message);
    }
    else if (normEvent == "containersadded")
    {
        onContainersAdded(message);
    }
    else if (normEvent == "serverreset")
    {
        onServerReset(message);
    }
    else if (normEvent == "erroroccurred")
    {
        onErrorOccurred(message);
    }
    else if (normEvent == "terminalremoved")
    {
        onTerminalRemoved(message);
    }
    else if (normEvent == "terminalcount")
    {
        onTerminalCount(message);
    }
    else if (normEvent == "containersfetched")
    {
        onContainersFetched(message);
    }
    else if (normEvent == "capacityfetched")
    {
        onCapacityFetched(message);
    }
    else if (normEvent == "graphserialized")
    {
        Commons::ScopedWriteLock locker(m_dataMutex);
        m_serializedGraph = message["result"].toObject();
    }
    else if (normEvent == "pingresponse")
    {
        Commons::ScopedWriteLock locker(m_dataMutex);
        m_pingResponse = message["result"].toObject();
    }
    else
    {
        if (m_logger)
        {
            m_logger->logError(
                QString("Unknown event received:%1")
                    .arg(event),
                static_cast<int>(m_clientType));
        }
        else
        {
            // Log unknown events for debugging
            qWarning()
                << "Unknown event received:" << event;
        }
    }
}

// Handle terminal added event
void TerminalSimulationClient::onTerminalAdded(
    const QJsonObject &message)
{
    QJsonObject result = message["result"].toObject();
    QString     name   = result["terminal_name"].toString();
    Commons::ScopedWriteLock locker(m_dataMutex);
    Terminal                *existing =
        m_terminalStatus.value(name, nullptr);
    if (existing)
    {
        delete existing;
    }
    Terminal *terminal = Terminal::fromJson(result);
    terminal->setParent(this);

    m_terminalStatus[name] = terminal;

    // Update aliases if present
    if (result.contains("aliases"))
    {
        QJsonArray  aliases = result["aliases"].toArray();
        QStringList aliasList;
        for (const QJsonValue &val : aliases)
        {
            aliasList.append(val.toString());
        }
        m_terminalAliases[name] = aliasList;
    }
    qDebug() << "Terminal added:" << name;
}

void TerminalSimulationClient::onTerminalsAdded(
    const QJsonObject &message)
{
    // Extract array of terminal results
    QJsonArray terminalsArray = message["result"].toArray();

    // Lock mutex for thread-safe update
    Commons::ScopedWriteLock locker(m_dataMutex);

    // Process each terminal in the array
    for (const QJsonValue &terminalValue : terminalsArray)
    {
        QJsonObject terminalJson = terminalValue.toObject();
        QString     name =
            terminalJson["terminal_name"].toString();

        // Clean up existing terminal if present
        Terminal *existing =
            m_terminalStatus.value(name, nullptr);
        if (existing)
        {
            delete existing;
        }

        // Create new terminal from JSON
        Terminal *terminal =
            Terminal::fromJson(terminalJson);
        terminal->setParent(this);

        // Store in map
        m_terminalStatus[name] = terminal;

        // Update aliases if present
        if (terminalJson.contains("aliases"))
        {
            QJsonArray aliases =
                terminalJson["aliases"].toArray();
            QStringList aliasList;
            for (const QJsonValue &val : aliases)
            {
                aliasList.append(val.toString());
            }
            m_terminalAliases[name] = aliasList;
        }
    }
}

// Handle route added event
void TerminalSimulationClient::onRouteAdded(
    const QJsonObject &message)
{
    // Extract parameters and results
    QJsonObject params = message["result"].toObject();
    QString     startTerminal =
        params["start_terminal"].toString();
    QString endTerminal = params["end_terminal"].toString();

    // Lock mutex for thread-safe update
    Commons::ScopedWriteLock locker(m_dataMutex);

    // Log event for auditing
    qDebug() << "Route added from" << startTerminal << "to"
             << endTerminal;
}

void TerminalSimulationClient::onRoutesAdded(
    const QJsonObject &message)
{
    // Extract array of route results
    QJsonArray routesArray = message["result"].toArray();

    // Lock mutex for thread-safe update
    Commons::ScopedWriteLock locker(m_dataMutex);

    // Process each route in the array
    for (const QJsonValue &routeValue : routesArray)
    {
        QJsonObject routeJson = routeValue.toObject();
        QString     startTerminal =
            routeJson["start_terminal"].toString();
        QString endTerminal =
            routeJson["end_terminal"].toString();
    }
}

// Handle path found event
void TerminalSimulationClient::onPathsFound(
    const QJsonObject &message)
{
    // Extract result and parameters from message
    QJsonObject result = message["result"].toObject();
    QString     start = result["start_terminal"].toString();
    QString     end   = result["end_terminal"].toString();
    QJsonArray  paths = result["paths"].toArray();
    QString     key   = start + "-" + end;

    // Lock mutex for thread-safe update
    Commons::ScopedWriteLock locker(m_dataMutex);

    // Clean up old top paths
    QList<Path *> oldPaths = m_topPaths.value(key);
    for (Path *oldPath : oldPaths)
    {
        delete oldPath;
    }
    m_topPaths[key]
        .clear(); // Clear the list after deleting

    // Handle top paths
    if (paths.size() > 0)
    {
        for (const QJsonValue &pathVal : paths)
        {
            QJsonObject pathObj = pathVal.toObject();
            Path       *path    = Path::fromJson(
                pathObj, m_terminalStatus, this);
            m_topPaths[key].push_back(path);
        }
    }

    // Log event for auditing
    qDebug() << "Path found from" << start << "to" << end;
}

// Handle containers added event
void TerminalSimulationClient::onContainersAdded(
    const QJsonObject &message)
{
    // Extract parameters and results
    QJsonObject params = message["params"].toObject();
    QString terminalId = params["terminal_id"].toString();

    // Lock mutex for thread-safe update
    Commons::ScopedWriteLock locker(m_dataMutex);

    // Log event for auditing
    qDebug() << "Containers added to terminal:"
             << terminalId;
}

// Handle server reset event
void TerminalSimulationClient::onServerReset(
    const QJsonObject &message)
{
    // Lock mutex for thread-safe cleanup
    Commons::ScopedWriteLock locker(m_dataMutex);

    // Clean up all terminal status objects
    for (Terminal *terminal : m_terminalStatus.values())
    {
        delete terminal;
    }
    m_terminalStatus.clear();
    m_terminalAliases.clear();

    // Clean up all shortest path segments
    for (const QList<PathSegment *> &segments :
         m_shortestPaths.values())
    {
        for (PathSegment *segment : segments)
        {
            delete segment;
        }
    }
    m_shortestPaths.clear();

    // Clean up all top paths
    for (const QList<Path *> &paths : m_topPaths.values())
    {
        for (Path *path : paths)
        {
            delete path;
        }
    }
    m_topPaths.clear();

    // Clean up dequeued containers only
    for (const QList<ContainerCore::Container *>
             &containers : m_containers.values())
    {
        for (ContainerCore::Container *container :
             containers)
        {
            delete container;
        }
    }
    m_containers.clear();

    // Reset capacities and terminal count
    m_capacities.clear();
    m_serializedGraph = QJsonObject();
    m_pingResponse    = QJsonObject();
    m_terminalCount   = 0;

    // Log event for auditing
    qDebug() << "Server reset successfully";
}

// Handle error event
void TerminalSimulationClient::onErrorOccurred(
    const QJsonObject &message)
{
    // Extract error message from response
    QString error = message["error"].toString();

    // Log error for debugging and review
    qCritical() << "Error occurred:" << error;
}

// Handle terminal removed event
void TerminalSimulationClient::onTerminalRemoved(
    const QJsonObject &message)
{
    // Extract parameters from message
    QJsonObject params = message["params"].toObject();
    QString terminalId = params["terminal_name"].toString();

    // Lock mutex for thread-safe update
    Commons::ScopedWriteLock locker(m_dataMutex);

    // Remove and delete terminal if exists
    Terminal *terminal = m_terminalStatus.take(terminalId);
    if (terminal)
    {
        delete terminal;
    }

    m_terminalAliases.remove(terminalId);

    // Log event for auditing
    qDebug() << "Terminal removed:" << terminalId;
}

// Handle terminal count event
void TerminalSimulationClient::onTerminalCount(
    const QJsonObject &message)
{
    // Extract count from message
    int count = message["result"].toInt();

    // Lock mutex for thread-safe update
    Commons::ScopedWriteLock locker(m_dataMutex);
    m_terminalCount = count;

    // Log event for tracking
    qDebug() << "Terminal count updated:" << count;
}

// Handle containers fetched event
void TerminalSimulationClient::onContainersFetched(
    const QJsonObject &message)
{
    // Extract containers and parameters
    QJsonArray  containers = message["result"].toArray();
    QJsonObject params     = message["params"].toObject();
    QString terminalId = params["terminal_id"].toString();

    // Lock mutex for thread-safe update
    Commons::ScopedWriteLock locker(m_dataMutex);

    // Check if this is a dequeue operation
    bool isDequeue = params.contains("destination")
                     && params.contains("dequeue");

    // Clean up old containers if dequeued
    QList<ContainerCore::Container *> oldContainers =
        m_containers.value(terminalId);
    if (isDequeue)
    {
        for (ContainerCore::Container *container :
             oldContainers)
        {
            delete container; // Ownership transferred
        }
    }

    // Create new container list from response
    QList<ContainerCore::Container *> newContainers;
    for (const QJsonValue &val : containers)
    {
        ContainerCore::Container *container =
            new ContainerCore::Container(val.toObject());
        newContainers.append(container);
    }

    // Store new containers in map
    m_containers[terminalId] = newContainers;

    // Log event for auditing
    qDebug() << "Containers fetched for:" << terminalId;
}

// Handle capacity fetched event
void TerminalSimulationClient::onCapacityFetched(
    const QJsonObject &message)
{
    // Extract capacity and parameters
    double      capacity = message["result"].toDouble();
    QJsonObject params   = message["params"].toObject();
    QString terminalId   = params["terminal_id"].toString();

    // Lock mutex for thread-safe update
    Commons::ScopedWriteLock locker(m_dataMutex);

    // Store capacity in map
    m_capacities[terminalId] = capacity;

    // Log event for tracking
    qDebug() << "Capacity fetched for:" << terminalId;
}

} // namespace Backend
} // namespace CargoNetSim
