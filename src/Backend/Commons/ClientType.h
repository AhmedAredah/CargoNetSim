#pragma once

#include <QObject>
#include <QString>

namespace CargoNetSim {
namespace Backend {
class ClientType {
    Q_GADGET
public:
    enum Value {
        ShipClient = 0,
        TrainClient = 1,
        TruckClient = 2,
        PortClient = 3
    };
    Q_ENUM(Value)

    // Constructors
    constexpr ClientType() : m_value(ShipClient) {}
    constexpr ClientType(Value value) : m_value(value) {}
    
    // Implicit conversion from enum Value
    constexpr operator Value() const { return m_value; }
    
    // Explicit conversion to QString
    QString toString() const {
        switch (m_value) {
        case ShipClient: return QStringLiteral("ShipClient");
        case TrainClient: return QStringLiteral("TrainClient");
        case TruckClient: return QStringLiteral("TruckClient");
        case PortClient: return QStringLiteral("PortClient");
        default: return QStringLiteral("Unknown");
        }
    }

private:
    Value m_value;
};

// Define global operator overloads for comparison operations
inline bool operator==(ClientType lhs, ClientType rhs) { return lhs.operator ClientType::Value() == rhs.operator ClientType::Value(); }
inline bool operator!=(ClientType lhs, ClientType rhs) { return !(lhs == rhs); }

} // namespace Backend
} // namespace CargoNetSim
