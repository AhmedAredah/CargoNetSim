/**
 * @file CargoNetSimController.cpp
 * @brief Implements multi-modal simulation controller
 * @author [Your Name]
 * @date 2025-03-22
 */

#include "CargoNetSimController.h"

namespace CargoNetSim
{

void CargoNetSimControllerCleanup::cleanup()
{
    QWriteLocker writeLocker(
        &CargoNetSimController::m_instanceLock);
    if (CargoNetSimController::m_instance)
    {
        delete CargoNetSimController::m_instance;
        CargoNetSimController::m_instance = nullptr;
    }
}

// Initialize static members
CargoNetSimController *CargoNetSimController::m_instance =
    nullptr;
QReadWriteLock CargoNetSimController::m_instanceLock;

CargoNetSimController::CargoNetSimController(
    Backend::LoggerInterface *logger, QObject *parent)
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
    // Create the NetworkController first
    m_networkController =
        new Backend::NetworkController(this);

    // Then create the RegionDataController with the
    // NetworkController
    m_regionDataController =
        new Backend::RegionDataController(
            m_networkController, this);

    // Create other controllers as needed
    m_vehicleController =
        new Backend::VehicleController(this);

    // Initialize client status tracking
    m_clientInitialized[Backend::ClientType::TruckClient] =
        false;
    m_clientInitialized[Backend::ClientType::ShipClient] =
        false;
    m_clientInitialized[Backend::ClientType::TrainClient] =
        false;
    m_clientInitialized
        [Backend::ClientType::TerminalClient] = false;
}

CargoNetSimController &CargoNetSimController::getInstance(
    Backend::LoggerInterface *logger, QObject *parent)
{
    QReadLocker locker(&m_instanceLock);
    if (!m_instance)
    {
        locker.unlock();
        QWriteLocker writeLocker(&m_instanceLock);
        // Double-check pattern to ensure thread safety
        if (!m_instance)
        {
            m_instance =
                new CargoNetSimController(logger, parent);
        }
        writeLocker.unlock();
        locker.relock();
    }
    return *m_instance;
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

    // Clean up controllers
    // The Controllers will be deleted automatically
    // as a child of this object No need for explicit
    // deletion or cleanup classes
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

Backend::RegionDataController *
CargoNetSimController::getRegionDataController()
{
    return m_regionDataController;
}

Backend::VehicleController *
CargoNetSimController::getVehicleController()
{
    return m_vehicleController;
}

Backend::TruckClient::TruckSimulationManager *
CargoNetSimController::getTruckManager() const
{
    return m_truckManager;
}

Backend::NetworkController *
CargoNetSimController::getNetworkController() const
{
    return m_networkController;
}

Backend::ShipClient::ShipSimulationClient *
CargoNetSimController::getShipClient() const
{
    return m_shipClient;
}

Backend::TrainClient::TrainSimulationClient *
CargoNetSimController::getTrainClient() const
{
    return m_trainClient;
}

Backend::TerminalSimulationClient *
CargoNetSimController::getTerminalClient() const
{
    return m_terminalClient;
}

// Implementation of service methods
double CargoNetSimController::getTerminalCapacity(
    const QString &terminalId)
{
    double result = -1.0;
    emit requestTerminalCapacity(terminalId, result);
    return result;
}

int CargoNetSimController::getTerminalContainerCount(
    const QString &terminalId)
{
    int result = -1;
    emit requestContainerCount(terminalId, result);
    return result;
}

bool CargoNetSimController::addContainersToTerminal(
    const QString &terminalId,
    const QString &containersJson)
{
    bool result = false;
    emit requestAddContainers(terminalId, containersJson,
                              result);
    return result;
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
        new Backend::TruckClient::TruckSimulationClient(
            exePath, nullptr);

    // Create truck manager
    m_truckManager =
        new Backend::TruckClient::TruckSimulationManager(
            nullptr);

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
    m_clientInitialized[Backend::ClientType::TruckClient] =
        true;
    m_initializedClientCount++;
    emit clientInitialized(
        Backend::ClientType::TruckClient);

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
        new Backend::ShipClient::ShipSimulationClient(
            nullptr);
    m_shipClient->moveToThread(m_shipThread);

    // Connect thread signals
    connect(m_shipThread, &QThread::started, this,
            &CargoNetSimController::onThreadStarted);
    connect(m_shipThread, &QThread::finished, this,
            &CargoNetSimController::onThreadFinished);

    // Flag as initialized
    m_clientInitialized[Backend::ClientType::ShipClient] =
        true;
    m_initializedClientCount++;
    emit clientInitialized(Backend::ClientType::ShipClient);

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
        new Backend::TrainClient::TrainSimulationClient(
            nullptr);
    m_trainClient->moveToThread(m_trainThread);

    // Connect thread signals
    connect(m_trainThread, &QThread::started, this,
            &CargoNetSimController::onThreadStarted);
    connect(m_trainThread, &QThread::finished, this,
            &CargoNetSimController::onThreadFinished);

    // Flag as initialized
    m_clientInitialized[Backend::ClientType::TrainClient] =
        true;
    m_initializedClientCount++;
    emit clientInitialized(
        Backend::ClientType::TrainClient);

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
        new Backend::TerminalSimulationClient(nullptr);
    m_terminalClient->moveToThread(m_terminalThread);

    // Connect thread signals
    connect(m_terminalThread, &QThread::started, this,
            &CargoNetSimController::onThreadStarted);
    connect(m_terminalThread, &QThread::finished, this,
            &CargoNetSimController::onThreadFinished);

    // Connect signals to terminal client
    connect(
        this,
        &CargoNetSimController::requestTerminalCapacity,
        m_terminalClient,
        [this](const QString &terminalId, double &result) {
            result = m_terminalClient->getAvailableCapacity(
                terminalId);
        },
        Qt::BlockingQueuedConnection);

    connect(
        this, &CargoNetSimController::requestContainerCount,
        m_terminalClient,
        [this](const QString &terminalId, int &result) {
            result = m_terminalClient->getContainerCount(
                terminalId);
        },
        Qt::BlockingQueuedConnection);

    connect(
        this, &CargoNetSimController::requestAddContainers,
        m_terminalClient,
        [this](const QString &terminalId,
               const QString &containersJson,
               bool          &result) {
            result =
                m_terminalClient->addContainersFromJson(
                    terminalId, containersJson);
        },
        Qt::BlockingQueuedConnection);

    // Flag as initialized
    m_clientInitialized
        [Backend::ClientType::TerminalClient] = true;
    m_initializedClientCount++;
    emit clientInitialized(
        Backend::ClientType::TerminalClient);

    if (m_initializedClientCount == 4)
    {
        emit allClientsInitialized();
    }

    return true;
}
// namespace Backend
} // namespace CargoNetSim
