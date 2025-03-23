/**
 * @file TransportationGraph.cpp
 * @brief Explicit instantiations for TransportationGraph
 * @author [Your Name]
 * @date 2025-03-22
 */

#include "TransportationGraph.h"

namespace CargoNetSim
{
namespace Backend
{
namespace TruckClient
{

// Explicit instantiations for common types
template class TransportationGraph<int>;
template class TransportationGraph<QString>;

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
