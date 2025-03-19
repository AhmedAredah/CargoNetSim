// integration_network.h
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
#include <QMutex>
#include <QDir>
#include <QPair>
#include <limits>

#include "../Commons/DirectedGraph.h"

namespace CargoNetSim {
namespace Backend {

class IntegrationNode : public QObject {
    Q_OBJECT

public:
    IntegrationNode(QObject* parent = nullptr);
    IntegrationNode(
        int nodeId,
        float xCoordinate,
        float yCoordinate,
        int nodeType,
        int macroZoneCluster,
        int informationAvailability,
        const QString& description,
        float xScale,
        float yScale,
        QObject* parent = nullptr
        );
    
    // Constructor from JSON
    IntegrationNode(const QJsonObject& json, QObject* parent = nullptr);
    
    QJsonObject toDict() const;
    static IntegrationNode* fromDict(const QJsonObject& data, QObject* parent = nullptr);

    // Getters
    int nodeId() const { return m_nodeId; }
    float xCoordinate() const { return m_xCoordinate; }
    float yCoordinate() const { return m_yCoordinate; }
    int nodeType() const { return m_nodeType; }
    int macroZoneCluster() const { return m_macroZoneCluster; }
    int informationAvailability() const { return m_informationAvailability; }
    QString description() const { return m_description; }
    float xScale() const { return m_xScale; }
    float yScale() const { return m_yScale; }

    // Setters
    void setNodeId(int nodeId);
    void setXCoordinate(float xCoordinate);
    void setYCoordinate(float yCoordinate);
    void setNodeType(int nodeType);
    void setMacroZoneCluster(int macroZoneCluster);
    void setInformationAvailability(int informationAvailability);
    void setDescription(const QString& description);
    void setXScale(float xScale);
    void setYScale(float yScale);

signals:
    void nodeChanged();

private:
    int m_nodeId;
    float m_xCoordinate;
    float m_yCoordinate;
    int m_nodeType;
    int m_macroZoneCluster;
    int m_informationAvailability;
    QString m_description;
    float m_xScale;
    float m_yScale;
};

class IntegrationLink : public QObject {
    Q_OBJECT

public:
    IntegrationLink(QObject* parent = nullptr);
    IntegrationLink(
        int linkId,
        int upstreamNodeId,
        int downstreamNodeId,
        float length,
        float freeSpeed,
        float saturationFlow,
        float lanes,
        float speedCoeffVariation,
        float speedAtCapacity,
        float jamDensity,
        int turnProhibition,
        int prohibitionStart,
        int prohibitionEnd,
        int opposingLink1,
        int opposingLink2,
        int trafficSignal,
        int phase1,
        int phase2,
        int vehicleClassProhibition,
        int surveillanceLevel,
        const QString& description,
        float lengthScale,
        float speedScale,
        float saturationFlowScale,
        float speedAtCapacityScale,
        float jamDensityScale,
        QObject* parent = nullptr
        );
    
    // Constructor from JSON
    IntegrationLink(const QJsonObject& json, QObject* parent = nullptr);
    
    QJsonObject toDict() const;
    static IntegrationLink* fromDict(const QJsonObject& data, QObject* parent = nullptr);

    // Getters
    int linkId() const { return m_linkId; }
    int upstreamNodeId() const { return m_upstreamNodeId; }
    int downstreamNodeId() const { return m_downstreamNodeId; }
    float length() const { return m_length; }
    float freeSpeed() const { return m_freeSpeed; }
    float saturationFlow() const { return m_saturationFlow; }
    float lanes() const { return m_lanes; }
    float speedCoeffVariation() const { return m_speedCoeffVariation; }
    float speedAtCapacity() const { return m_speedAtCapacity; }
    float jamDensity() const { return m_jamDensity; }
    int turnProhibition() const { return m_turnProhibition; }
    int prohibitionStart() const { return m_prohibitionStart; }
    int prohibitionEnd() const { return m_prohibitionEnd; }
    int opposingLink1() const { return m_opposingLink1; }
    int opposingLink2() const { return m_opposingLink2; }
    int trafficSignal() const { return m_trafficSignal; }
    int phase1() const { return m_phase1; }
    int phase2() const { return m_phase2; }
    int vehicleClassProhibition() const { return m_vehicleClassProhibition; }
    int surveillanceLevel() const { return m_surveillanceLevel; }
    QString description() const { return m_description; }
    float lengthScale() const { return m_lengthScale; }
    float speedScale() const { return m_speedScale; }
    float saturationFlowScale() const { return m_saturationFlowScale; }
    float speedAtCapacityScale() const { return m_speedAtCapacityScale; }
    float jamDensityScale() const { return m_jamDensityScale; }

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
    void setVehicleClassProhibition(int vehicleClassProhibition);
    void setSurveillanceLevel(int surveillanceLevel);
    void setDescription(const QString& description);
    void setLengthScale(float lengthScale);
    void setSpeedScale(float speedScale);
    void setSaturationFlowScale(float saturationFlowScale);
    void setSpeedAtCapacityScale(float speedAtCapacityScale);
    void setJamDensityScale(float jamDensityScale);

signals:
    void linkChanged();

private:
    int m_linkId;
    int m_upstreamNodeId;
    int m_downstreamNodeId;
    float m_length;
    float m_freeSpeed;
    float m_saturationFlow;
    float m_lanes;
    float m_speedCoeffVariation;
    float m_speedAtCapacity;
    float m_jamDensity;
    int m_turnProhibition;
    int m_prohibitionStart;
    int m_prohibitionEnd;
    int m_opposingLink1;
    int m_opposingLink2;
    int m_trafficSignal;
    int m_phase1;
    int m_phase2;
    int m_vehicleClassProhibition;
    int m_surveillanceLevel;
    QString m_description;
    float m_lengthScale;
    float m_speedScale;
    float m_saturationFlowScale;
    float m_speedAtCapacityScale;
    float m_jamDensityScale;
};

class IntegrationNodeDataReader : public QObject {
    Q_OBJECT

public:
    static QVector<QMap<QString, QString>> readNodesFile(const QString& filename);
};

class IntegrationLinkDataReader : public QObject {
    Q_OBJECT

public:
    static QVector<QMap<QString, QString>> readLinksFile(const QString& filename);
};

class IntegrationNetworkBase : public QObject {
    Q_OBJECT

public:
    IntegrationNetworkBase(QObject* parent = nullptr);
    ~IntegrationNetworkBase();
    
    void initializeNetwork(const QVector<IntegrationNode*>& nodes, const QVector<IntegrationLink*>& links);
    
    bool nodeExists(int nodeId) const;
    
    QJsonObject findShortestPath(int startNodeId, int endNodeId);
    
    float getPathLengthByNodes(const QVector<int>& pathNodes) const;
    float getPathLengthByLinks(const QVector<int>& pathLinks) const;
    
    QVector<int> getEndNodes() const;
    QVector<int> getStartNodes() const;
    QJsonObject getStartAndEndNodes() const;
    
    QVector<QJsonObject> getNodes() const;
    QVector<QJsonObject> getLinks() const;
    
    QJsonObject nodesToJson() const;
    QJsonObject linksToJson() const;

signals:
    void networkChanged();
    void nodesChanged();
    void linksChanged();

private:
    QVector<int> getPathLinks(const QVector<int>& pathNodes) const;
    void addNodesWithCoordinates(const QVector<IntegrationNode*>& nodes);
    void buildGraph(const QVector<IntegrationLink*>& links);
    
    QVector<QJsonObject> m_nodes;
    QVector<QJsonObject> m_links;
    DirectedGraph<int> m_graph;
    
    // Store actual objects for internal use
    QVector<IntegrationNode*> m_nodeObjects;
    QVector<IntegrationLink*> m_linkObjects;
    
    // Thread synchronization
    mutable QMutex m_mutex;
};

class IntegrationSimulationFormatIConfigBase : public QObject {
    Q_OBJECT

public:
    IntegrationSimulationFormatIConfigBase(QObject* parent = nullptr);
    ~IntegrationSimulationFormatIConfigBase();
    
    void addVariable(const QString& key, const QString& value);
    QString getVariable(const QString& key) const;
    QMap<QString, QString> getVariables() const;
    
    void initializeConfig(
        const QString& configDir,
        const QString& title,
        float simTime,
        int outputFreq10,
        int outputFreq1214,
        int routingOption,
        int pauseFlag,
        const QString& inputFolder,
        const QString& outputFolder,
        const QMap<QString, QString>& inputFiles,
        const QMap<QString, QString>& outputFiles
        );
    
    QString getInputFilePath(const QString& fileKey) const;
    QString getOutputFilePath(const QString& fileKey) const;
    
    IntegrationNetworkBase* getNetwork() const;
    
    QJsonObject toDict() const;
    static IntegrationSimulationFormatIConfigBase* fromDict(const QJsonObject& data, QObject* parent = nullptr);

signals:
    void configChanged();

private:
    QString m_configDir;
    QString m_title;
    float m_simTime;
    int m_outputFreq10;
    int m_outputFreq1214;
    int m_routingOption;
    int m_pauseFlag;
    QString m_inputFolder;
    QString m_outputFolder;
    QMap<QString, QString> m_inputFiles;
    QMap<QString, QString> m_outputFiles;
    QMap<QString, QVector<QMap<QString, QString>>> m_networkData;
    QMap<QString, QString> m_variables;
    
    // Thread synchronization
    mutable QMutex m_mutex;
};

class IntegrationSimulationConfigReader : public QObject {
    Q_OBJECT

public:
    static QJsonObject readConfig(const QString& configFilePath);
};

} // namespace Backend
} // namespace CargoNetSim
