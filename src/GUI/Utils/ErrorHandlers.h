#pragma once
#include <QString>
#include <QObject>
#include <QRunnable>

namespace CargoNetSim {
namespace GUI {

/**
 * @brief Error handling utilities for the CargoNetSim application
 *
 * This class provides centralized error handling functionality, including
 * global exception handling, Qt message handling, and logging of errors.
 */
class ErrorHandlers : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Install all exception handlers for the application
     *
     * This installs handlers for:
     * - Uncaught exceptions in the main thread
     * - Qt internal messages and warnings
     */
    static void installExceptionHandlers();

    /**
     * @brief Handle uncaught exceptions globally
     *
     * @param exc_type The exception type
     * @param exc_value The exception value/object
     * @param exc_traceback The exception traceback
     */
    static void handleException(int exceptionType,
                                void* exceptionValue,
                                void* exceptionTraceback);

    /**
     * @brief Handler for Qt's debug/warning/critical/fatal messages
     *
     * @param type Message type (debug, warning, critical, fatal)
     * @param context Message context
     * @param message The actual message text
     */
    static void qtMessageHandler(QtMsgType type,
                                 const QMessageLogContext& context,
                                 const QString& message);

    /**
     * @brief Get the singleton instance
     *
     * @return Reference to the singleton instance
     */
    static ErrorHandlers& getInstance();

    /**
     * @brief Write error to log file
     *
     * @param errorText The error text to log
     */
    static void writeToErrorLog(const QString& errorText);

signals:
    /**
     * @brief Signal emitted when an error occurs
     *
     * @param errorMessage The error message
     * @param severity Error severity level
     *        (0=info, 1=warning, 2=error, 3=fatal)
     */
    void errorOccurred(const QString& errorMessage, int severity);

private:

    // Private constructor to enforce singleton pattern
    ErrorHandlers();
    ~ErrorHandlers() = default;

    // Disable copy and move
    ErrorHandlers(const ErrorHandlers&) = delete;
    ErrorHandlers& operator=(const ErrorHandlers&) = delete;
    ErrorHandlers(ErrorHandlers&&) = delete;
    ErrorHandlers& operator=(ErrorHandlers&&) = delete;
};

/**
 * @brief Safe runnable class for thread exception handling
 *
 * Provides exception handling wrapper for QRunnable objects
 */
class SafeRunnable : public QRunnable {
public:
    void run() override;

    /**
     * @brief Override this method to implement thread-safe work
     */
    virtual void runSafe() = 0;
};

// Global function for installing exception handlers (for convenience)
void installExceptionHandlers();

} // namespace GUI
} // namespace CargoNetSim
