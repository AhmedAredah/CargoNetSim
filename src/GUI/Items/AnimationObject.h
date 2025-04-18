#ifndef ANIMATIONOBJECT_H
#define ANIMATIONOBJECT_H

#include <QGraphicsPathItem>
#include <QObject>

namespace CargoNetSim
{
namespace GUI
{

class AnimationObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity
                   NOTIFY opacityChanged)

public:
    AnimationObject(QObject *parent = nullptr)
        : QObject(parent)
        , _opacity(1.0)
        , _overlay(nullptr)
        , _rect(nullptr)
    {
    }

    qreal opacity() const
    {
        return _opacity;
    }

    void setOpacity(qreal value)
    {
        _opacity = value;

        if (_overlay)
            _overlay->setOpacity(value);

        if (_rect)
            _rect->setOpacity(value);

        emit opacityChanged();
    }

    void setOverlay(QGraphicsPathItem *overlay)
    {
        _overlay = overlay;
    }

    void setRect(QGraphicsRectItem *rect)
    {
        _rect = rect;
    }

    QGraphicsRectItem *rect() const
    {
        return _rect;
    }

signals:
    void opacityChanged();

private:
    qreal              _opacity;
    QGraphicsPathItem *_overlay = nullptr;
    QGraphicsRectItem *_rect;
};

} // namespace GUI
} // namespace CargoNetSim

#endif // ANIMATIONOBJECT_H
