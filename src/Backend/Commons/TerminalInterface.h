/**
 * @file TerminalTypes.h
 * @brief Defines the terminal interface types for the
 * CargoNetSim system.
 * @author Ahmed Aredah
 */

#pragma once
#include <QObject>
#include <QString>

namespace CargoNetSim
{
namespace Backend
{

/**
 * @class TerminalTypes
 * @brief Class that defines terminal interface categories
 * in the cargo network system.
 *
 * This class provides an enumeration of different terminal
 * interfaces and utility functions to convert between the
 * enum values and their string representations. It inherits
 * from QObject to enable Qt meta-object features,
 * particularly for the Q_ENUM macro to register the enum
 * with Qt's meta-object system.
 */
class TerminalTypes : public QObject
{
    Q_OBJECT
public:
    /**
     * @enum TerminalInterface
     * @brief Enumeration of interface types for cargo
     * terminals.
     *
     * Defines the different sides or interfaces that a
     * cargo terminal might have, which affects how cargo is
     * loaded/unloaded and what transport modes can connect.
     */
    enum class TerminalInterface
    {
        LAND_SIDE, /**< Interface for land-based
                      transportation (e.g., trucks, trains)
                    */
        SEA_SIDE, /**< Interface for maritime transportation
                     (e.g., ships, barges) */
        AIR_SIDE  /**< Interface for air transportation
                     (e.g., cargo planes) */
    };
    Q_ENUM(TerminalInterface)

    /**
     * @brief Converts a TerminalInterface enum value to its
     * string representation.
     * @param interface The TerminalInterface enum value to
     * convert.
     * @return A human-readable string representing the
     * terminal interface type.
     */
    static QString toString(TerminalInterface interface);
};

} // namespace Backend
} // namespace CargoNetSim

// Register custom types with Qt's meta-object system
Q_DECLARE_METATYPE(
    CargoNetSim::Backend::TerminalTypes::TerminalInterface)
Q_DECLARE_METATYPE(
    CargoNetSim::Backend::TerminalTypes::TerminalInterface
        *)
