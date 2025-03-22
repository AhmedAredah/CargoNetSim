/**
 * @file ClientType.h
 * @brief Defines the ClientType enumeration class for
 * CargoNetSim's client classification.
 * @author Ahmed Aredah
 */

#pragma once
#include <QObject>
#include <QString>

namespace CargoNetSim {
namespace Backend {

/**
 * @class ClientType
 * @brief Strongly-typed enumeration class representing
 * different types of clients in the cargo network.
 *
 * ClientType wraps an enumeration that identifies various
 * transport clients within the CargoNetSim system. It is
 * implemented as a Q_GADGET to allow for Qt meta-type
 * system integration, enabling use in QML, signal-slot
 * connections, and property systems.
 */
class ClientType {
    Q_GADGET
public:
    /**
     * @enum Value
     * @brief Enumeration of available client types in the
     * cargo network system.
     */
    enum Value {
        ShipClient     = 0, /**< Maritime vessel client */
        TrainClient    = 1, /**< Railway transport client */
        TruckClient    = 2, /**< Road transport client */
        TerminalClient = 3  /**< Port facility client */
    };
    Q_ENUM(Value)

    /**
     * @brief Default constructor.
     * Initializes with ShipClient as the default value.
     */
    constexpr ClientType()
        : m_value(ShipClient) {}

    /**
     * @brief Constructor from enum value.
     * @param value The ClientType::Value to initialize
     * with.
     */
    constexpr ClientType(Value value)
        : m_value(value) {}

    /**
     * @brief Implicit conversion operator to enum Value.
     * @return The underlying enum value.
     */
    constexpr operator Value() const {
        return m_value;
    }

    /**
     * @brief Converts the client type to a human-readable
     * string.
     * @return String representation of the client type.
     */
    QString toString() const {
        switch (m_value) {
        case ShipClient:
            return QStringLiteral("ShipClient");
        case TrainClient:
            return QStringLiteral("TrainClient");
        case TruckClient:
            return QStringLiteral("TruckClient");
        case TerminalClient:
            return QStringLiteral("TerminalClient");
        default:
            return QStringLiteral("Unknown");
        }
    }

private:
    Value m_value; /**< The underlying enum value */
};

/**
 * @brief Equality comparison operator for ClientType.
 * @param lhs Left-hand side operand.
 * @param rhs Right-hand side operand.
 * @return True if the client types are the same, false
 * otherwise.
 */
inline bool operator==(ClientType lhs, ClientType rhs) {
    return lhs.operator ClientType::Value()
           == rhs.operator ClientType::Value();
}

/**
 * @brief Inequality comparison operator for ClientType.
 * @param lhs Left-hand side operand.
 * @param rhs Right-hand side operand.
 * @return True if the client types are different, false
 * otherwise.
 */
inline bool operator!=(ClientType lhs, ClientType rhs) {
    return !(lhs == rhs);
}

} // namespace Backend
} // namespace CargoNetSim

// Register the ClientType class with Qt's meta-object
// system
Q_DECLARE_METATYPE(CargoNetSim::Backend::ClientType)
