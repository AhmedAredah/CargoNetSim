#ifndef COLORUTIL_H
#define COLORUTIL_H

#include <QColor>

/*!
 * \brief The ColorUtil namespace provides utility functions
 * for color operations.
 */
namespace CargoNetSim {
namespace GUI {
namespace ColorUtils {

/*!
 * \brief Generates a random QColor with RGB values between
 * 50 and 205.
 *
 * This ensures good visibility by avoiding colors that are
 * too dark or too light.
 *
 * \return A randomly generated QColor.
 */
QColor getRandomColor();

} // namespace ColorUtils
} // namespace GUI
} // namespace CargoNetSim

#endif // COLORUTIL_H
