/**
 * @file IntegrationLink.h
 * @brief Defines link in transportation network
 * @author [Your Name]
 * @date 2025-03-22
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>

namespace CargoNetSim
{
namespace Backend
{
namespace TruckClient
{

/**
 * @class IntegrationLink
 * @brief Represents a link in the truck network
 *
 * Models a connection between nodes with detailed traffic
 * properties such as speed, capacity, and signal phasing.
 */
class IntegrationLink : public QObject
{
    Q_OBJECT

    // Link properties as Q_PROPERTY for meta-object access
    Q_PROPERTY(int linkId READ linkId WRITE setLinkId NOTIFY
                   linkChanged)
    Q_PROPERTY(int upstreamNodeId READ upstreamNodeId WRITE
                   setUpstreamNodeId NOTIFY linkChanged)
    Q_PROPERTY(
        int downstreamNodeId READ downstreamNodeId WRITE
            setDownstreamNodeId NOTIFY linkChanged)
    Q_PROPERTY(float length READ length WRITE setLength
                   NOTIFY linkChanged)
    Q_PROPERTY(float freeSpeed READ freeSpeed WRITE
                   setFreeSpeed NOTIFY linkChanged)
    Q_PROPERTY(
        float saturationFlow READ saturationFlow WRITE
            setSaturationFlow NOTIFY linkChanged)
    Q_PROPERTY(float lanes READ lanes WRITE setLanes NOTIFY
                   linkChanged)
    Q_PROPERTY(
        float speedCoeffVariation READ speedCoeffVariation
            WRITE setSpeedCoeffVariation NOTIFY linkChanged)
    Q_PROPERTY(
        float speedAtCapacity READ speedAtCapacity WRITE
            setSpeedAtCapacity NOTIFY linkChanged)
    Q_PROPERTY(float jamDensity READ jamDensity WRITE
                   setJamDensity NOTIFY linkChanged)
    Q_PROPERTY(
        int turnProhibition READ turnProhibition WRITE
            setTurnProhibition NOTIFY linkChanged)
    Q_PROPERTY(
        int prohibitionStart READ prohibitionStart WRITE
            setProhibitionStart NOTIFY linkChanged)
    Q_PROPERTY(int prohibitionEnd READ prohibitionEnd WRITE
                   setProhibitionEnd NOTIFY linkChanged)
    Q_PROPERTY(int opposingLink1 READ opposingLink1 WRITE
                   setOpposingLink1 NOTIFY linkChanged)
    Q_PROPERTY(int opposingLink2 READ opposingLink2 WRITE
                   setOpposingLink2 NOTIFY linkChanged)
    Q_PROPERTY(int trafficSignal READ trafficSignal WRITE
                   setTrafficSignal NOTIFY linkChanged)
    Q_PROPERTY(int phase1 READ phase1 WRITE setPhase1 NOTIFY
                   linkChanged)
    Q_PROPERTY(int phase2 READ phase2 WRITE setPhase2 NOTIFY
                   linkChanged)
    Q_PROPERTY(int vehicleClassProhibition READ
                   vehicleClassProhibition WRITE
                       setVehicleClassProhibition NOTIFY
                           linkChanged)
    Q_PROPERTY(
        int surveillanceLevel READ surveillanceLevel WRITE
            setSurveillanceLevel NOTIFY linkChanged)
    Q_PROPERTY(QString description READ description WRITE
                   setDescription NOTIFY linkChanged)
    Q_PROPERTY(float lengthScale READ lengthScale WRITE
                   setLengthScale NOTIFY linkChanged)
    Q_PROPERTY(float speedScale READ speedScale WRITE
                   setSpeedScale NOTIFY linkChanged)
    Q_PROPERTY(
        float saturationFlowScale READ saturationFlowScale
            WRITE setSaturationFlowScale NOTIFY linkChanged)
    Q_PROPERTY(
        float speedAtCapacityScale READ speedAtCapacityScale
            WRITE setSpeedAtCapacityScale NOTIFY
                linkChanged)
    Q_PROPERTY(
        float jamDensityScale READ jamDensityScale WRITE
            setJamDensityScale NOTIFY linkChanged)

public:
    /**
     * @brief Default constructor
     * @param parent The parent QObject
     */
    explicit IntegrationLink(QObject *parent = nullptr);

    /**
     * @brief Parameterized constructor
     * @param linkId Unique identifier for the link
     * @param upstreamNodeId Source node ID
     * @param downstreamNodeId Destination node ID
     * @param length Length of link in km
     * @param freeSpeed Free flow speed in km/h
     * @param saturationFlow Maximum flow in veh/h
     * @param lanes Number of lanes
     * @param speedCoeffVariation Speed variation
     * coefficient
     * @param speedAtCapacity Speed at capacity in km/h
     * @param jamDensity Maximum vehicle density in veh/km
     * @param turnProhibition Turn prohibition flag
     * @param prohibitionStart Start time of prohibition
     * @param prohibitionEnd End time of prohibition
     * @param opposingLink1 ID of first opposing link
     * @param opposingLink2 ID of second opposing link
     * @param trafficSignal Signal control type
     * @param phase1 First signal phase
     * @param phase2 Second signal phase
     * @param vehicleClassProhibition Vehicle class
     * restrictions
     * @param surveillanceLevel Monitoring level
     * @param description Text description of the link
     * @param lengthScale Scale factor for length
     * @param speedScale Scale factor for speed
     * @param saturationFlowScale Scale factor for flow
     * @param speedAtCapacityScale Scale factor for capacity
     * speed
     * @param jamDensityScale Scale factor for jam density
     * @param parent The parent QObject
     */
    IntegrationLink(
        int linkId, int upstreamNodeId,
        int downstreamNodeId, float length, float freeSpeed,
        float saturationFlow, float lanes,
        float speedCoeffVariation, float speedAtCapacity,
        float jamDensity, int turnProhibition,
        int prohibitionStart, int prohibitionEnd,
        int opposingLink1, int opposingLink2,
        int trafficSignal, int phase1, int phase2,
        int vehicleClassProhibition, int surveillanceLevel,
        const QString &description, float lengthScale,
        float speedScale, float saturationFlowScale,
        float speedAtCapacityScale, float jamDensityScale,
        QObject *parent = nullptr);

    /**
     * @brief Constructor from JSON data
     * @param json JSON object containing link data
     * @param parent The parent QObject
     */
    IntegrationLink(const QJsonObject &json,
                    QObject           *parent = nullptr);

    /**
     * @brief Converts the link to a JSON object
     * @return QJsonObject representation of the link
     */
    QJsonObject toDict() const;

    /**
     * @brief Creates a link from JSON data
     * @param data JSON object containing link data
     * @param parent The parent QObject
     * @return Pointer to the new link
     */
    static IntegrationLink *
    fromDict(const QJsonObject &data,
             QObject           *parent = nullptr);

    // Getters
    int linkId() const
    {
        return m_linkId;
    }
    int upstreamNodeId() const
    {
        return m_upstreamNodeId;
    }
    int downstreamNodeId() const
    {
        return m_downstreamNodeId;
    }
    float length() const
    {
        return m_length;
    }
    float freeSpeed() const
    {
        return m_freeSpeed;
    }
    float saturationFlow() const
    {
        return m_saturationFlow;
    }
    float lanes() const
    {
        return m_lanes;
    }
    float speedCoeffVariation() const
    {
        return m_speedCoeffVariation;
    }
    float speedAtCapacity() const
    {
        return m_speedAtCapacity;
    }
    float jamDensity() const
    {
        return m_jamDensity;
    }
    int turnProhibition() const
    {
        return m_turnProhibition;
    }
    int prohibitionStart() const
    {
        return m_prohibitionStart;
    }
    int prohibitionEnd() const
    {
        return m_prohibitionEnd;
    }
    int opposingLink1() const
    {
        return m_opposingLink1;
    }
    int opposingLink2() const
    {
        return m_opposingLink2;
    }
    int trafficSignal() const
    {
        return m_trafficSignal;
    }
    int phase1() const
    {
        return m_phase1;
    }
    int phase2() const
    {
        return m_phase2;
    }
    int vehicleClassProhibition() const
    {
        return m_vehicleClassProhibition;
    }
    int surveillanceLevel() const
    {
        return m_surveillanceLevel;
    }
    QString description() const
    {
        return m_description;
    }
    float lengthScale() const
    {
        return m_lengthScale;
    }
    float speedScale() const
    {
        return m_speedScale;
    }
    float saturationFlowScale() const
    {
        return m_saturationFlowScale;
    }
    float speedAtCapacityScale() const
    {
        return m_speedAtCapacityScale;
    }
    float jamDensityScale() const
    {
        return m_jamDensityScale;
    }

    // Setters
    void setLinkId(int linkId);
    void setUpstreamNodeId(int upstreamNodeId);
    void setDownstreamNodeId(int downstreamNodeId);
    void setLength(float length);
    void setFreeSpeed(float freeSpeed);
    void setSaturationFlow(float saturationFlow);
    void setLanes(float lanes);
    void setSpeedCoeffVariation(float speedCoeffVariation);
    void setSpeedAtCapacity(float speedAtCapacity);
    void setJamDensity(float jamDensity);
    void setTurnProhibition(int turnProhibition);
    void setProhibitionStart(int prohibitionStart);
    void setProhibitionEnd(int prohibitionEnd);
    void setOpposingLink1(int opposingLink1);
    void setOpposingLink2(int opposingLink2);
    void setTrafficSignal(int trafficSignal);
    void setPhase1(int phase1);
    void setPhase2(int phase2);
    void
    setVehicleClassProhibition(int vehicleClassProhibition);
    void setSurveillanceLevel(int surveillanceLevel);
    void setDescription(const QString &description);
    void setLengthScale(float lengthScale);
    void setSpeedScale(float speedScale);
    void setSaturationFlowScale(float saturationFlowScale);
    void
    setSpeedAtCapacityScale(float speedAtCapacityScale);
    void setJamDensityScale(float jamDensityScale);

signals:
    /**
     * @brief Signal emitted when any link property changes
     */
    void linkChanged();

private:
    int m_linkId;           ///< Unique link identifier
    int m_upstreamNodeId;   ///< Source node identifier
    int m_downstreamNodeId; ///< Destination node identifier
    float m_length;         ///< Link length in km
    float m_freeSpeed;      ///< Free flow speed in km/h
    float m_saturationFlow; ///< Maximum flow in veh/h
    float m_lanes;          ///< Number of lanes
    float m_speedCoeffVariation; ///< Speed variation
                                 ///< coefficient
    float m_speedAtCapacity;  ///< Speed at capacity in km/h
    float m_jamDensity;       ///< Max density in veh/km
    int   m_turnProhibition;  ///< Turn prohibition flag
    int   m_prohibitionStart; ///< Start time of prohibition
    int   m_prohibitionEnd;   ///< End time of prohibition
    int   m_opposingLink1;    ///< ID of first opposing link
    int   m_opposingLink2; ///< ID of second opposing link
    int   m_trafficSignal; ///< Signal control type
    int   m_phase1;        ///< First signal phase
    int   m_phase2;        ///< Second signal phase
    int   m_vehicleClassProhibition; ///< Vehicle class
                                     ///< restrictions
    int     m_surveillanceLevel;     ///< Monitoring level
    QString m_description;           ///< Link description
    float   m_lengthScale; ///< Scale factor for length
    float   m_speedScale;  ///< Scale factor for speed
    float m_saturationFlowScale;  ///< Scale factor for flow
    float m_speedAtCapacityScale; ///< Factor for capacity
                                  ///< speed
    float m_jamDensityScale; ///< Factor for jam density
};

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
