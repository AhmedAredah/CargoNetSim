#import "NetworkController.h"
#import "../MainWindow.h"
#include "Backend/Controllers/CargoNetSimController.h"
#include "Backend/Controllers/RegionDataController.h"
#include "GUI/Controllers/UtilityFunctions.h"
#include "GUI/Controllers/ViewController.h"

#include <QColor>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QString>
#include <exception>
#include <memory>

namespace CargoNetSim
{
namespace GUI
{

QString NetworkController::importNetwork(
    MainWindow *mainWindow, NetworkType networkType,
    Backend::RegionData *regionData)
{

    QString networkName;
    // Network name input loop
    while (true)
    {
        QString networkTypeStr =
            NetworkController::getNetworkTypeString(
                networkType);
        bool    ok;
        QString networkName_user;
        networkName_user = QInputDialog::getText(
            mainWindow, "Network Name",
            QString("Enter a name for the %1 network:")
                .arg(networkTypeStr),
            QLineEdit::Normal, QString(), &ok);

        if (!ok || networkName_user.trimmed().isEmpty())
        {
            return QString();
        }

        // Add train prefix if not already present
        if (!networkName_user.toLower().startsWith("rail_")
            || !networkName_user.toLower().startsWith(
                "truck_"))
        {
            if (networkType == NetworkType::Train)
            {
                networkName = "rail_" + networkName_user;
            }
            else if (networkType == NetworkType::Truck)
            {
                networkName = "truck_" + networkName_user;
            }
        }

        // Check for name conflicts
        try
        {
            if (regionData->checkNetworkNameConflict(
                    networkName))
            {
                QMessageBox::warning(
                    mainWindow, "Name Already Exists",
                    QString("A network named '%1' already "
                            "exists. Please choose a "
                            "different name.")
                        .arg(networkName_user));
                continue;
            }
            break;
        }
        catch (const std::exception &e)
        {
            QMessageBox::warning(mainWindow, "Invalid Name",
                                 e.what());
            continue;
        }
    }

    if (networkType == NetworkType::Train)
    {
        if (NetworkController::importTrainNetwork(
                mainWindow, regionData, networkName)
            == true)
        {
            return networkName;
        }
    }
    else if (networkType == NetworkType::Truck)
    {
        if (NetworkController::importTruckNetwork(
                mainWindow, regionData, networkName))
        {
            return networkName;
        }
    }

    return QString();
}

bool NetworkController::importTrainNetwork(
    MainWindow *mainWindow, Backend::RegionData *regionData,
    QString &networkName)
{
    // Select node file
    QString nodeFile = QFileDialog::getOpenFileName(
        mainWindow, "Select Train Network Node File",
        QString(), "All Files (*.*)", nullptr,
        QFileDialog::DontUseNativeDialog);

    if (nodeFile.isEmpty())
    {
        return false;
    }

    // Select link file
    QString linkFile = QFileDialog::getOpenFileName(
        mainWindow, "Select Train Network Link File",
        QString(), "All Files (*.*)", nullptr,
        QFileDialog::DontUseNativeDialog);

    if (linkFile.isEmpty())
    {
        return false;
    }

    try
    {
        // load the network
        regionData->addTrainNetwork(networkName, nodeFile,
                                    linkFile);
        // Draw the network on map
        ViewController::drawNetwork(mainWindow, regionData,
                                    NetworkType::Train,
                                    networkName);

        mainWindow->showStatusBarMessage(
            "Importing train network!", 2000);
        return true;
    }
    catch (const std::exception &e)
    {
        QMessageBox::warning(mainWindow, "Error", e.what());
        return false;
    }
}

bool NetworkController::importTruckNetwork(
    MainWindow *mainWindow, Backend::RegionData *regionData,
    QString &networkName)
{
    // Select node file
    QString configFile = QFileDialog::getOpenFileName(
        mainWindow, "Select Truck Network Master File",
        QString(), "All Files (*.*)", nullptr,
        QFileDialog::DontUseNativeDialog);

    if (configFile.isEmpty())
    {
        return false;
    }

    try
    {
        // load the network
        regionData->addTruckNetwork(networkName,
                                    configFile);

        // Draw the network on map
        ViewController::drawNetwork(mainWindow, regionData,
                                    NetworkType::Truck,
                                    networkName);
        mainWindow->showStatusBarMessage(
            "Importing truck network!", 2000);
        return true;
    }
    catch (const std::exception &e)
    {
        QMessageBox::warning(mainWindow, "Error", e.what());
        return false;
    }
}

QString
NetworkController::getNetworkTypeString(NetworkType type)
{
    switch (type)
    {
    case NetworkType::Train:
        return "Rail";
    case NetworkType::Truck:
        return "Truck";
    default:
        return "";
    }
}

bool NetworkController::removeNetwork(
    MainWindow *mainWindow, NetworkType networkType,
    QString &networkName, Backend::RegionData *regionData)
{
    // handle the visual delete first since it depends on
    // the backend
    ViewController::removeNetwork(mainWindow, networkType,
                                  regionData, networkName);

    // remove the network from the backend
    if (networkType == NetworkType::Train)
    {
        return regionData->removeTrainNetwork(networkName);
    }
    else if (networkType == NetworkType::Truck)
    {
        return regionData->removeTruckNetwork(networkName);
    }
    return false;
}

CargoNetSim::Backend::ShortestPathResult CargoNetSim::GUI::
    NetworkController::findNetworkShortestPath(
        const QString                &regionName,
        const QString                &networkName,
        CargoNetSim::GUI::NetworkType networkType,
        int startNodeId, int endNodeId)
{
    try
    {
        if (networkType
            == CargoNetSim::GUI::NetworkType::Train)
        {
            auto network =
                CargoNetSim::CargoNetSimController::
                    getInstance()
                        .getRegionDataController()
                        ->getRegionData(regionName)
                        ->getTrainNetwork(networkName);

            // Find the shortest path
            return network->findShortestPath(startNodeId,
                                             endNodeId);
        }
        else if (networkType
                 == CargoNetSim::GUI::NetworkType::Truck)
        {
            auto network =
                CargoNetSim::CargoNetSimController::
                    getInstance()
                        .getRegionDataController()
                        ->getRegionData(regionName)
                        ->getTruckNetwork(networkName);

            return network->findShortestPath(startNodeId,
                                             endNodeId);
        }
    }
    catch (const std::exception &e)
    {
        qWarning() << "Error finding shortest path:"
                   << e.what();
    }

    return CargoNetSim::Backend::ShortestPathResult();
}

bool NetworkController::clearAllNetworks(
    MainWindow *mainWindow)
{
    if (!mainWindow)
    {
        return false;
    }

    auto regionController =
        CargoNetSim::CargoNetSimController::getInstance()
            .getRegionDataController();
    auto regionNames =
        regionController->getAllRegionNames();

    for (auto regionName : regionNames)
    {
        Backend::RegionData *regionData =
            regionController->getRegionData(regionName);
        if (regionData)
        {
            for (auto netName :
                 regionData->getTrainNetworks())
            {
                // Remove the network from the canvas
                CargoNetSim::GUI::ViewController::
                    removeNetwork(mainWindow,
                                  NetworkType::Train,
                                  regionData, netName);
            }

            for (auto netName :
                 regionData->getTruckNetworks())
            {
                // Remove the network from the canvas
                CargoNetSim::GUI::ViewController::
                    removeNetwork(mainWindow,
                                  NetworkType::Truck,
                                  regionData, netName);
            }
        }
    }

    return true;
}

} // namespace GUI
} // namespace CargoNetSim
