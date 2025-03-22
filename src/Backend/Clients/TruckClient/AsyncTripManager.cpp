/**
 * @file AsyncTripManager.cpp
 * @brief Implements asynchronous trip operations
 * @author Ahmed Aredah
 * @date 2025-03-22
 */

#include "AsyncTripManager.h"

namespace CargoNetSim {
namespace Backend {
namespace TruckClient {

AsyncTripManager::AsyncTripManager(QObject *parent)
    : QObject(parent) {}

QFuture<TripResult>
AsyncTripManager::addTripAsync(const TripRequest &request) {
    // Create a placeholder trip ID
    // (the actual ID will be assigned by the simulator)
    QString tempTripId =
        QString("%1_%2_%3_%4")
            .arg(request.networkName)
            .arg(request.originId)
            .arg(request.destinationId)
            .arg(QDateTime::currentMSecsSinceEpoch());

    // Register the trip with the placeholder ID
    return registerTrip(tempTripId, request);
}

QFuture<TripResult>
AsyncTripManager::registerTrip(const QString     &tripId,
                               const TripRequest &request) {
    // Create a promise for this trip
    QPromise<TripResult> promise;
    QFuture<TripResult>  future = promise.future();

    // Store the promise with request details
    TripPromiseData *promiseData = new TripPromiseData();
    promiseData->promise         = std::move(promise);
    promiseData->request         = request;

    // Add to pending trips map
    m_pendingTrips.insert(tripId, promiseData);

    return future;
}

bool AsyncTripManager::cancelTrip(const QString &tripId) {
    if (!m_pendingTrips.contains(tripId)) {
        return false;
    }

    // Get the promise data
    TripPromiseData *data = m_pendingTrips[tripId];

    // Create a cancelled result
    TripResult result;
    result.tripId          = tripId;
    result.networkName     = data->request.networkName;
    result.originId        = data->request.originId;
    result.destinationId   = data->request.destinationId;
    result.distance        = 0.0;
    result.fuelConsumption = 0.0;
    result.travelTime      = 0.0;
    result.successful      = false;
    result.errorMessage    = "Trip cancelled by user";

    // Set the result and finish the promise
    data->promise.addResult(result);
    data->promise.finish();

    // Remove from pending trips
    m_pendingTrips.remove(tripId);

    return true;
}

void AsyncTripManager::onTripEnded(
    const QString &networkName, const QString &tripId,
    const QJsonObject &resultData) {
    if (!m_pendingTrips.contains(tripId)) {
        // Unknown trip - might be handled elsewhere
        return;
    }

    // Get the promise data
    TripPromiseData *data = m_pendingTrips[tripId];

    // Create the result
    TripResult result;
    result.tripId      = tripId;
    result.networkName = networkName;
    result.originId    = resultData["Origin"].toInt();
    result.destinationId =
        resultData["Destination"].toInt();
    result.distance =
        resultData["Trip_Distance"].toDouble();
    result.fuelConsumption =
        resultData["Fuel_Consumption"].toDouble();
    result.travelTime =
        resultData["Travel_Time"].toDouble();
    result.successful   = true;
    result.errorMessage = "";

    // Set the result and finish the promise
    data->promise.addResult(result);
    data->promise.finish();

    // Remove from pending trips
    m_pendingTrips.remove(tripId);
}

void AsyncTripManager::onTripError(
    const QString &tripId, const QString &errorMessage) {
    if (!m_pendingTrips.contains(tripId)) {
        // Unknown trip - might be handled elsewhere
        return;
    }

    // Get the promise data
    TripPromiseData *data = m_pendingTrips[tripId];

    // Create the result with error
    TripResult result;
    result.tripId          = tripId;
    result.networkName     = data->request.networkName;
    result.originId        = data->request.originId;
    result.destinationId   = data->request.destinationId;
    result.distance        = 0.0;
    result.fuelConsumption = 0.0;
    result.travelTime      = 0.0;
    result.successful      = false;
    result.errorMessage    = errorMessage;

    // Set the result and finish the promise
    data->promise.addResult(result);
    data->promise.finish();

    // Remove from pending trips
    m_pendingTrips.remove(tripId);
}

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
