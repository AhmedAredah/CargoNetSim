#include "CommandProcessor.h"
#include "RabbitMQHandler.h"

#include <QDebug>
#include <QUuid>
#include <QDateTime>
#include <QThread>
#include <QtConcurrent/QtConcurrent>

namespace CargoNetSim {
namespace Backend {

/**
 * Constructor
 */
CommandProcessor::CommandProcessor(
    RabbitMQHandler* rabbitMQHandler,
    ClientType clientType,
    QObject* parent)
    : QObject(parent),
      m_rabbitMQHandler(rabbitMQHandler),
      m_clientType(clientType)
{
    // Connect to RabbitMQ message received signal
    connect(m_rabbitMQHandler, &RabbitMQHandler::messageReceived,
            this, &CommandProcessor::handleResponse);
    
    qDebug() << "Command processor initialized for" 
             << getClientTypeString();
}

/**
 * Destructor
 */
CommandProcessor::~CommandProcessor()
{
    qDebug() << "Command processor destroyed for" 
             << getClientTypeString();
}

/**
 * Sends a command to the server.
 * Protected method to be used by derived classes.
 */
bool CommandProcessor::sendCommand(
    const QString& command,
    const QJsonObject& params,
    const QString& routingKey)
{
    QJsonObject commandObj = createCommandObject(command, params);
    
    // Add command ID for tracking
    QString commandId = generateCommandId();
    commandObj["commandId"] = commandId;
    
    qDebug() << "Sending command" << command 
             << "with ID" << commandId;
    
    // Send the command
    bool success = m_rabbitMQHandler->sendCommand(
        commandObj, routingKey);
    
    if (success) {
        emit commandSent(commandId, command);
    } else {
        QString errorMsg = "Failed to send command to RabbitMQ";
        emit commandSendFailed(commandId, command, errorMsg);
        emit errorOccurred(errorMsg);
    }
    
    return success;
}

/**
 * Sends a command asynchronously with callback for result.
 * Protected method to be used by derived classes.
 */
QString CommandProcessor::sendCommandAsync(
    const QString& command,
    CommandResultCallback callback,
    const QJsonObject& params,
    const QString& routingKey)
{
    QJsonObject commandObj = createCommandObject(command, params);
    
    // Add command ID for tracking
    QString commandId = generateCommandId();
    commandObj["commandId"] = commandId;
    
    qDebug() << "Sending async command" << command 
             << "with ID" << commandId;
    
    // Register callback
    {
        QMutexLocker locker(&m_commandMutex);
        m_pendingCommands[commandId] = callback;
    }
    
    // Send the command
    bool success = m_rabbitMQHandler->sendCommand(
        commandObj, routingKey);
    
    if (success) {
        emit commandSent(commandId, command);
        
        // Set up timeout for the command
        QtConcurrent::run([this, commandId, command]() {
            QThread::sleep(COMMAND_TIMEOUT_MS / 1000);
            
            // Check if command is still pending
            QMutexLocker locker(&m_commandMutex);
            if (m_pendingCommands.contains(commandId)) {
                // Get callback and remove from pending
                auto callback = m_pendingCommands.take(commandId);
                locker.unlock();
                
                // Create timeout error
                QJsonObject error;
                error["error"] = "Command timed out";
                error["command"] = command;
                error["commandId"] = commandId;
                
                // Call callback with failure
                if (callback) {
                    callback(false, error);
                }
                
                emit commandResultReceived(commandId, false, error);
                emit errorOccurred("Command timed out: " + command);
            }
        });
    } else {
        // Remove callback if send failed
        QMutexLocker locker(&m_commandMutex);
        m_pendingCommands.remove(commandId);
        
        // Create error
        QString errorMsg = "Failed to send command to RabbitMQ";
        QJsonObject error;
        error["error"] = errorMsg;
        error["command"] = command;
        error["commandId"] = commandId;
        
        emit commandSendFailed(commandId, command, errorMsg);
        emit errorOccurred(errorMsg + ": " + command);
        
        // Call callback with failure
        if (callback) {
            callback(false, error);
        }
    }
    
    return commandId;
}

/**
 * Creates a JSON object from a command name and parameters.
 */
QJsonObject CommandProcessor::createCommandObject(
    const QString& command,
    const QJsonObject& params)
{
    QJsonObject commandObj;
    commandObj["command"] = command;
    commandObj["timestamp"] = QDateTime::currentDateTime()
                             .toString(Qt::ISODate);
    commandObj["clientType"] = static_cast<int>(m_clientType);
    
    // Add parameters if they exist
    if (!params.isEmpty()) {
        commandObj["params"] = params;
    }
    
    return commandObj;
}

/**
 * Handles a response from the server.
 * Base implementation handles command responses.
 * Derived classes should override to handle additional response types.
 */
void CommandProcessor::handleResponse(const QJsonObject& response)
{
    // Check if this is a command response
    if (!response.contains("commandId")) {
        return; // Not a command response
    }
    
    QString commandId = response["commandId"].toString();
    if (commandId.isEmpty()) {
        return; // Invalid command ID
    }
    
    bool success = response["success"].toBool(false);
    
    // Check if we have a pending callback for this command
    QMutexLocker locker(&m_commandMutex);
    if (!m_pendingCommands.contains(commandId)) {
        return; // No callback registered for this command
    }
    
    // Get callback and remove from pending
    auto callback = m_pendingCommands.take(commandId);
    locker.unlock();
    
    // Emit signal
    emit commandResultReceived(commandId, success, response);
    
    // Call callback if exists
    if (callback) {
        callback(success, response);
    }
    
    // Log error if command failed
    if (!success && response.contains("error")) {
        QString errorMsg = response["error"].toString();
        emit errorOccurred(errorMsg);
    }
}

/**
 * Generates a unique command ID.
 */
QString CommandProcessor::generateCommandId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

/**
 * Helper method to get client type as string for logging.
 */
QString CommandProcessor::getClientTypeString() const
{
    switch (m_clientType) {
        case ClientType::ShipClient:
            return "ShipClient";
        case ClientType::TrainClient:
            return "TrainClient";
        case ClientType::TruckClient:
            return "TruckClient";
        default:
            return "BaseClient";
    }
}

} // namespace Backend
} // namespace CargoNetSim
