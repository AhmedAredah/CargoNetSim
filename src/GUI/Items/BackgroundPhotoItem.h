#pragma once

#include <QGraphicsObject>
#include <QMap>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QVariant>

namespace CargoNetSim {
namespace GUI {

/**
 * @brief A custom QGraphicsObject for displaying background
 * photos in regions
 *
 * This class handles background images that can be
 * positioned and scaled on the scene. It maintains its own
 * properties and can be serialized/deserialized for project
 * saving.
 */
class BackgroundPhotoItem : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    /**
     * @brief Constructor for BackgroundPhotoItem
     * @param pixmap The image to display
     * @param regionName The region this background belongs
     * to
     * @param parent Optional parent item
     */
    BackgroundPhotoItem(const QPixmap &pixmap,
                        const QString &regionName,
                        QGraphicsItem *parent = nullptr);

    /**
     * @brief Destructor
     */
    virtual ~BackgroundPhotoItem() = default;

    /**
     * @brief Lock or unlock the background from being moved
     * @param locked True to prevent movement, false to
     * allow movement
     */
    void setLocked(bool locked);

    /**
     * @brief Check if the background is locked
     * @return True if locked, false otherwise
     */
    bool isLocked() const;

    /**
     * @brief Set the position using WGS84 (geographic)
     * coordinates
     * @param lat Latitude in degrees
     * @param lon Longitude in degrees
     */
    void setFromWGS84(double lat, double lon);

    /**
     * @brief Update the scale based on the "Scale" property
     */
    void updateScale();

    /**
     * @brief Set the current region name
     * @param region The region name as a QString
     */
    void setRegion(const QString &region);

    /**
     * @brief Update the properties of the background item
     * @param newProperties A map of property key-value
     * pairs to update
     */
    void updateProperties(
        const QMap<QString, QVariant> &newProperties);

    /**
     * @brief Update a property and emit propertyChanged
     * signal
     * @param key The property key
     * @param value The new value
     */
    void updateProperty(const QString  &key,
                        const QVariant &value);

    /**
     * @brief get Properties
     * @return QMap<QString, QVariant>
     */
    QMap<QString, QVariant> getProperties() const {
        return properties;
    }

    /**
     * @brief Get the current scale factor
     * @return The scale factor as a float
     */
    float getScale() const;

    /**
     * @brief Set the scale factor
     * @param scale The new scale factor
     */
    void setScale(float scale);

    /**
     * @brief Get the opacity of the background
     * @return The opacity value (0.0 to 1.0)
     */
    qreal opacity() const;

    /**
     * @brief Set the opacity of the background
     * @param opacity The new opacity value (0.0 to 1.0)
     */
    void setOpacity(qreal opacity);

    // QGraphicsItem required overrides
    QRectF boundingRect() const override;
    void   paint(QPainter                       *painter,
                 const QStyleOptionGraphicsItem *option,
                 QWidget *widget = nullptr) override;

    // Serialization
    QMap<QString, QVariant> toDict() const;
    static BackgroundPhotoItem *
    fromDict(const QMap<QString, QVariant> &data,
             QGraphicsItem *parent = nullptr);

signals:
    /**
     * @brief Signal emitted when the item is clicked
     * @param item A pointer to this item
     */
    void clicked(BackgroundPhotoItem *item);

    /**
     * @brief Signal emitted when the item's position
     * changes
     * @param newPos The new position in scene coordinates
     */
    void positionChanged(const QPointF &newPos);

    /**
     * @brief Signal emitted when the scale changes
     * @param scale The new scale value
     */
    void scaleChanged(float scale);

    /**
     * @brief Signal emitted when the opacity changes
     * @param opacity The new opacity value
     */
    void opacityChanged(qreal opacity);

    /**
     * @brief Signal emitted when the region name changes
     * @param region The new region name
     */
    void regionChanged(QString region);

    /**
     * @brief Signal emitted when the locked state changes
     * @param locked True if the item is now locked, false
     * otherwise
     */
    void lockStateChanged(bool locked);

    /**
     * @brief Signal emitted when properties change
     */
    void propertiesChanged();

    /**
     * @brief Signal emitted when any property changes
     * @param key The property key
     * @param value The new property value
     */
    void propertyChanged(const QString  &key,
                         const QVariant &value);

protected:
    void mousePressEvent(
        QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change,
                        const QVariant    &value) override;

private:
    /**
     * @brief Update coordinate properties when position
     * changes
     */
    void updateCoordinates();

    QPixmap pixmap;     ///< The image to display
    QString regionName; ///< The region this background
                        ///< belongs to
    QMap<QString, QVariant> properties; ///< Properties map
    bool locked;        ///< Whether the item is locked from
                        ///< movement
    QPointF dragOffset; ///< Offset for dragging
    qreal   currentOpacity; ///< Current opacity value
};

} // namespace GUI
} // namespace CargoNetSim

// Register the type for QVariant
Q_DECLARE_METATYPE(CargoNetSim::GUI::BackgroundPhotoItem)
Q_DECLARE_METATYPE(CargoNetSim::GUI::BackgroundPhotoItem *)
