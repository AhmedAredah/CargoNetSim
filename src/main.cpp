#include <QApplication>
#include <QObject>
#include <QSplashScreen>
#include <QThread>
#include <QTimer>
#include <signal.h>

#include "Backend/BackendInit.h"
#include "GUI/MainWindow.h"
#include "GUI/Utils/ApplicationLogger.h"
#include "GUI/Utils/ErrorHandlers.h"
#include "GUI/Widgets/SplashScreen.h"

#include "Backend/Controllers/CargoNetSimController.h"

using namespace CargoNetSim::GUI;

// Signal handler for SIGINT (Ctrl+C)
void signalHandler(int signal)
{
    qDebug() << "Received signal:" << signal;

    // Perform cleanup
    {
        // Clean up CargoNetSimController before quitting
        CargoNetSim::CargoNetSimControllerCleanup::
            cleanup();
    }

    if (MainWindow::getInstance())
    {
        MainWindow::getInstance()->shutdown();
    }
    else
    {
        QApplication::quit();
    }
}

int main(int argc, char *argv[])
{
    // Install error handlers
    installExceptionHandlers();

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    // Initialize the logger first
    CargoNetSim::Backend::LoggerInterface *logger =
        ApplicationLogger::getInstance();
    ApplicationLogger::getInstance()->start();

    // Set application info
    app.setApplicationName("CargoNetSim");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("CargoNetSim Org");

    // Initialize backend metatypes
    CargoNetSim::Backend::initializeBackend(logger);

    // Set up signal handling
    signal(SIGINT, signalHandler);
    QObject::connect(&app, &QApplication::aboutToQuit,
                     []() { signalHandler(SIGINT); });

    // Create splash screen
    SplashScreen splash;
    splash.show();

    // Create main window with delay
    QTimer::singleShot(1000, [&]() {
        MainWindow *mainWindow = MainWindow::getInstance();
        splash.finish(mainWindow);
        mainWindow->showMaximized();

        // Signal initialization complete
        ApplicationLogger::signalInitComplete();
    });

    return app.exec();
}
