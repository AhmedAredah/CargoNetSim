/**
 * @file SimulationConfig.h
 * @brief Manages simulation configuration
 * @author Ahmed Aredah
 * @date 2025-03-22
 */

#pragma once

#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

namespace CargoNetSim {
namespace Backend {
namespace TruckClient {

/**
 * @class SimulationConfig
 * @brief Manages and validates simulation configuration
 *
 * Provides a centralized configuration management system
 * for simulation parameters, file paths, and options.
 */
class SimulationConfig : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent The parent QObject
     */
    explicit SimulationConfig(QObject *parent = nullptr);

    /**
     * @brief Loads configuration from a file
     * @param filePath Path to the configuration file
     * @return True if loading was successful
     */
    bool loadFromFile(const QString &filePath);

    /**
     * @brief Saves configuration to a file
     * @param filePath Path to save the configuration
     * @return True if saving was successful
     */
    bool saveToFile(const QString &filePath);

    /**
     * @brief Loads configuration from a JSON object
     * @param config Configuration as JSON
     * @return True if loading was successful
     */
    bool loadFromJson(const QJsonObject &config);

    /**
     * @brief Exports configuration to a JSON object
     * @return Configuration as JSON
     */
    QJsonObject toJson() const;

    /**
     * @brief Validates the current configuration
     * @param errors List to populate with error messages
     * @return True if configuration is valid
     */
    bool validate(QStringList *errors = nullptr) const;

    /**
     * @brief Gets a configuration value
     * @param key Configuration key
     * @param defaultValue Value to return if key not found
     * @return Configuration value
     */
    QVariant getValue(
        const QString  &key,
        const QVariant &defaultValue = QVariant()) const;

    /**
     * @brief Sets a configuration value
     * @param key Configuration key
     * @param value New value
     */
    void setValue(const QString  &key,
                  const QVariant &value);

    /**
     * @brief Gets a nested configuration value
     * @param path Path to configuration (sections/key)
     * @param defaultValue Value to return if path not found
     * @return Configuration value
     */
    QVariant getNestedValue(
        const QString  &path,
        const QVariant &defaultValue = QVariant()) const;

    /**
     * @brief Sets a nested configuration value
     * @param path Path to configuration (sections/key)
     * @param value New value
     */
    void setNestedValue(const QString  &path,
                        const QVariant &value);

    /**
     * @brief Gets a file path from configuration
     * @param key Configuration key for file path
     * @param basePath Base path for relative paths
     * @return Absolute file path
     */
    QString
    getFilePath(const QString &key,
                const QString &basePath = QString()) const;

    /**
     * @brief Merges another configuration into this one
     * @param other Configuration to merge
     * @param overwrite If true, overwrites existing values
     */
    void merge(const SimulationConfig &other,
               bool                    overwrite = true);

signals:
    /**
     * @brief Signal emitted when configuration changes
     */
    void configurationChanged();

private:
    // Root configuration object
    QJsonObject m_config;

    // Configuration file path
    QString m_configFilePath;

    // Helper functions for nested access
    QJsonValue getJsonValue(const QString &path) const;
    void       setJsonValue(const QString    &path,
                            const QJsonValue &value);
};

} // namespace TruckClient
} // namespace Backend
} // namespace CargoNetSim
