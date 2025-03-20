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
};

} // namespace GUI
} // namespace CargoNetSim
