#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QMap>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QTableWidget>
#include <QVariant>
#include <QWidget>

namespace CargoNetSim {
namespace GUI {

/**
 * @brief Settings widget for configuring simulation
 * parameters.
 *
 * This widget provides a graphical interface for
 * configuration of all simulation parameters including fuel
 * types, carbon emissions, and transportation mode
 * settings.
 */
class SettingsWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Constructs a new Settings Widget
     * @param parent The parent widget
     */
    explicit SettingsWidget(QWidget *parent = nullptr);

    /**
     * @brief Retrieves the current settings
     * @return QMap<QString, QVariant> The current settings
     * values
     */
    QMap<QString, QVariant> getSettings() const;

signals:
    /**
     * @brief Signal emitted when settings are applied
     * @param settings The new settings values
     */
    void settingsChanged(
        const QMap<QString, QVariant> &settings);

public slots:
    /**
     * @brief Applies current settings values
     */
    void applySettings();

private slots:
    /**
     * @brief Updates the fuel type table with current
     * values
     */
    void updateFuelTable();

    /**
     * @brief Updates a specific fuel's data when edited
     * @param fuelType The fuel type being updated
     * @param key The property key being updated
     * @param value The new value
     */
    void updateFuelData(const QString  &fuelType,
                        const QString  &key,
                        const QVariant &value);

    /**
     * @brief Updates fuel type dropdown menus
     */
    void updateFuelTypeDropdowns();

    /**
     * @brief Adds a new fuel type
     */
    void addFuelType();

    /**
     * @brief Handles dwell time method changes
     * @param method The new dwell time method
     * @param dwellGroup The group box containing dwell time
     * settings
     */
    void onDwellMethodChanged(const QString &method,
                              QGroupBox     *dwellGroup);

    /**
     * @brief Shows the energy consumption calculator
     * @param mode The transportation mode
     */
    void showEnergyCalculator(const QString &mode);

private:
    /**
     * @brief Initialize the UI components
     */
    void initUI();

    /**
     * @brief Load settings from configuration
     * @return bool True if settings loaded successfully
     */
    bool loadSettings();

    /**
     * @brief Add a section for nested properties
     * @param item The item containing the properties
     * @param sectionName The section name
     * @param propertiesKey The properties key
     */
    void addNestedPropertiesSection(
        const QMap<QString, QVariant> &item,
        const QString                 &sectionName,
        const QString                 &propertiesKey);

    /**
     * @brief Creates parameter fields for dwell time method
     * @param method The dwell time method
     * @param currentParams Current parameter values
     * @return Pair of layout and field map
     */
    QPair<QFormLayout *, QMap<QString, QWidget *>>
    createDwellTimeParameters(
        const QString                 &method,
        const QMap<QString, QVariant> &currentParams =
            QMap<QString, QVariant>());

    // UI Components
    QTableWidget   *fuelTable;
    QSpinBox       *timeStepSpin;
    QDoubleSpinBox *timeValueOfMoneySpin;
    QSpinBox       *shortestPathsSpin;
    QDoubleSpinBox *carbonRateSpin;
    QDoubleSpinBox *shipMultiplierSpin;
    QDoubleSpinBox *truckMultiplierSpin;
    QDoubleSpinBox *trainMultiplierSpin;

    // Ship settings
    QDoubleSpinBox *shipSpeedSpin;
    QComboBox      *shipFuelType;
    QDoubleSpinBox *shipFuelSpin;
    QSpinBox       *shipContainers;
    QDoubleSpinBox *shipRiskSpin;

    // Train settings
    QDoubleSpinBox *trainSpeedSpin;
    QCheckBox      *trainUseNetwork;
    QComboBox      *trainFuelType;
    QDoubleSpinBox *trainFuelSpin;
    QSpinBox       *trainContainers;
    QDoubleSpinBox *trainRiskSpin;

    // Truck settings
    QDoubleSpinBox *truckSpeedSpin;
    QCheckBox      *truckUseNetwork;
    QComboBox      *truckFuelType;
    QDoubleSpinBox *truckFuelSpin;
    QSpinBox       *truckContainers;
    QDoubleSpinBox *truckRiskSpin;

    // Groups
    QGroupBox *simulationGroup;
    QGroupBox *fuelTypesGroup;
    QGroupBox *carbonGroup;
    QGroupBox *transportGroup;
    QGroupBox *shipGroup;
    QGroupBox *trainGroup;
    QGroupBox *truckGroup;

    // Apply button
    QPushButton *applyButton;

    QVBoxLayout *containerLayout;

    // Data
    QMap<QString, QMap<QString, QVariant>> fuelTypes;
    QMap<QString, QVariant>                settings;

    // Configuration
    QObject *configLoader;
};

} // namespace GUI
} // namespace CargoNetSim
