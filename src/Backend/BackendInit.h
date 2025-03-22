#pragma once

#include "Backend/Clients/BaseClient/RabbitMQHandler.h"
#include "Backend/Clients/BaseClient/SimulationClientBase.h"
#include <QtCore>

#include "Backend/Clients/ShipClient/ShipSimulationClient.h"
#include "Backend/Clients/ShipClient/ShipState.h"
#include "Backend/Clients/ShipClient/SimulationResults.h"
#include "Backend/Clients/ShipClient/SimulationSummaryData.h"

#include "Backend/Clients/TrainClient/SimulationResults.h"
#include "Backend/Clients/TrainClient/SimulationSummaryData.h"
#include "Backend/Clients/TrainClient/TrainNetwork.h"
#include "Backend/Clients/TrainClient/TrainSimulationClient.h"
#include "Backend/Clients/TrainClient/TrainState.h"

#include "Backend/Clients/TruckClient/IntegrationLink.h"
#include "Backend/Clients/TruckClient/IntegrationNode.h"
#include "Backend/Clients/TruckClient/SimulationResults.h"
#include "Backend/Clients/TruckClient/SimulationSummaryData.h"
#include "Backend/Clients/TruckClient/TruckNetwork.h"
#include "Backend/Clients/TruckClient/TruckSimulationClient.h"
#include "Backend/Clients/TruckClient/TruckSimulationManager.h"
#include "Backend/Clients/TruckClient/TruckState.h"

#include "Backend/Clients/TerminalClient/TerminalSimulationClient.h"

#include "Backend/Commons/ClientType.h"

namespace CargoNetSim {
namespace Backend {

/**
 * @brief Initializes backend components including metatype
 * registrations
 *
 * This function should be called once at application
 * startup before using any backend components, especially
 * those that use signals/slots across thread boundaries.
 */
inline void initializeBackend() {
    // Base classes
    qRegisterMetaType<RabbitMQHandler>(
        "CargoNetSim::Backend::RabbitMQHandler");
    qRegisterMetaType<RabbitMQHandler *>(
        "CargoNetSim::Backend::RabbitMQHandler*");
    qRegisterMetaType<SimulationClientBase>(
        "CargoNetSim::Backend::SimulationClientBase");
    qRegisterMetaType<SimulationClientBase *>(
        "CargoNetSim::Backend::SimulationClientBase*");

    // ShipClient classes
    qRegisterMetaType<ShipClient::ShipState>(
        "CargoNetSim::Backend::ShipClient::ShipState");
    qRegisterMetaType<ShipClient::ShipState *>(
        "CargoNetSim::Backend::ShipClient::ShipState*");
    qRegisterMetaType<ShipClient::SimulationSummaryData>(
        "CargoNetSim::Backend::ShipClient::"
        "SimulationSummaryData");
    qRegisterMetaType<ShipClient::SimulationSummaryData *>(
        "CargoNetSim::Backend::ShipClient::"
        "SimulationSummaryData*");
    qRegisterMetaType<ShipClient::SimulationResults>(
        "CargoNetSim::Backend::ShipClient::"
        "SimulationResults");
    qRegisterMetaType<ShipClient::SimulationResults *>(
        "CargoNetSim::Backend::ShipClient::"
        "SimulationResults*");
    qRegisterMetaType<ShipClient::ShipSimulationClient>(
        "CargoNetSim::Backend::ShipClient::"
        "ShipSimulationClient");
    qRegisterMetaType<ShipClient::ShipSimulationClient *>(
        "CargoNetSim::Backend::ShipClient::"
        "ShipSimulationClient*");

    // TrainClient classes
    qRegisterMetaType<TrainClient::TrainState>(
        "CargoNetSim::Backend::TrainClient::TrainState");
    qRegisterMetaType<TrainClient::TrainState *>(
        "CargoNetSim::Backend::TrainClient::TrainState*");
    qRegisterMetaType<TrainClient::SimulationSummaryData>(
        "CargoNetSim::Backend::TrainClient::"
        "SimulationSummaryData");
    qRegisterMetaType<TrainClient::SimulationSummaryData *>(
        "CargoNetSim::Backend::TrainClient::"
        "SimulationSummaryData*");
    qRegisterMetaType<TrainClient::SimulationResults>(
        "CargoNetSim::Backend::TrainClient::"
        "SimulationResults");
    qRegisterMetaType<TrainClient::SimulationResults *>(
        "CargoNetSim::Backend::TrainClient::"
        "SimulationResults*");
    qRegisterMetaType<TrainClient::TrainSimulationClient>(
        "CargoNetSim::Backend::TrainClient::"
        "TrainSimulationClient");
    qRegisterMetaType<TrainClient::TrainSimulationClient *>(
        "CargoNetSim::Backend::TrainClient::"
        "TrainSimulationClient*");
    qRegisterMetaType<TrainClient::NeTrainSimNetwork>(
        "CargoNetSim::Backend::TrainClient::"
        "NeTrainSimNetwork");
    qRegisterMetaType<TrainClient::NeTrainSimNetwork>(
        "CargoNetSim::Backend::TrainClient::"
        "NeTrainSimNetwork*");

    // TruckClient classes
    qRegisterMetaType<CargoNetSim::Backend::TruckClient::
                          AsyncTripManager>(
        "CargoNetSim::Backend::TruckClient::"
        "AsyncTripManager");
    qRegisterMetaType<
        CargoNetSim::Backend::TruckClient::AsyncTripManager
            *>("CargoNetSim::Backend::TruckClient::"
               "AsyncTripManager*");
    qRegisterMetaType<CargoNetSim::Backend::TruckClient::
                          ContainerManager>(
        "CargoNetSim::Backend::TruckClient::"
        "ContainerManager");
    qRegisterMetaType<
        CargoNetSim::Backend::TruckClient::ContainerManager
            *>("CargoNetSim::Backend::TruckClient::"
               "ContainerManager*");
    qRegisterMetaType<
        CargoNetSim::Backend::TruckClient::IntegrationNode>(
        "CargoNetSim::Backend::TruckClient::"
        "IntegrationNode");
    qRegisterMetaType<
        CargoNetSim::Backend::TruckClient::IntegrationNode
            *>("CargoNetSim::Backend::TruckClient::"
               "IntegrationNode*");
    qRegisterMetaType<
        CargoNetSim::Backend::TruckClient::IntegrationLink>(
        "CargoNetSim::Backend::TruckClient::"
        "IntegrationLink");
    qRegisterMetaType<
        CargoNetSim::Backend::TruckClient::IntegrationLink
            *>("CargoNetSim::Backend::TruckClient::"
               "IntegrationLink*");
    qRegisterMetaType<CargoNetSim::Backend::TruckClient::
                          MessageFormatter>(
        "CargoNetSim::Backend::TruckClient::"
        "MessageFormatter");
    qRegisterMetaType<
        CargoNetSim::Backend::TruckClient::MessageFormatter
            *>("CargoNetSim::Backend::TruckClient::"
               "MessageFormatter*");

    qRegisterMetaType<CargoNetSim::Backend::TruckClient::
                          TripEndCallbackManager>(
        "CargoNetSim::Backend::TruckClient::"
        "TripEndCallbackManager");
    qRegisterMetaType<CargoNetSim::Backend::TruckClient::
                          TripEndCallbackManager *>(
        "CargoNetSim::Backend::TruckClient::"
        "TripEndCallbackManager*");

    qRegisterMetaType<CargoNetSim::Backend::TruckClient::
                          IntegrationSimulationConfig>(
        "CargoNetSim::Backend::TruckClient::"
        "IntegrationSimulationConfig");
    qRegisterMetaType<CargoNetSim::Backend::TruckClient::
                          IntegrationSimulationConfig *>(
        "CargoNetSim::Backend::TruckClient::"
        "IntegrationSimulationConfig*");
    qRegisterMetaType<CargoNetSim::Backend::TruckClient::
                          TransportationGraph<int>>(
        "CargoNetSim::Backend::TruckClient::"
        "TransportationGraph<int>");
    qRegisterMetaType<CargoNetSim::Backend::TruckClient::
                          TransportationGraph<int> *>(
        "CargoNetSim::Backend::TruckClient::"
        "TransportationGraph<int>*");

    qRegisterMetaType<CargoNetSim::Backend::TruckClient::
                          IntegrationNetwork>(
        "CargoNetSim::Backend::TruckClient::"
        "IntegrationNetwork");
    qRegisterMetaType<CargoNetSim::Backend::TruckClient::
                          IntegrationNetwork *>(
        "CargoNetSim::Backend::TruckClient::"
        "IntegrationNetwork");

    qRegisterMetaType<CargoNetSim::Backend::TruckClient::
                          IntegrationSimulationConfig>(
        "CargoNetSim::Backend::TruckClient::"
        "IntegrationSimulationConfig");
    qRegisterMetaType<CargoNetSim::Backend::TruckClient::
                          IntegrationSimulationConfig *>(
        "CargoNetSim::Backend::TruckClient::"
        "IntegrationSimulationConfig");

    qRegisterMetaType<
        CargoNetSim::Backend::TruckClient::TripRequest>(
        "CargoNetSim::Backend::TruckClient::TripRequest");
    qRegisterMetaType<
        CargoNetSim::Backend::TruckClient::TripRequest *>(
        "CargoNetSim::Backend::TruckClient::TripRequest*");
    qRegisterMetaType<
        CargoNetSim::Backend::TruckClient::TripResult>(
        "CargoNetSim::Backend::TruckClient::TripResult");
    qRegisterMetaType<
        CargoNetSim::Backend::TruckClient::TripResult *>(
        "CargoNetSim::Backend::TruckClient::TripResult*");
    qRegisterMetaType<TruckClient::TruckState>(
        "CargoNetSim::Backend::TruckClient::TruckState");
    qRegisterMetaType<TruckClient::TruckState *>(
        "CargoNetSim::Backend::TruckClient::TruckState*");
    qRegisterMetaType<TruckClient::SimulationSummaryData>(
        "CargoNetSim::Backend::TruckClient::"
        "SimulationSummaryData");
    qRegisterMetaType<TruckClient::SimulationSummaryData *>(
        "CargoNetSim::Backend::TruckClient::"
        "SimulationSummaryData*");
    qRegisterMetaType<TruckClient::SimulationResults>(
        "CargoNetSim::Backend::TruckClient::"
        "SimulationResults");
    qRegisterMetaType<TruckClient::SimulationResults *>(
        "CargoNetSim::Backend::TruckClient::"
        "SimulationResults*");
    qRegisterMetaType<TruckClient::TruckSimulationClient>(
        "CargoNetSim::Backend::TruckClient::"
        "TruckSimulationClient");
    qRegisterMetaType<TruckClient::TruckSimulationClient *>(
        "CargoNetSim::Backend::TruckClient::"
        "TruckSimulationClient*");
    qRegisterMetaType<TruckClient::TruckSimulationManager>(
        "CargoNetSim::Backend::TruckClient::"
        "TruckSimulationManager");
    qRegisterMetaType<
        TruckClient::TruckSimulationManager *>(
        "CargoNetSim::Backend::TruckClient::"
        "TruckSimulationManager*");

    // TerminalClient
    qRegisterMetaType<TerminalSimulationClient>(
        "CargoNetSim::Backend::TerminalSimulationClient");

    // Register ClientType
    qRegisterMetaType<ClientType>(
        "CargoNetSim::Backend::ClientType");

    qDebug() << "Backend metatypes registered successfully";
}

} // namespace Backend
} // namespace CargoNetSim
