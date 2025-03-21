#include "ShipSimulationClient.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QThread>
#include <QCoreApplication>
#include <QFile>
#include <QDir>

// Placeholder includes (you would have these implemented)
#include "Backend/Models/ShipSystem.h"
// #include "TerminalGraphServer.h"
// #include "SimulatorTimeServer.h"
// #include "ProgressBarManager.h"
// #include "ApplicationLogger.h"

namespace CargoNetSim {
namespace Backend {
namespace ShipClient {

ShipState::ShipState(const QJsonObject& shipData)
    : m_shipId(shipData.value("shipID").toString("Unknown")),
      m_travelledDistance(
          shipData.value("travelledDistance").toDouble(0.0)),
      m_currentAcceleration(
          shipData.value("currentAcceleration").toDouble(0.0)),
      m_previousAcceleration(
          shipData.value("previousAcceleration").toDouble(0.0)),
      m_currentSpeed(
          shipData.value("currentSpeed").toDouble(0.0)),
      m_previousSpeed(
          shipData.value("previousSpeed").toDouble(0.0)),
      m_totalThrust(
          shipData.value("totalThrust").toDouble(0.0)),
      m_totalResistance(
          shipData.value("totalResistance").toDouble(0.0)),
      m_vesselWeight(
          shipData.value("vesselWeight").toDouble(0.0)),
      m_cargoWeight(
          shipData.value("cargoWeight").toDouble(0.0)),
      m_isOn(
          shipData.value("isOn").toBool(false)),
      m_outOfEnergy(
          shipData.value("outOfEnergy").toBool(false)),
      m_loaded(
          shipData.value("loaded").toBool(false)),
      m_reachedDestination(
          shipData.value("reachedDestination").toBool(false)),
      m_tripTime(
          shipData.value("tripTime").toDouble(0.0)),
      m_containersCount(
          shipData.value("containersCount").toInt(0)),
      m_closestPort(
          shipData.value("closestPort").toString("Unknown")),
      m_energyConsumption(0.0),
      m_carbonDioxideEmitted(
          shipData.value("carbonDioxideEmitted").toDouble(0.0))
{
    // Parse consumption data
    QJsonObject consumption = 
        shipData.value("consumption").toObject();
    m_energyConsumption = 
        consumption.value("energyConsumption").toDouble(0.0);
    
    // Parse fuel consumption
    QJsonArray fuelConsumption = 
        consumption.value("fuelConsumption").toArray();
    for (const QJsonValue& fuelEntry : fuelConsumption) {
        QJsonObject fuelObj = fuelEntry.toObject();
        QString fuelType = 
            fuelObj.value("fuelType").toString("Unknown");
        double consumedVolume = 
            fuelObj.value("consumedVolumeLiters").toDouble(0.0);
        m_fuelConsumption[fuelType] = consumedVolume;
    }
    
    // Parse energy sources
    QJsonArray energySources = 
        shipData.value("energySources").toArray();
    for (const QJsonValue& sourceValue : energySources) {
        QJsonObject source = sourceValue.toObject();
        QVariantMap sourceMap;
        sourceMap["capacity"] = source.value("capacity").toDouble(0.0);
        sourceMap["fuelType"] = 
            source.value("fuelType").toString("Unknown");
        sourceMap["energyConsumed"] = 
            source.value("energyConsumed").toDouble(0.0);
        sourceMap["weight"] = source.value("weight").toDouble(0.0);
        m_energySources.append(sourceMap);
    }
    
    // Parse position data
    QJsonObject position = shipData.value("position").toObject();
    m_latitude = position.value("latitude").toDouble(0.0);
    m_longitude = position.value("longitude").toDouble(0.0);
    
    QJsonArray posArray = position.value("position").toArray();
    for (const QJsonValue& pos : posArray) {
        m_position.append(pos.toDouble(0.0));
    }
    
    // Parse environmental data
    QJsonObject env = shipData.value("environment").toObject();
    m_waterDepth = env.value("waterDepth").toDouble(0.0);
    m_salinity = env.value("salinity").toDouble(0.0);
    m_temperature = env.value("temperature").toDouble(0.0);
    m_waveHeight = env.value("waveHeight").toDouble(0.0);
    m_waveLength = env.value("waveLength").toDouble(0.0);
    m_waveAngularFrequency = 
        env.value("waveAngularFrequency").toDouble(0.0);
}

QVariant ShipState::getMetric(const QString& metricName) const
{
    if (metricName == "shipId") return m_shipId;
    if (metricName == "travelledDistance") 
        return m_travelledDistance;
    if (metricName == "currentAcceleration") 
        return m_currentAcceleration;
    if (metricName == "previousAcceleration") 
        return m_previousAcceleration;
    if (metricName == "currentSpeed") return m_currentSpeed;
    if (metricName == "previousSpeed") return m_previousSpeed;
    if (metricName == "totalThrust") return m_totalThrust;
    if (metricName == "totalResistance") return m_totalResistance;
    if (metricName == "vesselWeight") return m_vesselWeight;
    if (metricName == "cargoWeight") return m_cargoWeight;
    if (metricName == "isOn") return m_isOn;
    if (metricName == "outOfEnergy") return m_outOfEnergy;
    if (metricName == "loaded") return m_loaded;
    if (metricName == "reachedDestination") 
        return m_reachedDestination;
    if (metricName == "tripTime") return m_tripTime;
    if (metricName == "containersCount") return m_containersCount;
    if (metricName == "closestPort") return m_closestPort;
    if (metricName == "energyConsumption") 
        return m_energyConsumption;
    if (metricName == "carbonDioxideEmitted") 
        return m_carbonDioxideEmitted;
    
    // If we don't have a direct match
    return QVariant();
}

QVariantMap ShipState::info() const
{
    QVariantMap info;
    info["shipId"] = m_shipId;
    info["travelledDistance"] = m_travelledDistance;
    info["currentAcceleration"] = m_currentAcceleration;
    info["previousAcceleration"] = m_previousAcceleration;
    info["currentSpeed"] = m_currentSpeed;
    info["previousSpeed"] = m_previousSpeed;
    info["totalThrust"] = m_totalThrust;
    info["totalResistance"] = m_totalResistance;
    info["vesselWeight"] = m_vesselWeight;
    info["cargoWeight"] = m_cargoWeight;
    info["isOn"] = m_isOn;
    info["outOfEnergy"] = m_outOfEnergy;
    info["loaded"] = m_loaded;
    info["reachedDestination"] = m_reachedDestination;
    info["tripTime"] = m_tripTime;
    info["containersCount"] = m_containersCount;
    info["closestPort"] = m_closestPort;
    info["energyConsumption"] = m_energyConsumption;
    
    // Add fuel consumption
    QVariantMap fuelMap;
    for (auto it = m_fuelConsumption.constBegin(); 
         it != m_fuelConsumption.constEnd(); 
         ++it) {
        fuelMap[it.key()] = it.value();
    }
    info["fuelConsumption"] = fuelMap;
    
    info["carbonDioxideEmitted"] = m_carbonDioxideEmitted;
    // info["energySources"] = QVariant(m_energySources);  // TODO
    
    // Position information
    QVariantMap positionMap;
    positionMap["latitude"] = m_latitude;
    positionMap["longitude"] = m_longitude;
    // positionMap["position"] = QVariant(m_position);  // TODO
    info["position"] = positionMap;
    
    // Environmental information
    QVariantMap envMap;
    envMap["waterDepth"] = m_waterDepth;
    envMap["salinity"] = m_salinity;
    envMap["temperature"] = m_temperature;
    envMap["waveHeight"] = m_waveHeight;
    envMap["waveLength"] = m_waveLength;
    envMap["waveAngularFrequency"] = m_waveAngularFrequency;
    info["environment"] = envMap;
    
    return info;
}

QJsonObject ShipState::toJson() const
{
    QJsonObject json;
    json["shipID"] = m_shipId;
    json["travelledDistance"] = m_travelledDistance;
    json["currentAcceleration"] = m_currentAcceleration;
    json["previousAcceleration"] = m_previousAcceleration;
    json["currentSpeed"] = m_currentSpeed;
    json["previousSpeed"] = m_previousSpeed;
    json["totalThrust"] = m_totalThrust;
    json["totalResistance"] = m_totalResistance;
    json["vesselWeight"] = m_vesselWeight;
    json["cargoWeight"] = m_cargoWeight;
    json["isOn"] = m_isOn;
    json["outOfEnergy"] = m_outOfEnergy;
    json["loaded"] = m_loaded;
    json["reachedDestination"] = m_reachedDestination;
    json["tripTime"] = m_tripTime;
    json["containersCount"] = m_containersCount;
    json["closestPort"] = m_closestPort;
    
    // Consumption
    QJsonObject consumption;
    consumption["energyConsumption"] = m_energyConsumption;
    
    QJsonArray fuelConsumption;
    for (auto it = m_fuelConsumption.constBegin(); 
         it != m_fuelConsumption.constEnd(); 
         ++it) {
        QJsonObject fuelObj;
        fuelObj["fuelType"] = it.key();
        fuelObj["consumedVolumeLiters"] = it.value();
        fuelConsumption.append(fuelObj);
    }
    consumption["fuelConsumption"] = fuelConsumption;
    json["consumption"] = consumption;
    
    json["carbonDioxideEmitted"] = m_carbonDioxideEmitted;
    
    // Energy sources
    QJsonArray energySources;
    for (const QVariantMap& source : m_energySources) {
        QJsonObject sourceObj;
        sourceObj["capacity"] = source["capacity"].toDouble();
        sourceObj["fuelType"] = source["fuelType"].toString();
        sourceObj["energyConsumed"] = 
            source["energyConsumed"].toDouble();
        sourceObj["weight"] = source["weight"].toDouble();
        energySources.append(sourceObj);
    }
    json["energySources"] = energySources;
    
    // Position
    QJsonObject position;
    position["latitude"] = m_latitude;
    position["longitude"] = m_longitude;
    
    QJsonArray positionArray;
    for (double pos : m_position) {
        positionArray.append(pos);
    }
    position["position"] = positionArray;
    json["position"] = position;
    
    // Environment
    QJsonObject environment;
    environment["waterDepth"] = m_waterDepth;
    environment["salinity"] = m_salinity;
    environment["temperature"] = m_temperature;
    environment["waveHeight"] = m_waveHeight;
    environment["waveLength"] = m_waveLength;
    environment["waveAngularFrequency"] = m_waveAngularFrequency;
    json["environment"] = environment;
    
    return json;
}

QString ShipState::shipId() const
{
    return m_shipId;
}

double ShipState::travelledDistance() const
{
    return m_travelledDistance;
}

double ShipState::currentSpeed() const
{
    return m_currentSpeed;
}

double ShipState::currentAcceleration() const
{
    return m_currentAcceleration;
}

bool ShipState::isLoaded() const
{
    return m_loaded;
}

bool ShipState::reachedDestination() const
{
    return m_reachedDestination;
}

double ShipState::tripTime() const
{
    return m_tripTime;
}

int ShipState::containersCount() const
{
    return m_containersCount;
}

QString ShipState::closestPort() const
{
    return m_closestPort;
}

} // namespace ShipClient
} // namespace Backend
} // namespace CargoNetSim
