#include "CustomMainWindow.h"

#include <QHBoxLayout>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace CargoNetSim {
namespace GUI {

CustomMainWindow::CustomMainWindow(QWidget *parent)
    : QMainWindow(parent) {
    // Create a central widget
    centralWidget = new QWidget();
    setCentralWidget(centralWidget);

    // Create the main horizontal layout
    mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create center widget that will fill the space between
    // docks
    centerWidget = new QWidget();
    centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);

    // Create vertical splitter for the center area
    centerSplitter = new QSplitter(Qt::Vertical);
    centerLayout->addWidget(centerSplitter);

    // Set expanding size policy for center widget
    centerWidget->setSizePolicy(QSizePolicy::Expanding,
                                QSizePolicy::Expanding);

    // Add center widget to main layout
    mainLayout->addWidget(centerWidget);
}

CustomMainWindow::~CustomMainWindow() {
    // Qt's parent-child mechanism handles cleanup
}

void CustomMainWindow::addDockWidget(
    Qt::DockWidgetArea area, QDockWidget *dockWidget) {
    // Check if this is a special case dock that should go
    // in the center splitter
    if (dockWidget
        && dockWidget->objectName()
               == "ShortestPathTableDock") {
        // Add bottom dock to center splitter
        centerSplitter->addWidget(dockWidget);
    } else {
        // Use normal docking for other widgets
        QMainWindow::addDockWidget(area, dockWidget);
    }
}

} // namespace GUI
} // namespace CargoNetSim
