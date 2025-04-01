#include "ThreadSafetyUtils.h"

namespace CargoNetSim
{
namespace Backend
{
namespace Commons
{

// Initialize static member
QMap<QThread*, QSet<QMutex*>> DeadlockDetector::heldLocks;

} // namespace Commons
} // namespace Backend
} // namespace CargoNetSim 