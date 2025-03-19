// ToolbarController.h
#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QToolButton>
#include <QGroupBox>
#include <QMenu>

// In ToolbarController.h
namespace CargoNetSim {
namespace GUI {

class MainWindow;
class ShipManagerDialog;
class TrainManagerDialog;

/**
 * @brief Handles toolbar creation and management
 *
 * This class provides utility functions for setting up and managing
 * a ribbon-style toolbar for the main application window.
 */
class ToolbarController {
public:
    /**
     * @brief Sets up the ribbon-style toolbar for the main window
     * @param mainWindow Pointer to the main window
     */
    static void setupToolbar(MainWindow* mainWindow);

    /**
     * @brief Toggles visibility of a dock widget
     * @param checked Whether the dock widget should be visible
     * @param dockWidget The dock widget to toggle
     * @param button The button that controls the dock widget
     * @param widgetName The name of the widget
     */
    static void toggleDockWidget(bool checked,
                                 QDockWidget* dockWidget,
                                 QToolButton* button,
                                 const QString& widgetName);

    /**
     * @brief Shows the train manager dialog
     * @param mainWindow Pointer to the main window
     */
    static void showTrainManager(MainWindow* mainWindow);

    /**
     * @brief Shows the ship manager dialog
     * @param mainWindow Pointer to the main window
     */
    static void showShipManager(MainWindow* mainWindow);
};

} // namespace GUI
} // namespace CargoNetSim
