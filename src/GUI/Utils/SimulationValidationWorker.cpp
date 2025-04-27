// SimulationValidationWorker.cpp
#include "SimulationValidationWorker.h"
#include "GUI/Controllers/UtilityFunctions.h"
#include "GUI/Items/ConnectionLine.h"
#include "GUI/Items/GlobalTerminalItem.h"
#include "GUI/Items/MapPoint.h"
#include "GUI/Items/TerminalItem.h"
#include "GUI/Widgets/GraphicsView.h"
#include "GUI/Widgets/ShortestPathTable.h"

namespace CargoNetSim
{
namespace GUI
{

SimulationValidationWorker::SimulationValidationWorker()
    : QObject(nullptr)
    , mainWindow(nullptr)
{
}

void SimulationValidationWorker::initialize(
    MainWindow *window)
{
    mainWindow = window;
}

void SimulationValidationWorker::process()
{
    if (!mainWindow)
    {
        emit errorMessage(
            "Worker not properly initialized");
        emit finished();
        return;
    }

    try
    {
        // Validate configuration
        if (!validateConfiguration())
        {
            emit finished();
            return;
        }

        // Validate terminals
        if (!validateTerminals())
        {
            emit finished();
            return;
        }

        // Process selected paths
        if (!processSelectedPaths())
        {
            emit finished();
            return;
        }

        // Extract results
        extractResults();

        emit statusMessage(
            "Simulation validation completed successfully");
    }
    catch (const std::exception &e)
    {
        emit errorMessage(
            QString(
                "Error during simulation validation: %1")
                .arg(e.what()));
    }
    catch (...)
    {
        emit errorMessage(
            "Unknown error during simulation validation");
    }

    emit finished();
}

bool SimulationValidationWorker::validateConfiguration()
{
    auto vehicleController =
        CargoNetSim::CargoNetSimController::getInstance()
            .getVehicleController();
    auto configController =
        CargoNetSim::CargoNetSimController::getInstance()
            .getConfigController();

    QMap<QString, QVariant> transModes =
        configController->getTransportModes();

    int shipContainerCount =
        transModes.value("ship")
            .toMap()
            .value("average_container_number", -1)
            .toInt();
    int trainContainerCount =
        transModes.value("rail")
            .toMap()
            .value("average_container_number", -1)
            .toInt();
    int truckContainerCount =
        transModes.value("truck")
            .toMap()
            .value("average_container_number", -1)
            .toInt();

    // Check if we have valid container count
    if (shipContainerCount < 0 || trainContainerCount < 0
        || truckContainerCount < 0)
    {
        emit errorMessage(
            "Container count cannot be less than 0!");
        return false;
    }

    return true;
}

bool SimulationValidationWorker::validateTerminals()
{
    auto originTerminal =
        UtilitiesFunctions::getOriginTerminal(mainWindow);
    if (!originTerminal)
    {
        emit errorMessage(
            "There is no origin in the region map");
        return false;
    }

    QVariant containersVar =
        originTerminal->getProperty("Containers");
    QList<ContainerCore::Container *> containers;
    if (containersVar.canConvert<
            QList<ContainerCore::Container *>>())
    {
        containers =
            containersVar
                .value<QList<ContainerCore::Container *>>();
    }

    if (containers.isEmpty())
    {
        emit errorMessage(
            "No containers in the origin terminal");
        return false;
    }

    // Get the selected paths for simulation validation
    auto selectedPathsData = mainWindow->shortestPathTable_
                                 ->getCheckedPathData();
    if (selectedPathsData.isEmpty())
    {
        emit errorMessage(
            "No paths selected for simulation");
        return false;
    }

    return true;
}

bool SimulationValidationWorker::processSelectedPaths()
{
    // Storage for simulation data
    QMap<QString, QList<ShipSimData>>  shipSimulationData;
    QMap<QString, QList<TrainSimData>> trainSimulationData;
    QMap<QString, QList<TruckSimData>> truckSimulationData;
    QMap<QString, Backend::TrainClient::NeTrainSimNetwork *>
        trainNetworks;
    QMap<QString,
         Backend::TruckClient::IntegrationNetwork *>
        truckNetworks;

    // Setup simulation data based on selected paths
    if (!setupSimulationData(shipSimulationData,
                             trainSimulationData,
                             truckSimulationData,
                             trainNetworks, truckNetworks))
    {
        return false;
    }

    // Run simulations with the collected data
    if (!runSimulations(shipSimulationData,
                        trainSimulationData,
                        truckSimulationData, trainNetworks,
                        truckNetworks))
    {
        return false;
    }

    return true;
}

bool SimulationValidationWorker::setupSimulationData(
    QMap<QString, QList<ShipSimData>>  &shipSimulationData,
    QMap<QString, QList<TrainSimData>> &trainSimulationData,
    QMap<QString, QList<TruckSimData>> &truckSimulationData,
    QMap<QString, Backend::TrainClient::NeTrainSimNetwork *>
                         &trainNetworks,
    QMap<QString, Backend::TruckClient::IntegrationNetwork
                      *> &truckNetworks)
{
    auto vehicleController =
        CargoNetSim::CargoNetSimController::getInstance()
            .getVehicleController();
    auto configController =
        CargoNetSim::CargoNetSimController::getInstance()
            .getConfigController();
    QMap<QString, QVariant> transModes =
        configController->getTransportModes();

    int shipContainerCount =
        transModes.value("ship")
            .toMap()
            .value("average_container_number", -1)
            .toInt();
    int trainContainerCount =
        transModes.value("rail")
            .toMap()
            .value("average_container_number", -1)
            .toInt();
    int truckContainerCount =
        transModes.value("truck")
            .toMap()
            .value("average_container_number", -1)
            .toInt();

    auto originTerminal =
        UtilitiesFunctions::getOriginTerminal(mainWindow);
    QVariant containersVar =
        originTerminal->getProperty("Containers");
    QList<ContainerCore::Container *> containers;
    if (containersVar.canConvert<
            QList<ContainerCore::Container *>>())
    {
        containers =
            containersVar
                .value<QList<ContainerCore::Container *>>();
    }

    // Get the selected paths for simulation validation
    auto selectedPathsData = mainWindow->shortestPathTable_
                                 ->getCheckedPathData();

    // Process each selected path
    for (auto pathData : selectedPathsData)
    {
        Backend::Path *path = pathData->path;
        if (!path)
        {
            continue;
        }

        // Get segments from the path
        QList<Backend::PathSegment *> segments =
            path->getSegments();

        // Keep track of the number of vehicles needed
        // per path
        int shipCounter      = 0;
        int trainCounter     = 0;
        int truckCounter     = 0;
        int containerCounter = 0;

        // Process each segment in the path
        for (auto segment : segments)
        {
            if (!segment)
            {
                continue;
            }

            QString startID = segment->getStart();
            QString endID   = segment->getEnd();

            // Get terminal items corresponding to the
            // segment endpoints
            TerminalItem *startTerminal =
                mainWindow->regionScene_
                    ->getItemById<TerminalItem>(startID);
            TerminalItem *endTerminal =
                mainWindow->regionScene_
                    ->getItemById<TerminalItem>(endID);

            // If not found in region scene, try global map
            // scene
            if (!startTerminal)
            {
                GlobalTerminalItem *startGlobalTerminal =
                    mainWindow->globalMapScene_
                        ->getItemById<GlobalTerminalItem>(
                            startID);
                if (startGlobalTerminal)
                {
                    startTerminal =
                        startGlobalTerminal
                            ->getLinkedTerminalItem();
                }
            }

            if (!endTerminal)
            {
                GlobalTerminalItem *endGlobalTerminal =
                    mainWindow->globalMapScene_
                        ->getItemById<GlobalTerminalItem>(
                            endID);
                if (endGlobalTerminal)
                {
                    endTerminal =
                        endGlobalTerminal
                            ->getLinkedTerminalItem();
                }
            }

            if (!startTerminal || !endTerminal)
            {
                continue;
            }

            // Get the common modes to explore connectivity
            // options
            // QList<QString> commonModes =
            //     UtilitiesFunctions::getCommonModes(
            //         startTerminal, endTerminal);

            // Check for Rail connectivity
            if (segment->getMode()
                == Backend::TransportationTypes::
                    TransportationMode::Train)
            {
                // Get map points linked to these terminals
                // for train networks
                QList<MapPoint *> startNetworkPoints =
                    UtilitiesFunctions::
                        getMapPointsOfTerminal(
                            mainWindow->regionScene_,
                            startTerminal, "*", "*",
                            NetworkType::Train);
                QList<MapPoint *> endNetworkPoints =
                    UtilitiesFunctions::
                        getMapPointsOfTerminal(
                            mainWindow->regionScene_,
                            endTerminal, "*", "*",
                            NetworkType::Train);

                // Find common networks between the
                // terminals
                auto commonNets = UtilitiesFunctions::
                    getCommonNetworksOfNetworkType(
                        startNetworkPoints,
                        endNetworkPoints,
                        NetworkType::Train);

                for (auto networkPair : commonNets)
                {
                    MapPoint *startPoint =
                        networkPair.first;
                    MapPoint *endPoint = networkPair.second;

                    QString startNodeID =
                        startPoint
                            ->getReferencedNetworkNodeID();
                    QString endNodeID =
                        endPoint
                            ->getReferencedNetworkNodeID();

                    // Skip if same node (no path)
                    if (startNodeID == endNodeID)
                    {
                        continue;
                    }

                    // Get the network
                    auto network =
                        startPoint->getReferenceNetwork();
                    if (!network)
                    {
                        continue;
                    }

                    // Check if it's a train network
                    auto trainNetwork = dynamic_cast<
                        Backend::TrainClient::
                            NeTrainSimNetwork *>(network);
                    if (!trainNetwork)
                    {
                        continue;
                    }

                    int  startIDint = startNodeID.toInt();
                    int  endIDint   = endNodeID.toInt();
                    auto startNode =
                        trainNetwork->getNodeByID(
                            startIDint);
                    auto endNode =
                        trainNetwork->getNodeByID(endIDint);
                    if (!startNode || !endIDint)
                    {
                        continue;
                    }

                    QString startNodeUniqueID =
                        startNode->getInternalUniqueID();
                    QString endNodeUniqueID =
                        endNode->getInternalUniqueID();

                    // Store the network for later use
                    QString networkName =
                        trainNetwork->getNetworkName();
                    trainNetworks[networkName] =
                        trainNetwork;

                    // Make a copy of the containers to
                    // distribute to trains
                    QList<ContainerCore::Container *>
                        containersCopy = containers;

                    // Create trains based on number of
                    // containers
                    int numTrains =
                        (containersCopy.size()
                         + trainContainerCount - 1)
                        / trainContainerCount;
                    numTrains = qMax(
                        1, numTrains); // At least one train

                    for (int i = 0; i < numTrains; i++)
                    {
                        // Create a new train
                        QString trainId =
                            QString("%1_%2")
                                .arg(path->getPathId())
                                .arg(trainCounter++);
                        Backend::Train *train =
                            vehicleController
                                ->getRandomTrain()
                                ->copy();
                        train->setUserId(trainId);

                        // Set the train's path
                        QList<int> trainPath;
                        trainPath.append(
                            startNodeID.toInt());
                        trainPath.append(endNodeID.toInt());
                        train->setTrainPathOnNodeIds(
                            trainPath);

                        // Add loading time offset for each
                        // train
                        train->setLoadTime(
                            trainCounter
                            * 100); // 100 seconds between
                                    // trains

                        // Assign containers to this train
                        QList<ContainerCore::Container *>
                            trainContainers;
                        int containersToAdd =
                            qMin(trainContainerCount,
                                 containersCopy.size());

                        for (int j = 0; j < containersToAdd;
                             j++)
                        {
                            if (!containersCopy.isEmpty())
                            {
                                // Get a container and make
                                // a copy with unique ID
                                ContainerCore::Container
                                    *originalContainer =
                                        containersCopy
                                            .takeFirst();
                                ContainerCore::Container
                                    *containerCopy =
                                        originalContainer
                                            ->copy();

                                // Update the container's ID
                                // to make it unique
                                QString newId =
                                    QString("%1_%2_%3")
                                        .arg(
                                            path->getPathId())
                                        .arg(
                                            originalContainer
                                                ->getContainerID())
                                        .arg(
                                            containerCounter++);
                                containerCopy
                                    ->setContainerID(newId);

                                containerCopy
                                    ->setContainerCurrentLocation(
                                        startNodeID);
                                containerCopy
                                    ->addDestination(
                                        endNodeID);

                                trainContainers.append(
                                    containerCopy);
                            }
                        }

                        // Add the train data to our
                        // simulation data structure
                        TrainSimData trainData;
                        trainData.train = train;
                        trainData.containers =
                            trainContainers;

                        if (!trainSimulationData.contains(
                                networkName))
                        {
                            trainSimulationData
                                [networkName] =
                                    QList<TrainSimData>();
                        }

                        trainSimulationData[networkName]
                            .append(trainData);
                    }
                }
            }

            // Check for Truck connectivity
            else if (segment->getMode()
                     == Backend::TransportationTypes::
                         TransportationMode::Truck)
            {
                // Similar implementation for Truck networks
                QList<MapPoint *> startNetworkPoints =
                    UtilitiesFunctions::
                        getMapPointsOfTerminal(
                            mainWindow->regionScene_,
                            startTerminal, "*", "*",
                            NetworkType::Truck);
                QList<MapPoint *> endNetworkPoints =
                    UtilitiesFunctions::
                        getMapPointsOfTerminal(
                            mainWindow->regionScene_,
                            endTerminal, "*", "*",
                            NetworkType::Truck);

                // Find common networks between the
                // terminals
                auto commonNets = UtilitiesFunctions::
                    getCommonNetworksOfNetworkType(
                        startNetworkPoints,
                        endNetworkPoints,
                        NetworkType::Truck);

                for (auto networkPair : commonNets)
                {
                    MapPoint *startPoint =
                        networkPair.first;
                    MapPoint *endPoint = networkPair.second;

                    QString startNodeID =
                        startPoint
                            ->getReferencedNetworkNodeID();
                    QString endNodeID =
                        endPoint
                            ->getReferencedNetworkNodeID();

                    if (startNodeID == endNodeID)
                    {
                        continue;
                    }

                    // Get the network
                    auto network =
                        startPoint->getReferenceNetwork();
                    if (!network)
                    {
                        continue;
                    }

                    // Check if it's a truck network
                    auto truckNetwork = dynamic_cast<
                        Backend::TruckClient::
                            IntegrationNetwork *>(network);
                    if (!truckNetwork)
                    {
                        continue;
                    }

                    // Store the network for later use
                    QString networkName =
                        truckNetwork->getNetworkName();
                    truckNetworks[networkName] =
                        truckNetwork;

                    // Make a copy of the containers to
                    // distribute to trucks
                    QList<ContainerCore::Container *>
                        containersCopy = containers;

                    // Create truck trips based on number of
                    // containers
                    int numTrucks =
                        (containersCopy.size()
                         + truckContainerCount - 1)
                        / truckContainerCount;
                    numTrucks = qMax(
                        1, numTrucks); // At least one truck

                    for (int i = 0; i < numTrucks; i++)
                    {
                        // Create a new truck trip
                        QString tripId =
                            QString("%1_%2")
                                .arg(path->getPathId())
                                .arg(truckCounter++);

                        // Assign containers to this truck
                        QList<ContainerCore::Container *>
                            truckContainers;
                        int containersToAdd =
                            qMin(truckContainerCount,
                                 containersCopy.size());

                        for (int j = 0; j < containersToAdd;
                             j++)
                        {
                            if (!containersCopy.isEmpty())
                            {
                                // Get a container and make
                                // a copy with unique ID
                                ContainerCore::Container
                                    *originalContainer =
                                        containersCopy
                                            .takeFirst();
                                ContainerCore::Container
                                    *containerCopy =
                                        originalContainer
                                            ->copy();

                                // Update the container's ID
                                // to make it unique
                                QString newId =
                                    QString("%1_%2_%3")
                                        .arg(
                                            path->getPathId())
                                        .arg(
                                            originalContainer
                                                ->getContainerID())
                                        .arg(
                                            containerCounter++);
                                containerCopy
                                    ->setContainerID(newId);

                                // Update origin and
                                // destinations
                                containerCopy
                                    ->setContainerCurrentLocation(
                                        startTerminal
                                            ->getProperty(
                                                "Name")
                                            .toString());
                                containerCopy
                                    ->addDestination(
                                        endTerminal
                                            ->getProperty(
                                                "Name")
                                            .toString());

                                truckContainers.append(
                                    containerCopy);
                            }
                        }

                        // Add the truck data to our
                        // simulation data structure
                        TruckSimData truckData;
                        truckData.tripId = tripId;
                        truckData.originNode =
                            startNodeID.toInt();
                        truckData.destinationNode =
                            endNodeID.toInt();
                        truckData.containers =
                            truckContainers;

                        if (!truckSimulationData.contains(
                                networkName))
                        {
                            truckSimulationData
                                [networkName] =
                                    QList<TruckSimData>();
                        }

                        truckSimulationData[networkName]
                            .append(truckData);
                    }
                }
            }

            // Check for Ship connectivity
            else if (segment->getMode()
                     == Backend::TransportationTypes::
                         TransportationMode::Ship)
            {
                // Get global positions of terminals
                QPointF startGlobalPos;
                QPointF endGlobalPos;

                // Get global terminal items if they exist
                GlobalTerminalItem *startGlobalItem =
                    startTerminal->getGlobalTerminalItem();
                GlobalTerminalItem *endGlobalItem =
                    endTerminal->getGlobalTerminalItem();

                if (startGlobalItem && endGlobalItem)
                {
                    startGlobalPos =
                        mainWindow->globalMapView_
                            ->sceneToWGS84(
                                startGlobalItem->pos());
                    endGlobalPos =
                        mainWindow->globalMapView_
                            ->sceneToWGS84(
                                endGlobalItem->pos());

                    // Define network name - either same
                    // region or different regions
                    QString networkName;
                    if (startTerminal->getRegion()
                        == endTerminal->getRegion())
                    {
                        networkName =
                            startTerminal->getRegion();
                    }
                    else
                    {
                        networkName =
                            startTerminal->getRegion()
                            + "_to_"
                            + endTerminal->getRegion();
                    }

                    // Make a copy of the containers to
                    // distribute to ships
                    QList<ContainerCore::Container *>
                        containersCopy = containers;

                    // Create ships based on number of
                    // containers
                    int numShips =
                        (containersCopy.size()
                         + shipContainerCount - 1)
                        / shipContainerCount;
                    numShips = qMax(
                        1, numShips); // At least one ship

                    for (int i = 0; i < numShips; i++)
                    {
                        // Create a new ship
                        QString shipId =
                            QString("%1_%2")
                                .arg(path->getPathId())
                                .arg(shipCounter++);
                        Backend::Ship *ship =
                            vehicleController
                                ->getRandomShip()
                                ->copy();
                        ship->setUserId(shipId);

                        // Set the ship's path
                        QList<QPointF> shipPath;
                        shipPath.append(startGlobalPos);
                        shipPath.append(endGlobalPos);
                        ship->setPathCoordinates(shipPath);

                        // Assign containers to this ship
                        QList<ContainerCore::Container *>
                            shipContainers;
                        int containersToAdd =
                            qMin(shipContainerCount,
                                 containersCopy.size());

                        for (int j = 0; j < containersToAdd;
                             j++)
                        {
                            if (!containersCopy.isEmpty())
                            {
                                // Get a container and make
                                // a copy with unique ID
                                ContainerCore::Container
                                    *originalContainer =
                                        containersCopy
                                            .takeFirst();
                                ContainerCore::Container
                                    *containerCopy =
                                        originalContainer
                                            ->copy();

                                // Update the container's ID
                                // to make it unique
                                QString newId =
                                    QString("%1_%2_%3")
                                        .arg(
                                            path->getPathId())
                                        .arg(
                                            originalContainer
                                                ->getContainerID())
                                        .arg(
                                            containerCounter++);
                                containerCopy
                                    ->setContainerID(newId);

                                // Update origin and
                                // destinations
                                containerCopy
                                    ->setContainerCurrentLocation(
                                        startTerminal
                                            ->getID());
                                containerCopy
                                    ->addDestination(
                                        endTerminal
                                            ->getID());

                                shipContainers.append(
                                    containerCopy);
                            }
                        }

                        // Add the ship data to our
                        // simulation data structure
                        ShipSimData shipData;
                        shipData.ship = ship;
                        shipData.containers =
                            shipContainers;
                        shipData.destinationTerminal =
                            endTerminal->getID();

                        if (!shipSimulationData.contains(
                                networkName))
                        {
                            shipSimulationData
                                [networkName] =
                                    QList<ShipSimData>();
                        }

                        shipSimulationData[networkName]
                            .append(shipData);
                    }
                }
            }
        }
    }

    return true;
}

bool SimulationValidationWorker::runSimulations(
    const QMap<QString, QList<ShipSimData>>
        &shipSimulationData,
    const QMap<QString, QList<TrainSimData>>
        &trainSimulationData,
    const QMap<QString, QList<TruckSimData>>
        &truckSimulationData,
    const QMap<QString,
               Backend::TrainClient::NeTrainSimNetwork *>
        &trainNetworks,
    const QMap<QString,
               Backend::TruckClient::IntegrationNetwork *>
        &truckNetworks)
{
    // Get the simulation clients
    auto shipClient =
        CargoNetSim::CargoNetSimController::getInstance()
            .getShipClient();
    auto trainClient =
        CargoNetSim::CargoNetSimController::getInstance()
            .getTrainClient();
    auto truckClient =
        CargoNetSim::CargoNetSimController::getInstance()
            .getTruckManager();

    // Reset the simulation servers
    if (!shipSimulationData.isEmpty())
    {
        if (shipClient->getRabbitMQHandler()
                ->hasCommandQueueConsumers())
        {
            shipClient->resetServer();
            emit statusMessage(
                "Setting up ship simulations...");
        }
        else
        {
            emit errorMessage(
                "Ship client is not connected to RabbitMQ");
            return false;
        }
    }
    if (!trainSimulationData.isEmpty())
    {
        if (trainClient->getRabbitMQHandler()
                ->hasCommandQueueConsumers())
        {
            trainClient->resetServer();
            emit statusMessage(
                "Setting up train simulations...");
        }
        else
        {
            emit errorMessage("Train client is not "
                              "connected to RabbitMQ");
            return false;
        }
    }
    if (!truckSimulationData.isEmpty())
    {
        truckClient->resetServer(); // Reset all truck
                                    // client instances
        emit statusMessage(
            "Setting up truck simulations...");
    }

    // Setup train simulations
    for (auto networkName : trainSimulationData.keys())
    {
        auto trainNetwork = trainNetworks[networkName];
        auto trainDataList =
            trainSimulationData[networkName];

        // Collect all trains for this network
        QList<Backend::Train *> trains;
        for (const auto &trainData : trainDataList)
        {
            trains.append(trainData.train);
        }

        // Define the simulator with all trains
        trainClient->defineSimulator(trainNetwork, 1.0,
                                     trains);

        // Add containers to each train
        for (const auto &trainData : trainDataList)
        {
            if (!trainData.containers.isEmpty())
            {
                trainClient->addContainersToTrain(
                    networkName,
                    trainData.train->getUserId(),
                    trainData.containers);
            }
        }
    }

    // Setup ship simulations
    for (auto networkName : shipSimulationData.keys())
    {
        auto shipDataList = shipSimulationData[networkName];

        // Collect all ships and their destination terminals
        QList<Backend::Ship *>     ships;
        QMap<QString, QStringList> destinationTerminals;

        for (const auto &shipData : shipDataList)
        {
            ships.append(shipData.ship);
            destinationTerminals[shipData.ship
                                     ->getUserId()] =
                QStringList{shipData.destinationTerminal};
        }

        // Define the simulator with all ships
        shipClient->defineSimulator(
            networkName,
            1.0, // timeStep
            ships, destinationTerminals,
            "" // networkPath - not used in this case
        );

        // Add containers to each ship
        for (const auto &shipData : shipDataList)
        {
            if (!shipData.containers.isEmpty())
            {
                shipClient->addContainersToShip(
                    networkName, shipData.ship->getUserId(),
                    shipData.containers);
            }
        }
    }

    // Setup truck simulations
    for (auto networkName : truckSimulationData.keys())
    {
        auto truckDataList =
            truckSimulationData[networkName];
        auto truckNetwork = truckNetworks[networkName];

        // Create a client configuration
        Backend::TruckClient::ClientConfiguration config;
        config.masterFilePath =
            ""; // This needs to be filled with the correct
                // path
        config.simTime = 3600.0; // 1 hour simulation time

        // Create a truck client for this network
        truckClient->createClient(networkName, config);

        // Get the created client
        auto client = truckClient->getClient(networkName);
        if (!client)
        {
            emit errorMessage(
                QString("Failed to create truck client for "
                        "network %1")
                    .arg(networkName));
            continue;
        }

        // Add trips for each truck
        for (const auto &truckData : truckDataList)
        {
            // Add a trip
            QString tripId = client->addTrip(
                networkName,
                QString::number(truckData.originNode),
                QString::number(truckData.destinationNode),
                truckData.containers);
        }
    }

    // Run the train simulations
    if (!trainSimulationData.isEmpty())
    {
        emit statusMessage("Running train simulations...");
        trainClient->runSimulator(
            trainSimulationData.keys());
    }

    // Run the ship simulations
    if (!shipSimulationData.isEmpty())
    {
        emit statusMessage("Running ship simulations...");
        shipClient->runSimulator(shipSimulationData.keys());
    }

    // Run the truck simulations
    if (!truckSimulationData.isEmpty())
    {
        emit statusMessage("Running truck simulations...");
        truckClient->runSimulationAsync(
            truckSimulationData.keys());
    }

    emit statusMessage(
        "All simulations started successfully!");
    return true;
}

void SimulationValidationWorker::extractResults()
{
    // Get the simulation clients and configuration
    auto shipClient =
        CargoNetSim::CargoNetSimController::getInstance()
            .getShipClient();
    auto trainClient =
        CargoNetSim::CargoNetSimController::getInstance()
            .getTrainClient();
    auto truckClient =
        CargoNetSim::CargoNetSimController::getInstance()
            .getTruckManager();
    auto configController =
        CargoNetSim::CargoNetSimController::getInstance()
            .getConfigController();

    // Get cost function weights and transport modes
    QVariantMap costFunctionWeights =
        configController->getCostFunctionWeights();
    QVariantMap transportModes =
        configController->getTransportModes();

    // Get the selected paths for simulation validation
    auto selectedPathsData = mainWindow->shortestPathTable_
                                 ->getCheckedPathData();

    emit statusMessage("Extracting simulation results...");

    // Process each selected path
    for (auto pathData : selectedPathsData)
    {
        Backend::Path *path = pathData->path;
        if (!path)
        {
            continue;
        }

        // Storage for accumulated costs
        double totalPathCost      = 0.0;
        double totalEdgeCosts     = 0.0;
        double totalTerminalCosts = 0.0;

        // Get segments from the path
        QList<Backend::PathSegment *> segments =
            path->getSegments();
        QList<Backend::Terminal *> terminals =
            path->getTerminalsInPath();

        // Get container count
        int containerCount = getContainerCount(mainWindow);
        if (containerCount == 0)
        {
            emit errorMessage(
                "No containers at origin terminal!");
            continue;
        }

        // Process edge costs for each segment
        totalEdgeCosts = calculateEdgeCosts(
            path, segments, costFunctionWeights,
            transportModes, shipClient, trainClient,
            truckClient, containerCount);

        // Process terminal costs
        totalTerminalCosts = calculateTerminalCosts(
            segments, terminals, costFunctionWeights,
            containerCount);

        // Calculate total path cost
        totalPathCost = totalEdgeCosts + totalTerminalCosts;

        // Update the path with simulation costs
        int pathId = path->getPathId();
        mainWindow->shortestPathTable_
            ->updateSimulationCosts(pathId, totalPathCost,
                                    totalEdgeCosts,
                                    totalTerminalCosts);

        emit statusMessage(
            QString("Path %1 simulation cost: $%2 (edges: "
                    "$%3, terminals: $%4)")
                .arg(pathId)
                .arg(totalPathCost, 0, 'f', 2)
                .arg(totalEdgeCosts, 0, 'f', 2)
                .arg(totalTerminalCosts, 0, 'f', 2));
    }

    emit statusMessage(
        "Results extraction completed successfully");
}

int SimulationValidationWorker::getContainerCount(
    MainWindow *mainWindow)
{
    // Get containers from origin terminal to determine
    // count
    auto originTerminal =
        UtilitiesFunctions::getOriginTerminal(mainWindow);
    int containerCount = 0;

    if (originTerminal)
    {
        QVariant containersVar =
            originTerminal->getProperty("Containers");
        if (containersVar.canConvert<
                QList<ContainerCore::Container *>>())
        {
            QList<ContainerCore::Container *> containers =
                containersVar.value<
                    QList<ContainerCore::Container *>>();
            containerCount = containers.size();
        }
    }

    return containerCount;
}

double SimulationValidationWorker::calculateEdgeCosts(
    Backend::Path                       *path,
    const QList<Backend::PathSegment *> &segments,
    const QVariantMap &costFunctionWeights,
    const QVariantMap &transportModes,
    Backend::ShipClient::ShipSimulationClient *shipClient,
    Backend::TrainClient::TrainSimulationClient
        *trainClient,
    Backend::TruckClient::TruckSimulationManager
        *truckClient,
    int  containerCount)
{
    double totalEdgeCosts = 0.0;

    // Process each segment in the path
    for (auto segment : segments)
    {
        if (!segment)
        {
            continue;
        }

        QString startID = segment->getStart();
        QString endID   = segment->getEnd();
        Backend::TransportationTypes::TransportationMode
            mode = segment->getMode();

        // Get the mode-specific weights
        QVariantMap modeWeights;
        QString     modeKey =
            QString::number(static_cast<int>(mode));
        if (costFunctionWeights.contains(modeKey))
        {
            modeWeights =
                costFunctionWeights[modeKey].toMap();
        }
        else
        {
            // Fallback to default weights
            modeWeights =
                costFunctionWeights["default"].toMap();
        }

        // Calculate costs based on mode
        double segmentCost = 0.0;

        if (mode
            == Backend::TransportationTypes::
                TransportationMode::Ship)
        {
            segmentCost = calculateShipSegmentCost(
                path, shipClient, modeWeights,
                transportModes, containerCount);
        }
        else if (mode
                 == Backend::TransportationTypes::
                     TransportationMode::Train)
        {
            segmentCost = calculateTrainSegmentCost(
                path, trainClient, modeWeights,
                transportModes, containerCount);
        }
        else if (mode
                 == Backend::TransportationTypes::
                     TransportationMode::Truck)
        {
            segmentCost = calculateTruckSegmentCost(
                path, truckClient, modeWeights,
                transportModes, containerCount);
        }

        // Add to total edge costs
        totalEdgeCosts += segmentCost;
    }

    return totalEdgeCosts;
}

double SimulationValidationWorker::calculateShipSegmentCost(
    Backend::Path                             *path,
    Backend::ShipClient::ShipSimulationClient *shipClient,
    const QVariantMap                         &modeWeights,
    const QVariantMap &transportModes, int containerCount)
{
    // Extract ship simulation results
    double travelTime        = 0.0;
    double distance          = 0.0;
    double carbonEmissions   = 0.0;
    double energyConsumption = 0.0;
    double risk              = 0.0;
    int    shipCount         = 0;

    // Get all ship states from the client
    auto shipStates = shipClient->getAllShipsStates();

    // Find ships for this path segment
    for (auto networkName : shipStates.keys())
    {
        for (auto shipState : shipStates[networkName])
        {
            if (shipState)
            {
                // Check if this ship is part of our path
                QString shipId = shipState->shipId();
                if (shipId.startsWith(QString("%1_").arg(
                        path->getPathId())))
                {
                    // Count this ship
                    shipCount++;

                    // Extract metrics
                    travelTime += shipState->tripTime();
                    distance +=
                        shipState->travelledDistance();
                    carbonEmissions +=
                        shipState->carbonEmissions();
                    energyConsumption +=
                        shipState->energyConsumption();

                    // Use risk factor from transportModes
                    // config
                    QVariantMap shipData =
                        transportModes.value("ship")
                            .toMap();
                    risk +=
                        shipData.value("risk_factor", 0.025)
                            .toDouble();
                }
            }
        }
    }

    // If no ships found, return 0 cost
    if (shipCount == 0)
    {
        return 0.0;
    }

    // Calculate containers per ship
    double containersPerShip =
        (double)containerCount / shipCount;

    // Get ship capacity from transport modes
    QVariantMap shipData =
        transportModes.value("ship").toMap();
    int shipCapacity =
        shipData.value("average_container_number", 10000)
            .toInt();

    // Calculate container-to-capacity ratio (how full is
    // each ship)
    double containerToCapacityRatio =
        containersPerShip / shipCapacity;

    // Adjust metrics by ratio
    carbonEmissions *= containerToCapacityRatio;
    energyConsumption *= containerToCapacityRatio;
    risk *= containerToCapacityRatio;

    // Apply weights to metrics
    double segmentCost = 0.0;
    segmentCost += travelTime
                   * modeWeights["travelTime"].toDouble()
                   / 3600.0; // sec to hr
    segmentCost += distance
                   * modeWeights["distance"].toDouble()
                   / 1000.0; // m to km
    segmentCost +=
        carbonEmissions
        * modeWeights["carbonEmissions"].toDouble();
    segmentCost +=
        energyConsumption
        * modeWeights["energyConsumption"].toDouble();
    segmentCost += risk * modeWeights["risk"].toDouble();

    return segmentCost;
}

double
SimulationValidationWorker::calculateTrainSegmentCost(
    Backend::Path *path,
    Backend::TrainClient::TrainSimulationClient
                      *trainClient,
    const QVariantMap &modeWeights,
    const QVariantMap &transportModes, int containerCount)
{
    // Extract train simulation results
    double travelTime        = 0.0;
    double distance          = 0.0;
    double carbonEmissions   = 0.0;
    double energyConsumption = 0.0;
    double risk              = 0.0;
    int    trainCount        = 0;

    // Get all train states from the client
    auto trainStates = trainClient->getAllTrainsStates();

    // Find trains for this path segment
    for (auto networkName : trainStates.keys())
    {
        for (auto trainState : trainStates[networkName])
        {
            if (trainState)
            {
                // Check if this train is part of our path
                QString trainId = trainState->m_trainUserId;
                if (trainId.startsWith(QString("%1_").arg(
                        path->getPathId())))
                {
                    // Count this train
                    trainCount++;

                    // Extract metrics
                    travelTime += trainState->m_tripTime;
                    distance +=
                        trainState->m_travelledDistance;
                    carbonEmissions +=
                        trainState
                            ->m_totalCarbonDioxideEmitted;
                    energyConsumption +=
                        trainState->m_totalEnergyConsumed;

                    // Use risk factor from transportModes
                    // config
                    QVariantMap trainData =
                        transportModes.value("rail")
                            .toMap();
                    risk += trainData
                                .value("risk_factor", 0.006)
                                .toDouble();
                }
            }
        }
    }

    // If no trains found, return 0 cost
    if (trainCount == 0)
    {
        return 0.0;
    }

    // Calculate containers per train
    double containersPerTrain =
        (double)containerCount / trainCount;

    // Get train capacity from transport modes
    QVariantMap trainData =
        transportModes.value("rail").toMap();
    int trainCapacity =
        trainData.value("average_container_number", 400)
            .toInt();

    // Calculate container-to-capacity ratio (how full is
    // each train)
    double containerToCapacityRatio =
        containersPerTrain / trainCapacity;

    // Adjust metrics by ratio
    carbonEmissions *= containerToCapacityRatio;
    energyConsumption *= containerToCapacityRatio;
    risk *= containerToCapacityRatio;

    // Apply weights to metrics
    double segmentCost = 0.0;
    segmentCost += travelTime
                   * modeWeights["travelTime"].toDouble()
                   / 3600.0; // sec to hr
    segmentCost += distance
                   * modeWeights["distance"].toDouble()
                   / 1000.0; // m to km
    segmentCost +=
        carbonEmissions
        * modeWeights["carbonEmissions"].toDouble(); // kg
    segmentCost += energyConsumption
                   * modeWeights["energyConsumption"]
                         .toDouble(); // kWh
    segmentCost += risk * modeWeights["risk"].toDouble();

    return segmentCost;
}

double
SimulationValidationWorker::calculateTruckSegmentCost(
    Backend::Path *path,
    Backend::TruckClient::TruckSimulationManager
                      *truckClient,
    const QVariantMap &modeWeights,
    const QVariantMap &transportModes, int containerCount)
{
    // Extract truck simulation results
    double travelTime        = 0.0;
    double distance          = 0.0;
    double carbonEmissions   = 0.0;
    double energyConsumption = 0.0;
    double risk              = 0.0;
    int    truckCount        = 0;

    // Get truck simulation data - this would be implemented
    // based on truck client API For now, we'll use a
    // placeholder approach

    // If you had actual truck simulation results, you would
    // count them like this: for (auto networkName :
    // truckClient->getNetworkNames())
    // {
    //     auto client =
    //     truckClient->getClient(networkName); if (client)
    //     {
    //         auto results = client->getResults();
    //         for (auto tripId : results.keys())
    //         {
    //             if
    //             (tripId.startsWith(QString("%1_").arg(path->getPathId())))
    //             {
    //                 truckCount++;
    //                 auto tripResult = results[tripId];
    //                 travelTime += tripResult.travelTime;
    //                 distance += tripResult.distance;
    //                 carbonEmissions +=
    //                 tripResult.carbonEmissions;
    //                 energyConsumption +=
    //                 tripResult.energyConsumption;
    //             }
    //         }
    //     }
    // }

    // For now, estimate truck count based on container
    // count and capacity
    QVariantMap truckData =
        transportModes.value("truck").toMap();
    int truckCapacity =
        truckData.value("average_container_number", 1)
            .toInt();
    truckCount = (containerCount + truckCapacity - 1)
                 / truckCapacity; // Ceiling division

    // Get truck risk factor
    risk = truckData.value("risk_factor", 0.012).toDouble();

    // Calculate container-to-capacity ratio (usually 1.0
    // for trucks)
    double containerToCapacityRatio = 1.0;
    if (truckCount > 0)
    {
        double containersPerTruck =
            (double)containerCount / truckCount;
        containerToCapacityRatio =
            containersPerTruck / truckCapacity;
    }

    // Adjust risk by container-to-capacity ratio
    risk *= containerToCapacityRatio;

    // Apply weights to metrics
    double segmentCost = 0.0;
    segmentCost +=
        travelTime * modeWeights["travelTime"].toDouble();
    segmentCost +=
        distance * modeWeights["distance"].toDouble();
    segmentCost +=
        carbonEmissions
        * modeWeights["carbonEmissions"].toDouble();
    segmentCost +=
        energyConsumption
        * modeWeights["energyConsumption"].toDouble();
    segmentCost += risk * modeWeights["risk"].toDouble();

    return segmentCost;
}

double SimulationValidationWorker::calculateTerminalCosts(
    const QList<Backend::PathSegment *> &segments,
    const QList<Backend::Terminal *>    &terminals,
    const QVariantMap &costFunctionWeights,
    int                containerCount)
{
    double totalTerminalCosts = 0.0;

    // Process terminal costs - only for intermediate
    // terminals where mode changes Skip first (origin) and
    // last (destination) terminals
    for (int i = 1; i < terminals.size() - 1; i++)
    {
        // Check if this terminal represents a mode change
        if (i > 0 && i < segments.size())
        {
            Backend::TransportationTypes::TransportationMode
                prevMode = segments[i - 1]->getMode();
            Backend::TransportationTypes::TransportationMode
                nextMode = segments[i]->getMode();

            // If modes are different, include terminal
            // costs
            if (prevMode != nextMode)
            {
                double terminalCost =
                    calculateSingleTerminalCost(
                        terminals[i], costFunctionWeights,
                        containerCount);
                totalTerminalCosts += terminalCost;
            }
        }
    }

    return totalTerminalCosts;
}

double
SimulationValidationWorker::calculateSingleTerminalCost(
    Backend::Terminal *terminal,
    const QVariantMap &costFunctionWeights,
    int                containerCount)
{
    if (!terminal)
    {
        return 0.0;
    }

    // Get terminal properties from the terminal object
    QJsonObject config = terminal->getConfig();

    // Get default weights
    QVariantMap defaultWeights =
        costFunctionWeights["default"].toMap();

    // Process terminal costs - per container
    double terminalDelayPerContainer = 0.0; // Hours
    double terminalCostPerContainer  = 0.0; // Direct cost
    bool   customsApplied            = false;

    // Calculate dwell time
    terminalDelayPerContainer =
        calculateTerminalDwellTime(config);

    // Calculate customs costs and delays
    double customsDelay = 0.0;
    double customsCost  = 0.0;
    customsApplied      = calculateTerminalCustoms(
        config, customsDelay, customsCost);

    terminalDelayPerContainer += customsDelay;
    terminalCostPerContainer += customsCost;

    // Extract cost configuration
    terminalCostPerContainer +=
        calculateTerminalDirectCosts(config,
                                     customsApplied);

    // Calculate total terminal costs for all containers
    double totalTerminalDelay =
        terminalDelayPerContainer * containerCount;
    double totalTerminalDirectCost =
        terminalCostPerContainer * containerCount;

    // Apply weights to get the final terminal cost
    double terminalTotalCost =
        (totalTerminalDelay
         * defaultWeights["terminal_delay"].toDouble())
        + (totalTerminalDirectCost
           * defaultWeights["terminal_cost"].toDouble());

    return terminalTotalCost;
}

double
SimulationValidationWorker::calculateTerminalDwellTime(
    const QJsonObject &config)
{
    double terminalDelay = 0.0;

    // Extract dwell time configuration
    if (config.contains("dwell_time"))
    {
        QJsonObject dwellTimeObj =
            config["dwell_time"].toObject();
        QString method =
            dwellTimeObj["method"].toString("gamma");
        QJsonObject paramsObj =
            dwellTimeObj["parameters"].toObject();

        // Convert JSON parameters to QVariantMap
        QVariantMap dwellParams;
        for (auto it = paramsObj.constBegin();
             it != paramsObj.constEnd(); ++it)
        {
            dwellParams[it.key()] = it.value().toDouble();
        }

        // Use the mean/expected value for each distribution
        if (method.compare("gamma", Qt::CaseInsensitive)
            == 0)
        {
            double shape =
                dwellParams.value("shape", 2.0).toDouble();
            double scale =
                dwellParams.value("scale", 24.0 * 3600.0)
                    .toDouble();
            terminalDelay =
                shape * scale
                / 3600.0; // Convert seconds to hours
        }
        else if (method.compare("exponential",
                                Qt::CaseInsensitive)
                 == 0)
        {
            double scale =
                dwellParams
                    .value("scale", 2.0 * 24.0 * 3600.0)
                    .toDouble();
            terminalDelay =
                scale / 3600.0; // Convert seconds to hours
        }
        else if (method.compare("normal",
                                Qt::CaseInsensitive)
                 == 0)
        {
            double mean =
                dwellParams
                    .value("mean", 2.0 * 24.0 * 3600.0)
                    .toDouble();
            terminalDelay =
                mean / 3600.0; // Convert seconds to hours
        }
        else if (method.compare("lognormal",
                                Qt::CaseInsensitive)
                 == 0)
        {
            double mean =
                dwellParams
                    .value("mean",
                           std::log(2.0 * 24.0 * 3600.0))
                    .toDouble();
            double sigma =
                dwellParams.value("sigma", 0.25).toDouble();
            terminalDelay =
                std::exp(mean + sigma * sigma / 2.0)
                / 3600.0; // Convert seconds to hours
        }
        else
        {
            // Default to gamma distribution's expected
            // value
            double shape = 2.0;
            double scale = 24.0 * 3600.0;
            terminalDelay =
                shape * scale
                / 3600.0; // Convert seconds to hours
        }
    }

    return terminalDelay;
}

bool SimulationValidationWorker::calculateTerminalCustoms(
    const QJsonObject &config, double &customsDelay,
    double &customsCost)
{
    bool customsApplied = false;

    // Extract customs processing parameters
    if (config.contains("customs"))
    {
        QJsonObject customsObj =
            config["customs"].toObject();

        double probability =
            customsObj["probability"].toDouble(0.0);
        double delayMean =
            customsObj["delay_mean"].toDouble(0.0);

        // Apply expected customs delay (probability * mean
        // delay)
        if (probability > 0.0 && delayMean > 0.0)
        {
            customsDelay   = probability * delayMean;
            customsApplied = true;
        }
    }

    return customsApplied;
}

double
SimulationValidationWorker::calculateTerminalDirectCosts(
    const QJsonObject &config, bool customsApplied)
{
    double terminalCost = 0.0;

    // Extract cost configuration
    if (config.contains("cost"))
    {
        QJsonObject costObj = config["cost"].toObject();

        // Add fixed cost
        if (costObj.contains("fixed_fees"))
        {
            terminalCost +=
                costObj["fixed_fees"].toDouble(0.0);
        }

        // Add customs cost if applicable based on
        // probability
        if (customsApplied
            && costObj.contains("customs_fees"))
        {
            terminalCost +=
                costObj["customs_fees"].toDouble(0.0);
        }

        // Risk factor calculation would require container
        // value For simplicity, we'll use a nominal
        // container value of $1
        if (costObj.contains("risk_factor"))
        {
            double riskFactor =
                costObj["risk_factor"].toDouble(0.0);
            double nominalContainerValue =
                1.0; // Placeholder value
            terminalCost +=
                nominalContainerValue * riskFactor;
        }
    }

    return terminalCost;
}

} // namespace GUI
} // namespace CargoNetSim
