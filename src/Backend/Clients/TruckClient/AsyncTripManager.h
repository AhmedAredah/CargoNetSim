/**
 * @file AsyncTripManager.h
 * @brief Manages asynchronous trip operations
 * @author Ahmed Aredah
 * @date 2025-03-22
 */

#pragma once

#include <QFuture>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QPromise>
#include <QString>
#include <containerLib/container.h>

namespace CargoNetSim {
namespace Backend {
namespace TruckClient {

/**
 * @struct TripRequest
 * @brief Represents a trip request parameters
 */
struct TripRequest {
    QString networkName;   ///< Network identifier
    int     originId;      ///< Origin node ID
    int     destinationId; ///< Destination node ID
    QList<ContainerCore::Container *>
        containers; ///< Associated containers
};

/**
 * @struct TripResult
 * @brief Represents the result of a completed trip
 */
struct TripResult {
    QString tripId;          ///< Trip identifier
    QString networkName;     ///< Network identifier
    int     originId;        ///< Origin node ID
    int     destinationId;   ///< Destination node ID
    double  distance;        ///< Trip distance in km
    double  fuelConsumption; ///< Fuel consumed during trip
    double  travelTime;      ///< Trip duration
    bool
        successful; ///< True if trip completed successfully
    QString
        errorMessage; ///< Error message (if not successful)
};

/**
 * @class AsyncTripManager
 * @brief Manages asynchronous trip operations
 *
 * Provides a Future/Promise based API for handling trips
 * that complete asynchronously after simulation runs.
 */
class AsyncTripManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent The parent QObject
     */
    explicit AsyncTripManager(QObject *parent = nullptr);

    /**
     * @brief Adds a trip asynchronously
     * @param request The trip request
     * @return Future that will complete when trip ends
     */
    QFuture<TripResult>
    addTripAsync(const TripRequest &request);

    /**
     * @brief Registers a known trip for tracking
     * @param tripId Trip identifier
     * @param request Original trip request
     * @return Future that will complete when trip ends
     */
    QFuture<TripResult>
    registerTrip(const QString     &tripId,
                 const TripRequest &request);

    /**
     * @brief Cancels a pending trip
     * @param tripId Trip identifier
     * @return True if trip was cancelled
     */
    bool cancelTrip(const QString &tripId);

public slots:
    /**
     * @brief Handles trip end event
     * @param networkName Network identifier
     * @param tripId Trip identifier
     * @param resultData Trip result data
     */
    void onTripEnded(const QString     &networkName,
                     const QString     &tripId,
                     const QJsonObject &resultData);

    /**
     * @brief Handles trip error event
     * @param tripId Trip identifier
     * @param errorMessage Error message
     */
    void onTripError(const QString &tripId,
                     const QString &errorMessage);

private:
    // Container for trip promises
    struct TripPromiseData {
        QPromise<TripResult> promise;
        TripRequest          request;
    };

    // Map of trip IDs to promise data
    QMap<QString, TripPromiseData *> m_pendingTrips;
};

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
