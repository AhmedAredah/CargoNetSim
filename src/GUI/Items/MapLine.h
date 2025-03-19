#pragma once

#include <QGraphicsObject>
#include <QPointF>
#include <QString>
#include <QMap>
#include <QVariant>
#include <QPen>

namespace CargoNetSim {
namespace GUI {

/**
 * @brief Represents a line connecting two points in a network
 * 
 * MapLine is a visual representation of a connection between two points in a network.
 * It belongs to a specific network region and can have various properties.
 */
class MapLine : public QGraphicsObject {
    Q_OBJECT

public:
    /**
     * @brief Constructs a new MapLine object
     * 
     * @param startPoint The starting point of the line
     * @param endPoint The ending point of the line
     * @param region The network region this line belongs to
     * @param properties Optional additional properties
     */
    MapLine(const QPointF& startPoint,
            const QPointF& endPoint,
            const QString& region = "default",
            const QMap<QString, QVariant>& properties = QMap<QString, QVariant>());
    
    virtual ~MapLine() = default;
    
    /**
     * @brief Sets the color of the line
     * 
     * @param color The new color
     */
    void setColor(const QColor& color);
    
    /**
     * @brief Sets the pen used for drawing the line
     * 
     * @param pen The pen to use
     */
    void setPen(const QPen& pen);

    /** 
     * @brief Sets the region of the line
     */
    void setRegion(const QString region) { this->region = region; }
    
    /**
     * @brief Get the start point
     */
    const QPointF& getStartPoint() const { return startPoint; }
    
    /**
     * @brief Get the end point
     */
    const QPointF& getEndPoint() const { return endPoint; }
    
    /**
     * @brief Get the region
     */
    const QString& getRegion() const { return region; }
    
    /**
     * @brief Get the properties
     */
    const QMap<QString, QVariant>& getProperties() const { return properties; }
    
    /**
     * @brief Serializes the line to a dictionary for storage
     */
    QMap<QString, QVariant> toDict() const;
    
    /**
     * @brief Creates a MapLine from a serialized dictionary
     */
    static MapLine* fromDict(const QMap<QString, QVariant>& data);

signals:
    /**
     * @brief Signal emitted when the line is clicked
     */
    void clicked(MapLine* line);
    
    /**
     * @brief Signal emitted when the line's color changes
     */
    void colorChanged(const QColor& newColor);
    
    /**
     * @brief Signal emitted when any property changes
     */
    void propertyChanged(const QString& key, const QVariant& value);

protected:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void selectNetworkLines();
    
    QPointF startPoint;
    QPointF endPoint;
    QString region;
    QMap<QString, QVariant> properties;
    int baseWidth;
    QPen pen;
};

} // namespace GUI
} // namespace CargoNetSim

// Register the type for QVariant
Q_DECLARE_METATYPE(CargoNetSim::GUI::MapLine)
Q_DECLARE_METATYPE(CargoNetSim::GUI::MapLine*)
