/**
 * @file TruckNetwork.cpp
 * @brief Implements truck network model
 * @author Ahmed Aredah
 * @date 2025-03-22
 */

#include "TruckNetwork.h"
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QTextStream>

namespace CargoNetSim
{
namespace Backend
{
namespace TruckClient
{

// Node and Link class forward declarations
class IntegrationNode;
class IntegrationLink;

/**************** SharedIntegrationNetwork ****************/

IntegrationNetwork::IntegrationNetwork(QObject *parent)
    : BaseObject(parent)
{
}

IntegrationNetwork::~IntegrationNetwork()
{
    // Clean up resources
    qDeleteAll(m_nodeObjects);
    qDeleteAll(m_linkObjects);
    if (m_graph)
    {
        m_graph->deleteLater();
        m_graph = nullptr;
    }

    m_nodeObjects.clear();
    m_linkObjects.clear();
}

void IntegrationNetwork::initializeNetwork(
    const QVector<IntegrationNode *> &nodes,
    const QVector<IntegrationLink *> &links)
{
    QMutexLocker locker(&m_mutex);

    // Clean up existing resources
    qDeleteAll(m_nodeObjects);
    qDeleteAll(m_linkObjects);

    m_nodeObjects.clear();
    m_linkObjects.clear();
    if (m_graph)
    {
        m_graph->deleteLater();
        m_graph = nullptr;
    }
    m_graph = new TransportationGraph<int>(); // Reset graph

    // Store nodes and links
    m_nodeObjects = nodes;
    m_linkObjects = links;

    // Take ownership of objects
    for (auto *node : nodes)
    {
        node->setParent(this);

        // Add node to graph with relevant attributes
        QMap<QString, QVariant> attributes;
        attributes["x"]    = node->getXCoordinate();
        attributes["y"]    = node->getYCoordinate();
        attributes["type"] = node->getNodeType();
        m_graph->addNode(node->getNodeId(), attributes);
    }

    // Process links
    for (auto *link : links)
    {
        link->setParent(this);

        // Add edge to graph
        int   fromNode = link->getUpstreamNodeId();
        int   toNode   = link->getDownstreamNodeId();
        float weight   = link->getLength();

        QMap<QString, QVariant> attributes;
        attributes["link_id"]    = link->getLinkId();
        attributes["free_speed"] = link->getFreeSpeed();
        attributes["lanes"]      = link->getLanes();

        m_graph->addEdge(fromNode, toNode, weight,
                         attributes);
    }

    // Emit change signals
    emit networkChanged();
    emit nodesChanged();
    emit linksChanged();
}

bool IntegrationNetwork::nodeExists(int nodeId) const
{
    QMutexLocker locker(&m_mutex);
    return m_graph->hasNode(nodeId);
}

ShortestPathResult
IntegrationNetwork::findShortestPath(int startNodeId,
                                     int endNodeId)
{
    QMutexLocker       locker(&m_mutex);
    ShortestPathResult result;

    result.optimizationCriterion =
        "distance"; // Default optimization criterion

    // Find path using transportation graph
    result.pathNodes = m_graph->findShortestPath(
        startNodeId, endNodeId, "distance");

    // If no path found, return empty result (already
    // initialized with infinity values)
    if (result.pathNodes.isEmpty())
    {
        return result;
    }

    // Get corresponding links
    result.pathLinks = getPathLinks(result.pathNodes);

    // Calculate path metrics
    result.totalLength =
        getPathLengthByLinks(result.pathLinks);
    result.minTravelTime = m_graph->calculatePathMetric(
        result.pathNodes, "time");

    return result;
}

QVector<int> IntegrationNetwork::getEndNodes() const
{
    QMutexLocker locker(&m_mutex);
    QVector<int> endNodes;

    // Find nodes with no outgoing edges
    for (int nodeId : m_graph->getNodes())
    {
        if (m_graph->getOutDegree(nodeId) == 0)
        {
            endNodes.append(nodeId);
        }
    }

    return endNodes;
}

QVector<int> IntegrationNetwork::getStartNodes() const
{
    QMutexLocker locker(&m_mutex);
    QVector<int> startNodes;

    // Find nodes with no incoming edges
    for (int nodeId : m_graph->getNodes())
    {
        if (m_graph->getInDegree(nodeId) == 0)
        {
            startNodes.append(nodeId);
        }
    }

    return startNodes;
}

QVector<IntegrationNode *>
IntegrationNetwork::getNodes() const
{
    QMutexLocker locker(&m_mutex);
    return m_nodeObjects;
}

QVector<IntegrationLink *>
IntegrationNetwork::getLinks() const
{
    QMutexLocker locker(&m_mutex);
    return m_linkObjects;
}

const TransportationGraph<int> *
IntegrationNetwork::getGraph() const
{
    return m_graph;
}

IntegrationNode *
IntegrationNetwork::getNode(int nodeId) const
{
    QMutexLocker locker(&m_mutex);

    // Find node by ID
    for (IntegrationNode *node : m_nodeObjects)
    {
        if (node->getNodeId() == nodeId)
        {
            return node;
        }
    }

    return nullptr;
}

IntegrationLink *
IntegrationNetwork::getLink(int linkId) const
{
    QMutexLocker locker(&m_mutex);

    // Find link by ID
    for (IntegrationLink *link : m_linkObjects)
    {
        if (link->getLinkId() == linkId)
        {
            return link;
        }
    }

    return nullptr;
}

void IntegrationNetwork::setNetworkName(QString networkName)
{
    QMutexLocker locker(&m_mutex);
    m_networkName = networkName;
}

QString IntegrationNetwork::getNetworkName() const
{
    return m_networkName;
}

QList<QJsonObject> IntegrationNetwork::getMultiplePaths(
    int startNodeId, int endNodeId, int maxPaths)
{
    QMutexLocker       locker(&m_mutex);
    QList<QJsonObject> results;

    // Find k-shortest paths
    QList<QVector<int>> paths = m_graph->findKShortestPaths(
        startNodeId, endNodeId, maxPaths);

    // Process each path
    for (const QVector<int> &path : paths)
    {
        // Get links for path
        QVector<int> links = getPathLinks(path);

        // Calculate metrics
        double length = getPathLengthByLinks(links);
        double time =
            m_graph->calculatePathMetric(path, "time");

        // Build result object
        QJsonObject pathObj;

        // Add nodes
        QJsonArray nodesArray;
        for (int nodeId : path)
        {
            nodesArray.append(nodeId);
        }
        pathObj["path_nodes"] = nodesArray;

        // Add links
        QJsonArray linksArray;
        for (int linkId : links)
        {
            linksArray.append(linkId);
        }
        pathObj["path_links"] = linksArray;

        // Add metrics
        pathObj["total_length"]    = length;
        pathObj["min_travel_time"] = time;

        results.append(pathObj);
    }

    return results;
}

QJsonObject IntegrationNetwork::toJson() const
{
    QMutexLocker locker(&m_mutex);
    QJsonObject  result;

    // Add nodes
    QJsonArray nodesArray;
    for (const IntegrationNode *node : m_nodeObjects)
    {
        nodesArray.append(node->toDict());
    }
    result["nodes"] = nodesArray;

    // Add links
    QJsonArray linksArray;
    for (const IntegrationLink *link : m_linkObjects)
    {
        linksArray.append(link->toDict());
    }
    result["links"] = linksArray;

    return result;
}

QVector<int> IntegrationNetwork::getPathLinks(
    const QVector<int> &pathNodes) const
{
    QVector<int> pathLinks;

    // Skip empty paths
    if (pathNodes.size() <= 1)
    {
        return pathLinks;
    }

    // Find links connecting consecutive nodes
    for (int i = 0; i < pathNodes.size() - 1; ++i)
    {
        int fromNode = pathNodes[i];
        int toNode   = pathNodes[i + 1];

        // Get link ID from edge attributes
        QMap<QString, QVariant> edgeAttrs =
            m_graph->getEdgeAttributes(fromNode, toNode);

        if (edgeAttrs.contains("link_id"))
        {
            pathLinks.append(edgeAttrs["link_id"].toInt());
        }
    }

    return pathLinks;
}

double IntegrationNetwork::getPathLengthByLinks(
    const QVector<int> &linkIds) const
{
    double totalLength = 0.0;

    // Sum lengths of all links
    for (int linkId : linkIds)
    {
        // Find link
        for (const IntegrationLink *link : m_linkObjects)
        {
            if (link->getLinkId() == linkId)
            {
                totalLength += link->getLength();
                break;
            }
        }
    }

    return totalLength;
}

/********** IntegrationSimulationConfig ***************/

IntegrationSimulationConfig::IntegrationSimulationConfig(
    QObject *parent)
    : QObject(parent)
    , m_simTime(0.0)
{
    // Create network object
    m_network = new IntegrationNetwork(this);
}

IntegrationSimulationConfig::~IntegrationSimulationConfig()
{
    if (m_network)
    {
        m_network->deleteLater();
    }
}

bool IntegrationSimulationConfig::initialize(
    const QString &configDir, const QString &title,
    double                        simTime,
    const QMap<QString, QString> &inputFiles,
    const QMap<QString, QString> &outputFiles)
{
    QMutexLocker locker(&m_mutex);

    // Store basic configuration
    m_configDir   = configDir;
    m_title       = title;
    m_simTime     = simTime;
    m_inputFiles  = inputFiles;
    m_outputFiles = outputFiles;

    // Set default folders if not specified
    m_inputFolder  = ".";
    m_outputFolder = ".";

    try
    {
        // Read node data
        QString nodeFilePath =
            getInputFilePath("node_coordinates");
        QFile nodeFile(nodeFilePath);
        if (!nodeFile.open(QIODevice::ReadOnly
                           | QIODevice::Text))
        {
            return false;
        }

        // TODO: Parse node data
        // This would be implemented with node data parsing
        // logic

        // Read link data
        QString linkFilePath =
            getInputFilePath("link_structure");
        QFile linkFile(linkFilePath);
        if (!linkFile.open(QIODevice::ReadOnly
                           | QIODevice::Text))
        {
            return false;
        }

        // TODO: Parse link data
        // This would be implemented with link data parsing
        // logic

        // Initialize network with parsed data
        // This would use the parsed node and link data

        emit configChanged();
        return true;
    }
    catch (const std::exception &e)
    {
        // Log error and return failure
        return false;
    }
}

IntegrationNetwork *
IntegrationSimulationConfig::getNetwork() const
{
    return m_network;
}

QString IntegrationSimulationConfig::getInputFilePath(
    const QString &key) const
{
    if (!m_inputFiles.contains(key))
    {
        throw std::runtime_error(
            QString("No input file found with key '%1'")
                .arg(key)
                .toStdString());
    }

    return QDir(m_configDir)
        .filePath(QDir(m_inputFolder)
                      .filePath(m_inputFiles[key]));
}

QString IntegrationSimulationConfig::getOutputFilePath(
    const QString &key) const
{
    if (!m_outputFiles.contains(key))
    {
        throw std::runtime_error(
            QString("No output file found with key '%1'")
                .arg(key)
                .toStdString());
    }

    return QDir(m_configDir)
        .filePath(QDir(m_outputFolder)
                      .filePath(m_outputFiles[key]));
}

QJsonObject IntegrationSimulationConfig::toJson() const
{
    QMutexLocker locker(&m_mutex);
    QJsonObject  result;

    // Add basic properties
    result["config_dir"]    = m_configDir;
    result["title"]         = m_title;
    result["sim_time"]      = m_simTime;
    result["input_folder"]  = m_inputFolder;
    result["output_folder"] = m_outputFolder;

    // Add input files
    QJsonObject inputFilesObj;
    for (auto it = m_inputFiles.constBegin();
         it != m_inputFiles.constEnd(); ++it)
    {
        inputFilesObj[it.key()] = it.value();
    }
    result["input_files"] = inputFilesObj;

    // Add output files
    QJsonObject outputFilesObj;
    for (auto it = m_outputFiles.constBegin();
         it != m_outputFiles.constEnd(); ++it)
    {
        outputFilesObj[it.key()] = it.value();
    }
    result["output_files"] = outputFilesObj;

    // Add variables
    QJsonObject varsObj;
    for (auto it = m_variables.constBegin();
         it != m_variables.constEnd(); ++it)
    {
        varsObj[it.key()] = it.value();
    }
    result["variables"] = varsObj;

    // Add network
    result["network"] = m_network->toJson();

    return result;
}

/**************** IntegrationSimulationConfigReader
 * ****************/

IntegrationSimulationConfigReader::
    IntegrationSimulationConfigReader(
        const QString &configFilePath, QObject *parent)
    : QObject(parent)
{
    // Read configuration
    QJsonObject configJson = readConfig(configFilePath);

    // Create config object
    m_config = new IntegrationSimulationConfig(this);

    // Initialize with read data
    QString configDir =
        QFileInfo(configFilePath).dir().absolutePath();
    QString title   = configJson["title"].toString();
    double  simTime = configJson["sim_time"].toDouble();

    // Extract input files
    QMap<QString, QString> inputFiles;
    QJsonObject            inputFilesObj =
        configJson["input_files"].toObject();
    for (auto it = inputFilesObj.constBegin();
         it != inputFilesObj.constEnd(); ++it)
    {
        inputFiles[it.key()] = it.value().toString();
    }

    // Extract output files
    QMap<QString, QString> outputFiles;
    QJsonObject            outputFilesObj =
        configJson["output_files"].toObject();
    for (auto it = outputFilesObj.constBegin();
         it != outputFilesObj.constEnd(); ++it)
    {
        outputFiles[it.key()] = it.value().toString();
    }

    // Initialize config
    m_config->initialize(configDir, title, simTime,
                         inputFiles, outputFiles);
}

IntegrationSimulationConfigReader::
    ~IntegrationSimulationConfigReader()
{

    if (m_config)
    {
        m_config->deleteLater();
    }
}

QJsonObject IntegrationSimulationConfigReader::readConfig(
    const QString &configFilePath)
{
    QJsonObject result;

    try
    {
        // Get config directory
        QString configDir =
            QFileInfo(configFilePath).dir().absolutePath();

        // Open file
        QFile file(configFilePath);
        if (!file.open(QIODevice::ReadOnly
                       | QIODevice::Text))
        {
            throw std::runtime_error(
                QString("Error opening file: %1")
                    .arg(file.errorString())
                    .toStdString());
        }

        // Read lines
        QTextStream in(&file);
        QStringList lines;
        while (!in.atEnd())
        {
            lines.append(in.readLine());
        }
        file.close();

        // Validate file
        if (lines.isEmpty())
        {
            throw std::runtime_error(
                "Configuration file is empty");
        }

        // Parse title (line 1)
        result["title"] = lines[0].trimmed();

        // Parse simulation parameters (line 2)
        QStringList simParams = lines[1].trimmed().split(
            QRegularExpression("\\s+"));
        if (simParams.size() < 5)
        {
            throw std::runtime_error(
                "Invalid simulation parameters");
        }

        result["sim_time"]       = simParams[0].toDouble();
        result["output_freq_10"] = simParams[1].toInt();
        result["output_freq_12_14"] = simParams[2].toInt();
        result["routing_option"]    = simParams[3].toInt();
        result["pause_flag"]        = simParams[4].toInt();

        // Parse folders
        result["input_folder"]  = lines[2].trimmed();
        result["output_folder"] = lines[3].trimmed();

        // Parse input files
        QJsonObject inputFiles;
        inputFiles["node_coordinates"] = lines[4].trimmed();
        inputFiles["link_structure"]   = lines[5].trimmed();
        inputFiles["signal_timing"]    = lines[6].trimmed();
        inputFiles["traffic_demands"]  = lines[7].trimmed();
        inputFiles["incident_descriptions"] =
            lines[8].trimmed();
        result["input_files"] = inputFiles;

        // Parse output files
        QJsonObject outputFiles;
        int         lineIndex  = 9;
        QStringList outputKeys = {
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
            "time_space_output"};

        for (const QString &key : outputKeys)
        {
            if (lineIndex < lines.size())
            {
                outputFiles[key] =
                    lines[lineIndex++].trimmed();
            }
        }
        result["output_files"] = outputFiles;

        return result;
    }
    catch (const std::exception &e)
    {
        // Return empty object on error
        return QJsonObject();
    }
}

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
