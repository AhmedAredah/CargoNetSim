#pragma once

#include <QObject>
#include <QString>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <QEvent>
#include <QFile>
#include <QTextStream>
#include <QMap>

namespace CargoNetSim {
namespace GUI {

/**
 * @brief Log message entry class
 * 
 * Contains a single log message with its metadata
 */
class LogEntry {
public:
    /**
     * @brief Create a new log entry
     * 
     * @param message Log message text
     * @param clientIndex Client that generated the message
     * @param isError Flag indicating if this is an error message
     * @param timestamp Message timestamp
     */
    LogEntry(const QString& message, int clientIndex,
             bool isError, qint64 timestamp);
    
    QString message;   ///< The log message text
    int clientIndex;   ///< The client that generated the message
    bool isError;      ///< Flag indicating if this is an error message
    qint64 timestamp;  ///< Message timestamp
};

/**
 * @brief Thread-safe application logger
 * 
 * Provides a centralized logging system with support for multiple clients,
 * error levels, and signal-based notification.
 */
class ApplicationLogger : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Get the singleton instance
     * 
     * @return Pointer to the singleton instance
     */
    static ApplicationLogger* getInstance();
    
    /**
     * @brief Log a standard message
     * 
     * @param message Message text
     * @param clientType Client type/index
     */
    static void log(const QString& message, int clientType = 4);
    
    /**
     * @brief Log an error message
     * 
     * @param message Error message text
     * @param clientType Client type/index
     */
    static void logError(const QString& message, int clientType = 4);
    
    /**
     * @brief Update progress for a client
     * 
     * @param progressValue Progress value (0-100)
     * @param clientType Client type/index
     */
    static void updateProgress(float progressValue, int clientType = 4);
    
    /**
     * @brief Signal that initialization is complete
     */
    static void signalInitComplete();
    
    /**
     * @brief Wait for initialization to complete
     * 
     * @param timeoutMs Timeout in milliseconds (-1 for infinite)
     * @return True if initialization completed, false if timed out
     */
    static bool waitForInitComplete(int timeoutMs = -1);

public slots:
    /**
     * @brief Start the logging thread
     */
    void start();
    
    /**
     * @brief Stop the logging thread
     */
    void stop();
    
    /**
     * @brief Process pending log messages
     */
    void processLogQueue();
    
    /**
     * @brief Process pending progress updates
     */
    void processProgressQueue();
    
    /**
     * @brief Save logs to a file
     * 
     * @param filePath Path to save the log file
     * @return True if successful, false otherwise
     */
    bool saveLogsToFile(const QString& filePath);

signals:
    /**
     * @brief Signal emitted when a new log message is added
     * 
     * @param message Log message text
     * @param clientIndex Client that generated the message
     * @param isError Flag indicating if this is an error message
     */
    void newLogMessage(const QString& message, int clientIndex, bool isError);
    
    /**
     * @brief Signal emitted when progress is updated
     * 
     * @param progressValue Progress value (0-100)
     * @param clientIndex Client index
     */
    void progressUpdated(int progressValue, int clientIndex);
    
    /**
     * @brief Signal emitted when initialization is complete
     */
    void initializationComplete();

protected:
    /**
     * @brief Custom event handler
     * 
     * @param event The event to handle
     */
    void customEvent(QEvent* event) override;

private:
    /**
     * @brief Private constructor for singleton pattern
     */
    ApplicationLogger();
    
    /**
     * @brief Append a log entry to the internal storage
     * 
     * @param entry Log entry to append
     */
    void appendLogEntry(const LogEntry& entry);
    
    /**
     * @brief Add a log message to the queue
     * 
     * @param message Message text
     * @param clientType Client type/index
     * @param isError Error flag
     */
    static void logMessageInternal(const QString& message,
                                   int clientType,
                                   bool isError);

    // Thread synchronization primitives
    static QMutex s_logMutex;         ///< Mutex for log operations
    static QMutex s_progressMutex;    ///< Mutex for progress operations
    static QWaitCondition
        s_initCondition;              ///< Condition for initialization waiting
    static bool s_isInitialized;      ///< Initialization flag
    
    // Queues for messages and progress updates
    static QQueue<LogEntry>
        s_logQueue;                   ///< Queue of pending log messages
    static QQueue<QPair<float, int>>
        s_progressQueue;              ///< Queue of pending progress updates
    
    // Log storage
    QMap<int, QStringList>
        m_clientLogs;                 ///< Logs organized by client
    QMap<int, int> m_clientProgress;  ///< Current progress by client
    
    bool m_isRunning;                 ///< Flag indicating if logger is running
    
    // Singleton instance
    static ApplicationLogger* s_instance;
};

/**
 * @brief Custom event for log messages
 */
class LogEvent : public QEvent {
public:
    static const QEvent::Type LogEventType =
        static_cast<QEvent::Type>(QEvent::User + 1);
    
    LogEvent(const LogEntry& entry);
    LogEntry entry;  ///< The log entry
};

/**
 * @brief Custom event for progress updates
 */
class ProgressEvent : public QEvent {
public:
    static const QEvent::Type ProgressEventType =
        static_cast<QEvent::Type>(QEvent::User + 2);
    
    ProgressEvent(float value, int clientIndex);
    float value;        ///< Progress value
    int clientIndex;    ///< Client index
};

} // namespace GUI
} // namespace CargoNetSim
