#include "DirectedGraph.h"

namespace CargoNetSim {
namespace Backend {

// Explicit instantiations for common types
template class DirectedGraph<int>;
template class DirectedGraph<QString>;

} // namespace Backend
} // namespace CargoNetSim