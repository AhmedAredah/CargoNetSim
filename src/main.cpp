#include <QApplication>
#include <QObject>
#include <QSplashScreen>
#include <QTimer>
#include <QThread>
#include <signal.h>

#include "Backend/BackendInit.h"
#include "GUI/MainWindow.h"
#include "GUI/Utils/ErrorHandlers.h"
#include "GUI/Widgets/SplashScreen.h"
#include "GUI/Utils/ApplicationLogger.h"

#include "Backend/Controllers/NetworkController.h"
#include "Backend/Controllers/RegionDataController.h"
#include "Backend/Controllers/VehicleController.h"

using namespace CargoNetSim::GUI;

// Signal handler for SIGINT (Ctrl+C)
void signalHandler(int signal) {
    qDebug() << "Received signal:" << signal;

    // Perform cleanup
    {
        // Clean up NetworkController before quitting
        CargoNetSim::Backend::NetworkControllerCleanup::cleanup();
        // Clean up RegionDataController before quitting
        CargoNetSim::Backend::RegionDataControllerCleanup::cleanup();
        // Clean up VehicleManager before quitting
        CargoNetSim::Backend::VehicleController::cleanup();
    }
    
    if (MainWindow::getInstance()) {
        MainWindow::getInstance()->shutdown();
    } else {
        QApplication::quit();
    }
}

int main(int argc, char *argv[]) {
    // Install error handlers
    installExceptionHandlers();
    
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    
    // Set application info
    app.setApplicationName("CargoNetSim");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("CargoNetSim Org");

    // Initialize backend metatypes
    CargoNetSim::Backend::initializeBackend();
    
    // Set up signal handling
    signal(SIGINT, signalHandler);
    QObject::connect(&app, &QApplication::aboutToQuit, 
                     []() { signalHandler(SIGINT); });

    // Create splash screen
    SplashScreen splash;
    splash.show();

    // Create main window with delay
    QTimer::singleShot(1000, [&]() {
        MainWindow* mainWindow = MainWindow::getInstance();
        splash.finish(mainWindow);
        mainWindow->showMaximized();
        
        // Signal initialization complete
        ApplicationLogger::signalInitComplete();
    });

    return app.exec();
}
