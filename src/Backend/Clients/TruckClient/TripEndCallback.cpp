/**
 * @file TripEndCallback.cpp
 * @brief Implements trip end callback mechanism
 * @author Ahmed Aredah
 * @date 2025-03-22
 */

#include "TripEndCallback.h"

namespace CargoNetSim {
namespace Backend {
namespace TruckClient {

TripEndCallbackManager::TripEndCallbackManager(
    QObject *parent)
    : QObject(parent) {}

void TripEndCallbackManager::registerGlobalCallback(
    const QString                           &callbackId,
    std::function<void(const TripEndData &)> callback) {
    m_globalCallbacks[callbackId] = callback;
}

void TripEndCallbackManager::registerTripCallback(
    const QString &tripId, const QString &callbackId,
    std::function<void(const TripEndData &)> callback) {
    m_tripCallbacks[tripId][callbackId] = callback;
}

void TripEndCallbackManager::registerNetworkCallback(
    const QString &networkName, const QString &callbackId,
    std::function<void(const TripEndData &)> callback) {
    m_networkCallbacks[networkName][callbackId] = callback;
}

bool TripEndCallbackManager::unregisterGlobalCallback(
    const QString &callbackId) {
    if (!m_globalCallbacks.contains(callbackId)) {
        return false;
    }

    m_globalCallbacks.remove(callbackId);
    return true;
}

bool TripEndCallbackManager::unregisterTripCallback(
    const QString &tripId, const QString &callbackId) {
    if (!m_tripCallbacks.contains(tripId)
        || !m_tripCallbacks[tripId].contains(callbackId)) {
        return false;
    }

    m_tripCallbacks[tripId].remove(callbackId);

    // Clean up empty maps
    if (m_tripCallbacks[tripId].isEmpty()) {
        m_tripCallbacks.remove(tripId);
    }

    return true;
}

bool TripEndCallbackManager::unregisterNetworkCallback(
    const QString &networkName, const QString &callbackId) {
    if (!m_networkCallbacks.contains(networkName)
        || !m_networkCallbacks[networkName].contains(
            callbackId)) {
        return false;
    }

    m_networkCallbacks[networkName].remove(callbackId);

    // Clean up empty maps
    if (m_networkCallbacks[networkName].isEmpty()) {
        m_networkCallbacks.remove(networkName);
    }

    return true;
}

void TripEndCallbackManager::unregisterAllCallbacks() {
    m_globalCallbacks.clear();
    m_tripCallbacks.clear();
    m_networkCallbacks.clear();
}

void TripEndCallbackManager::onTripEnded(
    const TripEndData &data) {

    // Emit the signal for QObject connections
    emit tripEnded(data);

    // Execute global callbacks
    for (const auto &callback : m_globalCallbacks) {
        callback(data);
    }

    // Execute trip-specific callbacks
    if (m_tripCallbacks.contains(data.tripId)) {
        for (const auto &callback :
             m_tripCallbacks[data.tripId]) {
            callback(data);
        }
    }

    // Execute network-specific callbacks
    if (m_networkCallbacks.contains(data.networkName)) {
        for (const auto &callback :
             m_networkCallbacks[data.networkName]) {
            callback(data);
        }
    }
}

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
