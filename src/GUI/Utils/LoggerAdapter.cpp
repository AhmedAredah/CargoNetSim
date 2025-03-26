#include "LoggerAdapter.h"

namespace CargoNetSim
{
namespace GUI
{

LoggerAdapter::LoggerAdapter()
{
    // No additional initialization needed; relies on
    // ApplicationLogger singleton
}

void LoggerAdapter::log(const QString &message,
                        int            clientType)
{
    // Delegate to ApplicationLogger's log method
    ApplicationLogger::log(message, clientType);
}

void LoggerAdapter::logError(const QString &message,
                             int            clientType)
{
    // Delegate to ApplicationLogger's logError method
    ApplicationLogger::logError(message, clientType);
}

void LoggerAdapter::updateProgress(float progressValue,
                                   int   clientType)
{
    // Delegate to ApplicationLogger's updateProgress method
    ApplicationLogger::updateProgress(progressValue,
                                      clientType);
}

} // namespace GUI
} // namespace CargoNetSim