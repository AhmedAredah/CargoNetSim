/**
 * @file MessageFormatter.cpp
 * @brief Implements standard message formats for simulation
 * @author Ahmed Aredah
 * @date 2025-03-22
 */

#include "MessageFormatter.h"
#include <QJsonDocument>

namespace CargoNetSim {
namespace Backend {
namespace TruckClient {

QString MessageFormatter::formatMessage(
    int msgId, bool acknowledgement,
    MessageType messageType, MessageCode messageCode,
    const QString &content) {
    // Standard format:
    // id/ack/type/code/00/00/00/00/content/-1
    return QString("%1/%2/%3/%4/00/00/00/00/%5/-1")
        .arg(msgId)
        .arg(acknowledgement ? "1" : "0")
        .arg(static_cast<int>(messageType))
        .arg(static_cast<int>(messageCode))
        .arg(content);
}

QString MessageFormatter::formatSyncRequest(
    int msgId, double simTime, double simHorizon) {
    QString content =
        QString("%1/%2").arg(simTime).arg(simHorizon);

    return formatMessage(msgId, false, MessageType::SYNC,
                         MessageCode::SYNC_REQ, content);
}

QString MessageFormatter::formatSyncGo(int    msgId,
                                       double currentTime,
                                       double nextTime) {
    QString content =
        QString("%1/%2")
            .arg(static_cast<int>(currentTime))
            .arg(static_cast<int>(nextTime));

    return formatMessage(msgId, true, MessageType::SYNC,
                         MessageCode::SYNC_GO, content);
}

QString MessageFormatter::formatSyncEnd(int    msgId,
                                        double simTime) {
    QString content =
        QString("%1").arg(static_cast<int>(simTime));

    return formatMessage(msgId, true, MessageType::SYNC,
                         MessageCode::SYNC_END, content);
}

QString MessageFormatter::formatAddTrip(
    int msgId, int tripId, int originId, int destinationId,
    double startTime, const QList<int> &linkIds) {
    // Format content with link count and links
    QString content = QString("%1/%2/%3/%4/%5")
                          .arg(tripId)
                          .arg(originId)
                          .arg(destinationId)
                          .arg(static_cast<int>(startTime))
                          .arg(linkIds.size());

    // Add each link ID to the content
    for (int linkId : linkIds) {
        content += QString("/%1").arg(linkId);
    }

    return formatMessage(msgId, false,
                         MessageType::TRIP_CTRL,
                         MessageCode::ADD_TRIP, content);
}

QJsonObject
MessageFormatter::parseMessage(const QString &message) {
    QJsonObject result;
    QStringList parts = message.split('/');

    if (parts.size() < 9) {
        // Invalid message format
        result["valid"] = false;
        return result;
    }

    // Extract message parts
    result["valid"]           = true;
    result["msgId"]           = parts[0].toInt();
    result["acknowledgement"] = (parts[1] == "1");
    result["messageType"]     = parts[2].toInt();
    result["messageCode"]     = parts[3].toInt();

    // Extract content
    QString content;
    for (int i = 8; i < parts.size(); ++i) {
        if (parts[i] == "-1") {
            break;
        }

        if (i > 8) {
            content += "/";
        }

        content += parts[i];
    }

    result["content"] = content;

    return result;
}

QJsonObject
MessageFormatter::parseTripInfo(const QString &message) {
    QJsonObject parsed = parseMessage(message);

    if (!parsed["valid"].toBool()
        || parsed["messageType"].toInt()
               != static_cast<int>(MessageType::TRIPS_INFO)
        || parsed["messageCode"].toInt()
               != static_cast<int>(
                   MessageCode::TRIP_INFO)) {
        // Not a valid trip info message
        return QJsonObject();
    }

    // For trip info, the content is usually JSON
    QString       content = parsed["content"].toString();
    QJsonDocument doc =
        QJsonDocument::fromJson(content.toUtf8());

    if (doc.isNull() || !doc.isObject()) {
        return QJsonObject();
    }

    return doc.object();
}

QJsonObject
MessageFormatter::parseTripEnd(const QString &message) {
    QJsonObject parsed = parseMessage(message);

    if (!parsed["valid"].toBool()
        || parsed["messageType"].toInt()
               != static_cast<int>(MessageType::TRIPS_INFO)
        || parsed["messageCode"].toInt()
               != static_cast<int>(MessageCode::TRIP_END)) {
        // Not a valid trip end message
        return QJsonObject();
    }

    // For trip end, the content is usually JSON
    QString       content = parsed["content"].toString();
    QJsonDocument doc =
        QJsonDocument::fromJson(content.toUtf8());

    if (doc.isNull() || !doc.isObject()) {
        return QJsonObject();
    }

    return doc.object();
}

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
