#pragma once

#include <QColor>
#include <QGraphicsObject>
#include <QMap>
#include <QRectF>
#include <QString>
#include <QVariant>

namespace CargoNetSim
{
namespace GUI
{

class ConnectionLabel : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit ConnectionLabel(
        QGraphicsItem *parent = nullptr);
    virtual ~ConnectionLabel() = default;

    // Accessors
    QString text() const
    {
        return m_text;
    }
    QColor color() const
    {
        return m_color;
    }
    bool isSelected() const
    {
        return m_isSelected;
    }

    // Mutators
    void setText(const QString &text);
    void setColor(const QColor &color);
    void setSelected(bool selected);

    // Serialization
    QMap<QString, QVariant> toDict() const;
    static ConnectionLabel *
    fromDict(const QMap<QString, QVariant> &data,
             QGraphicsItem *parent = nullptr);

signals:
    void clicked();
    void textChanged(const QString &newText);
    void colorChanged(const QColor &newColor);
    void selectionChanged(bool isSelected);

protected:
    // QGraphicsItem overrides
    QRectF boundingRect() const override;
    void   paint(QPainter                       *painter,
                 const QStyleOptionGraphicsItem *option,
                 QWidget *widget = nullptr) override;
    void   mousePressEvent(
          QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(
        QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(
        QGraphicsSceneHoverEvent *event) override;

private:
    QString m_text;
    QColor  m_color;
    bool    m_isHovered;
    bool    m_isSelected;
    QRectF  m_boundingRect;
};

} // namespace GUI
} // namespace CargoNetSim

// Register the type for QVariant
Q_DECLARE_METATYPE(CargoNetSim::GUI::ConnectionLabel)
Q_DECLARE_METATYPE(CargoNetSim::GUI::ConnectionLabel *)
