#include "TrainState.h"

/**
 * @file TrainState.cpp
 * @brief Implementation of TrainState class
 * @author Ahmed Aredah
 * @date March 20, 2025
 *
 * Implements TrainState with detailed comments for audit
 * and review, managing train state in the simulation.
 */

namespace CargoNetSim {
namespace Backend {
namespace TrainClient {

TrainState::TrainState(const QJsonObject &trainData)
    : m_cumEnergyStat(trainData["cumEnergyStat"].toDouble())
    , m_cumulativeDelayTime(
          trainData["cumulativeDelayTimeStat"].toDouble())
    , m_cumulativeMaxDelayTime(
          trainData["cumulativeMaxDelayTimeStat"]
              .toDouble())
    , m_cumulativeStoppedStat(
          trainData["cumulativeStoppedStat"].toDouble())
    , m_tripTime(trainData["tripTime"].toDouble())
    , m_currentAcceleration(
          trainData["currentAcceleration"].toDouble())
    , m_currentResistanceForces(
          trainData["currentResistanceForces"].toDouble())
    , m_currentSpeed(trainData["currentSpeed"].toDouble())
    , m_currentTractiveForce(
          trainData["currentTractiveForce"].toDouble())
    , m_currentUsedTractivePower(
          trainData["currentUsedTractivePower"].toDouble())
    , m_isLoaded(trainData["isLoaded"].toBool())
    , m_isOn(trainData["isOn"].toBool())
    , m_outOfEnergy(trainData["outOfEnergy"].toBool())
    , m_reachedDestination(
          trainData["reachedDestination"].toBool())
    , m_totalEnergyConsumed(
          trainData["totalEnergyConsumed"].toDouble())
    , m_totalEnergyRegenerated(
          trainData["totalEnergyRegenerated"].toDouble())
    , m_totalCarbonDioxideEmitted(
          trainData["totalCarbonDioxideEmitted"].toDouble())
    , m_totalLength(trainData["totalLength"].toInt())
    , m_totalMass(trainData["totalMass"].toDouble())
    , m_trainUserId(
          trainData["trainUserID"].toString("Unknown"))
    , m_travelledDistance(
          trainData["travelledDistance"].toDouble())
    , m_containersCount(
          trainData["containersCount"].toInt()) {
    // Extract fuel consumption data from JSON
    QJsonObject fuelObj =
        trainData["totalFuelConsumed"].toObject();

    // Iterate over fuel types and their values
    for (auto it = fuelObj.begin(); it != fuelObj.end();
         ++it) {
        // Store each fuel type and its consumption
        m_totalFuelConsumed[it.key()] =
            it.value().toDouble();
    }

    // Populate the metrics map for dynamic access
    m_metrics["totalFuelConsumed"] =
        QVariant::fromValue(m_totalFuelConsumed);
    m_metrics["cumEnergyStat"] = m_cumEnergyStat;
    m_metrics["cumulativeDelayTime"] =
        m_cumulativeDelayTime;
    m_metrics["cumulativeMaxDelayTime"] =
        m_cumulativeMaxDelayTime;
    m_metrics["cumulativeStoppedStat"] =
        m_cumulativeStoppedStat;
    m_metrics["tripTime"] = m_tripTime;
    m_metrics["currentAcceleration"] =
        m_currentAcceleration;
    m_metrics["currentResistanceForces"] =
        m_currentResistanceForces;
    m_metrics["currentSpeed"] = m_currentSpeed;
    m_metrics["currentTractiveForce"] =
        m_currentTractiveForce;
    m_metrics["currentUsedTractivePower"] =
        m_currentUsedTractivePower;
    m_metrics["isLoaded"]           = m_isLoaded;
    m_metrics["isOn"]               = m_isOn;
    m_metrics["outOfEnergy"]        = m_outOfEnergy;
    m_metrics["reachedDestination"] = m_reachedDestination;
    m_metrics["totalEnergyConsumed"] =
        m_totalEnergyConsumed;
    m_metrics["totalEnergyRegenerated"] =
        m_totalEnergyRegenerated;
    m_metrics["totalCarbonDioxideEmitted"] =
        m_totalCarbonDioxideEmitted;
    m_metrics["totalLength"]       = m_totalLength;
    m_metrics["totalMass"]         = m_totalMass;
    m_metrics["trainUserId"]       = m_trainUserId;
    m_metrics["travelledDistance"] = m_travelledDistance;
    m_metrics["containersCount"]   = m_containersCount;
}

QVariant
TrainState::getMetric(const QString &metricName) const {
    // Return the value of the requested metric
    return m_metrics.value(metricName);
}

QJsonObject TrainState::toJson() const {
    // Create a JSON object to hold the train state
    QJsonObject obj;

    // Convert fuel map to JSON object
    QMap<QString, QVariant> fuelVariantMap;
    for (const QString &key : m_totalFuelConsumed.keys()) {
        fuelVariantMap[key] =
            QVariant(m_totalFuelConsumed[key]);
    }
    obj["totalFuelConsumed"] =
        QJsonObject::fromVariantMap(fuelVariantMap);

    // Add scalar metrics to JSON
    obj["cumEnergyStat"]           = m_cumEnergyStat;
    obj["cumulativeDelayTimeStat"] = m_cumulativeDelayTime;
    obj["cumulativeMaxDelayTimeStat"] =
        m_cumulativeMaxDelayTime;
    obj["cumulativeStoppedStat"] = m_cumulativeStoppedStat;
    obj["tripTime"]              = m_tripTime;
    obj["currentAcceleration"]   = m_currentAcceleration;
    obj["currentResistanceForces"] =
        m_currentResistanceForces;
    obj["currentSpeed"]         = m_currentSpeed;
    obj["currentTractiveForce"] = m_currentTractiveForce;
    obj["currentUsedTractivePower"] =
        m_currentUsedTractivePower;

    // Add boolean states to JSON
    obj["isLoaded"]           = m_isLoaded;
    obj["isOn"]               = m_isOn;
    obj["outOfEnergy"]        = m_outOfEnergy;
    obj["reachedDestination"] = m_reachedDestination;

    // Add remaining scalar metrics
    obj["totalEnergyConsumed"] = m_totalEnergyConsumed;
    obj["totalEnergyRegenerated"] =
        m_totalEnergyRegenerated;
    obj["totalCarbonDioxideEmitted"] =
        m_totalCarbonDioxideEmitted;
    obj["totalLength"]       = m_totalLength;
    obj["totalMass"]         = m_totalMass;
    obj["trainUserID"]       = m_trainUserId;
    obj["travelledDistance"] = m_travelledDistance;
    obj["containersCount"]   = m_containersCount;

    // Return the completed JSON object
    return obj;
}

} // namespace TrainClient
} // namespace Backend
} // namespace CargoNetSim
