#ifndef ANIMATIONOBJECT_H
#define ANIMATIONOBJECT_H

#include <QGraphicsPathItem>
#include <QObject>
#include <QtWidgets/qgraphicsscene.h>

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
        , _wasHidden(false)
        , _restoreVisibility(false)
        , _rect(nullptr)
        , _overlay(nullptr)
    {
    }

    qreal opacity() const
    {
        return _opacity;
    }

    void setOpacity(qreal value)
    {
        _opacity = value;
        if (_rect)
            _rect->setOpacity(value);
        if (_overlay)
            _overlay->setOpacity(value);
        emit opacityChanged();
    }

    void setRect(QGraphicsRectItem *rect)
    {
        _rect = rect;
    }

    QGraphicsRectItem *rect() const
    {
        return _rect;
    }

    void setOverlay(QGraphicsPathItem *overlay)
    {
        _overlay = overlay;
    }

    QGraphicsPathItem *overlay() const
    {
        return _overlay;
    }

    void clearVisuals()
    {
        if (_rect)
        {
            if (_rect->scene())
            {
                _rect->scene()->removeItem(_rect);
            }
            delete _rect;
            _rect = nullptr;
        }

        if (_overlay)
        {
            if (_overlay->scene())
            {
                _overlay->scene()->removeItem(_overlay);
            }
            delete _overlay;
            _overlay = nullptr;
        }
    }

    void setWasHidden(bool hidden)
    {
        _wasHidden = hidden;
    }
    bool wasHidden() const
    {
        return _wasHidden;
    }

    void setRestoreVisibility(bool restore)
    {
        _restoreVisibility = restore;
    }
    bool shouldRestoreVisibility() const
    {
        return _restoreVisibility;
    }

signals:
    void opacityChanged();

private:
    qreal              _opacity;
    bool               _wasHidden;
    bool               _restoreVisibility;
    QGraphicsRectItem *_rect;
    QGraphicsPathItem *_overlay;
};

} // namespace GUI
} // namespace CargoNetSim

#endif // ANIMATIONOBJECT_H
