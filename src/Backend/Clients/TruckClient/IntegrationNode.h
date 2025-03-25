/**
 * @file IntegrationNode.h
 * @brief Defines node in transportation network
 * @author [Your Name]
 * @date 2025-03-22
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>

#include "Backend/Models/BaseObject.h"

namespace CargoNetSim
{
namespace Backend
{
namespace TruckClient
{

/**
 * @class IntegrationNode
 * @brief Represents a node in the truck network
 *
 * Models a network node with spatial coordinates and other
 * properties such as node type and information
 * availability.
 */
class IntegrationNode : public BaseObject
{
    Q_OBJECT

    // Node properties as Q_PROPERTY for meta-object access
    Q_PROPERTY(int nodeId READ getNodeId WRITE setNodeId
                   NOTIFY nodeChanged)
    Q_PROPERTY(float xCoordinate READ getXCoordinate WRITE
                   setXCoordinate NOTIFY nodeChanged)
    Q_PROPERTY(float yCoordinate READ getYCoordinate WRITE
                   setYCoordinate NOTIFY nodeChanged)
    Q_PROPERTY(int nodeType READ getNodeType WRITE
                   setNodeType NOTIFY nodeChanged)
    Q_PROPERTY(
        int macroZoneCluster READ getMacroZoneCluster WRITE
            setMacroZoneCluster NOTIFY nodeChanged)
    Q_PROPERTY(int informationAvailability READ
                   getInformationAvailability WRITE
                       setInformationAvailability NOTIFY
                           nodeChanged)
    Q_PROPERTY(QString description READ getDescription WRITE
                   setDescription NOTIFY nodeChanged)
    Q_PROPERTY(float xScale READ getXScale WRITE setXScale
                   NOTIFY nodeChanged)
    Q_PROPERTY(float yScale READ getYScale WRITE setYScale
                   NOTIFY nodeChanged)

public:
    /**
     * @brief Default constructor
     * @param parent The parent QObject
     */
    explicit IntegrationNode(QObject *parent = nullptr);

    /**
     * @brief Parameterized constructor
     * @param nodeId Unique identifier for the node
     * @param xCoordinate X-coordinate on the network
     * @param yCoordinate Y-coordinate on the network
     * @param nodeType Type identifier for the node
     * @param macroZoneCluster Cluster identifier for zoning
     * @param informationAvailability Information level at
     * node
     * @param description Text description of the node
     * @param xScale Scaling factor for X-coordinate
     * @param yScale Scaling factor for Y-coordinate
     * @param parent The parent QObject
     */
    IntegrationNode(int nodeId, float xCoordinate,
                    float yCoordinate, int nodeType,
                    int            macroZoneCluster,
                    int            informationAvailability,
                    const QString &description,
                    float xScale, float yScale,
                    QObject *parent = nullptr);

    /**
     * @brief Constructor from JSON data
     * @param json JSON object containing node data
     * @param parent The parent QObject
     */
    IntegrationNode(const QJsonObject &json,
                    QObject           *parent = nullptr);

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
    static IntegrationNode *
    fromDict(const QJsonObject &data,
             QObject           *parent = nullptr);

    // Getters
    int getNodeId() const
    {
        return m_nodeId;
    }
    float getXCoordinate() const
    {
        return m_xCoordinate;
    }
    float getYCoordinate() const
    {
        return m_yCoordinate;
    }
    int getNodeType() const
    {
        return m_nodeType;
    }
    int getMacroZoneCluster() const
    {
        return m_macroZoneCluster;
    }
    int getInformationAvailability() const
    {
        return m_informationAvailability;
    }
    QString getDescription() const
    {
        return m_description;
    }
    float getXScale() const
    {
        return m_xScale;
    }
    float getYScale() const
    {
        return m_yScale;
    }

    // Setters
    void setNodeId(int nodeId);
    void setXCoordinate(float xCoordinate);
    void setYCoordinate(float yCoordinate);
    void setNodeType(int nodeType);
    void setMacroZoneCluster(int macroZoneCluster);
    void
    setInformationAvailability(int informationAvailability);
    void setDescription(const QString &description);
    void setXScale(float xScale);
    void setYScale(float yScale);

signals:
    /**
     * @brief Signal emitted when any node property changes
     */
    void nodeChanged();

private:
    int   m_nodeId;           ///< Unique node identifier
    float m_xCoordinate;      ///< X-coordinate on network
    float m_yCoordinate;      ///< Y-coordinate on network
    int   m_nodeType;         ///< Type identifier
    int   m_macroZoneCluster; ///< Cluster identifier
    int   m_informationAvailability; ///< Information level
    QString m_description;           ///< Node description
    float   m_xScale;                ///< X coordinate scale
    float   m_yScale;                ///< Y coordinate scale
};

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
