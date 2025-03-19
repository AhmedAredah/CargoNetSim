// TrainNetwork.h
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
#include <QPair>
#include <QQueue>
#include <float.h>
#include <QVariant>

#include "../Commons/DirectedGraph.h"

namespace CargoNetSim {
namespace Backend {

class NeTrainSimNode : public QObject {
    Q_OBJECT

public:
    NeTrainSimNode(QObject* parent = nullptr);
    NeTrainSimNode(
        int simulatorId,
        int userId,
        float x,
        float y,
        const QString& description,
        float xScale,
        float yScale,
        bool isTerminal = false,
        float dwellTime = 0.0f,
        QObject* parent = nullptr
        );
    
    // Constructor from JSON
    NeTrainSimNode(const QJsonObject& json, QObject* parent = nullptr);
    
    QJsonObject toDict() const;
    static NeTrainSimNode* fromDict(const QJsonObject& data, QObject* parent = nullptr);

    // Getters
    int simulatorId() const { return m_simulatorId; }
    int userId() const { return m_userId; }
    float x() const { return m_x; }
    float y() const { return m_y; }
    QString description() const { return m_description; }
    float xScale() const { return m_xScale; }
    float yScale() const { return m_yScale; }
    bool isTerminal() const { return m_isTerminal; }
    float dwellTime() const { return m_dwellTime; }

    // Setters
    void setSimulatorId(int simulatorId);
    void setUserId(int userId);
    void setX(float x);
    void setY(float y);
    void setDescription(const QString& description);
    void setXScale(float xScale);
    void setYScale(float yScale);
    void setIsTerminal(bool isTerminal);
    void setDwellTime(float dwellTime);

signals:
    void nodeChanged();

private:
    int m_simulatorId;
    int m_userId;
    float m_x;
    float m_y;
    QString m_description;
    float m_xScale;
    float m_yScale;
    bool m_isTerminal;
    float m_dwellTime;
};

class NeTrainSimLink : public QObject {
    Q_OBJECT

public:
    NeTrainSimLink(QObject* parent = nullptr);
    NeTrainSimLink(
        int simulatorId,
        int userId,
        NeTrainSimNode* fromNode,
        NeTrainSimNode* toNode,
        float length,
        float maxSpeed,
        int signalId,
        const QString& signalsAtNodes,
        float grade,
        float curvature,
        int numDirections,
        float speedVariationFactor,
        bool hasCatenary,
        const QString& region,
        float lengthScale,
        float speedScale,
        QObject* parent = nullptr
        );
    
    // Constructor from JSON
    NeTrainSimLink(const QJsonObject& json, QObject* parent = nullptr);
    
    QJsonObject toDict() const;
    static NeTrainSimLink* fromDict(const QJsonObject& data, QObject* parent = nullptr);

    // Getters
    int simulatorId() const { return m_simulatorId; }
    int userId() const { return m_userId; }
    NeTrainSimNode* fromNode() const { return m_fromNode; }
    NeTrainSimNode* toNode() const { return m_toNode; }
    float length() const { return m_length; }
    float maxSpeed() const { return m_maxSpeed; }
    int signalId() const { return m_signalId; }
    QString signalsAtNodes() const { return m_signalsAtNodes; }
    float grade() const { return m_grade; }
    float curvature() const { return m_curvature; }
    int numDirections() const { return m_numDirections; }
    float speedVariationFactor() const { return m_speedVariationFactor; }
    bool hasCatenary() const { return m_hasCatenary; }
    QString region() const { return m_region; }
    float lengthScale() const { return m_lengthScale; }
    float speedScale() const { return m_speedScale; }

    // Setters
    void setSimulatorId(int simulatorId);
    void setUserId(int userId);
    void setFromNode(NeTrainSimNode* fromNode);
    void setToNode(NeTrainSimNode* toNode);
    void setLength(float length);
    void setMaxSpeed(float maxSpeed);
    void setSignalId(int signalId);
    void setSignalsAtNodes(const QString& signalsAtNodes);
    void setGrade(float grade);
    void setCurvature(float curvature);
    void setNumDirections(int numDirections);
    void setSpeedVariationFactor(float speedVariationFactor);
    void setHasCatenary(bool hasCatenary);
    void setRegion(const QString& region);
    void setLengthScale(float lengthScale);
    void setSpeedScale(float speedScale);

signals:
    void linkChanged();

private:
    int m_simulatorId;
    int m_userId;
    NeTrainSimNode* m_fromNode;
    NeTrainSimNode* m_toNode;
    float m_length;
    float m_maxSpeed;
    int m_signalId;
    QString m_signalsAtNodes;
    float m_grade;
    float m_curvature;
    int m_numDirections;
    float m_speedVariationFactor;
    bool m_hasCatenary;
    QString m_region;
    float m_lengthScale;
    float m_speedScale;
};

class NeTrainSimNodeDataReader : public QObject {
    Q_OBJECT

public:
    static QVector<QMap<QString, QString>> readNodesFile(const QString& filename);
};

class NeTrainSimLinkDataReader : public QObject {
    Q_OBJECT

public:
    static QVector<QMap<QString, QString>> readLinksFile(const QString& filename);
};

class NeTrainSimNetworkBase : public QObject {
    Q_OBJECT

public:
    NeTrainSimNetworkBase(QObject* parent = nullptr);
    ~NeTrainSimNetworkBase();
    
    void addVariable(const QString& key, const QString& value);
    QString getVariable(const QString& key) const;
    QMap<QString, QString> getVariables() const;
    
    void loadNetwork(const QString& nodesFile, const QString& linksFile);
    
    QVector<QJsonObject> getNodes() const;
    QVector<QJsonObject> getLinks() const;
    
    QPair<QVector<int>, QVector<float>> getPathLinks(const QVector<int>& path) const;
    
    QJsonObject findShortestPath(int startNodeId, int endNodeId, const QString& optimizeFor = "distance");
    
    QJsonObject nodesToJson() const;
    QJsonObject linksToJson() const;
    
    void setNodesAndLinks(const QVector<QJsonObject>& nodes, const QVector<QJsonObject>& links);
    void initializeGraph();

signals:
    void networkChanged();
    void nodesChanged();
    void linksChanged();

private:
    QVector<NeTrainSimNode*> generateNodes(const QVector<QMap<QString, QString>>& nodeRecords);
    QVector<NeTrainSimLink*> generateLinks(const QVector<QMap<QString, QString>>& linkRecords);
    NeTrainSimNode* getNodeByUserId(int userId) const;
    void buildGraph();
    
    QVector<QJsonObject> m_nodes;
    QVector<QJsonObject> m_links;
    DirectedGraph<int>* m_graph;
    QMap<QString, QString> m_variables;
    
    // Store actual objects for internal use
    QVector<NeTrainSimNode*> m_nodeObjects;
    QVector<NeTrainSimLink*> m_linkObjects;
    
    // Thread synchronization
    mutable QMutex m_mutex;
};

} // namespace Backend
} // namespace CargoNetSim
