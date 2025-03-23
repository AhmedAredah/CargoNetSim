#pragma once

#include <QGraphicsObject>
#include <QMap>
#include <QPixmap>
#include <QPointF>
#include <QPropertyAnimation>
#include <QString>
#include <QVariant>

namespace CargoNetSim
{
namespace GUI
{

/**
 * @brief Graphical representation of a terminal in the
 * freight network
 *
 * This class represents terminals such as ports, train
 * depots, and truck stops within the CargoNetSim
 * application. Terminals can be dragged, selected, and
 * connected to form a transportation network.
 */
class TerminalItem : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)

public:
    /**
     * @brief Construct a new Terminal Item
     *
     * @param pixmap Visual representation of the terminal
     * @param properties Terminal properties as key-value
     * pairs
     * @param region Region the terminal belongs to
     * @param parent Parent graphics item (if any)
     * @param terminalType Type of terminal (e.g., "Origin",
     * "Sea Port Terminal")
     */
    TerminalItem(const QPixmap                 &pixmap,
                 const QMap<QString, QVariant> &properties =
                     QMap<QString, QVariant>(),
                 const QString &region = "Default Region",
                 QGraphicsItem *parent = nullptr,
                 const QString &terminalType = QString());

    /**
     * @brief Destructor
     */
    virtual ~TerminalItem();

    /**
     * @brief Set the terminal's region
     *
     * @param region New region name
     */
    void setRegion(const QString &region);

    /**
     * @brief Get the terminal's region
     *
     * @return Current region name
     */
    QString getRegion() const
    {
        return region;
    }

    /**
     * @brief Get the terminal's pixmap
     *
     * @return Current pixmap
     */
    QPixmap getPixmap() const
    {
        return pixmap;
    }

    /**
     * @brief Get the terminal type
     *
     * @return Terminal type string
     */
    QString getTerminalType() const
    {
        return terminalType;
    }

    /**
     * @brief Get terminal properties
     *
     * @return Reference to properties map
     */
    const QMap<QString, QVariant> &getProperties() const
    {
        return properties;
    }

    /**
     * @brief Update terminal properties
     *
     * @param newProperties Map of new properties to set
     */
    void updateProperties(
        const QMap<QString, QVariant> &newProperties);

    /**
     * @brief Update a specific property
     *
     * @param key Property key
     * @param value New property value
     */
    void setProperty(const QString  &key,
                     const QVariant &value);

    /**
     * @brief Get a specific property
     *
     * @param key Property key
     * @param defaultValue Value to return if property
     * doesn't exist
     * @return Property value or defaultValue
     */
    QVariant getProperty(
        const QString  &key,
        const QVariant &defaultValue = QVariant()) const;

    /**
     * @brief Reset class ID counter to 0
     */
    static void resetClassIDs();

    /**
     * @brief Set class ID counters based on existing
     * terminals
     *
     * @param allTerminalsById Map of terminal IDs to
     * terminal pointers
     */
    static void setClassIDs(
        const QMap<int, TerminalItem *> &allTerminalsById);

    /**
     * @brief Get a new terminal ID
     *
     * @param terminalType Optional terminal type for
     * type-specific IDs
     * @return New unique ID
     */
    static int getNewTerminalID(
        const QString &terminalType = QString());

    /**
     * @brief Serialize terminal to a dictionary
     *
     * @return Map containing all terminal data
     */
    QMap<QString, QVariant> toDict() const;

    /**
     * @brief Create terminal from serialized data
     *
     * @param data Serialized terminal data
     * @param pixmap Terminal pixmap
     * @param parent Optional parent item
     * @return Pointer to new TerminalItem
     */
    static TerminalItem *
    fromDict(const QMap<QString, QVariant> &data,
             const QPixmap                 &pixmap,
             QGraphicsItem *parent = nullptr);

    /**
     * @brief Create a visual highlighting effect
     *
     * Creates a pulsing highlight effect to draw attention
     * to this terminal
     *
     * @param evenIfHidden Make the terminal temporarily
     * visible if it's hidden
     * @param color Highlight color (semi-transparent red by
     * default)
     */
    void flash(bool          evenIfHidden = false,
               const QColor &color = QColor(255, 0, 0,
                                            180));

signals:
    /**
     * @brief Emitted when terminal is clicked
     *
     * @param item Pointer to this terminal
     */
    void clicked(TerminalItem *item);

    /**
     * @brief Emitted when terminal position changes
     *
     * @param newPos New position in scene coordinates
     */
    void positionChanged(const QPointF &newPos);

    /**
     * @brief Emitted when terminal region changes
     *
     * @param newRegion New region name
     */
    void regionChanged(const QString &newRegion);

    /**
     * @brief Emitted when a terminal property changes
     *
     * @param key Property that changed
     * @param value New value
     */
    void propertyChanged(const QString  &key,
                         const QVariant &value);

    /**
     * @brief Emitted when all properties change
     */
    void propertiesChanged();

    /**
     * @brief Emitted when terminal selection changes
     *
     * @param selected New selection state
     */
    void selectionChanged(bool selected);

protected:
    /**
     * @brief Define the bounding rectangle for painting and
     * interaction
     *
     * @return Rectangle in local coordinates
     */
    QRectF boundingRect() const override;

    /**
     * @brief Paint the terminal on the canvas
     *
     * @param painter Painter to use
     * @param option Style options
     * @param widget Widget being painted on
     */
    void paint(QPainter                       *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    /**
     * @brief Handle mouse press events
     *
     * @param event Mouse event details
     */
    void mousePressEvent(
        QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief Handle item changes (position, selection,
     * etc.)
     *
     * @param change Type of change
     * @param value New value
     * @return Modified value or original value
     */
    QVariant itemChange(GraphicsItemChange change,
                        const QVariant    &value) override;

    /**
     * @brief Handle mouse hover enter events
     *
     * @param event Hover event details
     */
    void hoverEnterEvent(
        QGraphicsSceneHoverEvent *event) override;

    /**
     * @brief Handle mouse hover leave events
     *
     * @param event Hover event details
     */
    void hoverLeaveEvent(
        QGraphicsSceneHoverEvent *event) override;

private:
    QPixmap pixmap; ///< Visual representation
    QString region; ///< Region this terminal belongs to
    QString terminalType; ///< Type of terminal
    QMap<QString, QVariant>
           properties;        ///< Terminal properties
    QRectF boundingRectValue; ///< Cached bounding rectangle
    QPointF dragOffset;       ///< Offset for dragging
    bool    wasSelected;      ///< Previous selection state

    // Animation-related members
    QObject *animObject; ///< Animation object
    QPropertyAnimation
        *animation; ///< Property animation for effects

    // Static ID management
    static int TERMINAL_ID; ///< Next general terminal ID
    static QMap<QString, int>
        TERMINAL_TYPES_IDs; ///< Next ID by terminal type

    /**
     * @brief Initialize default properties based on
     * terminal type
     */
    void initializeDefaultProperties();
};

} // namespace GUI
} // namespace CargoNetSim

// Register the type for QVariant
Q_DECLARE_METATYPE(CargoNetSim::GUI::TerminalItem)
Q_DECLARE_METATYPE(CargoNetSim::GUI::TerminalItem *)
