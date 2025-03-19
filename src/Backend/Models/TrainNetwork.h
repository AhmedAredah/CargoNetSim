/**
 * @file TrainNetwork.h
 * @brief Train network simulation classes for CargoNetSim
 * @author Ahmed Aredah
 * @date 2025-03-19
 * @copyright Copyright (c) 2025
 */

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

/**
 * @class NeTrainSimNode
 * @brief Represents a node in the train simulation network
 *
 * The NeTrainSimNode class models a network node with spatial
 * coordinates and other properties such as dwell time.
 *
 * This class must be registered with Qt's meta-object system
 * using Q_DECLARE_METATYPE(CargoNetSim::Backend::NeTrainSimNode)
 * after class definition.
 */
class NeTrainSimNode : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Default constructor
     * @param parent The parent QObject
     */
    NeTrainSimNode(QObject* parent = nullptr);

    /**
     * @brief Parameterized constructor
     * @param simulatorId Internal identifier for the simulator
     * @param userId User-defined identifier
     * @param x X-coordinate on the network
     * @param y Y-coordinate on the network
     * @param description Text description of the node
     * @param xScale Scaling factor for X-coordinate
     * @param yScale Scaling factor for Y-coordinate
     * @param isTerminal Whether node is a terminal station
     * @param dwellTime Time spent at this node in hours
     * @param parent The parent QObject
     */
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

    /**
     * @brief Constructor from JSON data
     * @param json JSON object containing node data
     * @param parent The parent QObject
     */
    NeTrainSimNode(const QJsonObject& json, QObject* parent = nullptr);

    /**
     * @brief Converts the node to a JSON object
     * @return QJsonObject representation of the node
     */
    QJsonObject toDict() const;

    /**
     * @brief Creates a node from JSON data
     * @param data JSON object containing node data
     * @param parent The parent QObject
     * @return Pointer to the new node
     */
    static NeTrainSimNode* fromDict(const QJsonObject& data,
                                    QObject* parent = nullptr);

    /**
     * @brief Gets the simulator ID
     * @return Simulator identifier
     */
    int getSimulatorId() const { return m_simulatorId; }

    /**
     * @brief Gets the user ID
     * @return User-defined identifier
     */
    int getUserId() const { return m_userId; }

    /**
     * @brief Gets the X coordinate
     * @return X coordinate value
     */
    float getX() const { return m_x; }

    /**
     * @brief Gets the Y coordinate
     * @return Y coordinate value
     */
    float getY() const { return m_y; }

    /**
     * @brief Gets the node getDescription
     * @return Description text
     */
    QString getDescription() const { return m_description; }

    /**
     * @brief Gets the X scaling factor
     * @return X scale value
     */
    float getXScale() const { return m_xScale; }

    /**
     * @brief Gets the Y scaling factor
     * @return Y scale value
     */
    float getYScale() const { return m_yScale; }

    /**
     * @brief Gets the terminal status
     * @return True if node is a terminal
     */
    bool isTerminal() const { return m_isTerminal; }

    /**
     * @brief Gets the dwell time
     * @return Dwell time in hours
     */
    float getDwellTime() const { return m_dwellTime; }

    /**
     * @brief Sets the simulator ID
     * @param simulatorId Simulator identifier
     */
    void setSimulatorId(int simulatorId);

    /**
     * @brief Sets the user ID
     * @param userId User-defined identifier
     */
    void setUserId(int userId);

    /**
     * @brief Sets the X coordinate
     * @param x X coordinate value
     */
    void setX(float x);

    /**
     * @brief Sets the Y coordinate
     * @param y Y coordinate value
     */
    void setY(float y);

    /**
     * @brief Sets the node description
     * @param description Description text
     */
    void setDescription(const QString& description);

    /**
     * @brief Sets the X scaling factor
     * @param xScale X scale value
     */
    void setXScale(float xScale);

    /**
     * @brief Sets the Y scaling factor
     * @param yScale Y scale value
     */
    void setYScale(float yScale);

    /**
     * @brief Sets the terminal status
     * @param isTerminal True if node is a terminal
     */
    void setIsTerminal(bool isTerminal);

    /**
     * @brief Sets the dwell time
     * @param dwellTime Dwell time in hours
     */
    void setDwellTime(float dwellTime);

signals:
    /**
     * @brief Signal emitted when any node property changes
     */
    void nodeChanged();

private:
    int m_simulatorId;      ///< Internal simulator identifier
    int m_userId;           ///< User-defined identifier
    float m_x;              ///< X-coordinate on the network
    float m_y;              ///< Y-coordinate on the network
    QString m_description;  ///< Text description of the node
    float m_xScale;         ///< Scaling factor for X-coordinate
    float m_yScale;         ///< Scaling factor for Y-coordinate
    bool m_isTerminal;      ///< Whether node is a terminal station
    float m_dwellTime;      ///< Time spent at this node in hours
};

/**
 * @class NeTrainSimLink
 * @brief Represents a link between nodes in train network
 *
 * The NeTrainSimLink class models a connection between two nodes
 * with properties such as length, speed limits, grade, etc.
 *
 * This class must be registered with Qt's meta-object system
 * using Q_DECLARE_METATYPE(CargoNetSim::Backend::NeTrainSimLink)
 * after class definition.
 */
class NeTrainSimLink : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Default constructor
     * @param parent The parent QObject
     */
    NeTrainSimLink(QObject* parent = nullptr);

    /**
     * @brief Parameterized constructor
     * @param simulatorId Internal identifier for the simulator
     * @param userId User-defined identifier
     * @param fromNode Source node of the link
     * @param toNode Destination node of the link
     * @param length Length of the link in km
     * @param maxSpeed Maximum speed allowed on link in km/h
     * @param signalId Identifier for the signal system
     * @param signalsAtNodes Signal configuration at nodes
     * @param grade Grade percentage (positive = uphill)
     * @param curvature Curvature measure in degrees
     * @param numDirections Number of possible travel directions
     * @param speedVariationFactor Factor affecting speed variation
     * @param hasCatenary Whether link has overhead power
     * @param region Geographic region identifier
     * @param lengthScale Scaling factor for link length
     * @param speedScale Scaling factor for speed limits
     * @param parent The parent QObject
     */
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

    /**
     * @brief Constructor from JSON data
     * @param json JSON object containing link data
     * @param parent The parent QObject
     */
    NeTrainSimLink(const QJsonObject& json, QObject* parent = nullptr);

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
    static NeTrainSimLink* fromDict(const QJsonObject& data,
                                    QObject* parent = nullptr);

    /**
     * @brief Gets the simulator ID
     * @return Simulator identifier
     */
    int getSimulatorId() const { return m_simulatorId; }

    /**
     * @brief Gets the user ID
     * @return User-defined identifier
     */
    int getUserId() const { return m_userId; }

    /**
     * @brief Gets the source node
     * @return Pointer to the source node
     */
    NeTrainSimNode* getFromNode() const { return m_fromNode; }

    /**
     * @brief Gets the destination node
     * @return Pointer to the destination node
     */
    NeTrainSimNode* getToNode() const { return m_toNode; }

    /**
     * @brief Gets the link getLength
     * @return Length in km
     */
    float getLength() const { return m_length; }

    /**
     * @brief Gets the maximum speed
     * @return Speed in km/h
     */
    float getMaxSpeed() const { return m_maxSpeed; }

    /**
     * @brief Gets the signal ID
     * @return Signal system identifier
     */
    int getSignalId() const { return m_signalId; }

    /**
     * @brief Gets the signals at nodes configuration
     * @return Signal configuration string
     */
    QString getSignalsAtNodes() const { return m_signalsAtNodes; }

    /**
     * @brief Gets the getGrade percentage
     * @return Grade value (positive = uphill)
     */
    float getGrade() const { return m_grade; }

    /**
     * @brief Gets the getCurvature
     * @return Curvature in degrees
     */
    float getCurvature() const { return m_curvature; }

    /**
     * @brief Gets the number of directions
     * @return Number of travel directions
     */
    int getNumDirections() const { return m_numDirections; }

    /**
     * @brief Gets the speed variation factor
     * @return Speed variation factor
     */
    float getSpeedVariationFactor() const {
        return m_speedVariationFactor;
    }

    /**
     * @brief Gets the catenary status
     * @return True if link has overhead power
     */
    bool hasCatenary() const { return m_hasCatenary; }

    /**
     * @brief Gets the getRegion
     * @return Region identifier string
     */
    QString getRegion() const { return m_region; }

    /**
     * @brief Gets the length scale
     * @return Length scaling factor
     */
    float getLengthScale() const { return m_lengthScale; }

    /**
     * @brief Gets the speed scale
     * @return Speed scaling factor
     */
    float getSpeedScale() const { return m_speedScale; }

    /**
     * @brief Sets the simulator ID
     * @param simulatorId Simulator identifier
     */
    void setSimulatorId(int simulatorId);

    /**
     * @brief Sets the user ID
     * @param userId User-defined identifier
     */
    void setUserId(int userId);

    /**
     * @brief Sets the source node
     * @param fromNode Pointer to the source node
     */
    void setFromNode(NeTrainSimNode* fromNode);

    /**
     * @brief Sets the destination node
     * @param toNode Pointer to the destination node
     */
    void setToNode(NeTrainSimNode* toNode);

    /**
     * @brief Sets the link length
     * @param length Length in km
     */
    void setLength(float length);

    /**
     * @brief Sets the maximum speed
     * @param maxSpeed Speed in km/h
     */
    void setMaxSpeed(float maxSpeed);

    /**
     * @brief Sets the signal ID
     * @param signalId Signal system identifier
     */
    void setSignalId(int signalId);

    /**
     * @brief Sets the signals at nodes configuration
     * @param signalsAtNodes Signal configuration string
     */
    void setSignalsAtNodes(const QString& signalsAtNodes);

    /**
     * @brief Sets the grade percentage
     * @param grade Grade value (positive = uphill)
     */
    void setGrade(float grade);

    /**
     * @brief Sets the curvature
     * @param curvature Curvature in degrees
     */
    void setCurvature(float curvature);

    /**
     * @brief Sets the number of directions
     * @param numDirections Number of travel directions
     */
    void setNumDirections(int numDirections);

    /**
     * @brief Sets the speed variation factor
     * @param speedVariationFactor Speed variation factor
     */
    void setSpeedVariationFactor(float speedVariationFactor);

    /**
     * @brief Sets the catenary status
     * @param hasCatenary True if link has overhead power
     */
    void setHasCatenary(bool hasCatenary);

    /**
     * @brief Sets the region
     * @param region Region identifier string
     */
    void setRegion(const QString& region);

    /**
     * @brief Sets the length scale
     * @param lengthScale Length scaling factor
     */
    void setLengthScale(float lengthScale);

    /**
     * @brief Sets the speed scale
     * @param speedScale Speed scaling factor
     */
    void setSpeedScale(float speedScale);

signals:
    /**
     * @brief Signal emitted when any link property changes
     */
    void linkChanged();

private:
    int m_simulatorId;       ///< Internal simulator identifier
    int m_userId;            ///< User-defined identifier
    NeTrainSimNode* m_fromNode; ///< Source node of the link
    NeTrainSimNode* m_toNode;   ///< Destination node of the link
    float m_length;          ///< Length of the link in km
    float m_maxSpeed;        ///< Maximum speed allowed in km/h
    int m_signalId;          ///< Identifier for the signal system
    QString m_signalsAtNodes; ///< Signal configuration at nodes
    float m_grade;           ///< Grade percentage
    float m_curvature;       ///< Curvature measure in degrees
    int m_numDirections;     ///< Number of possible directions
    float m_speedVariationFactor; ///< Speed variation factor
    bool m_hasCatenary;      ///< Whether link has overhead power
    QString m_region;        ///< Geographic region identifier
    float m_lengthScale;     ///< Scaling factor for link length
    float m_speedScale;      ///< Scaling factor for speed limits
};

/**
 * @class NeTrainSimNodeDataReader
 * @brief Utility class for reading node data from files
 *
 * The NeTrainSimNodeDataReader provides static methods to parse
 * and load node data from text files.
 *
 * This class must be registered with Qt's meta-object system
 * using
 * Q_DECLARE_METATYPE(CargoNetSim::Backend::NeTrainSimNodeDataReader)
 * after class definition.
 */
class NeTrainSimNodeDataReader : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Reads node data from a file
     * @param filename Path to the nodes data file
     * @return Vector of maps containing node data records
     */
    static QVector<QMap<QString, QString>> readNodesFile(
        const QString& filename);
};

/**
 * @class NeTrainSimLinkDataReader
 * @brief Utility class for reading link data from files
 *
 * The NeTrainSimLinkDataReader provides static methods to parse
 * and load link data from text files.
 *
 * This class must be registered with Qt's meta-object system
 * using
 * Q_DECLARE_METATYPE(CargoNetSim::Backend::NeTrainSimLinkDataReader)
 * after class definition.
 */
class NeTrainSimLinkDataReader : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Reads link data from a file
     * @param filename Path to the links data file
     * @return Vector of maps containing link data records
     */
    static QVector<QMap<QString, QString>> readLinksFile(
        const QString& filename);
};

/**
 * @class NeTrainSimNetworkBase
 * @brief Base class for train simulation network
 *
 * The NeTrainSimNetworkBase manages a network of nodes and links,
 * providing path-finding capabilities and network operations.
 */
class NeTrainSimNetworkBase : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Default constructor
     * @param parent The parent QObject
     */
    NeTrainSimNetworkBase(QObject* parent = nullptr);

    /**
     * @brief Destructor
     *
     * Properly cleans up all network resources.
     */
    ~NeTrainSimNetworkBase();

    /**
     * @brief Adds a variable to the network configuration
     * @param key Variable name
     * @param value Variable value
     */
    void addVariable(const QString& key, const QString& value);

    /**
     * @brief Gets a variable from the network configuration
     * @param key Variable name
     * @return Variable value as string
     */
    QString getVariable(const QString& key) const;

    /**
     * @brief Gets all variables in the network configuration
     * @return Map of all variables
     */
    QMap<QString, QString> getVariables() const;

    /**
     * @brief Loads a network from node and link files
     * @param nodesFile Path to the nodes data file
     * @param linksFile Path to the links data file
     */
    void loadNetwork(const QString& nodesFile,
                     const QString& linksFile);

    /**
     * @brief Gets all nodes in the network
     * @return Vector of node data as JSON objects
     */
    QVector<QJsonObject> getNodes() const;

    /**
     * @brief Gets all links in the network
     * @return Vector of link data as JSON objects
     */
    QVector<QJsonObject> getLinks() const;

    /**
     * @brief Gets link information for a path
     * @param path Vector of node IDs representing a path
     * @return Pair of vectors containing link IDs and lengths
     */
    QPair<QVector<int>, QVector<float>> getPathLinks(
        const QVector<int>& path) const;

    /**
     * @brief Finds the shortest path between two nodes
     * @param startNodeId Starting node ID
     * @param endNodeId Ending node ID
     * @param optimizeFor Optimization criteria ("distance" or
     *                    another metric)
     * @return JSON object with path details
     */
    QJsonObject findShortestPath(int startNodeId, int endNodeId,
                                 const QString& optimizeFor = "distance");

    /**
     * @brief Converts all nodes to a JSON object
     * @return JSON object containing all nodes
     */
    QJsonObject nodesToJson() const;

    /**
     * @brief Converts all links to a JSON object
     * @return JSON object containing all links
     */
    QJsonObject linksToJson() const;

    /**
     * @brief Sets nodes and links from JSON data
     * @param nodes Vector of node data as JSON objects
     * @param links Vector of link data as JSON objects
     */
    void setNodesAndLinks(const QVector<QJsonObject>& nodes,
                          const QVector<QJsonObject>& links);

    /**
     * @brief Initializes the directed graph from nodes and links
     */
    void initializeGraph();

signals:
    /**
     * @brief Signal emitted when the network changes
     */
    void networkChanged();

    /**
     * @brief Signal emitted when nodes are modified
     */
    void nodesChanged();

    /**
     * @brief Signal emitted when links are modified
     */
    void linksChanged();

private:
    /**
     * @brief Generates node objects from data records
     * @param nodeRecords Vector of node data records
     * @return Vector of node objects
     */
    QVector<NeTrainSimNode*> generateNodes(
        const QVector<QMap<QString, QString>>& nodeRecords);

    /**
     * @brief Generates link objects from data records
     * @param linkRecords Vector of link data records
     * @return Vector of link objects
     */
    QVector<NeTrainSimLink*> generateLinks(
        const QVector<QMap<QString, QString>>& linkRecords);

    /**
     * @brief Finds a node by its user ID
     * @param userId User-defined node identifier
     * @return Pointer to the node or nullptr if not found
     */
    NeTrainSimNode* getNodeByUserId(int userId) const;

    /**
     * @brief Builds the directed graph from nodes and links
     */
    void buildGraph();

    QVector<QJsonObject> m_nodes; ///< Node data as JSON objects
    QVector<QJsonObject> m_links; ///< Link data as JSON objects
    DirectedGraph<int>* m_graph;  ///< Directed graph of network
    QMap<QString, QString> m_variables; ///< Network variables

    /// Node objects for internal use
    QVector<NeTrainSimNode*> m_nodeObjects;

    /// Link objects for internal use
    QVector<NeTrainSimLink*> m_linkObjects;

    mutable QMutex m_mutex; ///< Thread synchronization mutex
};

} // namespace Backend
} // namespace CargoNetSim

// Register custom types with Qt's meta-object system
Q_DECLARE_METATYPE(CargoNetSim::Backend::NeTrainSimNode)
Q_DECLARE_METATYPE(CargoNetSim::Backend::NeTrainSimNode*)
Q_DECLARE_METATYPE(CargoNetSim::Backend::NeTrainSimLink)
Q_DECLARE_METATYPE(CargoNetSim::Backend::NeTrainSimLink*)
Q_DECLARE_METATYPE(CargoNetSim::Backend::NeTrainSimNodeDataReader)
Q_DECLARE_METATYPE(CargoNetSim::Backend::NeTrainSimNodeDataReader*)
Q_DECLARE_METATYPE(CargoNetSim::Backend::NeTrainSimLinkDataReader)
Q_DECLARE_METATYPE(CargoNetSim::Backend::NeTrainSimLinkDataReader*)
Q_DECLARE_METATYPE(CargoNetSim::Backend::NeTrainSimNetworkBase)
Q_DECLARE_METATYPE(CargoNetSim::Backend::NeTrainSimNetworkBase*)
