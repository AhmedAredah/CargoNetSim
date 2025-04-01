/**
 * @file CargoNetSimController.h
 * @brief Controls multi-modal simulation clients
 * @author [Your Name]
 * @date 2025-03-22
 */

#pragma once

#include <QMap>
#include <QObject>
#include <QString>
#include <QThread>
#include <memory>

#include "Backend/Clients/ShipClient/ShipSimulationClient.h"
#include "Backend/Clients/TerminalClient/TerminalSimulationClient.h"
#include "Backend/Clients/TrainClient/TrainSimulationClient.h"
#include "Backend/Clients/TruckClient/TruckSimulationClient.h"
#include "Backend/Clients/TruckClient/TruckSimulationManager.h"
#include "Backend/Controllers/ConfigController.h"
#include "Backend/Controllers/NetworkController.h"
#include "Backend/Controllers/RegionDataController.h"
#include "Backend/Controllers/VehicleController.h"

namespace CargoNetSim
{

/**
 * @class CargoNetSimControllerCleanup
 * @brief Utility class to handle singleton cleanup.
 */
class CargoNetSimControllerCleanup
{
public:
    /**
     * @brief Cleanup the CargoNetSimController singleton
     *        instance.
     */
    static void cleanup();
};

/**
 * @class CargoNetSimController
 * @brief Central controller for multi-modal simulation
 *
 * Manages truck, ship, train, and terminal simulation
 * clients, each running in its own thread, and provides a
 * unified interface for simulation control and
 * synchronization.
 */
class CargoNetSimController : public QObject
{
    Q_OBJECT

    friend class CargoNetSimControllerCleanup;

public:
    /**
     * @brief Get the singleton instance of
     *        CargoNetSimController.
     * @param logger Optional logger interface for logging
     * @param parent Optional parent QObject.
     * @return Reference to the singleton instance.
     */
    static CargoNetSimController &
    getInstance(Backend::LoggerInterface *logger = nullptr,
                QObject                  *parent = nullptr);

    /**
     * @brief Destructor
     */
    ~CargoNetSimController();

    // Delete copy and move constructors and operators
    CargoNetSimController(const CargoNetSimController &) =
        delete;
    CargoNetSimController &
    operator=(const CargoNetSimController &) = delete;
    CargoNetSimController(CargoNetSimController &&) =
        delete;
    CargoNetSimController &
    operator=(CargoNetSimController &&) = delete;

    /**
     * @brief Initializes the controller and clients
     * @param truckExePath Path to truck simulation
     * executable
     * @return True if initialization was successful
     */
    bool initialize(const QString &truckExePath);

    /**
     * @brief Starts all simulation clients
     * @return True if all clients started successfully
     */
    bool startAll();

    /**
     * @brief Stops all simulation clients
     * @return True if all clients stopped successfully
     */
    bool stopAll();

    // Controller access methods

    /**
     * @brief Gets RegionDataController
     * @return
     */
    Backend::RegionDataController *
    getRegionDataController();

    /**
     * @brief Gets the vehicle controller
     * @return Pointer to vehicle controller
     */
    Backend::VehicleController *getVehicleController();

    /**
     *  @bried Gets the config controller
     *  @return Pointer to config controller
     */
    Backend::ConfigController *getConfigController();

    /**
     * @brief Gets the network controller
     * @return Pointer to network controller
     */
    Backend::NetworkController *
    getNetworkController() const;

    /**
     * @brief Gets the truck simulation manager
     * @return Pointer to truck simulation manager
     */
    Backend::TruckClient::TruckSimulationManager *
    getTruckManager() const;

    /**
     * @brief Gets the ship simulation client
     * @return Pointer to ship simulation client
     */
    Backend::ShipClient::ShipSimulationClient *
    getShipClient() const;

    /**
     * @brief Gets the train simulation client
     * @return Pointer to train simulation client
     */
    Backend::TrainClient::TrainSimulationClient *
    getTrainClient() const;

    /**
     * @brief Gets the terminal simulation client
     * @return Pointer to terminal simulation client
     */
    Backend::TerminalSimulationClient *
    getTerminalClient() const;

public slots:
    /**
     * @brief Gets terminal capacity
     * @param terminalId Terminal identifier
     * @return Available capacity
     */
    double getTerminalCapacity(const QString &terminalId);

    /**
     * @brief Gets terminal container count
     * @param terminalId Terminal identifier
     * @return Container count
     */
    int
    getTerminalContainerCount(const QString &terminalId);

    /**
     * @brief Adds containers to a terminal
     * @param terminalId Terminal identifier
     * @param containersJson Containers as JSON
     * @return True if successful
     */
    bool
    addContainersToTerminal(const QString &terminalId,
                            const QString &containersJson);

signals:
    /**
     * @brief Signal emitted when a client is initialized
     * @param clientType Type of client initialized
     */
    void clientInitialized(
        CargoNetSim::Backend::ClientType clientType);

    /**
     * @brief Signal emitted when all clients are
     * initialized
     */
    void allClientsInitialized();

    /**
     * @brief Signal emitted when a client is ready
     * @param clientType Type of client ready
     */
    void clientReady(
        CargoNetSim::Backend::ClientType clientType);

    /**
     * @brief Signal emitted when all clients are ready
     */
    void allClientsReady();

    ////////////// Terminal Client ////////////////////
    /**
     * @brief Signal to request terminal capacity
     * @param terminalId Terminal identifier
     * @param result Reference to store result
     */
    void requestTerminalCapacity(const QString &terminalId,
                                 double        &result);

    /**
     * @brief Signal to request container count
     * @param terminalId Terminal identifier
     * @param result Reference to store result
     */
    void requestContainerCount(const QString &terminalId,
                               int           &result);

    /**
     * @brief Signal to request container addition
     * @param terminalId Terminal identifier
     * @param containersJson Containers as JSON
     * @param result Reference to store result
     */
    void requestAddContainers(const QString &terminalId,
                              const QString &containersJson,
                              bool          &result);

private slots:
    /**
     * @brief Slot called when a thread has started
     */
    void onThreadStarted();

    /**
     * @brief Slot called when a thread has finished
     */
    void onThreadFinished();

protected:
    /**
     * @brief Constructor
     * @param parent The parent QObject
     */
    explicit CargoNetSimController(
        Backend::LoggerInterface *logger = nullptr,
        QObject                  *parent = nullptr);

    static CargoNetSimController *m_instance;

private:
    /**
     * @brief Creates and initializes the truck client
     * @param exePath Path to truck simulation executable
     * @return True if initialization was successful
     */
    bool initializeTruckClient(const QString &exePath);

    /**
     * @brief Creates and initializes the ship client
     * @return True if initialization was successful
     */
    bool initializeShipClient();

    /**
     * @brief Creates and initializes the train client
     * @return True if initialization was successful
     */
    bool initializeTrainClient();

    /**
     * @brief Creates and initializes the terminal client
     * @return True if initialization was successful
     */
    bool initializeTerminalClient();

    // Client threads
    QThread *m_truckThread;
    QThread *m_shipThread;
    QThread *m_trainThread;
    QThread *m_terminalThread;

    // Simulation clients
    Backend::TruckClient::TruckSimulationManager
        *m_truckManager;
    Backend::ShipClient::ShipSimulationClient *m_shipClient;
    Backend::TrainClient::TrainSimulationClient
                                      *m_trainClient;
    Backend::TerminalSimulationClient *m_terminalClient;

    // Controller instances
    Backend::RegionDataController *m_regionDataController;
    Backend::VehicleController    *m_vehicleController;
    Backend::NetworkController    *m_networkController;
    Backend::ConfigController     *m_configController;

    // Logger
    Backend::LoggerInterface *m_logger;

    // Track client initialization status
    QMap<Backend::ClientType, bool> m_clientInitialized;
    int                    m_initializedClientCount;
    int                    m_readyClientCount;

    /** @brief Lock for thread safety of singleton creation
     */
    static QReadWriteLock m_instanceLock;
};

} // namespace CargoNetSim
