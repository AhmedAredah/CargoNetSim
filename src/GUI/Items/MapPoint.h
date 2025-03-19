#pragma once

#include <QGraphicsObject>
#include <QColor>
#include <QString>
#include <QPointF>
#include <QMap>
#include <QVariant>

namespace CargoNetSim {
namespace GUI {

class TerminalItem;

/**
 * @brief Represents a network point/node in the scene
 * 
 * MapPoint is a visual representation of a network node that can be linked to a terminal.
 * It can have different shapes and belongs to a specific network region.
 */
class MapPoint : public QGraphicsObject {
    Q_OBJECT

public:
    /**
     * @brief Constructs a new MapPoint object
     * 
     * @param referencedNetworkID The network ID this point references
     * @param x The x coordinate
     * @param y The y coordinate
     * @param shape The shape to draw ("circle", "rectangle", "triangle")
     * @param region The network region this point belongs to
     * @param terminal Optional terminal linked to this point
     * @param properties Optional additional properties
     */
    MapPoint(const QString& referencedNetworkID,
             qreal x,
             qreal y,
             const QString& shape = "circle",
             const QString& region = "default",
             TerminalItem* terminal = nullptr,
             const QMap<QString, QVariant>& properties = QMap<QString, QVariant>());
    
    virtual ~MapPoint() = default;

    /**
     * @brief Sets the terminal linked to this point
     * 
     * @param terminal Terminal to link to, or nullptr to unlink
     */
    void setLinkedTerminal(TerminalItem* terminal);

    /**
     * @brief Sets the color of the point
     * 
     * @param color The new color
     */
    void setColor(const QColor& color);

    /**
     * @brief Sets the region of the point
     */
    void setRegion(const QString& region) { this->region = region; }

    /**
     * @brief Updates the properties of the point
     * @param newProperties A map of properties to update
     */
    void updateProperties(const QMap<QString, QVariant>& newProperties);

    /**
     * @brief Get the current terminal linked to this point
     * 
     * @return TerminalItem* The linked terminal or nullptr
     */
    TerminalItem* getLinkedTerminal() const { return terminal; }

    /**
     * @brief Get the x coordinate
     */
    qreal getX() const { return x; }

    /**
     * @brief Get the y coordinate
     */
    qreal getY() const { return y; }

    /**
     * @brief Get the network region
     */
    QString getRegion() const { return region; }

    /**
     * @brief Get the properties map
     */
    const QMap<QString, QVariant>& getProperties() const { return properties; }

    /**
     * @brief Serializes the point to a dictionary for storage
     */
    QMap<QString, QVariant> toDict() const;

    /**
     * @brief Creates a MapPoint from a serialized dictionary
     */
    static MapPoint* fromDict(const QMap<QString, QVariant>& data,
                             const QMap<int, TerminalItem*>& terminalsById = QMap<int, TerminalItem*>());

signals:
    /**
     * @brief Signal emitted when the point is clicked
     */
    void clicked(MapPoint* point);

    /**
     * @brief Signal emitted when the linked terminal changes
     */
    void terminalChanged(TerminalItem* oldTerminal, TerminalItem* newTerminal);

    /**
     * @brief Signal emitted when the color changes
     */
    void colorChanged(const QColor& newColor);

    /**
     * @brief Signal emitted when the properties of the point change
     */
    void propertiesChanged();

    /**
     * @brief Signal emitted when any property changes
     */
    void propertyChanged(const QString& key, const QVariant& value);

protected:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
    void showContextMenu(QGraphicsSceneContextMenuEvent* event);
    void createTerminalAtPosition(const QString& terminalType);

    static int POINT_ID;
    
    int id;
    qreal x;
    qreal y;
    QString shape;
    QString region;
    TerminalItem* terminal;
    QColor color;
    QMap<QString, QVariant> properties;
};

} // namespace GUI
} // namespace CargoNetSim

// Register the type for QVariant
Q_DECLARE_METATYPE(CargoNetSim::GUI::MapPoint)
Q_DECLARE_METATYPE(CargoNetSim::GUI::MapPoint*)
