# Define the source files for the GUI library
set(GUI_SOURCES
    MainWindow.cpp
    MainWindow.h

    # Commons
    Commons/NetworkType.h
    
    # Controllers
    Controllers/BasicButtonController.cpp
    Controllers/BasicButtonController.h
    Controllers/HeartbeatController.cpp
    Controllers/HeartbeatController.h
    Controllers/NetworkController.cpp
    Controllers/NetworkController.h
    Controllers/ToolbarController.cpp
    Controllers/ToolbarController.h
    Controllers/UtilityFunctions.cpp
    Controllers/UtilityFunctions.h
    Controllers/ViewController.cpp
    Controllers/ViewController.h
    
    # Items
    Items/BackgroundPhotoItem.cpp
    Items/BackgroundPhotoItem.h
    Items/ConnectionLabel.cpp
    Items/ConnectionLabel.h
    Items/ConnectionLine.cpp
    Items/ConnectionLine.h
    Items/DistanceMeasurementTool.cpp
    Items/DistanceMeasurementTool.h
    Items/GlobalTerminalItem.cpp
    Items/GlobalTerminalItem.h
    Items/MapLine.cpp
    Items/MapLine.h
    Items/MapPoint.cpp
    Items/MapPoint.h
    Items/RegionCenterPoint.cpp
    Items/RegionCenterPoint.h
    Items/ShapeIcon.cpp
    Items/ShapeIcon.h
    Items/TerminalItem.cpp
    Items/TerminalItem.h
    Items/GraphicsObjectBase.h
    Items/GraphicsObjectBase.cpp
    
    # Serializers
    Serializers/ProjectSerializer.cpp
    Serializers/ProjectSerializer.h
    
    # Utils
    Utils/ApplicationLogger.cpp
    Utils/ApplicationLogger.h
    Utils/ColorPalette.cpp
    Utils/ColorPalette.h
    Utils/ColorUtils.cpp
    Utils/ColorUtils.h
    Utils/ErrorHandlers.cpp
    Utils/ErrorHandlers.h
    Utils/IconCreator.cpp
    Utils/IconCreator.h
    Utils/PathFindingWorker.h
    Utils/PathFindingWorker.cpp
    Utils/SimulationValidationWorker.h
    Utils/SimulationValidationWorker.cpp
    
    # Widgets
    Widgets/ColorPickerDialog.cpp
    Widgets/ColorPickerDialog.h
    Widgets/ContainerManagerWidget.cpp
    Widgets/ContainerManagerWidget.h
    Widgets/CustomMainWindow.cpp
    Widgets/CustomMainWindow.h
    Widgets/GraphicsScene.cpp
    Widgets/GraphicsScene.h
    Widgets/GraphicsView.cpp
    Widgets/GraphicsView.h
    Widgets/NetworkManagerDialog.cpp
    Widgets/NetworkManagerDialog.h
    Widgets/PropertiesPanel.cpp
    Widgets/PropertiesPanel.h
    Widgets/RegionManagerWidget.cpp
    Widgets/RegionManagerWidget.h
    Widgets/SetCoordinatesDialog.cpp
    Widgets/SetCoordinatesDialog.h
    Widgets/SettingsWidget.cpp
    Widgets/SettingsWidget.h
    Widgets/ShipManagerDialog.cpp
    Widgets/ShipManagerDialog.h
    Widgets/ShortestPathTable.cpp
    Widgets/ShortestPathTable.h
    Widgets/SplashScreen.cpp
    Widgets/SplashScreen.h
    Widgets/TrainManagerDialog.cpp
    Widgets/TrainManagerDialog.h
)

# Create the GUI library
add_library(CargoNetSimGUI STATIC ${GUI_SOURCES})

# Link against dependencies
target_link_libraries(CargoNetSimGUI
    PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Xml
    Container::Container
    rabbitmq::rabbitmq
    CargoNetSimBackend
)

# Set include directories
target_include_directories(CargoNetSimGUI
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)
