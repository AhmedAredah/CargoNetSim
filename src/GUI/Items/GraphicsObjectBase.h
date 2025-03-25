#pragma once
#include <QGraphicsObject>
#include <QObject>
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
    {
        m_ID = QUuid::createUuid().toString(
            QUuid::WithoutBraces);
    }

    /**
     * @brief Virtual destructor ensuring proper cleanup of
     * derived classes
     */
    virtual ~GraphicsObjectBase() = default;

    /**
     * @brief Get the unique UUID for this object
     * @return QString The unique UUID as string without
     * braces
     */
    QString getID() const
    {
        return m_ID;
    }

signals:
    void IDChanged(const QString &newID);

protected:
    QString m_ID; ///< Unique UUID for this object

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
