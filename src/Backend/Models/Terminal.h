#pragma once

#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QStringList>

#include "Backend/Commons/TerminalInterface.h"
#include "Backend/Commons/TransportationMode.h"

namespace CargoNetSim
{
namespace Backend
{

/**
 * @brief Represents a terminal for simulation
 */
class Terminal : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param names Terminal names (first is canonical)
     * @param config Custom configuration
     * @param interfaces Interfaces and transportation modes
     * @param region Region (optional)
     * @param parent Parent QObject
     */
    explicit Terminal(
        const QStringList &names, const QJsonObject &config,
        const QMap<
            TerminalTypes::TerminalInterface,
            QSet<TransportationTypes::TransportationMode>>
                      &interfaces,
        const QString &region = "",
        QObject       *parent = nullptr);

    /**
     * @brief Get terminal names
     * @return List of names
     */
    QStringList getNames() const
    {
        return m_names;
    }

    /**
     * @brief Get canonical name
     * @return First name in the list
     */
    QString getCanonicalName() const
    {
        return m_names.first();
    }

    /**
     * @brief Get custom configuration
     * @return Configuration as JSON
     */
    QJsonObject getConfig() const
    {
        return m_config;
    }

    /**
     * @brief Get interfaces
     * @return Map of interfaces to modes
     */
    QMap<TerminalTypes::TerminalInterface,
         QSet<TransportationTypes::TransportationMode>>
    getInterfaces() const
    {
        return m_interfaces;
    }

    /**
     * @brief Get region
     * @return Region string
     */
    QString getRegion() const
    {
        return m_region;
    }

    /**
     * @brief Convert to JSON for server command
     * @return JSON representation
     */
    QJsonObject toJson() const;

private:
    QStringList          m_names;
    QJsonObject          m_config;
    QMap<TerminalTypes::TerminalInterface,
         QSet<TransportationTypes::TransportationMode>>
                         m_interfaces;
    QString              m_region;
};

} // namespace Backend
} // namespace CargoNetSim
