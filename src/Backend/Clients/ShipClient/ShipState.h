#pragma once

#include <QObject>
#include <QMutex>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <QMap>
#include <QList>
#include <QString>
#include <QThread>
#include <QDateTime>
#include <QWaitCondition>

#include "Backend/Commons/ClientType.h"
#include "Backend/Clients/BaseClient/SimulationClientBase.h"


namespace CargoNetSim {
namespace Backend {

/**
 * @brief Represents the state of a ship in the simulation
 */
class ShipState {
public:
    /**
     * @brief Constructor from JSON data
     * @param shipData Ship state data as JSON
     */
    explicit ShipState(const QJsonObject& shipData);

    /**
     * @brief Get value of a specific metric
     * @param metricName Name of the metric
     * @return Value as QVariant, invalid if not found
     */
    QVariant getMetric(const QString& metricName) const;

    /**
     * @brief Get all metrics as a map
     * @return Map of all metrics
     */
    QVariantMap info() const;

    /**
     * @brief Convert to JSON
     * @return JSON representation of ship state
     */
    QJsonObject toJson() const;

    // Accessor methods for common properties
    QString shipId() const;
    double travelledDistance() const;
    double currentSpeed() const;
    double currentAcceleration() const;
    bool isLoaded() const;
    bool reachedDestination() const;
    double tripTime() const;
    int containersCount() const;
    QString closestPort() const;

private:
    QString m_shipId;
    double m_travelledDistance;
    double m_currentAcceleration;
    double m_previousAcceleration;
    double m_currentSpeed;
    double m_previousSpeed;
    double m_totalThrust;
    double m_totalResistance;
    double m_vesselWeight;
    double m_cargoWeight;
    bool m_isOn;
    bool m_outOfEnergy;
    bool m_loaded;
    bool m_reachedDestination;
    double m_tripTime;
    int m_containersCount;
    QString m_closestPort;

    // Energy and fuel consumption
    double m_energyConsumption;
    QMap<QString, double> m_fuelConsumption;
    double m_carbonDioxideEmitted;

    // Energy sources
    QList<QVariantMap> m_energySources;

    // Position
    double m_latitude;
    double m_longitude;
    QList<double> m_position;

    // Environmental conditions
    double m_waterDepth;
    double m_salinity;
    double m_temperature;
    double m_waveHeight;
    double m_waveLength;
    double m_waveAngularFrequency;
};

} // namespace Backend
} // namespace CargoNetSim

// Declare metatypes
Q_DECLARE_METATYPE(CargoNetSim::Backend::ShipState)
Q_DECLARE_METATYPE(CargoNetSim::Backend::ShipState*)
