#pragma once

#include <QObject>
#include <QString>

namespace CargoNetSim {
namespace Backend {

class TerminalTypes : public QObject
{
    Q_OBJECT
public:
    enum class TerminalInterface {
        LAND_SIDE,
        SEA_SIDE,
        AIR_SIDE
    };
    Q_ENUM(TerminalInterface)

    static QString toString(TerminalInterface interface);
};

} // namespace Backend
} // namespace CargoNetSim
