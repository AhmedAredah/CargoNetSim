#pragma once
/**
 * @file LoggerInterface.h
 * @brief Defines Logger Interface for CargoNetSim backend
 * @details This interface provides a contract for logging
 * messages, errors, and progress updates in the CargoNetSim
 * backend. It allows different components to log messages
 * without depending on a specific logger implementation.
 * @note This interface is designed to be implemented by GUI
 * logger.
 * @author Ahmed Aredah
 */

#include <QString>

namespace CargoNetSim
{
namespace Backend
{

/**
 * @brief Abstract interface for logging
 *
 * Defines a contract for logging messages, allowing backend
 * components to log without depending on a specific logger.
 */
class LoggerInterface
{
public:
    virtual ~LoggerInterface() = default;

    /**
     * @brief Log a standard message
     * @param message Message text
     * @param clientType Client type/index
     */
    virtual void log(const QString &message,
                     int            clientType) = 0;

    /**
     * @brief Log an error message
     * @param message Error message text
     * @param clientType Client type/index
     */
    virtual void logError(const QString &message,
                          int            clientType) = 0;

    /**
     * @brief Update progress for a client
     * @param progressValue Progress value (0-100)
     * @param clientType Client type/index
     */
    virtual void updateProgress(float progressValue,
                                int   clientType) = 0;
};

} // namespace Backend
} // namespace CargoNetSim
