#include "TransportationMode.h"

namespace CargoNetSim
{
namespace Backend
{

ContainerCore::Container::HaulerType
TransportationTypes::toContainerHauler(
    TransportationMode mode)
{
    switch (mode)
    {
    case TransportationMode::Ship:
        return ContainerCore::Container::HaulerType::
            waterTransport;
    case TransportationMode::Truck:
        return ContainerCore::Container::HaulerType::truck;
    case TransportationMode::Train:
        return ContainerCore::Container::HaulerType::train;
    default:
        throw std::invalid_argument(
            "Invalid transportation mode");
    }
}

TransportationTypes::TransportationMode
TransportationTypes::fromContainerHauler(
    ContainerCore::Container::HaulerType hauler)
{
    switch (hauler)
    {
    case ContainerCore::Container::HaulerType::
        waterTransport:
        return TransportationMode::Ship;
    case ContainerCore::Container::HaulerType::truck:
        return TransportationMode::Truck;
    case ContainerCore::Container::HaulerType::train:
        return TransportationMode::Train;
    default:
        throw std::invalid_argument(
            "Invalid container hauler");
    }
}

TransportationTypes::TransportationMode
TransportationTypes::fromInt(int value)
{
    switch (value)
    {
    case static_cast<int>(TransportationMode::Ship):
        return TransportationMode::Ship;
    case static_cast<int>(TransportationMode::Truck):
        return TransportationMode::Truck;
    case static_cast<int>(TransportationMode::Train):
        return TransportationMode::Train;
    default:
        throw std::invalid_argument(
            "Invalid transportation mode value");
    }
}

QString
TransportationTypes::toString(TransportationMode mode)
{
    const QMetaEnum metaEnum =
        QMetaEnum::fromType<TransportationMode>();
    return QString(
        metaEnum.valueToKey(static_cast<int>(mode)));
}

int TransportationTypes::toInt(TransportationMode mode)
{
    return static_cast<int>(mode);
}

TransportationTypes::TransportationMode
TransportationTypes::fromString(const QString &str)
{
    // Convert to lowercase for case-insensitive comparison
    QString lowerStr = str.toLower().trimmed();

    if (lowerStr == "ship")
    {
        return TransportationMode::Ship;
    }
    else if (lowerStr == "truck")
    {
        return TransportationMode::Truck;
    }
    else if (lowerStr == "train" || lowerStr == "rail")
    {
        return TransportationMode::Train;
    }
    else
    {
        throw std::invalid_argument(
            "Invalid transportation mode string");
    }
}

} // namespace Backend
} // namespace CargoNetSim
