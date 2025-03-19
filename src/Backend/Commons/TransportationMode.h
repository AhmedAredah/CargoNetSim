#pragma once

#include <QObject>
#include <QString>
#include <QMetaEnum>
#include <containerLib/container.h>

namespace CargoNetSim {
namespace Backend {
class TransportationTypes : public QObject
{
    Q_OBJECT
public:
    enum class TransportationMode {
        Ship = 0,
        Truck = 1,
        Train = 2
    };
    Q_ENUM(TransportationMode)

    static ContainerCore::Container::HaulerType toContainerHauler(TransportationMode mode);
    static TransportationMode fromContainerHauler(ContainerCore::Container::HaulerType hauler);
    static TransportationMode fromInt(int value);
    static QString toString(TransportationMode mode);
};
} // namespace Backend
} // namespace CargoNetSim
