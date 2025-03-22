#include "ColorUtils.h"
#include <QRandomGenerator>

namespace CargoNetSim {
namespace GUI {
namespace ColorUtils {

QColor getRandomColor() {
    // Generate random integers between 50 and 205
    // (inclusive)
    int r = QRandomGenerator::global()->bounded(50, 206);
    int g = QRandomGenerator::global()->bounded(50, 206);
    int b = QRandomGenerator::global()->bounded(50, 206);
    return QColor(r, g, b);
}

} // namespace ColorUtils
} // namespace GUI
} // namespace CargoNetSim
