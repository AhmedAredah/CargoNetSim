#ifndef COLORPALETTE_H
#define COLORPALETTE_H

#include <QColor>
#include <QMap>
#include <QString>
#include <QStringList>

namespace CargoNetSim
{
namespace GUI
{

/*!
 * \brief The ColorPalette class provides a professional
 * color palette with predefined colors for region
 * management.
 *
 * It offers static methods to get a QColor by its name and
 * to obtain a list of all available color names.
 */
class ColorPalette
{
public:
    /*!
     * \brief Returns the QColor corresponding to the given
     * name.
     * \param name The name of the color.
     * \return A QColor object if the name exists;
     * otherwise, black.
     */
    static QColor getColor(const QString &name);

    /*!
     * \brief Returns a list of all available color names.
     * \return A QStringList of color names.
     */
    static QStringList getAllColors();

private:
    // A static map of color names to their corresponding
    // QColor values.
    static const QMap<QString, QColor> COLORS;
};

#endif // COLORPALETTE_H
} // namespace GUI
} // namespace CargoNetSim
