#pragma once

#include <QColor>
#include <QString>
#include <QWidget>

namespace CargoNetSim {
namespace GUI {

/**
 * @brief The ShapeIcon class draws different geometric
 * shapes that can be used as visual indicators in the UI.
 *
 * Supports various shapes like circle, rectangle, triangle,
 * diamond.
 */
class ShapeIcon : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QString shapeType READ shapeType WRITE
                   setShapeType NOTIFY shapeTypeChanged)
    Q_PROPERTY(QColor fillColor READ fillColor WRITE
                   setFillColor NOTIFY fillColorChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE
                   setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(int borderWidth READ borderWidth WRITE
                   setBorderWidth NOTIFY borderWidthChanged)

public:
    /**
     * @brief Constructs a ShapeIcon with the specified
     * shape type
     * @param shapeType The type of shape to draw (circle,
     * rectangle, triangle, diamond)
     * @param parent The parent widget
     */
    explicit ShapeIcon(const QString &shapeType = "circle",
                       QWidget       *parent    = nullptr);

    /**
     * @brief Destructor
     */
    virtual ~ShapeIcon() = default;

    /**
     * @brief Get the current shape type
     * @return The shape type string
     */
    QString shapeType() const;

    /**
     * @brief Set the shape type
     * @param type The shape type string (circle, rectangle,
     * triangle, diamond)
     */
    void setShapeType(const QString &type);

    /**
     * @brief Get the fill color of the shape
     * @return The fill color
     */
    QColor fillColor() const;

    /**
     * @brief Set the fill color of the shape
     * @param color The fill color
     */
    void setFillColor(const QColor &color);

    /**
     * @brief Get the border color of the shape
     * @return The border color
     */
    QColor borderColor() const;

    /**
     * @brief Set the border color of the shape
     * @param color The border color
     */
    void setBorderColor(const QColor &color);

    /**
     * @brief Get the border width
     * @return The border width in pixels
     */
    int borderWidth() const;

    /**
     * @brief Set the border width
     * @param width The border width in pixels
     */
    void setBorderWidth(int width);

    /**
     * @brief Returns the recommended size for the widget
     * @return The recommended size
     */
    QSize sizeHint() const override;

    /**
     * @brief Returns the minimum size for the widget
     * @return The minimum size
     */
    QSize minimumSizeHint() const override;

signals:
    /**
     * @brief Signal emitted when the shape type changes
     * @param type The new shape type
     */
    void shapeTypeChanged(const QString &type);

    /**
     * @brief Signal emitted when the fill color changes
     * @param color The new fill color
     */
    void fillColorChanged(const QColor &color);

    /**
     * @brief Signal emitted when the border color changes
     * @param color The new border color
     */
    void borderColorChanged(const QColor &color);

    /**
     * @brief Signal emitted when the border width changes
     * @param width The new border width
     */
    void borderWidthChanged(int width);

protected:
    /**
     * @brief Paints the shape
     * @param event The paint event
     */
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_shapeType;  ///< The type of shape to draw
    QColor  m_fillColor;  ///< The fill color of the shape
    QColor m_borderColor; ///< The border color of the shape
    int    m_borderWidth; ///< The border width in pixels
};

} // namespace GUI
} // namespace CargoNetSim

// Register the type for QVariant
Q_DECLARE_METATYPE(CargoNetSim::GUI::ShapeIcon)
Q_DECLARE_METATYPE(CargoNetSim::GUI::ShapeIcon *)
