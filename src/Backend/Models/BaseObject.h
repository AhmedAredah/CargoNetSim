/**
 * @file baseobject.h
 * @brief Defines the BaseObject class which provides unique
 * object identification.
 * @author Ahmed Aredah
 * @date 2025-03-24
 */

#ifndef BASEOBJECT_H
#define BASEOBJECT_H

#include <QObject>
#include <QString>
#include <QUuid>

/**
 * @class BaseObject
 * @brief Base class for objects requiring unique
 * identification.
 *
 * The BaseObject class extends QObject to provide automatic
 * unique identification for derived classes. Each instance
 * receives a UUID at construction time that is guaranteed
 * to be globally unique across all systems and time.
 */
class BaseObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(
        QString uniqueId READ getInternalUniqueID CONSTANT)

public:
    /**
     * @brief Constructs a BaseObject with a unique
     * identifier.
     * @param parent Optional parent object in the QObject
     * hierarchy.
     *
     * The constructor automatically assigns a globally
     * unique identifier to each instance using the QUuid
     * mechanism.
     */
    explicit BaseObject(QObject *parent = nullptr);

    /**
     * @brief Virtual destructor ensuring proper cleanup of
     * derived classes.
     */
    virtual ~BaseObject();

    /**
     * @brief Retrieves the unique identifier for this
     * object.
     * @return QString containing the unique identifier
     * without braces.
     */
    QString getInternalUniqueID() const;

private:
    /**
     * @brief The unique identifier for this object
     * instance.
     *
     * This identifier is generated at construction time
     * using QUuid::createUuid() and is guaranteed to be
     * unique across all systems.
     */
    QString m_uniqueId;
};

#endif // BASEOBJECT_H
