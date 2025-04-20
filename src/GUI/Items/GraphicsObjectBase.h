#pragma once

#include "GUI/Items/AnimationObject.h"
#include <QColor>
#include <QGraphicsObject>
#include <QGraphicsRectItem>
#include <QPropertyAnimation>

namespace CargoNetSim::GUI
{

class GraphicsObjectBase : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ getID CONSTANT)
public:
    explicit GraphicsObjectBase(
        QGraphicsItem *parent = nullptr);
    ~GraphicsObjectBase() override;

    QString getID() const;

    /** Pulsing highlight effect */
    void flash(bool          evenIfHidden = false,
               const QColor &color = QColor(255, 0, 0,
                                            180));

signals:
    void idChanged(const QString &newId);

protected:
    /** Clear any overlay visuals */
    virtual void clearAnimationVisuals();
    /** Create the overlay */
    virtual void createAnimationVisual(const QColor &color);
    void         onAnimationFinished();
    AnimationObject    *m_animObject = nullptr;
    QPropertyAnimation *m_animation  = nullptr;

private:
    QString m_id;
};

} // namespace CargoNetSim::GUI
