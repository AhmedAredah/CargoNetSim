/**
 * @file TruckState.h
 * @brief Represents truck state in simulation
 * @author Ahmed Aredah
 * @date March 21, 2025
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QVariantMap>

namespace CargoNetSim
{
namespace Backend
{
namespace TruckClient
{

class TruckState : public QObject
{
    Q_OBJECT

public:
    explicit TruckState(const QString &networkName,
                        int tripId, const QString &originId,
                        const QString &destinationId,
                        QObject       *parent = nullptr);
    explicit TruckState(const QJsonObject &jsonData,
                        QObject *parent = nullptr);

    QVariant    getMetric(const QString &metricName) const;
    QVariantMap info() const;
    QJsonObject toJson() const;

    void updateFromJson(const QJsonObject &jsonData);
    void updateInfoFromJson(const QJsonObject &jsonData);

    QString networkName() const
    {
        return m_networkName;
    }
    QString tripId() const
    {
        return QString::number(m_tripId);
    }
    QString originId() const
    {
        return m_originId;
    }
    QString destinationId() const
    {
        return m_destinationId;
    }
    QString linkId() const
    {
        return m_linkId;
    }
    double distance() const
    {
        return m_distance;
    }
    double speed() const
    {
        return m_speed;
    }
    double fuelConsumption() const
    {
        return m_fuelConsumption;
    }
    double travelTime() const
    {
        return m_travelTime;
    }
    bool isCompleted() const
    {
        return m_isCompleted;
    }

signals:
    void tripEnded();

private:
    QString m_networkName;
    int     m_tripId;
    QString m_originId;
    QString m_destinationId;
    QString m_linkId;
    double  m_distance        = 0.0;
    double  m_speed           = 0.0;
    double  m_fuelConsumption = 0.0;
    double  m_travelTime      = 0.0;
    bool    m_isCompleted     = false;
};

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim

Q_DECLARE_METATYPE(
    CargoNetSim::Backend::TruckClient::TruckState)
Q_DECLARE_METATYPE(
    CargoNetSim::Backend::TruckClient::TruckState *)
