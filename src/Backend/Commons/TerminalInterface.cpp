#include "TerminalInterface.h"

namespace CargoNetSim {
namespace Backend {
QString
TerminalTypes::toString(TerminalInterface interface) {
    switch (interface) {
    case TerminalInterface::LAND_SIDE:
        return "land_side";
    case TerminalInterface::SEA_SIDE:
        return "sea_side";
    case TerminalInterface::AIR_SIDE:
        return "air_side";
    default:
        return "unknown";
    }
}
} // namespace Backend
} // namespace CargoNetSim
