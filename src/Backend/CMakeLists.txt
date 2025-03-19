# Define the source files for the GUI library
set(Backend_SOURCES

    # Commons
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

    # Controllers
    Controllers/VehicleController.h
    Controllers/VehicleController.cpp
    Controllers/NetworkController.h
    Controllers/NetworkController.cpp
    Controllers/RegionDataController.h
    Controllers/RegionDataController.cpp
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
