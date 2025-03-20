#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>
#include <QMap>
#include <QMutex>
#include <QFuture>
#include <functional>
#include "Backend/Commons/ClientType.h"

namespace CargoNetSim {
namespace Backend {

class RabbitMQHandler;

/**
 * @brief Base class for processing commands sent to the simulation server
 *
 * This class provides the basic infrastructure for sending commands.
 * Client-specific processors should inherit from this class and
 * implement client-specific command methods.
 */
class CommandProcessor : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief Command result callback type
     */
    using CommandResultCallback = std::function<void(
        bool success, const QJsonObject& result)>;

    /**
     * @brief Constructor
     * @param rabbitMQHandler RabbitMQ handler for sending commands
     * @param clientType Type of client this processor belongs to
     * @param parent Parent QObject
     */
    explicit CommandProcessor(
        RabbitMQHandler* rabbitMQHandler,
        ClientType clientType,
        QObject* parent = nullptr);

    /**
     * @brief Destructor
     */
    virtual ~CommandProcessor();
    
signals:
    /**
     * @brief Emitted when a command is sent
     * @param commandId Unique ID of the command
     * @param command Command name
     */
    void commandSent(
        const QString& commandId,
        const QString& command);

    /**
     * @brief Emitted when a command fails to send
     * @param commandId Unique ID of the command
     * @param command Command name
     * @param error Error message
     */
    void commandSendFailed(
        const QString& commandId,
        const QString& command,
        const QString& error);

    /**
     * @brief Emitted when a command result is received
     * @param commandId Unique ID of the command
     * @param success Whether the command was successful
     * @param result Result data
     */
    void commandResultReceived(
        const QString& commandId,
        bool success,
        const QJsonObject& result);

    /**
     * @brief Emitted when an error occurs
     * @param errorMessage Error message
     */
    void errorOccurred(const QString& errorMessage);
    
protected:
    /**
     * @brief Sends a command to the server
     * @param command Command name
     * @param params Command parameters (optional)
     * @param routingKey Custom routing key (optional)
     * @return True if command was sent successfully
     */
    bool sendCommand(
        const QString& command,
        const QJsonObject& params = QJsonObject(),
        const QString& routingKey = QString());

    /**
     * @brief Sends a command with async callback for result
     * @param command Command name
     * @param callback Function to call with result
     * @param params Command parameters (optional)
     * @param routingKey Custom routing key (optional)
     * @return Command ID for tracking
     */
    QString sendCommandAsync(
        const QString& command,
        CommandResultCallback callback,
        const QJsonObject& params = QJsonObject(),
        const QString& routingKey = QString());

    /**
     * @brief Creates a JSON object from command and parameters
     * @param command Command name
     * @param params Command parameters
     * @return JSON object with command and parameters
     */
    virtual QJsonObject createCommandObject(
        const QString& command,
        const QJsonObject& params = QJsonObject());

    /**
     * @brief Handles a response from the server
     * @param response Response JSON object
     */
    virtual void handleResponse(const QJsonObject& response);

    /**
     * @brief Generates a unique command ID
     * @return Unique command ID
     */
    virtual QString generateCommandId() const;

    /**
     * @brief Get client type as string
     * @return String representation of client type
     */
    QString getClientTypeString() const;
    
    // References and properties
    RabbitMQHandler* m_rabbitMQHandler;
    ClientType m_clientType;

    // Command tracking
    QMap<QString, CommandResultCallback> m_pendingCommands;
    mutable QMutex m_commandMutex;

    // Constants
    static const int COMMAND_TIMEOUT_MS = 30000; // 30 seconds
};

} // namespace Backend
} // namespace CargoNetSim

Q_DECLARE_METATYPE(CargoNetSim::Backend::CommandProcessor)
Q_DECLARE_METATYPE(CargoNetSim::Backend::CommandProcessor*)


