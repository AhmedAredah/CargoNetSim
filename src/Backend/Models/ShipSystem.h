// ship_system.h
#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QtMath>
#include <QDir>

#include "../Commons/ClientType.h"

namespace CargoNetSim {
namespace Backend {

class ApplicationLogger {
public:
    static void log_error(const QString& message, ClientType clientType) {
        qCritical() << "Error [" << static_cast<int>(clientType) << "]: " << message;
    }
};

class Ship : public QObject {
    Q_OBJECT

public:
    Ship(QObject* parent = nullptr);
    Ship(
        const QString& shipId,
        const QVector<QVector<float>>& pathCoordinates,
        float maxSpeed,
        float waterlineLength,
        float lengthBetweenPerpendiculars,
        float beam,
        float draftAtForward,
        float draftAtAft,
        float volumetricDisplacement = -1.0f,
        float wettedHullSurface = -1.0f,
        float areaAboveWaterline = 0.0f,
        float bulbousBowCenterHeight = 0.0f,
        float bulbousBowArea = 0.0f,
        float immersedTransomArea = 0.0f,
        float entranceAngle = -1.0f,
        float surfaceRoughness = 0.0f,
        float buoyancyCenter = 0.0f,
        int sternShapeParam = -1,
        float midshipSectionCoef = -1.0f,
        float waterplaneAreaCoef = -1.0f,
        float prismaticCoef = -1.0f,
        float blockCoef = -1.0f,
        const QVector<QMap<QString, float>>& tanksDetails = QVector<QMap<QString, float>>(),
        int enginesPerPropeller = 1,
        const QVector<QMap<QString, float>>& engineTierII = QVector<QMap<QString, float>>(),
        const QVector<QMap<QString, float>>& engineTierIII = QVector<QMap<QString, float>>(),
        const QVector<QMap<QString, float>>& engineTierIICurve = QVector<QMap<QString, float>>(),
        const QVector<QMap<QString, float>>& engineTierIIICurve = QVector<QMap<QString, float>>(),
        float gearboxRatio = 0.0f,
        float gearboxEfficiency = 1.0f,
        float shaftEfficiency = 1.0f,
        int propellerCount = 1,
        float propellerDiameter = 0.0f,
        float propellerPitch = 0.0f,
        int propellerBladesCount = 4,
        float expandedAreaRatio = 0.0f,
        bool stopIfNoEnergy = false,
        float maxRudderAngle = -1.0f,
        float vesselWeight = 0.0f,
        float cargoWeight = 0.0f,
        const QMap<int, float>& appendagesWettedSurfaces = QMap<int, float>(),
        QObject* parent = nullptr
        );
    
    // Constructor from JSON
    Ship(const QJsonObject& json, QObject* parent = nullptr);
    
    QJsonObject toJson() const;
    QJsonObject toDict() const;
    Ship* copy() const;

    // Getters
    QString shipId() const { return m_shipId; }
    QVector<QVector<float>> pathCoordinates() const { return m_pathCoordinates; }
    float maxSpeed() const { return m_maxSpeed; }
    float waterlineLength() const { return m_waterlineLength; }
    float lengthBetweenPerpendiculars() const { return m_lengthBetweenPerpendiculars; }
    float beam() const { return m_beam; }
    float draftAtForward() const { return m_draftAtForward; }
    float draftAtAft() const { return m_draftAtAft; }
    float volumetricDisplacement() const { return m_volumetricDisplacement; }
    float wettedHullSurface() const { return m_wettedHullSurface; }
    float areaAboveWaterline() const { return m_areaAboveWaterline; }
    float bulbousBowCenterHeight() const { return m_bulbousBowCenterHeight; }
    float bulbousBowArea() const { return m_bulbousBowArea; }
    float immersedTransomArea() const { return m_immersedTransomArea; }
    float entranceAngle() const { return m_entranceAngle; }
    float surfaceRoughness() const { return m_surfaceRoughness; }
    float buoyancyCenter() const { return m_buoyancyCenter; }
    int sternShapeParam() const { return m_sternShapeParam; }
    float midshipSectionCoef() const { return m_midshipSectionCoef; }
    float waterplaneAreaCoef() const { return m_waterplaneAreaCoef; }
    float prismaticCoef() const { return m_prismaticCoef; }
    float blockCoef() const { return m_blockCoef; }
    QVector<QMap<QString, float>> tanksDetails() const { return m_tanksDetails; }
    int enginesPerPropeller() const { return m_enginesPerPropeller; }
    QVector<QMap<QString, float>> engineTierII() const { return m_engineTierII; }
    QVector<QMap<QString, float>> engineTierIII() const { return m_engineTierIII; }
    QVector<QMap<QString, float>> engineTierIICurve() const { return m_engineTierIICurve; }
    QVector<QMap<QString, float>> engineTierIIICurve() const { return m_engineTierIIICurve; }
    float gearboxRatio() const { return m_gearboxRatio; }
    float gearboxEfficiency() const { return m_gearboxEfficiency; }
    float shaftEfficiency() const { return m_shaftEfficiency; }
    int propellerCount() const { return m_propellerCount; }
    float propellerDiameter() const { return m_propellerDiameter; }
    float propellerPitch() const { return m_propellerPitch; }
    int propellerBladesCount() const { return m_propellerBladesCount; }
    float expandedAreaRatio() const { return m_expandedAreaRatio; }
    bool stopIfNoEnergy() const { return m_stopIfNoEnergy; }
    float maxRudderAngle() const { return m_maxRudderAngle; }
    float vesselWeight() const { return m_vesselWeight; }
    float cargoWeight() const { return m_cargoWeight; }
    QMap<int, float> appendagesWettedSurfaces() const { return m_appendagesWettedSurfaces; }

    // Setters
    void setShipId(const QString& shipId);
    void setPathCoordinates(const QVector<QVector<float>>& pathCoordinates);
    void setMaxSpeed(float maxSpeed);
    void setWaterlineLength(float waterlineLength);
    void setLengthBetweenPerpendiculars(float lengthBetweenPerpendiculars);
    void setBeam(float beam);
    void setDraftAtForward(float draftAtForward);
    void setDraftAtAft(float draftAtAft);
    void setVolumetricDisplacement(float volumetricDisplacement);
    void setWettedHullSurface(float wettedHullSurface);
    void setAreaAboveWaterline(float areaAboveWaterline);
    void setBulbousBowCenterHeight(float bulbousBowCenterHeight);
    void setBulbousBowArea(float bulbousBowArea);
    void setImmersedTransomArea(float immersedTransomArea);
    void setEntranceAngle(float entranceAngle);
    void setSurfaceRoughness(float surfaceRoughness);
    void setBuoyancyCenter(float buoyancyCenter);
    void setSternShapeParam(int sternShapeParam);
    void setMidshipSectionCoef(float midshipSectionCoef);
    void setWaterplaneAreaCoef(float waterplaneAreaCoef);
    void setPrismaticCoef(float prismaticCoef);
    void setBlockCoef(float blockCoef);
    void setTanksDetails(const QVector<QMap<QString, float>>& tanksDetails);
    void setEnginesPerPropeller(int enginesPerPropeller);
    void setEngineTierII(const QVector<QMap<QString, float>>& engineTierII);
    void setEngineTierIII(const QVector<QMap<QString, float>>& engineTierIII);
    void setEngineTierIICurve(const QVector<QMap<QString, float>>& engineTierIICurve);
    void setEngineTierIIICurve(const QVector<QMap<QString, float>>& engineTierIIICurve);
    void setGearboxRatio(float gearboxRatio);
    void setGearboxEfficiency(float gearboxEfficiency);
    void setShaftEfficiency(float shaftEfficiency);
    void setPropellerCount(int propellerCount);
    void setPropellerDiameter(float propellerDiameter);
    void setPropellerPitch(float propellerPitch);
    void setPropellerBladesCount(int propellerBladesCount);
    void setExpandedAreaRatio(float expandedAreaRatio);
    void setStopIfNoEnergy(bool stopIfNoEnergy);
    void setMaxRudderAngle(float maxRudderAngle);
    void setVesselWeight(float vesselWeight);
    void setCargoWeight(float cargoWeight);
    void setAppendagesWettedSurfaces(const QMap<int, float>& appendagesWettedSurfaces);

    // Static method for creating from dict
    static Ship* fromDict(const QJsonObject& data, QObject* parent = nullptr);

signals:
    void shipChanged();
    void pathChanged();
    void propertiesChanged();

private:
    QString m_shipId;
    QVector<QVector<float>> m_pathCoordinates;
    float m_maxSpeed;
    float m_waterlineLength;
    float m_lengthBetweenPerpendiculars;
    float m_beam;
    float m_draftAtForward;
    float m_draftAtAft;
    float m_volumetricDisplacement;
    float m_wettedHullSurface;
    float m_areaAboveWaterline;
    float m_bulbousBowCenterHeight;
    float m_bulbousBowArea;
    float m_immersedTransomArea;
    float m_entranceAngle;
    float m_surfaceRoughness;
    float m_buoyancyCenter;
    int m_sternShapeParam;
    float m_midshipSectionCoef;
    float m_waterplaneAreaCoef;
    float m_prismaticCoef;
    float m_blockCoef;
    QVector<QMap<QString, float>> m_tanksDetails;
    int m_enginesPerPropeller;
    QVector<QMap<QString, float>> m_engineTierII;
    QVector<QMap<QString, float>> m_engineTierIII;
    QVector<QMap<QString, float>> m_engineTierIICurve;
    QVector<QMap<QString, float>> m_engineTierIIICurve;
    float m_gearboxRatio;
    float m_gearboxEfficiency;
    float m_shaftEfficiency;
    int m_propellerCount;
    float m_propellerDiameter;
    float m_propellerPitch;
    int m_propellerBladesCount;
    float m_expandedAreaRatio;
    bool m_stopIfNoEnergy;
    float m_maxRudderAngle;
    float m_vesselWeight;
    float m_cargoWeight;
    QMap<int, float> m_appendagesWettedSurfaces;

    bool containsNaN(const QVariant& value) const;
};

class ShipsReader : public QObject {
    Q_OBJECT

public:
    static QVector<Ship*> readShipsFile(const QString& filePath, QObject* parent = nullptr);
    // Make Ship class a friend of ShipsReader
    friend class Ship;

protected:
    static const QVector<QPair<QString, bool>> FILE_ORDERED_PARAMETERS;
    
    static QMap<QString, QVariant> parseShipParameters(const QStringList& parts);
    static QVector<QVector<float>> parsePath(const QString& pathString);
    static QVector<QMap<QString, float>> parseEnginePoints(const QString& enginePointsStr);
    static QMap<int, float> parseAppendages(const QString& appendagesStr);
    static QVector<QMap<QString, float>> parseTanksDetails(const QString& tanksStr);
    static bool containsNa(const QVariant& value);
};

} // namespace Backend
} // namespace CargoNetSim
