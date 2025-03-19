/**
 * @file TruckNetwork.h
 * @brief Truck network simulation classes for CargoNetSim
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
#include <QDir>
#include <QPair>
#include <limits>

#include "../Commons/DirectedGraph.h"

namespace CargoNetSim {
namespace Backend {

/**
 * @class IntegrationNode
 * @brief Represents a node in the truck network simulation
 *
 * The IntegrationNode class models a network node with spatial
 * coordinates and other properties such as node type and
 * information availability.
 *
 * This class must be registered with Qt's meta-object system
 * using Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationNode)
 * after class definition.
 */
class IntegrationNode : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Default constructor
     * @param parent The parent QObject
     */
    IntegrationNode(QObject* parent = nullptr);

    /**
     * @brief Parameterized constructor
     * @param nodeId Unique identifier for the node
     * @param xCoordinate X-coordinate on the network
     * @param yCoordinate Y-coordinate on the network
     * @param nodeType Type identifier for the node
     * @param macroZoneCluster Cluster identifier for zoning
     * @param informationAvailability Information level at node
     * @param description Text description of the node
     * @param xScale Scaling factor for X-coordinate
     * @param yScale Scaling factor for Y-coordinate
     * @param parent The parent QObject
     */
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

    /**
     * @brief Constructor from JSON data
     * @param json JSON object containing node data
     * @param parent The parent QObject
     */
    IntegrationNode(const QJsonObject& json, QObject* parent = nullptr);

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
    static IntegrationNode* fromDict(const QJsonObject& data,
                                     QObject* parent = nullptr);

    /**
     * @brief Gets the node ID
     * @return Node identifier
     */
    int nodeId() const { return m_nodeId; }

    /**
     * @brief Gets the X coordinate
     * @return X coordinate value
     */
    float xCoordinate() const { return m_xCoordinate; }

    /**
     * @brief Gets the Y coordinate
     * @return Y coordinate value
     */
    float yCoordinate() const { return m_yCoordinate; }

    /**
     * @brief Gets the node type
     * @return Node type identifier
     */
    int nodeType() const { return m_nodeType; }

    /**
     * @brief Gets the macro zone cluster
     * @return Cluster identifier
     */
    int macroZoneCluster() const { return m_macroZoneCluster; }

    /**
     * @brief Gets the information availability level
     * @return Information availability level
     */
    int informationAvailability() const {
        return m_informationAvailability;
    }

    /**
     * @brief Gets the node description
     * @return Description text
     */
    QString description() const { return m_description; }

    /**
     * @brief Gets the X scaling factor
     * @return X scale value
     */
    float xScale() const { return m_xScale; }

    /**
     * @brief Gets the Y scaling factor
     * @return Y scale value
     */
    float yScale() const { return m_yScale; }

    /**
     * @brief Sets the node ID
     * @param nodeId Node identifier
     */
    void setNodeId(int nodeId);

    /**
     * @brief Sets the X coordinate
     * @param xCoordinate X coordinate value
     */
    void setXCoordinate(float xCoordinate);

    /**
     * @brief Sets the Y coordinate
     * @param yCoordinate Y coordinate value
     */
    void setYCoordinate(float yCoordinate);

    /**
     * @brief Sets the node type
     * @param nodeType Node type identifier
     */
    void setNodeType(int nodeType);

    /**
     * @brief Sets the macro zone cluster
     * @param macroZoneCluster Cluster identifier
     */
    void setMacroZoneCluster(int macroZoneCluster);

    /**
     * @brief Sets the information availability level
     * @param informationAvailability Information level
     */
    void setInformationAvailability(int informationAvailability);

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

signals:
    /**
     * @brief Signal emitted when any node property changes
     */
    void nodeChanged();

private:
    int m_nodeId;                  ///< Unique node identifier
    float m_xCoordinate;           ///< X-coordinate on network
    float m_yCoordinate;           ///< Y-coordinate on network
    int m_nodeType;                ///< Type identifier
    int m_macroZoneCluster;        ///< Cluster identifier
    int m_informationAvailability; ///< Information level
    QString m_description;         ///< Node description
    float m_xScale;                ///< X coordinate scale
    float m_yScale;                ///< Y coordinate scale
};

/**
 * @class IntegrationLink
 * @brief Represents a link in the truck network simulation
 *
 * The IntegrationLink class models a connection between nodes
 * with detailed traffic properties such as speed, capacity,
 * and signal phasing.
 *
 * This class must be registered with Qt's meta-object system
 * using Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationLink)
 * after class definition.
 */
class IntegrationLink : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Default constructor
     * @param parent The parent QObject
     */
    IntegrationLink(QObject* parent = nullptr);

    /**
     * @brief Parameterized constructor
     * @param linkId Unique identifier for the link
     * @param upstreamNodeId Source node ID
     * @param downstreamNodeId Destination node ID
     * @param length Length of link in km
     * @param freeSpeed Free flow speed in km/h
     * @param saturationFlow Maximum flow in veh/h
     * @param lanes Number of lanes
     * @param speedCoeffVariation Speed variation coefficient
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
     * @param vehicleClassProhibition Vehicle class restrictions
     * @param surveillanceLevel Monitoring level
     * @param description Text description of the link
     * @param lengthScale Scale factor for length
     * @param speedScale Scale factor for speed
     * @param saturationFlowScale Scale factor for flow
     * @param speedAtCapacityScale Scale factor for capacity speed
     * @param jamDensityScale Scale factor for jam density
     * @param parent The parent QObject
     */
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

    /**
     * @brief Constructor from JSON data
     * @param json JSON object containing link data
     * @param parent The parent QObject
     */
    IntegrationLink(const QJsonObject& json, QObject* parent = nullptr);

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
    static IntegrationLink* fromDict(const QJsonObject& data,
                                     QObject* parent = nullptr);

    /**
     * @brief Gets the link ID
     * @return Link identifier
     */
    int linkId() const { return m_linkId; }

    /**
     * @brief Gets the upstream node ID
     * @return Source node identifier
     */
    int upstreamNodeId() const { return m_upstreamNodeId; }

    /**
     * @brief Gets the downstream node ID
     * @return Destination node identifier
     */
    int downstreamNodeId() const { return m_downstreamNodeId; }

    /**
     * @brief Gets the link length
     * @return Length in km
     */
    float length() const { return m_length; }

    /**
     * @brief Gets the free flow speed
     * @return Speed in km/h
     */
    float freeSpeed() const { return m_freeSpeed; }

    /**
     * @brief Gets the saturation flow
     * @return Flow in vehicles per hour
     */
    float saturationFlow() const { return m_saturationFlow; }

    /**
     * @brief Gets the number of lanes
     * @return Number of lanes
     */
    float lanes() const { return m_lanes; }

    /**
     * @brief Gets the speed coefficient of variation
     * @return Variation coefficient
     */
    float speedCoeffVariation() const {
        return m_speedCoeffVariation;
    }

    /**
     * @brief Gets the speed at capacity
     * @return Speed at capacity in km/h
     */
    float speedAtCapacity() const { return m_speedAtCapacity; }

    /**
     * @brief Gets the jam density
     * @return Jam density in vehicles per km
     */
    float jamDensity() const { return m_jamDensity; }

    /**
     * @brief Gets the turn prohibition status
     * @return Turn prohibition flag
     */
    int turnProhibition() const { return m_turnProhibition; }

    /**
     * @brief Gets the prohibition start time
     * @return Start time in hours
     */
    int prohibitionStart() const { return m_prohibitionStart; }

    /**
     * @brief Gets the prohibition end time
     * @return End time in hours
     */
    int prohibitionEnd() const { return m_prohibitionEnd; }

    /**
     * @brief Gets the first opposing link ID
     * @return First opposing link identifier
     */
    int opposingLink1() const { return m_opposingLink1; }

    /**
     * @brief Gets the second opposing link ID
     * @return Second opposing link identifier
     */
    int opposingLink2() const { return m_opposingLink2; }

    /**
     * @brief Gets the traffic signal type
     * @return Traffic signal identifier
     */
    int trafficSignal() const { return m_trafficSignal; }

    /**
     * @brief Gets the first signal phase
     * @return First phase identifier
     */
    int phase1() const { return m_phase1; }

    /**
     * @brief Gets the second signal phase
     * @return Second phase identifier
     */
    int phase2() const { return m_phase2; }

    /**
     * @brief Gets the vehicle class prohibition
     * @return Vehicle class prohibition flag
     */
    int vehicleClassProhibition() const {
        return m_vehicleClassProhibition;
    }

    /**
     * @brief Gets the surveillance level
     * @return Surveillance level identifier
     */
    int surveillanceLevel() const { return m_surveillanceLevel; }

    /**
     * @brief Gets the link description
     * @return Description text
     */
    QString description() const { return m_description; }

    /**
     * @brief Gets the length scale
     * @return Length scaling factor
     */
    float lengthScale() const { return m_lengthScale; }

    /**
     * @brief Gets the speed scale
     * @return Speed scaling factor
     */
    float speedScale() const { return m_speedScale; }

    /**
     * @brief Gets the saturation flow scale
     * @return Flow scaling factor
     */
    float saturationFlowScale() const {
        return m_saturationFlowScale;
    }

    /**
     * @brief Gets the speed at capacity scale
     * @return Capacity speed scaling factor
     */
    float speedAtCapacityScale() const {
        return m_speedAtCapacityScale;
    }

    /**
     * @brief Gets the jam density scale
     * @return Density scaling factor
     */
    float jamDensityScale() const { return m_jamDensityScale; }

    /**
     * @brief Sets the link ID
     * @param linkId Link identifier
     */
    void setLinkId(int linkId);

    /**
     * @brief Sets the upstream node ID
     * @param upstreamNodeId Source node identifier
     */
    void setUpstreamNodeId(int upstreamNodeId);

    /**
     * @brief Sets the downstream node ID
     * @param downstreamNodeId Destination node identifier
     */
    void setDownstreamNodeId(int downstreamNodeId);

    /**
     * @brief Sets the link length
     * @param length Length in km
     */
    void setLength(float length);

    /**
     * @brief Sets the free flow speed
     * @param freeSpeed Speed in km/h
     */
    void setFreeSpeed(float freeSpeed);

    /**
     * @brief Sets the saturation flow
     * @param saturationFlow Flow in vehicles per hour
     */
    void setSaturationFlow(float saturationFlow);

    /**
     * @brief Sets the number of lanes
     * @param lanes Number of lanes
     */
    void setLanes(float lanes);

    /**
     * @brief Sets the speed coefficient of variation
     * @param speedCoeffVariation Variation coefficient
     */
    void setSpeedCoeffVariation(float speedCoeffVariation);

    /**
     * @brief Sets the speed at capacity
     * @param speedAtCapacity Speed at capacity in km/h
     */
    void setSpeedAtCapacity(float speedAtCapacity);

    /**
     * @brief Sets the jam density
     * @param jamDensity Jam density in vehicles per km
     */
    void setJamDensity(float jamDensity);

    /**
     * @brief Sets the turn prohibition status
     * @param turnProhibition Turn prohibition flag
     */
    void setTurnProhibition(int turnProhibition);

    /**
     * @brief Sets the prohibition start time
     * @param prohibitionStart Start time in hours
     */
    void setProhibitionStart(int prohibitionStart);

    /**
     * @brief Sets the prohibition end time
     * @param prohibitionEnd End time in hours
     */
    void setProhibitionEnd(int prohibitionEnd);

    /**
     * @brief Sets the first opposing link ID
     * @param opposingLink1 First opposing link identifier
     */
    void setOpposingLink1(int opposingLink1);

    /**
     * @brief Sets the second opposing link ID
     * @param opposingLink2 Second opposing link identifier
     */
    void setOpposingLink2(int opposingLink2);

    /**
     * @brief Sets the traffic signal type
     * @param trafficSignal Traffic signal identifier
     */
    void setTrafficSignal(int trafficSignal);

    /**
     * @brief Sets the first signal phase
     * @param phase1 First phase identifier
     */
    void setPhase1(int phase1);

    /**
     * @brief Sets the second signal phase
     * @param phase2 Second phase identifier
     */
    void setPhase2(int phase2);

    /**
     * @brief Sets the vehicle class prohibition
     * @param vehicleClassProhibition Vehicle class prohibition
     */
    void setVehicleClassProhibition(int vehicleClassProhibition);

    /**
     * @brief Sets the surveillance level
     * @param surveillanceLevel Surveillance level identifier
     */
    void setSurveillanceLevel(int surveillanceLevel);

    /**
     * @brief Sets the link description
     * @param description Description text
     */
    void setDescription(const QString& description);

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

    /**
     * @brief Sets the saturation flow scale
     * @param saturationFlowScale Flow scaling factor
     */
    void setSaturationFlowScale(float saturationFlowScale);

    /**
     * @brief Sets the speed at capacity scale
     * @param speedAtCapacityScale Capacity speed scaling factor
     */
    void setSpeedAtCapacityScale(float speedAtCapacityScale);

    /**
     * @brief Sets the jam density scale
     * @param jamDensityScale Density scaling factor
     */
    void setJamDensityScale(float jamDensityScale);

signals:
    /**
     * @brief Signal emitted when any link property changes
     */
    void linkChanged();

private:
    int m_linkId;                 ///< Unique link identifier
    int m_upstreamNodeId;         ///< Source node identifier
    int m_downstreamNodeId;       ///< Destination node identifier
    float m_length;               ///< Link length in km
    float m_freeSpeed;            ///< Free flow speed in km/h
    float m_saturationFlow;       ///< Maximum flow in veh/h
    float m_lanes;                ///< Number of lanes
    float m_speedCoeffVariation;  ///< Speed variation coefficient
    float m_speedAtCapacity;      ///< Speed at capacity in km/h
    float m_jamDensity;           ///< Max density in veh/km
    int m_turnProhibition;        ///< Turn prohibition flag
    int m_prohibitionStart;       ///< Start time of prohibition
    int m_prohibitionEnd;         ///< End time of prohibition
    int m_opposingLink1;          ///< ID of first opposing link
    int m_opposingLink2;          ///< ID of second opposing link
    int m_trafficSignal;          ///< Signal control type
    int m_phase1;                 ///< First signal phase
    int m_phase2;                 ///< Second signal phase
    int m_vehicleClassProhibition; ///< Vehicle class restrictions
    int m_surveillanceLevel;      ///< Monitoring level
    QString m_description;        ///< Link description
    float m_lengthScale;          ///< Scale factor for length
    float m_speedScale;           ///< Scale factor for speed
    float m_saturationFlowScale;  ///< Scale factor for flow
    float m_speedAtCapacityScale; ///< Factor for capacity speed
    float m_jamDensityScale;      ///< Factor for jam density
};

/**
 * @class IntegrationNodeDataReader
 * @brief Utility class for reading node data from files
 *
 * The IntegrationNodeDataReader provides static methods to
 * parse and load node data from text files.
 *
 * This class must be registered with Qt's meta-object system
 * using
 * Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationNodeDataReader)
 * after class definition.
 */
class IntegrationNodeDataReader : public QObject {
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
 * @class IntegrationLinkDataReader
 * @brief Utility class for reading link data from files
 *
 * The IntegrationLinkDataReader provides static methods to
 * parse and load link data from text files.
 *
 * This class must be registered with Qt's meta-object system
 * using
 * Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationLinkDataReader)
 * after class definition.
 */
class IntegrationLinkDataReader : public QObject {
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
 * @class IntegrationNetworkBase
 * @brief Base class for truck network simulation
 *
 * The IntegrationNetworkBase manages a network of nodes and links,
 * providing path-finding capabilities and network operations
 * for truck routing.
 *
 * This class must be registered with Qt's meta-object system
 * using
 * Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationNetworkBase)
 * after class definition.
 */
class IntegrationNetworkBase : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Default constructor
     * @param parent The parent QObject
     */
    IntegrationNetworkBase(QObject* parent = nullptr);

    /**
     * @brief Destructor
     *
     * Properly cleans up all network resources.
     */
    ~IntegrationNetworkBase();

    /**
     * @brief Initializes the network with nodes and links
     * @param nodes Vector of node objects
     * @param links Vector of link objects
     */
    void initializeNetwork(const QVector<IntegrationNode*>& nodes,
                           const QVector<IntegrationLink*>& links);

    /**
     * @brief Checks if a node exists in the network
     * @param nodeId Node identifier to check
     * @return True if the node exists
     */
    bool nodeExists(int nodeId) const;

    /**
     * @brief Finds the shortest path between two nodes
     * @param startNodeId Starting node ID
     * @param endNodeId Ending node ID
     * @return JSON object with path details
     */
    QJsonObject findShortestPath(int startNodeId, int endNodeId);

    /**
     * @brief Calculates path length from a sequence of nodes
     * @param pathNodes Vector of node IDs representing a path
     * @return Total path length in km
     */
    float getPathLengthByNodes(const QVector<int>& pathNodes) const;

    /**
     * @brief Calculates path length from a sequence of links
     * @param pathLinks Vector of link IDs representing a path
     * @return Total path length in km
     */
    float getPathLengthByLinks(const QVector<int>& pathLinks) const;

    /**
     * @brief Gets all terminal node IDs
     * @return Vector of terminal node IDs
     */
    QVector<int> getEndNodes() const;

    /**
     * @brief Gets all origin node IDs
     * @return Vector of origin node IDs
     */
    QVector<int> getStartNodes() const;

    /**
     * @brief Gets both start and end nodes
     * @return JSON object with start and end node lists
     */
    QJsonObject getStartAndEndNodes() const;

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
     * @brief Converts all nodes to a JSON object
     * @return JSON object containing all nodes
     */
    QJsonObject nodesToJson() const;

    /**
     * @brief Converts all links to a JSON object
     * @return JSON object containing all links
     */
    QJsonObject linksToJson() const;

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
     * @brief Gets link IDs that form a path
     * @param pathNodes Vector of node IDs representing a path
     * @return Vector of link IDs that connect the nodes
     */
    QVector<int> getPathLinks(const QVector<int>& pathNodes) const;

    /**
     * @brief Adds nodes with coordinate data to the network
     * @param nodes Vector of node objects
     */
    void addNodesWithCoordinates(const QVector<IntegrationNode*>& nodes);

    /**
     * @brief Builds the network graph from links
     * @param links Vector of link objects
     */
    void buildGraph(const QVector<IntegrationLink*>& links);

    QVector<QJsonObject> m_nodes; ///< Node data as JSON objects
    QVector<QJsonObject> m_links; ///< Link data as JSON objects
    DirectedGraph<int> m_graph;   ///< Directed graph of network

    /// Node objects for internal use
    QVector<IntegrationNode*> m_nodeObjects;

    /// Link objects for internal use
    QVector<IntegrationLink*> m_linkObjects;

    mutable QMutex m_mutex; ///< Thread synchronization mutex
};

/**
 * @class IntegrationSimulationFormatIConfigBase
 * @brief Configuration for Integration simulation format I
 *
 * The IntegrationSimulationFormatIConfigBase manages simulation
 * configuration parameters, input/output file paths, and other
 * settings for the truck network simulation.
 *
 * This class must be registered with Qt's meta-object system
 * using
 * Q_DECLARE_METATYPE(CargoNetSim::Backend::
 *                   IntegrationSimulationFormatIConfigBase)
 * after class definition.
 */
class IntegrationSimulationFormatIConfigBase : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Default constructor
     * @param parent The parent QObject
     */
    IntegrationSimulationFormatIConfigBase(QObject* parent = nullptr);

    /**
     * @brief Destructor
     *
     * Properly cleans up all configuration resources.
     */
    ~IntegrationSimulationFormatIConfigBase();

    /**
     * @brief Adds a variable to the simulation configuration
     * @param key Variable name
     * @param value Variable value
     */
    void addVariable(const QString& key, const QString& value);

    /**
     * @brief Gets a variable from the simulation configuration
     * @param key Variable name
     * @return Variable value as string
     */
    QString getVariable(const QString& key) const;

    /**
     * @brief Gets all variables in the simulation configuration
     * @return Map of all variables
     */
    QMap<QString, QString> getVariables() const;

    /**
     * @brief Initializes the simulation configuration
     * @param configDir Configuration directory path
     * @param title Simulation title
     * @param simTime Total simulation time in hours
     * @param outputFreq10 Output frequency for type 10
     * @param outputFreq1214 Output frequency for types 12-14
     * @param routingOption Routing strategy option
     * @param pauseFlag Flag for simulation pausing
     * @param inputFolder Path to input files folder
     * @param outputFolder Path to output files folder
     * @param inputFiles Map of input file keys to filenames
     * @param outputFiles Map of output file keys to filenames
     */
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

    /**
     * @brief Gets the full path to an input file
     * @param fileKey Key identifier for the input file
     * @return Full path to the input file
     */
    QString getInputFilePath(const QString& fileKey) const;

    /**
     * @brief Gets the full path to an output file
     * @param fileKey Key identifier for the output file
     * @return Full path to the output file
     */
    QString getOutputFilePath(const QString& fileKey) const;

    /**
     * @brief Gets the network object
     * @return Pointer to the integration network
     */
    IntegrationNetworkBase* getNetwork() const;

    /**
     * @brief Converts the configuration to a JSON object
     * @return QJsonObject representation of the configuration
     */
    QJsonObject toDict() const;

    /**
     * @brief Creates a configuration from JSON data
     * @param data JSON object containing configuration data
     * @param parent The parent QObject
     * @return Pointer to the new configuration
     */
    static IntegrationSimulationFormatIConfigBase* fromDict(
        const QJsonObject& data,
        QObject* parent = nullptr);

signals:
    /**
     * @brief Signal emitted when configuration changes
     */
    void configChanged();

private:
    QString m_configDir;              ///< Configuration directory path
    QString m_title;                  ///< Simulation title
    float m_simTime;                  ///< Total simulation time in hours
    int m_outputFreq10;               ///< Output frequency for type 10
    int m_outputFreq1214;             ///< Output frequency for types 12-14
    int m_routingOption;              ///< Routing strategy option
    int m_pauseFlag;                  ///< Flag for simulation pausing
    QString m_inputFolder;            ///< Path to input files folder
    QString m_outputFolder;           ///< Path to output files folder
    QMap<QString, QString> m_inputFiles;  ///< Input file mappings
    QMap<QString, QString> m_outputFiles; ///< Output file mappings

    /// Network data records by type
    QMap<QString, QVector<QMap<QString, QString>>> m_networkData;

    QMap<QString, QString> m_variables; ///< Configuration variables

    mutable QMutex m_mutex; ///< Thread synchronization mutex
};

/**
 * @class IntegrationSimulationConfigReader
 * @brief Utility class for reading simulation configurations
 *
 * The IntegrationSimulationConfigReader provides static methods
 * to parse and load simulation configuration data from files.
 *
 * This class must be registered with Qt's meta-object system
 * using
 * Q_DECLARE_METATYPE(CargoNetSim::Backend::
 *                   IntegrationSimulationConfigReader)
 * after class definition.
 */
class IntegrationSimulationConfigReader : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Reads configuration data from a file
     * @param configFilePath Path to the configuration file
     * @return JSON object containing configuration data
     */
    static QJsonObject readConfig(const QString& configFilePath);
};

} // namespace Backend
} // namespace CargoNetSim

// Register custom types with Qt's meta-object system
Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationNode)
Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationNode*)
Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationLink)
Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationLink*)
Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationNodeDataReader)
Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationNodeDataReader*)
Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationLinkDataReader)
Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationLinkDataReader*)
Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationNetworkBase)
Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationNetworkBase*)
Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationSimulationFormatIConfigBase)
Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationSimulationFormatIConfigBase*)
Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationSimulationConfigReader)
Q_DECLARE_METATYPE(CargoNetSim::Backend::IntegrationSimulationConfigReader*)
