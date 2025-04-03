#ifndef BASENETWORK_H
#define BASENETWORK_H

#include "Backend/Models/BaseObject.h"
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>

class BaseNetwork : public BaseObject
{
    Q_OBJECT
public:
    explicit BaseNetwork(QObject *parent = nullptr);
    virtual ~BaseNetwork();

    virtual void setVariable(const QString  &key,
                             const QVariant &value) = 0;
    virtual QVariant
    getVariable(const QString &key) const = 0;

    virtual QMap<QString, QVariant>
    getVariables() const = 0;

    template <typename T>
    T getVariableAs(const QString &key) const
    {
        return getVariable(key).value<T>();
    }

protected:
    QVariantMap m_variables;

signals:
    // Add your signals here if needed
};

#endif // BASENETWORK_H
