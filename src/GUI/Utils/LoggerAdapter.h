#pragma once

#include "ApplicationLogger.h"
#include "Backend/Commons/LoggerInterface.h"

namespace CargoNetSim
{
namespace GUI
{

/**
 * @brief Adapter to bridge ApplicationLogger to
 * LoggerInterface
 *
 * Implements the backend LoggerInterface using the GUI's
 * ApplicationLogger singleton, enabling backend logging.
 */
class LoggerAdapter : public Backend::LoggerInterface
{
public:
    /**
     * @brief Constructor
     *
     * Uses the existing ApplicationLogger singleton
     * instance.
     */
    LoggerAdapter() = default;

    /**
     * @brief Log a standard message
     * @param message Message text to log
     * @param clientType Client type/index for the message
     */
    void log(const QString &message,
             int            clientType) override
    {
        ApplicationLogger::log(message, clientType);
    }

    /**
     * @brief Log an error message
     * @param message Error message text to log
     * @param clientType Client type/index for the error
     */
    void logError(const QString &message,
                  int            clientType) override
    {
        ApplicationLogger::logError(message, clientType);
    }

    /**
     * @brief Update progress for a client
     * @param progressValue Progress value (0-100)
     * @param clientType Client type/index for the update
     */
    void updateProgress(float progressValue,
                        int   clientType) override
    {
        ApplicationLogger::updateProgress(progressValue,
                                          clientType);
    }
};

} // namespace GUI
} // namespace CargoNetSim