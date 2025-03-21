#pragma once

#include <QtCore>
#include "Backend/Clients/BaseClient/RabbitMQHandler.h"
#include "Backend/Clients/BaseClient/SimulationClientBase.h"

#include "Backend/Clients/ShipClient/ShipState.h"
#include "Backend/Clients/ShipClient/SimulationSummaryData.h"
#include "Backend/Clients/ShipClient/SimulationResults.h"
#include "Backend/Clients/ShipClient/ShipSimulationClient.h"

#include "Backend/Clients/TrainClient/TrainState.h"
#include "Backend/Clients/TrainClient/SimulationSummaryData.h"
#include "Backend/Clients/TrainClient/SimulationResults.h"
#include "Backend/Clients/TrainClient/TrainSimulationClient.h"

#include "Backend/Commons/ClientType.h"

namespace CargoNetSim {
namespace Backend {

/**
 * @brief Initializes backend components including metatype registrations
 * 
 * This function should be called once at application startup before
 * using any backend components, especially those that use signals/slots
 * across thread boundaries.
 */
inline void initializeBackend() {
    // Register ShipClient metatypes
    qRegisterMetaType<RabbitMQHandler>("CargoNetSim::Backend::RabbitMQHandler");
    qRegisterMetaType<RabbitMQHandler*>("CargoNetSim::Backend::RabbitMQHandler*");
    qRegisterMetaType<SimulationClientBase>("CargoNetSim::Backend::SimulationClientBase");
    qRegisterMetaType<SimulationClientBase*>("CargoNetSim::Backend::SimulationClientBase*");

    qRegisterMetaType<ShipClient::ShipState>("CargoNetSim::Backend::ShipClient::ShipState");
    qRegisterMetaType<ShipClient::ShipState*>("CargoNetSim::Backend::ShipClient::ShipState*");
    qRegisterMetaType<ShipClient::SimulationSummaryData>("CargoNetSim::Backend::ShipClient::SimulationSummaryData");
    qRegisterMetaType<ShipClient::SimulationSummaryData*>("CargoNetSim::Backend::ShipClient::SimulationSummaryData*");
    qRegisterMetaType<ShipClient::SimulationResults>("CargoNetSim::Backend::ShipClient::SimulationResults");
    qRegisterMetaType<ShipClient::SimulationResults*>("CargoNetSim::Backend::ShipClient::SimulationResults*");
    qRegisterMetaType<ShipClient::ShipSimulationClient>("CargoNetSim::Backend::ShipClient::ShipSimulationClient");
    qRegisterMetaType<ShipClient::ShipSimulationClient*>("CargoNetSim::Backend::ShipClient::ShipSimulationClient*");

    qRegisterMetaType<TrainClient::TrainState>("CargoNetSim::Backend::TrainClient::TrainState");
    qRegisterMetaType<TrainClient::TrainState*>("CargoNetSim::Backend::TrainClient::TrainState*");
    qRegisterMetaType<TrainClient::SimulationSummaryData>("CargoNetSim::Backend::TrainClient::SimulationSummaryData");
    qRegisterMetaType<TrainClient::SimulationSummaryData*>("CargoNetSim::Backend::TrainClient::SimulationSummaryData*");
    qRegisterMetaType<TrainClient::SimulationResults>("CargoNetSim::Backend::TrainClient::SimulationResults");
    qRegisterMetaType<TrainClient::SimulationResults*>("CargoNetSim::Backend::TrainClient::SimulationResults*");
    qRegisterMetaType<TrainClient::TrainSimulationClient>("CargoNetSim::Backend::TrainClient::TrainSimulationClient");
    qRegisterMetaType<TrainClient::TrainSimulationClient*>("CargoNetSim::Backend::TrainClient::TrainSimulationClient*");
    
    // Register ClientType
    qRegisterMetaType<ClientType>("CargoNetSim::Backend::ClientType");
    
    
    qDebug() << "Backend metatypes registered successfully";
}

} // namespace Backend
} // namespace CargoNetSim
