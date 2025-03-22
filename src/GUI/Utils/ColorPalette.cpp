#include "ColorPalette.h"

namespace CargoNetSim {
namespace GUI {
// Initialize the COLORS map with the predefined colors.
const QMap<QString, QColor> ColorPalette::COLORS = {
    {"Steel Blue", QColor(70, 130, 180)},
    {"Forest Green", QColor(34, 139, 34)},
    {"Burgundy", QColor(128, 0, 32)},
    {"Navy Blue", QColor(0, 0, 128)},
    {"Slate Gray", QColor(112, 128, 144)},
    {"Deep Purple", QColor(72, 61, 139)},
    {"Dark Teal", QColor(0, 128, 128)},
    {"Olive Green", QColor(85, 107, 47)},
    {"Royal Blue", QColor(65, 105, 225)},
    {"Dark Red", QColor(139, 0, 0)},
    {"Charcoal", QColor(54, 69, 79)},
    {"Deep Orange", QColor(255, 140, 0)},
    {"Pine Green", QColor(1, 121, 111)},
    {"Indigo", QColor(75, 0, 130)},
    {"Chocolate Brown", QColor(139, 69, 19)}};

QColor ColorPalette::getColor(const QString &name) {
    // If the color exists in the map, return it; otherwise,
    // default to black.
    if (COLORS.contains(name))
        return COLORS.value(name);
    return QColor(0, 0, 0);
}

QStringList ColorPalette::getAllColors() {
    // Return the list of all color names (keys in the map).
    return COLORS.keys();
}

} // namespace GUI
} // namespace CargoNetSim
