#include "HeartbeatController.h"
#include "../MainWindow.h"
#include "Backend/Clients/BaseClient/RabbitMQHandler.h"
#include "Backend/Controllers/CargoNetSimController.h"
#include <QDateTime>
#include <QDebug>
#include <QLabel>
#include <QMap>
#include <QThread>
#include <QTimer>
#include <QVariant>
#include <QtWidgets/qstatusbar.h>
#include <exception>

namespace CargoNetSim
{
namespace GUI
{

HeartbeatController::HeartbeatController(
    MainWindow *mainWindow)
    : QObject(mainWindow)
    , mainWindow(mainWindow)
    , consumerCheckTimer(nullptr)
    , monitorThread(nullptr)
    , isRunning(false)
    , lastConsumerCheck(0)
    , consumerCheckInterval(20) // Check every 20 seconds
{
    // Initialize server indicators mapping from MainWindow
    // status bar
    QStatusBar *statusBar = mainWindow->statusBar();

    // Define servers with their IDs - these must match the
    // IDs in MainWindow.cpp
    QStringList serverIDs = {"TerminalSim", "NeTrainSim",
                             "ShipNetSim", "INTEGRATION"};

    // Find server indicators in the status bar
    for (const QString &serverId : serverIDs)
    {
        QMap<QString, QVariant> indicatorData;

        // Find the indicator label for this server
        QList<QLabel *> labels =
            statusBar->findChildren<QLabel *>();
        for (QLabel *label : labels)
        {
            if (label->toolTip().contains(serverId)
                || (label->size() == QSize(10, 10)
                    && label->parent()
                               ->findChild<QLabel *>(
                                   QString(),
                                   Qt::FindDirectChildrenOnly)
                               ->text()
                           == serverId))
            {
                // This is the indicator for this server
                indicatorData["indicator"] =
                    QVariant::fromValue<QLabel *>(label);
                indicatorData["description"] = serverId;
                serverIndicators[serverId] = indicatorData;
                qDebug() << "Found indicator for server:"
                         << serverId;
                break;
            }
        }
    }
}

HeartbeatController::~HeartbeatController()
{
    if (monitorThread)
    {
        isRunning = false;
        monitorThread->quit();
        monitorThread->wait();
        delete monitorThread;
    }

    if (consumerCheckTimer)
    {
        consumerCheckTimer->stop();
        delete consumerCheckTimer;
    }
}

void HeartbeatController::initialize()
{
    qDebug() << "Initializing HeartbeatController";

    // Make sure we have our server indicators set up
    if (serverIndicators.isEmpty())
    {
        // Since server indicators weren't found in the
        // constructor, set up manually
        QStringList serverIDs = {"TerminalSim",
                                 "NeTrainSim", "ShipNetSim",
                                 "INTEGRATION"};

        for (const QString &serverId : serverIDs)
        {
            QMap<QString, QVariant> indicatorData;
            indicatorData["description"] = serverId;
            serverIndicators[serverId]   = indicatorData;

            qDebug() << "Adding server indicator for"
                     << serverId;
        }
    }

    // Create and start monitor thread for consumer checks
    monitorThread = new QThread();

    // Create a timer for the monitoring thread
    consumerCheckTimer = new QTimer(nullptr);
    consumerCheckTimer->setInterval(consumerCheckInterval
                                    * 1000);
    consumerCheckTimer->moveToThread(monitorThread);

    connect(
        monitorThread, &QThread::started,
        consumerCheckTimer,
        static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(consumerCheckTimer, &QTimer::timeout, this,
            &HeartbeatController::checkQueueConsumers);
    connect(monitorThread, &QThread::finished,
            consumerCheckTimer, &QTimer::stop);
    connect(monitorThread, &QThread::finished,
            consumerCheckTimer, &QTimer::deleteLater);

    isRunning = true;
    monitorThread->start();

    qDebug() << "Consumer check timer started with interval"
             << consumerCheckInterval << "seconds";

    // Do an initial check of consumers
    QTimer::singleShot(
        2000, this,
        &HeartbeatController::checkQueueConsumers);
}

void HeartbeatController::checkQueueConsumers()
{
    // Get the current time
    qint64 currentTime =
        QDateTime::currentDateTime().toMSecsSinceEpoch();

    // Only check if enough time has passed since last check
    if (currentTime - lastConsumerCheck
        < (consumerCheckInterval * 1000))
    {
        return;
    }

    // Clear the activeConsumers map at the start of each
    // check
    activeConsumers.clear();

    // Update last check time
    lastConsumerCheck = currentTime;

    bool clientsAvailable = false;

    try
    {
        // Try to get the controller instance and check
        // clients
        qDebug() << "Getting controller instance";
        auto &controller = CargoNetSim::
            CargoNetSimController::getInstance();

        // Check Terminal client
        qDebug()
            << "Getting terminal client from controller";
        auto *terminalClient =
            controller.getTerminalClient();
        qDebug() << "Terminal client pointer:"
                 << (terminalClient ? "valid" : "nullptr");

        if (terminalClient && terminalClient->isConnected())
        {
            auto *handler =
                terminalClient->getRabbitMQHandler();
            if (handler)
            {
                bool hasConsumers =
                    handler->hasCommandQueueConsumers();
                qDebug() << "Terminal command queue name:"
                         << handler->getCommandQueueName();
                qDebug() << "Terminal server has consumers:"
                         << hasConsumers;
                updateServerStatus("TerminalSim",
                                   hasConsumers);
                activeConsumers["TerminalSim"] =
                    hasConsumers;
                clientsAvailable = true;
            }
        }

        // Check Train client
        auto *trainClient = controller.getTrainClient();
        if (trainClient && trainClient->isConnected())
        {
            auto *handler =
                trainClient->getRabbitMQHandler();
            if (handler)
            {
                bool hasConsumers =
                    handler->hasCommandQueueConsumers();
                qDebug() << "Train server has consumers:"
                         << hasConsumers;
                updateServerStatus("NeTrainSim",
                                   hasConsumers);
                activeConsumers["NeTrainSim"] =
                    hasConsumers;
                clientsAvailable = true;
            }
        }

        // Check Ship client
        auto *shipClient = controller.getShipClient();
        if (shipClient && shipClient->isConnected())
        {
            auto *handler =
                shipClient->getRabbitMQHandler();
            if (handler)
            {
                bool hasConsumers =
                    handler->hasCommandQueueConsumers();
                qDebug() << "Ship server has consumers:"
                         << hasConsumers;
                updateServerStatus("ShipNetSim",
                                   hasConsumers);
                activeConsumers["ShipNetSim"] =
                    hasConsumers;
                clientsAvailable = true;
            }
        }

        // Check Truck client (INTEGRATION)
        auto *truckManager = controller.getTruckManager();
        if (truckManager)
        {
            bool hasConsumers =
                truckManager->hasCommandQueueConsumers();
            updateServerStatus("INTEGRATION", hasConsumers);
            activeConsumers["INTEGRATION"] = hasConsumers;
            clientsAvailable               = true;
        }
    }
    catch (const std::exception &e)
    {
        qWarning() << "Exception during client check:"
                   << e.what();
    }

    // If no clients were available, try direct queue checks
    if (!clientsAvailable)
    {
        qDebug() << "No clients available - trying direct "
                    "queue checks";
        checkQueuesDirectly();
    }
}

void HeartbeatController::checkQueuesDirectly()
{
    // Map of server IDs to their command queue names
    QMap<QString, QString> serverToQueueMap;
    serverToQueueMap["TerminalSim"] =
        "CargoNetSim.CommandQueue.TerminalSim";
    serverToQueueMap["NeTrainSim"] =
        "CargoNetSim.CommandQueue.TrainSim";
    serverToQueueMap["ShipNetSim"] =
        "CargoNetSim.CommandQueue.ShipSim";
    serverToQueueMap["INTEGRATION"] =
        "CargoNetSim.CommandQueue.TruckSim";

    // Check each queue directly
    for (auto it = serverToQueueMap.begin();
         it != serverToQueueMap.end(); ++it)
    {
        QString serverId  = it.key();
        QString queueName = it.value();

        try
        {
            qDebug() << "Directly checking queue for server"
                     << serverId << ":" << queueName;

            // Create a temporary RabbitMQ handler to check
            // the consumers
            Backend::RabbitMQHandler tempHandler(
                nullptr, "localhost", 5672,
                "CargoNetSim.Exchange", queueName,
                "CargoNetSim.ResponseQueue.Temp",
                "CargoNetSim.Temp", QStringList());

            bool connected =
                tempHandler.establishConnection();
            qDebug() << "Direct connection to RabbitMQ for"
                     << serverId << ":"
                     << (connected ? "success" : "failed");

            if (connected)
            {
                bool hasConsumers =
                    tempHandler.hasCommandQueueConsumers();
                qDebug()
                    << "Direct check -" << serverId
                    << "has consumers:" << hasConsumers;
                updateServerStatus(serverId, hasConsumers);
                activeConsumers[serverId] =
                    hasConsumers; // Track active consumers
                tempHandler.disconnect();
            }
            else
            {
                qDebug() << "Could not connect to RabbitMQ "
                            "directly for"
                         << serverId;
                updateServerStatus(serverId, false);
                activeConsumers[serverId] = false;
            }
        }
        catch (const std::exception &e)
        {
            qWarning()
                << "Exception during direct queue check for"
                << serverId << ":" << e.what();
            updateServerStatus(serverId, false);
            activeConsumers[serverId] = false;
        }
    }
}

void HeartbeatController::updateServerStatus(
    const QString &serverId, bool connected)
{
    qDebug() << "Updating status for server:" << serverId
             << "to"
             << (connected ? "connected" : "disconnected");

    if (!serverIndicators.contains(serverId))
    {
        qWarning() << "Server indicator not found for"
                   << serverId;
        return;
    }

    // Get the indicator label
    QLabel *indicator = nullptr;

    if (serverIndicators[serverId].contains("indicator"))
    {
        indicator = serverIndicators[serverId]["indicator"]
                        .value<QLabel *>();
        if (indicator)
        {
            qDebug() << "Using existing indicator for"
                     << serverId;
        }
    }

    if (!indicator)
    {
        qDebug() << "Searching for indicator for"
                 << serverId << "in status bar";
        // Try to find the indicator in the main window's
        // status bar
        QList<QLabel *> labels =
            mainWindow->statusBar()
                ->findChildren<QLabel *>();
        qDebug() << "Found" << labels.size()
                 << "labels in status bar";

        for (QLabel *label : labels)
        {
            qDebug() << "Checking label:" << label->text()
                     << "size:" << label->size()
                     << "tooltip:" << label->toolTip();

            if (label->size() == QSize(10, 10))
            {
                // Check if this small dot label is part of
                // a layout containing the server name
                QWidget *parent = qobject_cast<QWidget *>(
                    label->parent());
                if (parent)
                {
                    // Find all labels in this parent
                    // container
                    QList<QLabel *> siblingLabels =
                        parent->findChildren<QLabel *>(
                            QString(),
                            Qt::FindDirectChildrenOnly);
                    qDebug()
                        << "Container has"
                        << siblingLabels.size() << "labels";

                    for (QLabel *siblingLabel :
                         siblingLabels)
                    {
                        qDebug() << "Sibling label:"
                                 << siblingLabel->text();
                        if (siblingLabel != label
                            && siblingLabel->text()
                                   == serverId)
                        {
                            // Found the right indicator
                            indicator = label;
                            serverIndicators
                                [serverId]["indicator"] =
                                    QVariant::fromValue<
                                        QLabel *>(
                                        indicator);
                            qDebug()
                                << "Found indicator for"
                                << serverId;
                            break;
                        }
                    }
                }

                if (indicator)
                    break; // Exit outer loop if indicator
                           // found
            }
        }
    }

    if (!indicator)
    {
        qWarning() << "Could not find indicator label for"
                   << serverId;
        return;
    }

    // Update the indicator color based on connection status
    if (connected)
    {
        indicator->setStyleSheet(
            "background-color: #00ff00; border-radius: "
            "5px;");
        indicator->setToolTip(
            serverIndicators[serverId]["description"]
                .toString()
            + " - Connected");
        qDebug() << "Set" << serverId
                 << "indicator to connected (green)";
    }
    else
    {
        indicator->setStyleSheet(
            "background-color: #ff0000; border-radius: "
            "5px;");
        indicator->setToolTip(
            serverIndicators[serverId]["description"]
                .toString()
            + " - Disconnected");
        qDebug() << "Set" << serverId
                 << "indicator to disconnected (red)";
    }
}

} // namespace GUI
} // namespace CargoNetSim
