/**
 * @file CargoNetSimController.cpp
 * @brief Implements multi-modal simulation controller
 * @author [Your Name]
 * @date 2025-03-22
 */

#include "CargoNetSimController.h"

namespace CargoNetSim
{
namespace Backend
{

CargoNetSimController::CargoNetSimController(
    LoggerInterface *logger, QObject *parent)
    : QObject(parent)
    , m_truckThread(nullptr)
    , m_shipThread(nullptr)
    , m_trainThread(nullptr)
    , m_terminalThread(nullptr)
    , m_truckManager(nullptr)
    , m_shipClient(nullptr)
    , m_trainClient(nullptr)
    , m_terminalClient(nullptr)
    , m_initializedClientCount(0)
    , m_readyClientCount(0)
    , m_logger(logger)
{
    // Initialize client status tracking
    m_clientInitialized[ClientType::TruckClient]    = false;
    m_clientInitialized[ClientType::ShipClient]     = false;
    m_clientInitialized[ClientType::TrainClient]    = false;
    m_clientInitialized[ClientType::TerminalClient] = false;
}

CargoNetSimController::~CargoNetSimController()
{
    stopAll();

    // Clean up threads
    if (m_truckThread)
    {
        m_truckThread->quit();
        m_truckThread->wait(3000);
        delete m_truckThread;
    }

    if (m_shipThread)
    {
        m_shipThread->quit();
        m_shipThread->wait(3000);
        delete m_shipThread;
    }

    if (m_trainThread)
    {
        m_trainThread->quit();
        m_trainThread->wait(3000);
        delete m_trainThread;
    }

    if (m_terminalThread)
    {
        m_terminalThread->quit();
        m_terminalThread->wait(3000);
        delete m_terminalThread;
    }
}

bool CargoNetSimController::initialize(
    const QString &truckExePath)
{
    // Create and start client threads
    bool success = true;

    success &= initializeTruckClient(truckExePath);
    success &= initializeShipClient();
    success &= initializeTrainClient();
    success &= initializeTerminalClient();

    return success;
}

bool CargoNetSimController::startAll()
{
    // Start all threads
    if (m_truckThread && !m_truckThread->isRunning())
    {
        m_truckThread->start();
    }

    if (m_shipThread && !m_shipThread->isRunning())
    {
        m_shipThread->start();
    }

    if (m_trainThread && !m_trainThread->isRunning())
    {
        m_trainThread->start();
    }

    if (m_terminalThread && !m_terminalThread->isRunning())
    {
        m_terminalThread->start();
    }

    return true;
}

bool CargoNetSimController::stopAll()
{
    // Stop all simulation clients
    if (m_truckManager)
    {
        // Stop all truck instances
        QStringList networks = {"*"};
        m_truckManager->runSimulationSync(networks);
    }

    if (m_shipClient)
    {
        m_shipClient->endSimulator({"*"});
    }

    if (m_trainClient)
    {
        m_trainClient->endSimulator({"*"});
    }

    return true;
}

TruckClient::TruckSimulationManager *
CargoNetSimController::getTruckManager() const
{
    return m_truckManager;
}

ShipClient::ShipSimulationClient *
CargoNetSimController::getShipClient() const
{
    return m_shipClient;
}

TrainClient::TrainSimulationClient *
CargoNetSimController::getTrainClient() const
{
    return m_trainClient;
}

TerminalSimulationClient *
CargoNetSimController::getTerminalClient() const
{
    return m_terminalClient;
}

void CargoNetSimController::onThreadStarted()
{
    QThread *senderThread =
        qobject_cast<QThread *>(sender());

    if (senderThread == m_shipThread)
    {
        if (m_shipClient)
        {
            m_shipClient->initializeClient(m_logger);
        }
    }
    else if (senderThread == m_trainThread)
    {
        if (m_trainClient)
        {
            m_trainClient->initializeClient(m_logger);
        }
    }
    else if (senderThread == m_terminalThread)
    {
        if (m_terminalClient)
        {
            m_terminalClient->initializeClient(m_logger);
        }
    }
}

void CargoNetSimController::onThreadFinished()
{
    // Handle thread completion if needed
}

bool CargoNetSimController::initializeTruckClient(
    const QString &exePath)
{
    // Create thread for truck client
    m_truckThread = new QThread();
    m_truckThread->setObjectName("TruckSimulationThread");

    // Create truck client
    auto truckClient =
        new TruckClient::TruckSimulationClient(exePath,
                                               nullptr);

    // Create truck manager
    m_truckManager =
        new TruckClient::TruckSimulationManager(nullptr);

    // Add client to manager
    // Ownership of the truckClient will be given to the
    // manager
    m_truckManager->addClient("MainTruckNetwork",
                              truckClient, m_logger);

    // Move truck manager to truck thread
    m_truckManager->moveToThread(m_truckThread);

    // Connect thread signals
    connect(m_truckThread, &QThread::started, this,
            &CargoNetSimController::onThreadStarted);
    connect(m_truckThread, &QThread::finished, this,
            &CargoNetSimController::onThreadFinished);

    // Flag as initialized
    m_clientInitialized[ClientType::TruckClient] = true;
    m_initializedClientCount++;
    emit clientInitialized(ClientType::TruckClient);

    if (m_initializedClientCount == 4)
    {
        emit allClientsInitialized();
    }

    return true;
}

bool CargoNetSimController::initializeShipClient()
{
    // Create thread for ship client
    m_shipThread = new QThread();
    m_shipThread->setObjectName("ShipSimulationThread");

    // Create ship client
    m_shipClient =
        new ShipClient::ShipSimulationClient(nullptr);
    m_shipClient->moveToThread(m_shipThread);

    // Connect thread signals
    connect(m_shipThread, &QThread::started, this,
            &CargoNetSimController::onThreadStarted);
    connect(m_shipThread, &QThread::finished, this,
            &CargoNetSimController::onThreadFinished);

    // Flag as initialized
    m_clientInitialized[ClientType::ShipClient] = true;
    m_initializedClientCount++;
    emit clientInitialized(ClientType::ShipClient);

    if (m_initializedClientCount == 4)
    {
        emit allClientsInitialized();
    }

    return true;
}

bool CargoNetSimController::initializeTrainClient()
{
    // Create thread for train client
    m_trainThread = new QThread();
    m_trainThread->setObjectName("TrainSimulationThread");

    // Create train client
    m_trainClient =
        new TrainClient::TrainSimulationClient(nullptr);
    m_trainClient->moveToThread(m_trainThread);

    // Connect thread signals
    connect(m_trainThread, &QThread::started, this,
            &CargoNetSimController::onThreadStarted);
    connect(m_trainThread, &QThread::finished, this,
            &CargoNetSimController::onThreadFinished);

    // Flag as initialized
    m_clientInitialized[ClientType::TrainClient] = true;
    m_initializedClientCount++;
    emit clientInitialized(ClientType::TrainClient);

    if (m_initializedClientCount == 4)
    {
        emit allClientsInitialized();
    }

    return true;
}

bool CargoNetSimController::initializeTerminalClient()
{
    // Create thread for terminal client
    m_terminalThread = new QThread();
    m_terminalThread->setObjectName(
        "TerminalSimulationThread");

    // Create terminal client
    m_terminalClient =
        new TerminalSimulationClient(nullptr);
    m_terminalClient->moveToThread(m_terminalThread);

    // Connect thread signals
    connect(m_terminalThread, &QThread::started, this,
            &CargoNetSimController::onThreadStarted);
    connect(m_terminalThread, &QThread::finished, this,
            &CargoNetSimController::onThreadFinished);

    // Flag as initialized
    m_clientInitialized[ClientType::TerminalClient] = true;
    m_initializedClientCount++;
    emit clientInitialized(ClientType::TerminalClient);

    if (m_initializedClientCount == 4)
    {
        emit allClientsInitialized();
    }

    return true;
}

} // namespace Backend
} // namespace CargoNetSim
