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
                                .arg(i + 1);
                        Backend::Train *train =
                            new Backend::Train(
                                vehicleController
                                    ->getRandomTrain());
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
                            i * 10); // 10 seconds between
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
                                        new ContainerCore::
                                            Container(
                                                *originalContainer);

                                // Update the container's ID
                                // to make it unique
                                QString newId =
                                    QString("%1_%2")
                                        .arg(
                                            path->getPathId())
                                        .arg(
                                            originalContainer
                                                ->getContainerID());
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
                                .arg(i + 1);

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
                                        new ContainerCore::
                                            Container(
                                                *originalContainer);

                                // Update the container's ID
                                // to make it unique
                                QString newId =
                                    QString("%1_%2")
                                        .arg(
                                            path->getPathId())
                                        .arg(
                                            originalContainer
                                                ->getContainerID());
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
                                .arg(i + 1);
                        Backend::Ship *ship =
                            new Backend::Ship(
                                vehicleController
                                    ->getRandomShip());
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
                                        new ContainerCore::
                                            Container(
                                                *originalContainer);

                                // Update the container's ID
                                // to make it unique
                                QString newId =
                                    QString("%1_%2")
                                        .arg(
                                            path->getPathId())
                                        .arg(
                                            originalContainer
                                                ->getContainerID());
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
    shipClient->resetServer();
    trainClient->resetServer();
    truckClient
        ->resetServer(); // Reset all truck client instances

    // Setup train simulations
    emit statusMessage("Setting up train simulations...");
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
    emit statusMessage("Setting up ship simulations...");
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
    emit statusMessage("Setting up truck simulations...");
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

} // namespace GUI
} // namespace CargoNetSim
