#pragma once

#include <QComboBox>
#include <QDialog>
#include <QSpinBox>
#include <QTableWidget>
#include <QtWidgets/qlistwidget.h>
#include <containerLib/container.h>
#include <memory>

namespace CargoNetSim
{
namespace GUI
{

class TerminalItem;

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
     * @param terminalItem The terminal that owns the
     * containers
     * @param parent Parent widget
     */
    explicit ContainerManagerWidget(
        TerminalItem *terminalItem,
        QWidget      *parent = nullptr);

    /**
     * @brief Get the modified containers
     * @return List of container objects
     */
    QList<ContainerCore::Container *> getContainers() const;

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
     * @brief Update the container list UI
     */
    void updateContainerList();

    /**
     * @brief Create a combo box for container size
     * selection
     * @param currentSize Current container size value
     * @return Configured combo box
     */
    QComboBox *createSizeComboBox(
        ContainerCore::Container::ContainerSize
            currentSize);

    /**
     * @brief Handle container size change in the UI
     * @param combo Combo box that changed
     */
    void onSizeChanged(QComboBox *combo);

    QTableWidget
        *containerList; ///< Table widget for container list
    QList<ContainerCore::Container *>
                  m_containers;   ///< Container data
    TerminalItem *m_terminalItem; ///< The terminal that
                                  ///< owns these containers
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
     * @param container Container to edit
     * @param parent Parent widget
     */
    explicit ContainerEditDialog(
        ContainerCore::Container *container,
        QWidget                  *parent = nullptr);

    /**
     * @brief Get the container with modified data
     * @param parent Parent Item (TerminalItem)
     * @return Container with updated properties
     */
    ContainerCore::Container *
    getContainer(TerminalItem *parent) const;

private slots:
    /**
     * @brief Add a new custom variable to the container
     */
    void addCustomVariable();

    /**
     * @brief Delete selected custom variable from the
     * container
     */
    void deleteCustomVariable();

    /**
     * @brief Add a new destination to the container
     */
    void addDestination();

    /**
     * @brief Remove selected destination from the container
     */
    void removeDestination();

private:
    /**
     * @brief Load container properties into the UI
     */
    void loadProperties();

    QTableWidget *customVarsTable; ///< Table widget for
                                   ///< custom variables
    QListWidget
        *destinationsList; ///< List widget for destinations
    QLineEdit *idEdit;     ///< Line edit for container ID
    QComboBox *sizeCombo;  ///< Combo box for container size

    ContainerCore::Container
        *m_container; ///< Container being edited
    ContainerCore::Container
        *m_originalContainer; ///< Original container for
                              ///< reference
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
     * @brief Get the generated containers
     * @return List of newly generated containers
     */
    QList<ContainerCore::Container *>
    getGeneratedContainers(TerminalItem *parent) const;

private:
    QSpinBox  *numberSpin; ///< Spin box for container count
    QComboBox *sizeCombo;  ///< Combo box for container size
};

} // namespace GUI
} // namespace CargoNetSim
