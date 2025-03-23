/**
 * @file MessageFormatter.h
 * @brief Defines standard message formats for simulation
 * @author Ahmed Aredah
 * @date 2025-03-22
 */

#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QList>
#include <QString>

namespace CargoNetSim
{
namespace Backend
{
namespace TruckClient
{

/**
 * @class MessageFormatter
 * @brief Utility class for formatting simulation messages
 *
 * Provides standardized message formatting for
 * communication with the simulation backend across
 * different modules.
 */
class MessageFormatter
{
public:
    /**
     * @enum MessageType
     * @brief Defines message type categories
     */
    enum class MessageType
    {
        SYNC       = 1000, ///< Synchronization messages
        TRIP_CTRL  = 1001, ///< Trip control messages
        TRIPS_INFO = 1002  ///< Trip information messages
    };

    /**
     * @enum MessageCode
     * @brief Defines specific message codes within types
     */
    enum class MessageCode
    {
        // Sync codes
        SYNC_REQ  = 0, ///< Request synchronization
        SYNC_GO   = 1, ///< Proceed with synchronization
        SYNC_WAIT = 2, ///< Wait for synchronization
        SYNC_END  = 9, ///< End synchronization

        // Trip control codes
        ADD_TRIP    = 0, ///< Add a new trip
        CANCEL_TRIP = 1, ///< Cancel an existing trip

        // Trip info codes
        TRIP_INFO = 0, ///< Trip information update
        TRIP_END  = 1  ///< Trip ended notification
    };

    /**
     * @brief Formats a standard message string
     * @param msgId Message identifier
     * @param acknowledgement If true, message needs
     * acknowledgement
     * @param messageType The message type
     * @param messageCode The message code
     * @param content The message content
     * @return Formatted message string
     */
    static QString formatMessage(int  msgId,
                                 bool acknowledgement,
                                 MessageType    messageType,
                                 MessageCode    messageCode,
                                 const QString &content);

    /**
     * @brief Formats a sync request message
     * @param msgId Message identifier
     * @param simTime Current simulation time
     * @param simHorizon Simulation time horizon
     * @return Formatted message string
     */
    static QString formatSyncRequest(int    msgId,
                                     double simTime,
                                     double simHorizon);

    /**
     * @brief Formats a sync go message
     * @param msgId Message identifier
     * @param currentTime Current simulation time
     * @param nextTime Next simulation time
     * @return Formatted message string
     */
    static QString formatSyncGo(int    msgId,
                                double currentTime,
                                double nextTime);

    /**
     * @brief Formats a sync end message
     * @param msgId Message identifier
     * @param simTime Current simulation time
     * @return Formatted message string
     */
    static QString formatSyncEnd(int msgId, double simTime);

    /**
     * @brief Formats an add trip message
     * @param msgId Message identifier
     * @param tripId Trip identifier
     * @param originId Origin node identifier
     * @param destinationId Destination node identifier
     * @param startTime Trip start time
     * @param linkIds List of link identifiers for route
     * @return Formatted message string
     */
    static QString formatAddTrip(int msgId, int tripId,
                                 int    originId,
                                 int    destinationId,
                                 double startTime,
                                 const QList<int> &linkIds);

    /**
     * @brief Parses a message string into components
     * @param message The message to parse
     * @return Components as a JSON object
     */
    static QJsonObject parseMessage(const QString &message);

    /**
     * @brief Parses a trip info message
     * @param message The message to parse
     * @return Trip information as a JSON object
     */
    static QJsonObject
    parseTripInfo(const QString &message);

    /**
     * @brief Parses a trip end message
     * @param message The message to parse
     * @return Trip end information as a JSON object
     */
    static QJsonObject parseTripEnd(const QString &message);
};

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
