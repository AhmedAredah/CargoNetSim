#pragma once

#include <QComboBox>
#include <QDialog>
#include <QMap>
#include <QSpinBox>
#include <QTableWidget>
#include <QVariant>
#include <memory>

namespace CargoNetSim
{
namespace GUI
{

/**
 * @brief Enum representing standard container sizes
 */
enum class ContainerSize
{
    TwentyFT,
    TwentyFT_HighCube,
    FourtyFT,
    FourtyFT_HighCube,
    FortyFiveFT,
    FortyFiveFT_HighCube,
    TenFT,
    ThirtyFT,
    FortyEightFT,
    FiftyThreeFT,
    SixtyFT
};

/**
 * @brief Widget for managing container data
 *
 * ContainerManagerWidget provides a dialog for viewing,
 * adding, editing, and deleting containers associated with
 * a terminal.
 */
class ContainerManagerWidget : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param containers Map of container data to edit
     * @param parent Parent widget
     */
    explicit ContainerManagerWidget(
        const QMap<QString, QVariant> &containers,
        QWidget                       *parent = nullptr);

    /**
     * @brief Get the modified containers data
     * @return Updated container data map
     */
    QMap<QString, QVariant> getContainers() const;

    /**
     * @brief Convert custom ContainerSize enum to
     * containerlib size enum
     * @param customSize Size value from ContainerSize enum
     * @return Corresponding containerlib size value
     */
    static int
    convertToContainerlibSize(ContainerSize customSize);

    /**
     * @brief Convert string representation of container
     * size to enum value
     * @param sizeStr String representation of container
     * size
     * @return Corresponding ContainerSize enum value
     */
    static ContainerSize
    containerSizeFromString(const QString &sizeStr);

    /**
     * @brief Convert ContainerSize enum to string
     * representation
     * @param size ContainerSize enum value
     * @return String representation of container size
     */
    static QString
    containerSizeToString(ContainerSize size);

private slots:
    /**
     * @brief Add a new container to the list
     */
    void addContainer();

    /**
     * @brief Delete selected container from the list
     */
    void deleteContainer();

    /**
     * @brief Edit properties of selected container
     */
    void editContainer();

    /**
     * @brief Generate multiple containers with same
     * properties
     */
    void generateContainers();

private:
    /**
     * @brief Create a combo box for container size
     * selection
     * @param currentSize Current container size
     * @return Configured combo box
     */
    QComboBox *
    createSizeComboBox(const QString &currentSize);

    /**
     * @brief Handle container size change in the UI
     * @param combo Combo box that changed
     */
    void onSizeChanged(QComboBox *combo);

    /**
     * @brief Update the container list UI
     */
    void updateContainerList();

    QTableWidget
        *containerList; ///< Table widget for container list
    QMap<QString, QVariant> containers; ///< Container data
};

/**
 * @brief Dialog for editing container properties
 */
class ContainerEditDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param containerData Container data to edit
     * @param parent Parent widget
     */
    explicit ContainerEditDialog(
        const QMap<QString, QVariant> &containerData =
            QMap<QString, QVariant>(),
        QWidget *parent = nullptr);

    /**
     * @brief Get the modified container data
     * @return Updated container data
     */
    QMap<QString, QVariant> getContainerData() const;

private slots:
    /**
     * @brief Add a new property to the container
     */
    void addProperty();

    /**
     * @brief Delete selected property from the container
     */
    void deleteProperty();

private:
    /**
     * @brief Load container properties into the UI
     * @param props Container properties to load
     */
    void
    loadProperties(const QMap<QString, QVariant> &props);

    QTableWidget
        *propTable; ///< Table widget for properties
};

/**
 * @brief Dialog for generating multiple containers
 */
class GenerateContainersDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit GenerateContainersDialog(
        QWidget *parent = nullptr);

    /**
     * @brief Get the generation parameters
     * @return Map containing count and size for container
     * generation
     */
    QMap<QString, QVariant> getGenerationData() const;

private:
    QSpinBox  *numberSpin; ///< Spin box for container count
    QComboBox *sizeCombo;  ///< Combo box for container size
};

} // namespace GUI
} // namespace CargoNetSim
