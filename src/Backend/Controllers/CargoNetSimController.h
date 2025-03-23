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

namespace CargoNetSim
{
namespace Backend
{

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

public:
    /**
     * @brief Constructor
     * @param parent The parent QObject
     */
    explicit CargoNetSimController(
        LoggerInterface *logger = nullptr,
        QObject         *parent = nullptr);

    /**
     * @brief Destructor
     */
    ~CargoNetSimController();

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

    /**
     * @brief Gets the truck simulation manager
     * @return Pointer to truck simulation manager
     */
    TruckClient::TruckSimulationManager *
    getTruckManager() const;

    /**
     * @brief Gets the ship simulation client
     * @return Pointer to ship simulation client
     */
    ShipClient::ShipSimulationClient *getShipClient() const;

    /**
     * @brief Gets the train simulation client
     * @return Pointer to train simulation client
     */
    TrainClient::TrainSimulationClient *
    getTrainClient() const;

    /**
     * @brief Gets the terminal simulation client
     * @return Pointer to terminal simulation client
     */
    TerminalSimulationClient *getTerminalClient() const;

signals:
    /**
     * @brief Signal emitted when a client is initialized
     * @param clientType Type of client initialized
     */
    void clientInitialized(ClientType clientType);

    /**
     * @brief Signal emitted when all clients are
     * initialized
     */
    void allClientsInitialized();

    /**
     * @brief Signal emitted when a client is ready
     * @param clientType Type of client ready
     */
    void clientReady(ClientType clientType);

    /**
     * @brief Signal emitted when all clients are ready
     */
    void allClientsReady();

private slots:
    /**
     * @brief Slot called when a thread has started
     */
    void onThreadStarted();

    /**
     * @brief Slot called when a thread has finished
     */
    void onThreadFinished();

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
    TruckClient::TruckSimulationManager *m_truckManager;
    ShipClient::ShipSimulationClient    *m_shipClient;
    TrainClient::TrainSimulationClient  *m_trainClient;
    TerminalSimulationClient            *m_terminalClient;

    // Logger
    LoggerInterface *m_logger;

    // Track client initialization status
    QMap<ClientType, bool> m_clientInitialized;
    int                    m_initializedClientCount;
    int                    m_readyClientCount;
};

} // namespace Backend
} // namespace CargoNetSim
