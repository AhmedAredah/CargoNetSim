# Define the source files for the GUI library
set(Backend_SOURCES

    # Commons
    Commons/LoggerInterface.h
    Commons/ClientType.h
    Commons/TerminalInterface.h
    Commons/TerminalInterface.cpp
    Commons/TransportationMode.h
    Commons/TransportationMode.cpp
    Commons/DirectedGraphBase.h
    Commons/DirectedGraphBase.cpp
    Commons/DirectedGraph.h
    Commons/DirectedGraph.cpp

    # Models
    Models/TrainSystem.h
    Models/TrainSystem.cpp
    Models/ShipSystem.h
    Models/ShipSystem.cpp
    Models/TrainNetwork.h
    Models/TrainNetwork.cpp
    Models/TruckNetwork.h
    Models/TruckNetwork.cpp
    Models/Terminal.h
    Models/Terminal.cpp
    Models/PathSegment.h
    Models/PathSegment.cpp
    Models/Path.h
    Models/Path.cpp

    # Controllers
    Controllers/VehicleController.h
    Controllers/VehicleController.cpp
    Controllers/NetworkController.h
    Controllers/NetworkController.cpp
    Controllers/RegionDataController.h
    Controllers/RegionDataController.cpp

    # Clients

    # BaseClient
    Clients/BaseClient/RabbitMQHandler.h
    Clients/BaseClient/RabbitMQHandler.cpp
    Clients/BaseClient/SimulationClientBase.h
    Clients/BaseClient/SimulationClientBase.cpp

    # TerminalClient
    Clients/TerminalClient/TerminalSimulationClient.h
    Clients/TerminalClient/TerminalSimulationClient.cpp

    # ShipClient
    Clients/ShipClient/ShipState.h
    Clients/ShipClient/ShipState.cpp
    Clients/ShipClient/SimulationResults.h
    Clients/ShipClient/SimulationResults.cpp
    Clients/ShipClient/SimulationSummaryData.h
    Clients/ShipClient/SimulationSummaryData.cpp
    Clients/ShipClient/ShipSimulationClient.h
    Clients/ShipClient/ShipSimulationClient.cpp

    # TrainClient
    Clients/TrainClient/TrainState.h
    Clients/TrainClient/TrainState.cpp
    Clients/TrainClient/SimulationResults.h
    Clients/TrainClient/SimulationResults.cpp
    Clients/TrainClient/SimulationSummaryData.h
    Clients/TrainClient/SimulationSummaryData.cpp
    Clients/TrainClient/TrainSimulationClient.h
    Clients/TrainClient/TrainSimulationClient.cpp


    # TruckClient
    Clients/TruckClient/TruckSimulationClient.h
    Clients/TruckClient/TruckSimulationClient.cpp




    # Initialization
    BackendInit.h
)

# Create the GUI library
add_library(CargoNetSimBackend STATIC ${Backend_SOURCES})

# Link against dependencies
target_link_libraries(CargoNetSimBackend
    PUBLIC
    Qt6::Core
    Container::Container
    rabbitmq::rabbitmq
)

# Set include directories
target_include_directories(CargoNetSimBackend
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)
