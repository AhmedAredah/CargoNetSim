#pragma once

#include <QtCore>
#include "Backend/Clients/BaseClient/RabbitMQHandler.h"
#include "Backend/Clients/BaseClient/SimulationClientBase.h"

#include "Backend/Clients/ShipClient/ShipState.h"
#include "Backend/Clients/ShipClient/SimulationSummaryData.h"
#include "Backend/Clients/ShipClient/SimulationResults.h"
#include "Backend/Clients/ShipClient/ShipSimulationClient.h"

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

    qRegisterMetaType<ShipState>("CargoNetSim::Backend::ShipState");
    qRegisterMetaType<ShipState*>("CargoNetSim::Backend::ShipState*");
    qRegisterMetaType<SimulationSummaryData>("CargoNetSim::Backend::SimulationSummaryData");
    qRegisterMetaType<SimulationSummaryData*>("CargoNetSim::Backend::SimulationSummaryData*");
    qRegisterMetaType<SimulationResults>("CargoNetSim::Backend::SimulationResults");
    qRegisterMetaType<SimulationResults*>("CargoNetSim::Backend::SimulationResults*");
    qRegisterMetaType<ShipSimulationClient>("CargoNetSim::Backend::ShipSimulationClient");
    qRegisterMetaType<ShipSimulationClient*>("CargoNetSim::Backend::ShipSimulationClient*");
    
    // Register ClientType
    qRegisterMetaType<ClientType>("CargoNetSim::Backend::ClientType");
    
    
    qDebug() << "Backend metatypes registered successfully";
}

} // namespace Backend
} // namespace CargoNetSim
