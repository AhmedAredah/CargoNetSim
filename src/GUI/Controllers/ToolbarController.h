// ToolbarController.h
#pragma once

#include <QGroupBox>
#include <QMainWindow>
#include <QMenu>
#include <QTabWidget>
#include <QToolButton>

// In ToolbarController.h
namespace CargoNetSim
{
namespace GUI
{

class MainWindow;
class ShipManagerDialog;
class TrainManagerDialog;

/**
 * @brief Handles toolbar creation and management
 *
 * This class provides utility functions for setting up and
 * managing a ribbon-style toolbar for the main application
 * window.
 */
class ToolbarController
{
public:
    /**
     * @brief Sets up the ribbon-style toolbar for the main
     * window
     * @param mainWindow Pointer to the main window
     */
    static void setupToolbar(MainWindow *mainWindow);

    static void storeButtonStates(MainWindow *mainWindow);
    static void restoreButtonStates(MainWindow *mainWindow);
    static void disableAllButtons(MainWindow *mainWindow);

private:
    static QMap<QWidget *, bool> widgetStates;
};

} // namespace GUI
} // namespace CargoNetSim
