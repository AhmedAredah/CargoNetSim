#pragma once

#include <QDockWidget>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QSplitter>
#include <QTabWidget>
#include <QToolBar>
#include <QVBoxLayout>

namespace CargoNetSim {
namespace GUI {

/**
 * @brief The CustomMainWindow class is a specialized
 * QMainWindow that provides enhanced docking capabilities
 * with a custom center widget implementation.
 *
 * This class allows positioning of dock widgets at specific
 * locations, including in the central area via a splitter
 * widget.
 */
class CustomMainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Constructor for CustomMainWindow.
     * @param parent Optional parent widget.
     */
    explicit CustomMainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~CustomMainWindow();

    /**
     * @brief Overridden method to add dock widgets with
     * custom behavior.
     * @param area The dock area where the widget should be
     * added.
     * @param dockWidget The dock widget to add.
     */
    void addDockWidget(Qt::DockWidgetArea area,
                       QDockWidget       *dockWidget);

protected:
    /** Central widget for the main window */
    QWidget *centralWidget;

    /** Main layout for the central widget */
    QHBoxLayout *mainLayout;

    /** Center widget that fills the space between docks */
    QWidget *centerWidget;

    /** Layout for the center widget */
    QVBoxLayout *centerLayout;

    /** Vertical splitter for the center area */
    QSplitter *centerSplitter;
};

} // namespace GUI
} // namespace CargoNetSim
