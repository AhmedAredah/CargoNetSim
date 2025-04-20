#pragma once
#include "GraphicsObjectBase.h"

#include <QGraphicsItem>
#include <QGraphicsObject>
#include <QMap>
#include <QPen>
#include <QPropertyAnimation>
#include <QVariant>

namespace CargoNetSim
{
namespace GUI
{

class TerminalItem;
class GlobalTerminalItem;
class ConnectionLabel;

class ConnectionLine : public GraphicsObjectBase
{
    Q_OBJECT

public:
    static const QMap<QString, QMap<QString, QVariant>>
        CONNECTION_STYLES;

    ConnectionLine(
        QGraphicsItem *startItem, QGraphicsItem *endItem,
        const QString &connectionType = "Truck",
        const QMap<QString, QVariant> &properties =
            QMap<QString, QVariant>(),
        const QString &region = "Default Region",
        QGraphicsItem *parent = nullptr);

    virtual ~ConnectionLine();

    void
    createAnimationVisual(const QColor &color) override;

    void clearAnimationVisuals() override;

    // Access to members
    QGraphicsItem *startItem() const
    {
        return m_startItem;
    }
    QGraphicsItem *endItem() const
    {
        return m_endItem;
    }
    QString connectionType() const
    {
        return m_connectionType;
    }
    int connectionId() const
    {
        return m_id;
    }
    void updateProperties(
        const QMap<QString, QVariant> &newProperties);
    const QMap<QString, QVariant> &getProperties() const
    {
        return m_properties;
    }

    /**
     * @brief Set the current region name
     * @param region The region name as a QString
     */
    void setRegion(const QString &region);

    /**
     * @brief Get the current region name
     * @return The region name as a QString
     */
    QString getRegion() const
    {
        return m_properties["Region"].toString();
    }
    void setConnectionType(const QString &type);
    void setProperty(const QString  &key,
                     const QVariant &value);

    // Update methods
    void updatePosition(const QPointF &newPos  = QPointF(),
                        bool           isStart = false);

    // Selection handling
    bool isSelected() const;
    void setSelected(bool selected);

    // Serialization methods
    QMap<QString, QVariant> toDict() const;
    static ConnectionLine  *fromDict(
         const QMap<QString, QVariant>    &data,
         const QMap<int, QGraphicsItem *> &terminalsByID,
         QGraphicsScene *globalScene = nullptr,
         QGraphicsItem  *parent      = nullptr);

    // Reset/set class static IDs
    static void resetClassIDs();
    static void
               setClassIDs(const QMap<int, ConnectionLine *>
                               &allConnectionsById);
    static int getNewConnectionID();

signals:
    void clicked(ConnectionLine *line);
    void startPositionChanged(const QPointF &newPos);
    void endPositionChanged(const QPointF &newPos);
    void propertyChanged(const QString  &key,
                         const QVariant &value);
    void propertiesChanged();
    void connectionTypeChanged(const QString &newType);
    void regionChanged(const QString &newRegion);

protected:
    // QGraphicsItem overrides
    QRectF       boundingRect() const override;
    QPainterPath shape() const override;
    void         paint(QPainter                       *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget = nullptr) override;
    void         mousePressEvent(
                QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(
        QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(
        QGraphicsSceneHoverEvent *event) override;

private:
    // Utility methods
    QLineF
    calculateOffsetLine(const QLineF &originalLine) const;
    void onStartItemPositionChanged(const QPointF &newPos);
    void onEndItemPositionChanged(const QPointF &newPos);
    void createConnections();
    void initializeProperties(QString region);

    // Member variables
    QGraphicsItem          *m_startItem;
    QGraphicsItem          *m_endItem;
    QString                 m_connectionType;
    QMap<QString, QVariant> m_properties;
    int                     m_id;
    bool                    m_isHovered;

    // Geometry
    QLineF  m_line;
    QPointF m_ctrlPoint; // Control point for curved lines
    QRectF  m_boundingRect;

    // Visual
    ConnectionLabel *m_label;

    // Static members
    static int CONNECTION_LINE_ID;
};

} // namespace GUI
} // namespace CargoNetSim

// Register the type for QVariant
Q_DECLARE_METATYPE(CargoNetSim::GUI::ConnectionLine)
Q_DECLARE_METATYPE(CargoNetSim::GUI::ConnectionLine *)
