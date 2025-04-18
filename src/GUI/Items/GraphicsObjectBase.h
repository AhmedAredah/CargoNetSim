#pragma once
#include "GUI/Items/AnimationObject.h"
#include <QBrush>
#include <QGraphicsObject>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QObject>
#include <QPen>
#include <QPropertyAnimation>
#include <QUuid>

namespace CargoNetSim
{
namespace GUI
{
/**
 * @brief Base class for all graphics objects in the
 * CargoNetSim application
 * This class provides a common interface for all
 * graphics objects in the application. It provides a UUID
 * property that can be used to uniquely identify the object
 * in the scene. UUID generation guarantees uniqueness even
 * when objects are created simultaneously across multiple
 * instances.
 */
class GraphicsObjectBase : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ getID CONSTANT)
public:
    /**
     * @brief Construct a new Graphics Object Base
     *
     * Creates a graphics object with a globally unique
     * UUID.
     *
     * @param parent Optional parent graphics item
     */
    explicit GraphicsObjectBase(
        QGraphicsItem *parent = nullptr)
        : QGraphicsObject(parent)
        , m_animObject(nullptr)
        , m_animation(nullptr)
    {
        m_ID = QUuid::createUuid().toString(
            QUuid::WithoutBraces);
    }

    /**
     * @brief Virtual destructor ensuring proper cleanup of
     * derived classes
     */
    virtual ~GraphicsObjectBase()
    {
        // Clean up any existing animation
        if (m_animation)
        {
            m_animation->stop();
            m_animation->deleteLater();
            m_animation = nullptr;
        }

        if (m_animObject)
        {
            m_animObject->deleteLater();
            m_animObject = nullptr;
        }
    }

    /**
     * @brief Get the unique UUID for this object
     * @return QString The unique UUID as string without
     * braces
     */
    QString getID() const
    {
        return m_ID;
    }

    /**
     * @brief Create a visual highlighting effect
     *      * Creates a pulsing highlight effect to draw
     * attention to this object
     *      * @param evenIfHidden Make the object
     * temporarily visible if it's hidden
     * @param color Highlight color (semi-transparent red by
     * default)
     */
    virtual void flash(bool          evenIfHidden = false,
                       const QColor &color = QColor(255, 0,
                                                    0, 180))
    {
        bool wasHidden = !isVisible();
        if (evenIfHidden && wasHidden)
        {
            setVisible(true);
        }

        // Clean up any existing animation
        if (m_animation)
        {
            m_animation->stop();
            m_animation->deleteLater();
            m_animation = nullptr;
        }

        if (m_animObject)
        {
            m_animObject->deleteLater();
            m_animObject = nullptr;
        }

        // Create a rectangle item as an overlay
        QGraphicsRectItem *rect = new QGraphicsRectItem(boundingRect(), this);
        rect->setBrush(QBrush(color));
        rect->setPen(QPen(Qt::NoPen));
        rect->setZValue(100);

        // Create and configure animation object
        m_animObject = new AnimationObject(this);
        static_cast<AnimationObject *>(m_animObject)->setRect(rect);

        // Create and configure animation
        m_animation = new QPropertyAnimation(m_animObject, "opacity", this);
        m_animation->setDuration(1000);
        m_animation->setLoopCount(3);
        m_animation->setStartValue(1.0);
        m_animation->setKeyValueAt(0.5, 0.0);
        m_animation->setEndValue(1.0);

        // Connect finished signal for cleanup
        connect(m_animation, &QPropertyAnimation::finished,
                [=]() {
                    if (rect && scene())
                    {
                        scene()->removeItem(rect);
                        delete rect;
                    }

                    if (evenIfHidden && wasHidden)
                    {
                        setVisible(false);
                    }

                    // Clean up animation resources
                    m_animation->deleteLater();
                    m_animation = nullptr;
                    m_animObject->deleteLater();
                    m_animObject = nullptr;
                });

        // Start animation
        m_animation->start();
    }

signals:
    void IDChanged(const QString &newID);

protected:
    QString m_ID; ///< Unique UUID for this object
    QObject *m_animObject; ///< Animation object for flash effect
    QPropertyAnimation *m_animation; ///< Property animation for effects

private:
    /**
     * @brief Set the unique UUID for this object
     * @param id The new UUID
     */
    void setID(const QString &id)
    {
        if (m_ID != id)
        {
            m_ID = id;
            emit IDChanged(id);
        }
    }
};

} // namespace GUI
} // namespace CargoNetSim
