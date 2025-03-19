#include "TruckNetwork.h"
#include <QQueue>
#include <QSet>
#include <algorithm>
#include <QRegularExpression>

namespace CargoNetSim {
namespace Backend {

// IntegrationNode Implementation
IntegrationNode::IntegrationNode(QObject* parent)
    : QObject(parent)
    , m_nodeId(0)
    , m_xCoordinate(0.0f)
    , m_yCoordinate(0.0f)
    , m_nodeType(0)
    , m_macroZoneCluster(0)
    , m_informationAvailability(0)
    , m_xScale(1.0f)
    , m_yScale(1.0f)
{
}

IntegrationNode::IntegrationNode(
    int nodeId,
    float xCoordinate,
    float yCoordinate,
    int nodeType,
    int macroZoneCluster,
    int informationAvailability,
    const QString& description,
    float xScale,
    float yScale,
    QObject* parent
    ) : QObject(parent)
    , m_nodeId(nodeId)
    , m_xCoordinate(xCoordinate)
    , m_yCoordinate(yCoordinate)
    , m_nodeType(nodeType)
    , m_macroZoneCluster(macroZoneCluster)
    , m_informationAvailability(informationAvailability)
    , m_description(description)
    , m_xScale(xScale)
    , m_yScale(yScale)
{
}

IntegrationNode::IntegrationNode(const QJsonObject& json, QObject* parent)
    : QObject(parent)
{
    m_nodeId = json["node_id"].toInt();
    m_xCoordinate = json["x_coordinate"].toDouble();
    m_yCoordinate = json["y_coordinate"].toDouble();
    m_nodeType = json["node_type"].toInt();
    m_macroZoneCluster = json["macro_zone_cluster"].toInt();
    m_informationAvailability = json["information_availability"].toInt();
    m_description = json["description"].toString();
    m_xScale = json["x_scale"].toDouble();
    m_yScale = json["y_scale"].toDouble();
}

QJsonObject IntegrationNode::toDict() const {
    QJsonObject dict;
    dict["node_id"] = m_nodeId;
    dict["x_coordinate"] = m_xCoordinate;
    dict["y_coordinate"] = m_yCoordinate;
    dict["node_type"] = m_nodeType;
    dict["macro_zone_cluster"] = m_macroZoneCluster;
    dict["information_availability"] = m_informationAvailability;
    dict["description"] = m_description;
    dict["x_scale"] = m_xScale;
    dict["y_scale"] = m_yScale;
    return dict;
}

IntegrationNode* IntegrationNode::fromDict(const QJsonObject& data, QObject* parent) {
    return new IntegrationNode(
        data["node_id"].toInt(),
        data["x_coordinate"].toDouble(),
        data["y_coordinate"].toDouble(),
        data["node_type"].toInt(),
        data["macro_zone_cluster"].toInt(),
        data["information_availability"].toInt(),
        data["description"].toString(),
        data["x_scale"].toDouble(),
        data["y_scale"].toDouble(),
        parent
        );
}

void IntegrationNode::setNodeId(int nodeId) {
    if (m_nodeId != nodeId) {
        m_nodeId = nodeId;
        emit nodeChanged();
    }
}

void IntegrationNode::setXCoordinate(float xCoordinate) {
    if (m_xCoordinate != xCoordinate) {
        m_xCoordinate = xCoordinate;
        emit nodeChanged();
    }
}

void IntegrationNode::setYCoordinate(float yCoordinate) {
    if (m_yCoordinate != yCoordinate) {
        m_yCoordinate = yCoordinate;
        emit nodeChanged();
    }
}

void IntegrationNode::setNodeType(int nodeType) {
    if (m_nodeType != nodeType) {
        m_nodeType = nodeType;
        emit nodeChanged();
    }
}

void IntegrationNode::setMacroZoneCluster(int macroZoneCluster) {
    if (m_macroZoneCluster != macroZoneCluster) {
        m_macroZoneCluster = macroZoneCluster;
        emit nodeChanged();
    }
}

void IntegrationNode::setInformationAvailability(int informationAvailability) {
    if (m_informationAvailability != informationAvailability) {
        m_informationAvailability = informationAvailability;
        emit nodeChanged();
    }
}

void IntegrationNode::setDescription(const QString& description) {
    if (m_description != description) {
        m_description = description;
        emit nodeChanged();
    }
}

void IntegrationNode::setXScale(float xScale) {
    if (m_xScale != xScale) {
        m_xScale = xScale;
        emit nodeChanged();
    }
}

void IntegrationNode::setYScale(float yScale) {
    if (m_yScale != yScale) {
        m_yScale = yScale;
        emit nodeChanged();
    }
}

// IntegrationLink Implementation
IntegrationLink::IntegrationLink(QObject* parent)
    : QObject(parent)
    , m_linkId(0)
    , m_upstreamNodeId(0)
    , m_downstreamNodeId(0)
    , m_length(0.0f)
    , m_freeSpeed(0.0f)
    , m_saturationFlow(0.0f)
    , m_lanes(0.0f)
    , m_speedCoeffVariation(0.0f)
    , m_speedAtCapacity(0.0f)
    , m_jamDensity(0.0f)
    , m_turnProhibition(0)
    , m_prohibitionStart(0)
    , m_prohibitionEnd(0)
    , m_opposingLink1(0)
    , m_opposingLink2(0)
    , m_trafficSignal(0)
    , m_phase1(0)
    , m_phase2(0)
    , m_vehicleClassProhibition(0)
    , m_surveillanceLevel(0)
    , m_lengthScale(1.0f)
    , m_speedScale(1.0f)
    , m_saturationFlowScale(1.0f)
    , m_speedAtCapacityScale(1.0f)
    , m_jamDensityScale(1.0f)
{
}

IntegrationLink::IntegrationLink(
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
    QObject* parent
    ) : QObject(parent)
    , m_linkId(linkId)
    , m_upstreamNodeId(upstreamNodeId)
    , m_downstreamNodeId(downstreamNodeId)
    , m_length(length)
    , m_freeSpeed(freeSpeed)
    , m_saturationFlow(saturationFlow)
    , m_lanes(lanes)
    , m_speedCoeffVariation(speedCoeffVariation)
    , m_speedAtCapacity(speedAtCapacity)
    , m_jamDensity(jamDensity)
    , m_turnProhibition(turnProhibition)
    , m_prohibitionStart(prohibitionStart)
    , m_prohibitionEnd(prohibitionEnd)
    , m_opposingLink1(opposingLink1)
    , m_opposingLink2(opposingLink2)
    , m_trafficSignal(trafficSignal)
    , m_phase1(phase1)
    , m_phase2(phase2)
    , m_vehicleClassProhibition(vehicleClassProhibition)
    , m_surveillanceLevel(surveillanceLevel)
    , m_description(description)
    , m_lengthScale(lengthScale)
    , m_speedScale(speedScale)
    , m_saturationFlowScale(saturationFlowScale)
    , m_speedAtCapacityScale(speedAtCapacityScale)
    , m_jamDensityScale(jamDensityScale)
{
}

IntegrationLink::IntegrationLink(const QJsonObject& json, QObject* parent)
    : QObject(parent)
{
    m_linkId = json["link_id"].toInt();
    m_upstreamNodeId = json["upstream_node_id"].toInt();
    m_downstreamNodeId = json["downstream_node_id"].toInt();
    m_length = json["length"].toDouble();
    m_freeSpeed = json["free_speed"].toDouble();
    m_saturationFlow = json["saturation_flow"].toDouble();
    m_lanes = json["lanes"].toDouble();
    m_speedCoeffVariation = json["speed_coeff_variation"].toDouble();
    m_speedAtCapacity = json["speed_at_capacity"].toDouble();
    m_jamDensity = json["jam_density"].toDouble();
    m_turnProhibition = json["turn_prohibition"].toInt();
    m_prohibitionStart = json["prohibition_start"].toInt();
    m_prohibitionEnd = json["prohibition_end"].toInt();
    m_opposingLink1 = json["opposing_link_1"].toInt();
    m_opposingLink2 = json["opposing_link_2"].toInt();
    m_trafficSignal = json["traffic_signal"].toInt();
    m_phase1 = json["phase_1"].toInt();
    m_phase2 = json["phase_2"].toInt();
    m_vehicleClassProhibition = json["vehicle_class_prohibition"].toInt();
    m_surveillanceLevel = json["surveillance_level"].toInt();
    m_description = json["description"].toString();
    m_lengthScale = json["length_scale"].toDouble();
    m_speedScale = json["speed_scale"].toDouble();
    m_saturationFlowScale = json["saturation_flow_scale"].toDouble();
    m_speedAtCapacityScale = json["speed_at_capacity_scale"].toDouble();
    m_jamDensityScale = json["jam_density_scale"].toDouble();
}

QJsonObject IntegrationLink::toDict() const {
    QJsonObject dict;
    dict["link_id"] = m_linkId;
    dict["upstream_node_id"] = m_upstreamNodeId;
    dict["downstream_node_id"] = m_downstreamNodeId;
    dict["length"] = m_length;
    dict["free_speed"] = m_freeSpeed;
    dict["saturation_flow"] = m_saturationFlow;
    dict["lanes"] = m_lanes;
    dict["speed_coeff_variation"] = m_speedCoeffVariation;
    dict["speed_at_capacity"] = m_speedAtCapacity;
    dict["jam_density"] = m_jamDensity;
    dict["turn_prohibition"] = m_turnProhibition;
    dict["prohibition_start"] = m_prohibitionStart;
    dict["prohibition_end"] = m_prohibitionEnd;
    dict["opposing_link_1"] = m_opposingLink1;
    dict["opposing_link_2"] = m_opposingLink2;
    dict["traffic_signal"] = m_trafficSignal;
    dict["phase_1"] = m_phase1;
    dict["phase_2"] = m_phase2;
    dict["vehicle_class_prohibition"] = m_vehicleClassProhibition;
    dict["surveillance_level"] = m_surveillanceLevel;
    dict["description"] = m_description;
    dict["length_scale"] = m_lengthScale;
    dict["speed_scale"] = m_speedScale;
    dict["saturation_flow_scale"] = m_saturationFlowScale;
    dict["speed_at_capacity_scale"] = m_speedAtCapacityScale;
    dict["jam_density_scale"] = m_jamDensityScale;
    return dict;
}

IntegrationLink* IntegrationLink::fromDict(const QJsonObject& data, QObject* parent) {
    return new IntegrationLink(
        data["link_id"].toInt(),
        data["upstream_node_id"].toInt(),
        data["downstream_node_id"].toInt(),
        data["length"].toDouble(),
        data["free_speed"].toDouble(),
        data["saturation_flow"].toDouble(),
        data["lanes"].toDouble(),
        data["speed_coeff_variation"].toDouble(),
        data["speed_at_capacity"].toDouble(),
        data["jam_density"].toDouble(),
        data["turn_prohibition"].toInt(),
        data["prohibition_start"].toInt(),
        data["prohibition_end"].toInt(),
        data["opposing_link_1"].toInt(),
        data["opposing_link_2"].toInt(),
        data["traffic_signal"].toInt(),
        data["phase_1"].toInt(),
        data["phase_2"].toInt(),
        data["vehicle_class_prohibition"].toInt(),
        data["surveillance_level"].toInt(),
        data["description"].toString(),
        data["length_scale"].toDouble(),
        data["speed_scale"].toDouble(),
        data["saturation_flow_scale"].toDouble(),
        data["speed_at_capacity_scale"].toDouble(),
        data["jam_density_scale"].toDouble(),
        parent
        );
}

void IntegrationLink::setLinkId(int linkId) {
    if (m_linkId != linkId) {
        m_linkId = linkId;
        emit linkChanged();
    }
}

void IntegrationLink::setUpstreamNodeId(int upstreamNodeId) {
    if (m_upstreamNodeId != upstreamNodeId) {
        m_upstreamNodeId = upstreamNodeId;
        emit linkChanged();
    }
}

void IntegrationLink::setDownstreamNodeId(int downstreamNodeId) {
    if (m_downstreamNodeId != downstreamNodeId) {
        m_downstreamNodeId = downstreamNodeId;
        emit linkChanged();
    }
}

void IntegrationLink::setLength(float length) {
    if (m_length != length) {
        m_length = length;
        emit linkChanged();
    }
}

void IntegrationLink::setFreeSpeed(float freeSpeed) {
    if (m_freeSpeed != freeSpeed) {
        m_freeSpeed = freeSpeed;
        emit linkChanged();
    }
}

void IntegrationLink::setSaturationFlow(float saturationFlow) {
    if (m_saturationFlow != saturationFlow) {
        m_saturationFlow = saturationFlow;
        emit linkChanged();
    }
}

void IntegrationLink::setLanes(float lanes) {
    if (m_lanes != lanes) {
        m_lanes = lanes;
        emit linkChanged();
    }
}

void IntegrationLink::setSpeedCoeffVariation(float speedCoeffVariation) {
    if (m_speedCoeffVariation != speedCoeffVariation) {
        m_speedCoeffVariation = speedCoeffVariation;
        emit linkChanged();
    }
}

void IntegrationLink::setSpeedAtCapacity(float speedAtCapacity) {
    if (m_speedAtCapacity != speedAtCapacity) {
        m_speedAtCapacity = speedAtCapacity;
        emit linkChanged();
    }
}

void IntegrationLink::setJamDensity(float jamDensity) {
    if (m_jamDensity != jamDensity) {
        m_jamDensity = jamDensity;
        emit linkChanged();
    }
}

void IntegrationLink::setTurnProhibition(int turnProhibition) {
    if (m_turnProhibition != turnProhibition) {
        m_turnProhibition = turnProhibition;
        emit linkChanged();
    }
}

void IntegrationLink::setProhibitionStart(int prohibitionStart) {
    if (m_prohibitionStart != prohibitionStart) {
        m_prohibitionStart = prohibitionStart;
        emit linkChanged();
    }
}

void IntegrationLink::setProhibitionEnd(int prohibitionEnd) {
    if (m_prohibitionEnd != prohibitionEnd) {
        m_prohibitionEnd = prohibitionEnd;
        emit linkChanged();
    }
}

void IntegrationLink::setOpposingLink1(int opposingLink1) {
    if (m_opposingLink1 != opposingLink1) {
        m_opposingLink1 = opposingLink1;
        emit linkChanged();
    }
}

void IntegrationLink::setOpposingLink2(int opposingLink2) {
    if (m_opposingLink2 != opposingLink2) {
        m_opposingLink2 = opposingLink2;
        emit linkChanged();
    }
}

void IntegrationLink::setTrafficSignal(int trafficSignal) {
    if (m_trafficSignal != trafficSignal) {
        m_trafficSignal = trafficSignal;
        emit linkChanged();
    }
}

void IntegrationLink::setPhase1(int phase1) {
    if (m_phase1 != phase1) {
        m_phase1 = phase1;
        emit linkChanged();
    }
}

void IntegrationLink::setPhase2(int phase2) {
    if (m_phase2 != phase2) {
        m_phase2 = phase2;
        emit linkChanged();
    }
}

void IntegrationLink::setVehicleClassProhibition(int vehicleClassProhibition) {
    if (m_vehicleClassProhibition != vehicleClassProhibition) {
        m_vehicleClassProhibition = vehicleClassProhibition;
        emit linkChanged();
    }
}

void IntegrationLink::setSurveillanceLevel(int surveillanceLevel) {
    if (m_surveillanceLevel != surveillanceLevel) {
        m_surveillanceLevel = surveillanceLevel;
        emit linkChanged();
    }
}

void IntegrationLink::setDescription(const QString& description) {
    if (m_description != description) {
        m_description = description;
        emit linkChanged();
    }
}

void IntegrationLink::setLengthScale(float lengthScale) {
    if (m_lengthScale != lengthScale) {
        m_lengthScale = lengthScale;
        emit linkChanged();
    }
}

void IntegrationLink::setSpeedScale(float speedScale) {
    if (m_speedScale != speedScale) {
        m_speedScale = speedScale;
        emit linkChanged();
    }
}

void IntegrationLink::setSaturationFlowScale(float saturationFlowScale) {
    if (m_saturationFlowScale != saturationFlowScale) {
        m_saturationFlowScale = saturationFlowScale;
        emit linkChanged();
    }
}

void IntegrationLink::setSpeedAtCapacityScale(float speedAtCapacityScale) {
    if (m_speedAtCapacityScale != speedAtCapacityScale) {
        m_speedAtCapacityScale = speedAtCapacityScale;
        emit linkChanged();
    }
}

void IntegrationLink::setJamDensityScale(float jamDensityScale) {
    if (m_jamDensityScale != jamDensityScale) {
        m_jamDensityScale = jamDensityScale;
        emit linkChanged();
    }
}

// IntegrationNodeDataReader Implementation
QVector<QMap<QString, QString>> IntegrationNodeDataReader::readNodesFile(const QString& filename) {
    QVector<QMap<QString, QString>> records;
    
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error(QString("Error reading nodes file: %1").arg(file.errorString()).toStdString());
    }
    
    QTextStream in(&file);
    QStringList lines;
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        // Filter out control characters at the end
        line = line.replace(QRegularExpression("\\x1a"), "").trimmed();
        lines.append(line);
    }
    
    file.close();
    
    // Remove empty lines at the end
    while (!lines.isEmpty() && lines.last().isEmpty()) {
        lines.removeLast();
    }
    
    if (lines.isEmpty()) {
        throw std::runtime_error("Nodes file is empty");
    }
    
    // Parse scale values from second line
    QStringList scales = lines[1].trimmed().split(QRegularExpression("\\s+"));
    if (scales.size() < 3) {
        throw std::runtime_error("Bad nodes file structure");
    }
    
    QString n = scales[0];
    QString scaleX = scales[1];
    QString scaleY = scales[2];
    
    // Parse node records
    for (int i = 2; i < lines.size(); ++i) {
        QStringList values = lines[i].trimmed().split(QRegularExpression("\\s+"));
        
        // Ensure we have enough values
        if (values.size() < 6) {
            values.append("");
        }
        
        QMap<QString, QString> record;
        record["node_id"] = values[0];
        record["x_coordinate"] = values[1];
        record["y_coordinate"] = values[2];
        record["node_type"] = values[3];
        record["macro_zone_cluster"] = values[4];
        record["information_availability"] = values[5];
        
        // Join the remaining values as the description
        if (values.size() > 6) {
            record["description"] = values.mid(6).join(" ");
        } else {
            record["description"] = "";
        }
        
        record["x_scale"] = scaleX;
        record["y_scale"] = scaleY;
        
        records.append(record);
    }
    
    return records;
}

// IntegrationLinkDataReader Implementation
QVector<QMap<QString, QString>> IntegrationLinkDataReader::readLinksFile(const QString& filename) {
    QVector<QMap<QString, QString>> records;
    
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error(QString("Error reading links file: %1").arg(file.errorString()).toStdString());
    }
    
    QTextStream in(&file);
    QStringList lines;
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        // Filter out control characters at the end
        line = line.replace(QRegularExpression("\\x1a"), "").trimmed();
        lines.append(line);
    }
    
    file.close();
    
    // Remove empty lines at the end
    while (!lines.isEmpty() && lines.last().isEmpty()) {
        lines.removeLast();
    }
    
    if (lines.isEmpty()) {
        throw std::runtime_error("Links file is empty");
    }
    
    // Parse scale values from second line
    QStringList scales = lines[1].trimmed().split(QRegularExpression("\\s+"));
    if (scales.size() < 6) {
        throw std::runtime_error("Bad links file structure");
    }
    
    QString n = scales[0];
    QString lengthScale = scales[1];
    QString speedScale = scales[2];
    QString saturationFlowScale = scales[3];
    QString speedAtCapacityScale = scales[4];
    QString jamDensityScale = scales[5];
    
    // Parse link records
    for (int i = 2; i < lines.size(); ++i) {
        QStringList values = lines[i].trimmed().split(QRegularExpression("\\s+"));
        
        // Ensure we have at least 20 values
        if (values.size() < 20) {
            continue;  // Skip malformed records
        }
        
        QMap<QString, QString> record;
        record["link_id"] = values[0];
        record["upstream_node_id"] = values[1];
        record["downstream_node_id"] = values[2];
        record["length"] = values[3];
        record["free_speed"] = values[4];
        record["saturation_flow"] = values[5];
        record["lanes"] = values[6];
        record["speed_coeff_variation"] = values[7];
        record["speed_at_capacity"] = values[8];
        record["jam_density"] = values[9];
        record["turn_prohibition"] = values[10];
        record["prohibition_start"] = values[11];
        record["prohibition_end"] = values[12];
        record["opposing_link_1"] = values[13];
        record["opposing_link_2"] = values[14];
        record["traffic_signal"] = values[15];
        record["phase_1"] = values[16];
        record["phase_2"] = values[17];
        record["vehicle_class_prohibition"] = values[18];
        record["surveillance_level"] = values[19];
        
        // Join the remaining values as the description
        if (values.size() > 20) {
            record["description"] = values.mid(20).join(" ");
        } else {
            record["description"] = "";
        }
        
        record["length_scale"] = lengthScale;
        record["speed_scale"] = speedScale;
        record["saturation_flow_scale"] = saturationFlowScale;
        record["speed_at_capacity_scale"] = speedAtCapacityScale;
        record["jam_density_scale"] = jamDensityScale;
        
        records.append(record);
    }
    
    return records;
}

// IntegrationNetworkBase Implementation
IntegrationNetworkBase::IntegrationNetworkBase(QObject* parent)
    : QObject(parent)
{
}

IntegrationNetworkBase::~IntegrationNetworkBase() {
    // Clean up node and link objects
    qDeleteAll(m_nodeObjects);
    qDeleteAll(m_linkObjects);
    
    m_nodeObjects.clear();
    m_linkObjects.clear();
}

void IntegrationNetworkBase::initializeNetwork(const QVector<IntegrationNode*>& nodes, const QVector<IntegrationLink*>& links) {
    QMutexLocker locker(&m_mutex);
    
    // Clean up existing objects
    qDeleteAll(m_nodeObjects);
    qDeleteAll(m_linkObjects);
    
    m_nodeObjects.clear();
    m_linkObjects.clear();
    m_nodes.clear();
    m_links.clear();
    m_graph.clear();
    
    // Store node objects and convert to JSON for storage
    m_nodeObjects = nodes;
    for (IntegrationNode* node : nodes) {
        node->setParent(this);  // Take ownership
        m_nodes.append(node->toDict());
    }
    
    // Store link objects and convert to JSON for storage
    m_linkObjects = links;
    for (IntegrationLink* link : links) {
        link->setParent(this);  // Take ownership
        m_links.append(link->toDict());
    }
    
    // Build the graph
    addNodesWithCoordinates(nodes);
    buildGraph(links);
    
    emit networkChanged();
    emit nodesChanged();
    emit linksChanged();
}

void IntegrationNetworkBase::addNodesWithCoordinates(const QVector<IntegrationNode*>& nodes) {
    for (IntegrationNode* node : nodes) {
        QMap<QString, QVariant> attributes;
        attributes["x"] = node->xCoordinate();
        attributes["y"] = node->yCoordinate();
        attributes["node_type"] = node->nodeType();
        attributes["macro_zone_cluster"] = node->macroZoneCluster();
        attributes["information_availability"] = node->informationAvailability();
        attributes["description"] = node->description();
        
        m_graph.addNode(node->nodeId(), attributes);
    }
}

void IntegrationNetworkBase::buildGraph(const QVector<IntegrationLink*>& links) {
    for (IntegrationLink* link : links) {
        QMap<QString, QVariant> attributes;
        attributes["link_id"] = link->linkId();
        
        m_graph.addEdge(link->upstreamNodeId(), link->downstreamNodeId(), link->length(), attributes);
    }
}

bool IntegrationNetworkBase::nodeExists(int nodeId) const {
    QMutexLocker locker(&m_mutex);
    return m_graph.hasNode(nodeId);
}

QJsonObject IntegrationNetworkBase::findShortestPath(int startNodeId, int endNodeId) {
    QMutexLocker locker(&m_mutex);
    
    try {
        QVector<int> pathNodes = m_graph.findShortestPath(startNodeId, endNodeId, "distance");
        
        // If no path found, return empty result
        if (pathNodes.isEmpty()) {
            return QJsonObject();
        }
        
        // Get path links from path nodes
        QVector<int> pathLinks = getPathLinks(pathNodes);
        
        // Calculate total path length
        float totalLength = getPathLengthByLinks(pathLinks);
        
        // Calculate total travel time
        float totalTravelTime = 0.0f;
        for (const QJsonObject& linkJson : m_links) {
            if (pathLinks.contains(linkJson["link_id"].toInt())) {
                // Convert length to meters (it's stored in km)
                float linkLength = linkJson["length"].toDouble() * 1000.0f;
                float linkFreeSpeed = linkJson["free_speed"].toDouble();
                totalTravelTime += linkLength / linkFreeSpeed;
            }
        }
        
        // Create result object
        QJsonObject result;
        
        // Convert path nodes to JSON array
        QJsonArray pathNodesArray;
        for (int nodeId : pathNodes) {
            pathNodesArray.append(nodeId);
        }
        result["path_nodes"] = pathNodesArray;
        
        // Convert path links to JSON array
        QJsonArray pathLinksArray;
        for (int linkId : pathLinks) {
            pathLinksArray.append(linkId);
        }
        result["path_links"] = pathLinksArray;
        
        result["total_length"] = totalLength;
        result["min_travel_time"] = totalTravelTime;
        
        return result;
    } catch (const std::exception& e) {
        qCritical() << "Error finding shortest path:" << e.what();
        return QJsonObject();
    }
}

QVector<int> IntegrationNetworkBase::getPathLinks(const QVector<int>& pathNodes) const {
    QVector<int> pathLinks;
    
    // For each consecutive pair of nodes in the path
    for (int i = 0; i < pathNodes.size() - 1; ++i) {
        int upstreamNodeId = pathNodes[i];
        int downstreamNodeId = pathNodes[i + 1];
        
        // Get the edge attributes from the graph
        QMap<QString, QVariant> edgeAttributes = m_graph.getEdgeAttributes(upstreamNodeId, downstreamNodeId);
        
        if (edgeAttributes.contains("link_id")) {
            pathLinks.append(edgeAttributes["link_id"].toInt());
        }
    }
    
    return pathLinks;
}

float IntegrationNetworkBase::getPathLengthByNodes(const QVector<int>& pathNodes) const {
    if (pathNodes.size() < 2) {
        return 0.0f;
    }
    
    float totalLength = 0.0f;
    for (int i = 0; i < pathNodes.size() - 1; ++i) {
        int upstreamNodeId = pathNodes[i];
        int downstreamNodeId = pathNodes[i + 1];
        
        if (m_graph.hasEdge(upstreamNodeId, downstreamNodeId)) {
            totalLength += m_graph.getEdgeWeight(upstreamNodeId, downstreamNodeId) * 1000.0f;
        } else {
            throw std::runtime_error(
                QString("No edge exists between nodes %1 and %2").arg(upstreamNodeId).arg(downstreamNodeId).toStdString()
                );
        }
    }
    
    return totalLength;
}

float IntegrationNetworkBase::getPathLengthByLinks(const QVector<int>& pathLinks) const {
    float totalLength = 0.0f;
    
    for (const QJsonObject& linkJson : m_links) {
        if (pathLinks.contains(linkJson["link_id"].toInt())) {
            totalLength += linkJson["length"].toDouble() * 1000.0f;
        }
    }
    
    return totalLength;
}

QVector<int> IntegrationNetworkBase::getEndNodes() const {
    QVector<int> endNodes;
    
    for (int nodeId : m_graph.getNodes()) {
        if (m_graph.getOutDegree(nodeId) == 0) {
            endNodes.append(nodeId);
        }
    }
    
    return endNodes;
}

QVector<int> IntegrationNetworkBase::getStartNodes() const {
    QVector<int> startNodes;
    
    for (int nodeId : m_graph.getNodes()) {
        if (m_graph.getInDegree(nodeId) == 0) {
            startNodes.append(nodeId);
        }
    }
    
    return startNodes;
}

QJsonObject IntegrationNetworkBase::getStartAndEndNodes() const {
    QVector<int> startNodes = getStartNodes();
    QVector<int> endNodes = getEndNodes();
    
    QJsonArray startNodesArray;
    for (int nodeId : startNodes) {
        startNodesArray.append(nodeId);
    }
    
    QJsonArray endNodesArray;
    for (int nodeId : endNodes) {
        endNodesArray.append(nodeId);
    }
    
    QJsonObject result;
    result["start_nodes"] = startNodesArray;
    result["end_nodes"] = endNodesArray;
    
    return result;
}

QVector<QJsonObject> IntegrationNetworkBase::getNodes() const {
    QMutexLocker locker(&m_mutex);
    return m_nodes;
}

QVector<QJsonObject> IntegrationNetworkBase::getLinks() const {
    QMutexLocker locker(&m_mutex);
    return m_links;
}

QJsonObject IntegrationNetworkBase::nodesToJson() const {
    QMutexLocker locker(&m_mutex);
    
    if (m_nodes.isEmpty()) {
        QJsonObject result;
        result["nodes"] = QJsonArray();
        return result;
    }
    
    QJsonArray nodesArray;
    for (const QJsonObject& nodeJson : m_nodes) {
        QJsonObject nodeData{
            {"node_id", nodeJson["node_id"].toInt()},
            {"x_coordinate", nodeJson["x_coordinate"].toDouble()},
            {"y_coordinate", nodeJson["y_coordinate"].toDouble()},
            {"node_type", nodeJson["node_type"].toInt()},
            {"macro_zone_cluster", nodeJson["macro_zone_cluster"].toInt()},
            {"information_availability", nodeJson["information_availability"].toInt()},
            {"description", nodeJson["description"].toString()}
        };
        nodesArray.append(nodeData);
    }
    
    QJsonObject result;
    result["nodes"] = nodesArray;
    
    return result;
}

QJsonObject IntegrationNetworkBase::linksToJson() const {
    QMutexLocker locker(&m_mutex);
    
    if (m_links.isEmpty()) {
        QJsonObject result;
        result["links"] = QJsonArray();
        return result;
    }
    
    QJsonArray linksArray;
    for (const QJsonObject& linkJson : m_links) {
        QJsonObject linkData{
            {"link_id", linkJson["link_id"].toInt()},
            {"upstream_node_id", linkJson["upstream_node_id"].toInt()},
            {"downstream_node_id", linkJson["downstream_node_id"].toInt()},
            {"length", linkJson["length"].toDouble()},
            {"free_speed", linkJson["free_speed"].toDouble()},
            {"saturation_flow", linkJson["saturation_flow"].toDouble()},
            {"lanes", linkJson["lanes"].toDouble()},
            {"speed_coeff_variation", linkJson["speed_coeff_variation"].toDouble()},
            {"speed_at_capacity", linkJson["speed_at_capacity"].toDouble()},
            {"jam_density", linkJson["jam_density"].toDouble()},
            {"description", linkJson["description"].toString()}
        };
        linksArray.append(linkData);
    }
    
    QJsonObject result;
    result["links"] = linksArray;
    
    return result;
}

// IntegrationSimulationFormatIConfigBase Implementation
IntegrationSimulationFormatIConfigBase::IntegrationSimulationFormatIConfigBase(QObject* parent)
    : QObject(parent)
    , m_simTime(0.0f)
    , m_outputFreq10(0)
    , m_outputFreq1214(0)
    , m_routingOption(0)
    , m_pauseFlag(0)
{
}

IntegrationSimulationFormatIConfigBase::~IntegrationSimulationFormatIConfigBase() {
}

void IntegrationSimulationFormatIConfigBase::addVariable(const QString& key, const QString& value) {
    QMutexLocker locker(&m_mutex);
    m_variables[key] = value;
}

QString IntegrationSimulationFormatIConfigBase::getVariable(const QString& key) const {
    QMutexLocker locker(&m_mutex);
    return m_variables.value(key, QString());
}

QMap<QString, QString> IntegrationSimulationFormatIConfigBase::getVariables() const {
    QMutexLocker locker(&m_mutex);
    return m_variables;
}

void IntegrationSimulationFormatIConfigBase::initializeConfig(
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
    ) {
    QMutexLocker locker(&m_mutex);
    
    m_configDir = configDir;
    m_title = title;
    m_simTime = simTime;
    m_outputFreq10 = outputFreq10;
    m_outputFreq1214 = outputFreq1214;
    m_routingOption = routingOption;
    m_pauseFlag = pauseFlag;
    m_inputFolder = inputFolder;
    m_outputFolder = outputFolder;
    m_inputFiles = inputFiles;
    m_outputFiles = outputFiles;
    
    try {
        // Read node data
        QVector<QMap<QString, QString>> nodes = IntegrationNodeDataReader::readNodesFile(
            getInputFilePath("node_coordinates")
            );
        
        // Read link data
        QVector<QMap<QString, QString>> links = IntegrationLinkDataReader::readLinksFile(
            getInputFilePath("link_structure")
            );
        
        if (nodes.isEmpty() || links.isEmpty()) {
            throw std::runtime_error("Invalid network data: empty nodes or links");
        }
        
        m_networkData["nodes"] = nodes;
        m_networkData["links"] = links;
        
        emit configChanged();
    } catch (const std::exception& e) {
        qCritical() << "Error initializing config:" << e.what();
        throw;
    }
}

QString IntegrationSimulationFormatIConfigBase::getInputFilePath(const QString& fileKey) const {
    if (m_inputFiles.contains(fileKey)) {
        return QDir(m_configDir).filePath(QDir(m_inputFolder).filePath(m_inputFiles[fileKey]));
    } else {
        throw std::runtime_error(
            QString("No input file found with key '%1'").arg(fileKey).toStdString()
            );
    }
}

QString IntegrationSimulationFormatIConfigBase::getOutputFilePath(const QString& fileKey) const {
    if (m_outputFiles.contains(fileKey) && !m_outputFiles[fileKey].isEmpty()) {
        return QDir(m_configDir).filePath(QDir(m_outputFolder).filePath(m_outputFiles[fileKey]));
    } else {
        throw std::runtime_error(
            QString("No output file found with key '%1'").arg(fileKey).toStdString()
            );
    }
}

IntegrationNetworkBase* IntegrationSimulationFormatIConfigBase::getNetwork() const {
    QMutexLocker locker(&m_mutex);
    
    if (m_networkData["nodes"].isEmpty() || m_networkData["links"].isEmpty()) {
        return nullptr;
    }
    
    IntegrationNetworkBase* network = new IntegrationNetworkBase();
    
    // Create nodes
    QVector<IntegrationNode*> nodes;
    for (const QMap<QString, QString>& nodeData : m_networkData["nodes"]) {
        QJsonObject jsonData;
        for (auto it = nodeData.constBegin(); it != nodeData.constEnd(); ++it) {
            jsonData[it.key()] = it.value();
        }
        
        IntegrationNode* node = IntegrationNode::fromDict(jsonData, network);
        nodes.append(node);
    }
    
    // Create links
    QVector<IntegrationLink*> links;
    for (const QMap<QString, QString>& linkData : m_networkData["links"]) {
        QJsonObject jsonData;
        for (auto it = linkData.constBegin(); it != linkData.constEnd(); ++it) {
            jsonData[it.key()] = it.value();
        }
        
        IntegrationLink* link = IntegrationLink::fromDict(jsonData, network);
        links.append(link);
    }
    
    // Initialize network
    network->initializeNetwork(nodes, links);
    
    return network;
}

QJsonObject IntegrationSimulationFormatIConfigBase::toDict() const {
    QMutexLocker locker(&m_mutex);
    
    try {
        IntegrationNetworkBase* network = getNetwork();
        QJsonObject dict;
        
        dict["config_dir"] = m_configDir;
        dict["title"] = m_title;
        dict["sim_time"] = m_simTime;
        dict["output_freq_10"] = m_outputFreq10;
        dict["output_freq_12_14"] = m_outputFreq1214;
        dict["routing_option"] = m_routingOption;
        dict["pause_flag"] = m_pauseFlag;
        dict["input_folder"] = m_inputFolder;
        dict["output_folder"] = m_outputFolder;
        
        // Convert input files
        QJsonObject inputFilesJson;
        for (auto it = m_inputFiles.constBegin(); it != m_inputFiles.constEnd(); ++it) {
            inputFilesJson[it.key()] = it.value();
        }
        dict["input_files"] = inputFilesJson;
        
        // Convert output files
        QJsonObject outputFilesJson;
        for (auto it = m_outputFiles.constBegin(); it != m_outputFiles.constEnd(); ++it) {
            outputFilesJson[it.key()] = it.value();
        }
        dict["output_files"] = outputFilesJson;
        
        // Convert variables
        QJsonObject variablesJson;
        for (auto it = m_variables.constBegin(); it != m_variables.constEnd(); ++it) {
            variablesJson[it.key()] = it.value();
        }
        dict["variables"] = variablesJson;
        
        // Convert network data
        QJsonObject networkDataJson;
        
        if (network) {
            QJsonArray nodesJson;
            for (const QJsonObject& node : network->getNodes()) {
                nodesJson.append(node);
            }
            
            QJsonArray linksJson;
            for (const QJsonObject& link : network->getLinks()) {
                linksJson.append(link);
            }
            
            networkDataJson["nodes"] = nodesJson;
            networkDataJson["links"] = linksJson;
        } else {
            networkDataJson["nodes"] = QJsonArray();
            networkDataJson["links"] = QJsonArray();
        }
        
        dict["network_data"] = networkDataJson;
        
        // Clean up
        delete network;
        
        return dict;
    } catch (const std::exception& e) {
        throw std::runtime_error(
            QString("Failed to serialize configuration: %1").arg(e.what()).toStdString()
            );
    }
}

IntegrationSimulationFormatIConfigBase* IntegrationSimulationFormatIConfigBase::fromDict(const QJsonObject& data, QObject* parent) {
    try {
        IntegrationSimulationFormatIConfigBase* config = new IntegrationSimulationFormatIConfigBase(parent);
        
        // Extract network data
        QJsonObject networkDataJson = data["network_data"].toObject();
        QMap<QString, QVector<QMap<QString, QString>>> networkData;
        
        // Parse nodes
        QJsonArray nodesJson = networkDataJson["nodes"].toArray();
        QVector<QMap<QString, QString>> nodes;
        
        for (const QJsonValue& nodeValue : nodesJson) {
            QJsonObject nodeJson = nodeValue.toObject();
            QMap<QString, QString> node;
            
            for (auto it = nodeJson.constBegin(); it != nodeJson.constEnd(); ++it) {
                node[it.key()] = it.value().toString();
            }
            
            nodes.append(node);
        }
        
        networkData["nodes"] = nodes;
        
        // Parse links
        QJsonArray linksJson = networkDataJson["links"].toArray();
        QVector<QMap<QString, QString>> links;
        
        for (const QJsonValue& linkValue : linksJson) {
            QJsonObject linkJson = linkValue.toObject();
            QMap<QString, QString> link;
            
            for (auto it = linkJson.constBegin(); it != linkJson.constEnd(); ++it) {
                link[it.key()] = it.value().toString();
            }
            
            links.append(link);
        }
        
        networkData["links"] = links;
        
        // Parse input files
        QJsonObject inputFilesJson = data["input_files"].toObject();
        QMap<QString, QString> inputFiles;
        
        for (auto it = inputFilesJson.constBegin(); it != inputFilesJson.constEnd(); ++it) {
            inputFiles[it.key()] = it.value().toString();
        }
        
        // Parse output files
        QJsonObject outputFilesJson = data["output_files"].toObject();
        QMap<QString, QString> outputFiles;
        
        for (auto it = outputFilesJson.constBegin(); it != outputFilesJson.constEnd(); ++it) {
            outputFiles[it.key()] = it.value().toString();
        }
        
        // Initialize core configuration
        config->initializeConfig(
            data["config_dir"].toString(),
            data["title"].toString(),
            data["sim_time"].toDouble(),
            data["output_freq_10"].toInt(),
            data["output_freq_12_14"].toInt(),
            data["routing_option"].toInt(),
            data["pause_flag"].toInt(),
            data["input_folder"].toString(),
            data["output_folder"].toString(),
            inputFiles,
            outputFiles
            );
        
        // Store network data
        config->m_networkData = networkData;
        
        // Restore variables
        QJsonObject variablesJson = data["variables"].toObject();
        for (auto it = variablesJson.constBegin(); it != variablesJson.constEnd(); ++it) {
            config->addVariable(it.key(), it.value().toString());
        }
        
        return config;
    } catch (const std::exception& e) {
        throw std::runtime_error(
            QString("Failed to deserialize configuration: %1").arg(e.what()).toStdString()
            );
    }
}

// IntegrationSimulationConfigReader Implementation
QJsonObject IntegrationSimulationConfigReader::readConfig(const QString& configFilePath) {
    try {
        QString configDir = QFileInfo(configFilePath).dir().absolutePath();
        
        QFile file(configFilePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error(
                QString("Error opening configuration file: %1").arg(file.errorString()).toStdString()
                );
        }
        
        QTextStream in(&file);
        QStringList lines;
        
        while (!in.atEnd()) {
            lines.append(in.readLine());
        }
        
        file.close();
        
        if (lines.isEmpty()) {
            throw std::runtime_error("Configuration file is empty");
        }
        
        // Read file title (Line 1)
        QString title = lines[0].trimmed();
        
        // Read simulation parameters (Line 2)
        QStringList simParams = lines[1].trimmed().split(QRegularExpression("\\s+"));
        if (simParams.size() < 5) {
            throw std::runtime_error("Bad simulation parameters structure");
        }
        
        float simTime = simParams[0].toFloat();
        int outputFreq10 = simParams[1].toInt();
        int outputFreq1214 = simParams[2].toInt();
        int routingOption = simParams[3].toInt();
        int pauseFlag = simParams[4].toInt();
        
        // Read input and output folders
        QString inputFolder = lines[2].trimmed();
        if (inputFolder.isEmpty()) {
            inputFolder = ".";
        }
        
        QString outputFolder = lines[3].trimmed();
        if (outputFolder.isEmpty()) {
            outputFolder = ".";
        }
        
        // Read input files
        QMap<QString, QString> inputFiles;
        inputFiles["node_coordinates"] = lines[4].trimmed();
        inputFiles["link_structure"] = lines[5].trimmed();
        inputFiles["signal_timing"] = lines[6].trimmed();
        inputFiles["traffic_demands"] = lines[7].trimmed();
        inputFiles["incident_descriptions"] = lines[8].trimmed();
        
        // Read output files
        QMap<QString, QString> outputFiles;
        QVector<QString> outputFileKeys = {
            "standard_output",
            "link_flow_microscopic",
            "link_flow_minimum_tree",
            "minimum_path_tree_routing",
            "trip_based_vehicle_probe",
            "second_by_second_vehicle_probe",
            "link_travel_time",
            "minimum_path_tree_output_1",
            "minimum_path_tree_output_2",
            "vehicle_departures",
            "individual_vehicle_path",
            "emission_concentration",
            "summary_output",
            "link_flow_mesoscopic",
            "time_space_output"
        };
        
        int lineIndex = 9;
        for (const QString& key : outputFileKeys) {
            if (lineIndex < lines.size()) {
                QString value = lines[lineIndex].trimmed();
                outputFiles[key] = value;
                lineIndex++;
            }
        }
        
        // Create result
        QJsonObject result;
        result["config_dir"] = configDir;
        result["title"] = title;
        result["sim_time"] = simTime;
        result["output_freq_10"] = outputFreq10;
        result["output_freq_12_14"] = outputFreq1214;
        result["routing_option"] = routingOption;
        result["pause_flag"] = pauseFlag;
        result["input_folder"] = inputFolder;
        result["output_folder"] = outputFolder;
        
        // Convert input files to JSON
        QJsonObject inputFilesJson;
        for (auto it = inputFiles.constBegin(); it != inputFiles.constEnd(); ++it) {
            inputFilesJson[it.key()] = it.value();
        }
        result["input_files"] = inputFilesJson;
        
        // Convert output files to JSON
        QJsonObject outputFilesJson;
        for (auto it = outputFiles.constBegin(); it != outputFiles.constEnd(); ++it) {
            outputFilesJson[it.key()] = it.value();
        }
        result["output_files"] = outputFilesJson;
        
        return result;
    } catch (const std::exception& e) {
        throw std::runtime_error(
            QString("Error reading configuration file: %1").arg(e.what()).toStdString()
            );
    }
}

} // namespace Backend
} // namespace CargoNetSim
