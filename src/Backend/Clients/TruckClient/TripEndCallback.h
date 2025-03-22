/**
 * @file TripEndCallback.h
 * @brief Defines trip end callback mechanism
 * @author Ahmed Aredah
 * @date 2025-03-22
 */

#pragma once

#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVector>
#include <functional>

namespace CargoNetSim {
namespace Backend {
namespace TruckClient {

/**
 * @struct TripEndData
 * @brief Data provided when a trip ends
 */
struct TripEndData {
    QString tripId;      ///< Trip identifier
    QString networkName; ///< Network identifier
    QString origin;      ///< Origin node identifier
    QString destination; ///< Destination node identifier
    double  distance;    ///< Trip distance in km
    double  fuelConsumption; ///< Fuel consumed during trip
    double  travelTime;      ///< Trip duration in seconds
    QJsonObject
        rawData; ///< Original raw data from simulator
};

/**
 * @class TripEndCallbackManager
 * @brief Manages callbacks for trip completion events
 *
 * Provides a Qt-based mechanism for registering callbacks
 * that trigger when trips end in the simulation.
 */
class TripEndCallbackManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent The parent QObject
     */
    explicit TripEndCallbackManager(
        QObject *parent = nullptr);

    /**
     * @brief Registers a global callback for all trip ends
     * @param callbackId Unique identifier for the callback
     * @param callback Function to call when any trip ends
     */
    void registerGlobalCallback(
        const QString                           &callbackId,
        std::function<void(const TripEndData &)> callback);

    /**
     * @brief Registers a callback for specific trip
     * @param tripId Trip identifier to watch
     * @param callbackId Unique identifier for the callback
     * @param callback Function to call when this trip ends
     */
    void registerTripCallback(
        const QString &tripId, const QString &callbackId,
        std::function<void(const TripEndData &)> callback);

    /**
     * @brief Registers a callback for specific network
     * @param networkName Network identifier to watch
     * @param callbackId Unique identifier for the callback
     * @param callback Function to call for trips in this
     * network
     */
    void registerNetworkCallback(
        const QString &networkName,
        const QString &callbackId,
        std::function<void(const TripEndData &)> callback);

    /**
     * @brief Unregisters a global callback
     * @param callbackId Identifier of callback to remove
     * @return True if callback was found and removed
     */
    bool
    unregisterGlobalCallback(const QString &callbackId);

    /**
     * @brief Unregisters a trip-specific callback
     * @param tripId Trip identifier
     * @param callbackId Identifier of callback to remove
     * @return True if callback was found and removed
     */
    bool unregisterTripCallback(const QString &tripId,
                                const QString &callbackId);

    /**
     * @brief Unregisters a network-specific callback
     * @param networkName Network identifier
     * @param callbackId Identifier of callback to remove
     * @return True if callback was found and removed
     */
    bool
    unregisterNetworkCallback(const QString &networkName,
                              const QString &callbackId);

    /**
     * @brief Unregisters all callbacks
     */
    void unregisterAllCallbacks();

public slots:
    /**
     * @brief Processes a trip end event
     * @param data Trip end data
     */
    void onTripEnded(const TripEndData &data);

signals:
    /**
     * @brief Signal emitted when a trip ends
     * @param tripData Trip end data
     */
    void tripEnded(const TripEndData &tripData);

private:
    // Global callbacks by ID
    QMap<QString, std::function<void(const TripEndData &)>>
        m_globalCallbacks;

    // Trip-specific callbacks by trip ID and callback ID
    QMap<QString,
         QMap<QString,
              std::function<void(const TripEndData &)>>>
        m_tripCallbacks;

    // Network-specific callbacks by network name and
    // callback ID
    QMap<QString,
         QMap<QString,
              std::function<void(const TripEndData &)>>>
        m_networkCallbacks;
};

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
