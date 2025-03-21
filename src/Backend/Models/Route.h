#pragma once

#include <QObject>
#include <QJsonObject>
#include <QString>

namespace CargoNetSim {
namespace Backend {

/**
 * @brief Represents a route between terminals
 */
class Route : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param routeId Route identifier
     * @param start Start terminal
     * @param end End terminal
     * @param mode Transportation mode
     * @param attributes Route attributes (optional)
     * @param parent Parent QObject
     */
    explicit Route(
        const QString& routeId,
        const QString& start,
        const QString& end,
        int mode,
        const QJsonObject& attributes = {},
        QObject* parent = nullptr);

    /**
     * @brief Get route ID
     * @return Route identifier
     */
    QString getRouteId() const { return m_routeId; }

    /**
     * @brief Get start terminal
     * @return Start terminal name
     */
    QString getStart() const { return m_start; }

    /**
     * @brief Get end terminal
     * @return End terminal name
     */
    QString getEnd() const { return m_end; }

    /**
     * @brief Get transportation mode
     * @return Mode as integer
     */
    int getMode() const { return m_mode; }

    /**
     * @brief Get attributes
     * @return Attributes as JSON
     */
    QJsonObject getAttributes() const { return m_attributes; }

    /**
     * @brief Convert to JSON for server command
     * @return JSON representation
     */
    QJsonObject toJson() const;

private:
    QString m_routeId;
    QString m_start;
    QString m_end;
    int m_mode;
    QJsonObject m_attributes;
};

} // namespace Backend
} // namespace CargoNetSim
