#pragma once

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QMap>

namespace CargoNetSim {
namespace GUI {

class MainWindow;

/**
 * @brief The RegionManager widget allows users to create, edit, and manage regions in the application.
 * 
 * This widget provides a UI for creating new regions, renaming or deleting existing regions,
 * and changing their colors. Each region is displayed in a list with a color swatch.
 */
class RegionManagerWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Constructs a RegionManagerWidget widget
     * @param mainWindow Pointer to the main window
     * @param parent Parent widget
     */
    explicit RegionManagerWidget(MainWindow* mainWindow, QWidget* parent = nullptr);
    
    /**
     * @brief Updates the region list with current regions and their colors
     */
    void updateRegionList();
    
    /**
     * @brief Clears all regions except the default region
     */
    void clearRegions();

private slots:
    /**
     * @brief Adds a new region to the application
     */
    void addRegion();
    
    /**
     * @brief Renames the currently selected region
     */
    void renameRegion();
    
    /**
     * @brief Deletes the currently selected region
     */
    void deleteRegion();
    
    /**
     * @brief Changes the color of the currently selected region
     */
    void changeRegionColor();
    
    /**
     * @brief Updates UI button states based on the selection state
     */
    void updateButtonStates();

private:
    /**
     * @brief Sets up the UI components
     */
    void setupUI();

    // UI elements
    QListWidget* regionList;
    QPushButton* addButton;
    QPushButton* renameButton;
    QPushButton* deleteButton;
    QPushButton* colorButton;
    
    // Reference to the main window
    MainWindow* mainWindow;
};

} // namespace GUI
} // namespace CargoNetSim
