#pragma once

#include <QGraphicsObject>
#include <QColor>
#include <QMap>
#include <QVariant>
#include <QPointF>

namespace CargoNetSim {
namespace GUI {

/**
 * @class RegionCenterPoint
 * @brief A visual representation of a region's center point in the scene.
 * 
 * RegionCenterPoint provides a visual indicator of a region's center and holds the
 * region's metadata, including geographic coordinates and shared coordinates that
 * are used for global map positioning.
 */
class RegionCenterPoint : public QGraphicsObject {
    Q_OBJECT

public:
    /**
     * @brief Constructs a RegionCenterPoint.
     * @param regionName The name of the region
     * @param color The color for this region
     * @param properties Optional properties map
     * @param parent Optional parent QGraphicsItem
     */
    RegionCenterPoint(
        const QColor& color,
        const QMap<QString, QVariant>& properties = QMap<QString, QVariant>(),
        QGraphicsItem* parent = nullptr
    );
    
    /**
     * @brief Destructor
     */
    virtual ~RegionCenterPoint() = default;

    /**
     * @brief Updates the region's center coordinates.
     * @param lat New latitude
     * @param lon New longitude
     */
    void updateCoordinates(double lat, double lon);

    /**
     * @brief Updates the region's shared coordinates for global mapping.
     * @param lat New shared latitude
     * @param lon New shared longitude
     */
    void updateSharedCoordinates(double lat, double lon);

    /**
     * @brief Sets the region's color.
     * @param color New color for the region
     */
    void setColor(const QColor& color);

    /**
     * @brief set new properties
     * @param newProperties is the QMap of the new properties
     */
    void updateProperties(const QMap<QString, QVariant>& newProperties);

    /**
     * @brief get properties
     * @return QMap<QString, QVariant> of the item properties
     */
    QMap<QString, QVariant> getProperties() const { return properties; }


    /**
     * @brief Converts the object to a serializable dictionary.
     * @return Dictionary containing all serializable data
     */
    QMap<QString, QVariant> toDict() const;

    /**
     * @brief Creates a RegionCenterPoint from dictionary data.
     * @param data Dictionary containing region center point data
     * @return New RegionCenterPoint instance
     */
    static RegionCenterPoint* fromDict(const QMap<QString, QVariant>& data);

signals:
    /**
     * @brief Signal emitted when the item is clicked.
     * @param item Pointer to this RegionCenterPoint
     */
    void clicked(RegionCenterPoint* item);

    /**
     * @brief Signal emitted when the position changes.
     * @param newPos New position in scene coordinates
     */
    void positionChanged(const QPointF& newPos);

    /**
     * @brief Signal emitted when the coordinates change.
     * @param lat New latitude
     * @param lon New longitude
     */
    void coordinatesChanged(double lat, double lon);

    /**
     * @brief Signal emitted when the shared coordinates change.
     * @param lat New shared latitude
     * @param lon New shared longitude
     */
    void sharedCoordinatesChanged(double lat, double lon);

    /**
     * @brief Signal emitted when the region color changes.
     * @param newColor The new color
     */
    void colorChanged(const QColor& newColor);

    /**
     * @brief Signal emitted when any property changes.
     * @param key Property key
     * @param value New property value
     */
    void propertyChanged(const QString& key, const QVariant& value);

    /**
     * @brief Signal emitted when all properties change.
     */
    void propertiesChanged();

protected:
    // QGraphicsItem overrides
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    /**
     * @brief Updates coordinates from position.
     * Called internally when position changes.
     */
    void updateCoordinatesFromPosition();

    QColor color;
    QMap<QString, QVariant> properties;
    QPointF dragOffset;
};

} // namespace GUI
} // namespace CargoNetSim

// Register the type for QVariant
Q_DECLARE_METATYPE(CargoNetSim::GUI::RegionCenterPoint)
Q_DECLARE_METATYPE(CargoNetSim::GUI::RegionCenterPoint*)
