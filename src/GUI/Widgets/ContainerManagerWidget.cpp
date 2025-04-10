#include "ContainerManagerWidget.h"

#include "GUI/Controllers/UtilityFunctions.h"
#include "GUI/Items/TerminalItem.h"
#include "GUI/MainWindow.h"
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace CargoNetSim
{
namespace GUI
{

// Implementation of ContainerManagerWidget

ContainerManagerWidget::ContainerManagerWidget(
    TerminalItem *terminalItem, QWidget *parent)
    : QDialog(parent)
    , m_terminalItem(terminalItem)
{
    setWindowTitle(tr("Container Management"));
    setMinimumWidth(600);

    // Get existing containers from the terminal
    QVariant containersVar =
        terminalItem->getProperty("Containers");

    // If property exists and is a container list
    if (containersVar.canConvert<
            QList<ContainerCore::Container *>>())
    {
        m_containers =
            containersVar
                .value<QList<ContainerCore::Container *>>();
    }

    // Main layout
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Container list
    containerList = new QTableWidget;
    containerList->setColumnCount(3);
    containerList->setHorizontalHeaderLabels(
        {tr("Container ID"), tr("Size"), tr("Packages")});

    // Make columns stretch to fill the space
    containerList->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Stretch);
    containerList->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::Stretch);
    containerList->horizontalHeader()->setSectionResizeMode(
        2, QHeaderView::Stretch);

    // Populate container list
    updateContainerList();

    // Button grid (2x2)
    QGridLayout *buttonLayout = new QGridLayout;

    QPushButton *addButton =
        new QPushButton(tr("Add Container"));
    QPushButton *deleteButton =
        new QPushButton(tr("Delete Container"));
    QPushButton *editButton =
        new QPushButton(tr("Edit Properties"));
    QPushButton *generateButton =
        new QPushButton(tr("Generate Containers"));

    connect(addButton, &QPushButton::clicked, this,
            &ContainerManagerWidget::addContainer);
    connect(deleteButton, &QPushButton::clicked, this,
            &ContainerManagerWidget::deleteContainer);
    connect(editButton, &QPushButton::clicked, this,
            &ContainerManagerWidget::editContainer);
    connect(generateButton, &QPushButton::clicked, this,
            &ContainerManagerWidget::generateContainers);

    buttonLayout->addWidget(addButton, 0, 0);
    buttonLayout->addWidget(deleteButton, 0, 1);
    buttonLayout->addWidget(editButton, 1, 0);
    buttonLayout->addWidget(generateButton, 1, 1);

    // Dialog buttons (OK/Cancel)
    QDialogButtonBox *dialogButtons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(dialogButtons, &QDialogButtonBox::accepted,
            this, &QDialog::accept);
    connect(dialogButtons, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    // Add widgets to main layout
    layout->addWidget(containerList);
    layout->addLayout(buttonLayout);
    layout->addWidget(dialogButtons);
}

QList<ContainerCore::Container *>
ContainerManagerWidget::getContainers() const
{
    return m_containers;
}

QComboBox *ContainerManagerWidget::createSizeComboBox(
    ContainerCore::Container::ContainerSize currentSize)
{
    QComboBox *combo = new QComboBox();

    // Add all container sizes with their display names
    combo->addItem(tr("20ft Standard"),
                   ContainerCore::Container::twentyFT);
    combo->addItem(
        tr("20ft High Cube"),
        ContainerCore::Container::twentyFT_HighCube);
    combo->addItem(tr("40ft Standard"),
                   ContainerCore::Container::fourtyFT);
    combo->addItem(
        tr("40ft High Cube"),
        ContainerCore::Container::fourtyFT_HighCube);
    combo->addItem(tr("45ft Standard"),
                   ContainerCore::Container::fortyFiveFT);
    combo->addItem(
        tr("45ft High Cube"),
        ContainerCore::Container::fortyFiveFT_HighCube);
    combo->addItem(tr("10ft Standard"),
                   ContainerCore::Container::tenFT);
    combo->addItem(tr("30ft Standard"),
                   ContainerCore::Container::thirtyFT);
    combo->addItem(tr("48ft Standard"),
                   ContainerCore::Container::fortyEightFT);
    combo->addItem(tr("53ft Standard"),
                   ContainerCore::Container::fiftyThreeFT);
    combo->addItem(tr("60ft Standard"),
                   ContainerCore::Container::sixtyFT);

    // Set current selection
    int sizeIndex = combo->findData(currentSize);
    if (sizeIndex >= 0)
    {
        combo->setCurrentIndex(sizeIndex);
    }

    return combo;
}

void ContainerManagerWidget::onSizeChanged(QComboBox *combo)
{
    int row = containerList->indexAt(combo->pos()).row();
    if (row >= 0 && row < m_containers.size())
    {
        ContainerCore::Container::ContainerSize newSize =
            static_cast<
                ContainerCore::Container::ContainerSize>(
                combo->currentData().toInt());
        m_containers[row]->setContainerSize(newSize);
    }
}

void ContainerManagerWidget::updateContainerList()
{
    containerList->setRowCount(m_containers.size());

    for (int row = 0; row < m_containers.size(); ++row)
    {
        ContainerCore::Container *container =
            m_containers[row];

        // Container ID
        QTableWidgetItem *idItem = new QTableWidgetItem(
            container->getContainerID());
        containerList->setItem(row, 0, idItem);

        // Size (ComboBox)
        QComboBox *sizeCombo = createSizeComboBox(
            container->getContainerSize());
        connect(sizeCombo,
                QOverload<int>::of(
                    &QComboBox::currentIndexChanged),
                [this, sizeCombo]() {
                    this->onSizeChanged(sizeCombo);
                });
        containerList->setCellWidget(row, 1, sizeCombo);

        // Package Count
        QTableWidgetItem *packagesItem =
            new QTableWidgetItem(QString::number(
                container->getPackages().size()));
        containerList->setItem(row, 2, packagesItem);
    }
}

void ContainerManagerWidget::addContainer()
{
    QString containerId = QString("CONT-%1").arg(
        m_containers.size() + 1, 4, 10, QChar('0'));

    // Create a new container with the terminal as parent
    ContainerCore::Container *container =
        new ContainerCore::Container(
            containerId, ContainerCore::Container::twentyFT,
            m_terminalItem);

    // Set initial location to terminal name
    container->setContainerCurrentLocation(
        m_terminalItem->getProperty("Name").toString());

    // Add to our list
    m_containers.append(container);

    // Update UI
    updateContainerList();

    // Select and edit the new container
    containerList->selectRow(m_containers.size() - 1);
    editContainer();
}

void ContainerManagerWidget::deleteContainer()
{
    int currentRow = containerList->currentRow();
    if (currentRow >= 0 && currentRow < m_containers.size())
    {
        // Ask for confirmation
        QMessageBox::StandardButton reply =
            QMessageBox::question(
                this, tr("Delete Container"),
                tr("Are you sure you want to delete this "
                   "container?"),
                QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            // Remove the container
            ContainerCore::Container *container =
                m_containers.takeAt(currentRow);
            delete container;
            updateContainerList();
        }
    }
}

void ContainerManagerWidget::editContainer()
{
    int currentRow = containerList->currentRow();
    if (currentRow >= 0 && currentRow < m_containers.size())
    {
        ContainerCore::Container *container =
            m_containers[currentRow];

        // Create a deep copy for editing
        ContainerCore::Container *editCopy =
            container->copy();

        ContainerEditDialog dialog(editCopy, this);
        if (dialog.exec() == QDialog::Accepted)
        {
            // Get the edited container
            ContainerCore::Container *editedContainer =
                dialog.getContainer(m_terminalItem);

            // Replace the old container with the edited one
            delete container;
            m_containers[currentRow] = editedContainer;
            editedContainer->setParent(m_terminalItem);

            updateContainerList();
        }
        else
        {
            // Dialog was cancelled, delete the copy
            delete editCopy;
        }
    }
}

void ContainerManagerWidget::generateContainers()
{
    GenerateContainersDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        QList<ContainerCore::Container *> newContainers =
            dialog.getGeneratedContainers(m_terminalItem);

        // Add all new containers to our list
        m_containers.append(newContainers);

        // Update UI
        updateContainerList();

        // Select the first new container
        if (!newContainers.isEmpty())
        {
            containerList->selectRow(
                m_containers.size() - newContainers.size());
        }
    }
}

// Implementation of ContainerEditDialog

ContainerEditDialog::ContainerEditDialog(
    ContainerCore::Container *container, QWidget *parent)
    : QDialog(parent)
    , m_container(container)
    , m_originalContainer(container)
{
    setWindowTitle(tr("Edit Container Properties"));
    setMinimumWidth(600);
    setMinimumHeight(500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Basic Properties Group
    QGroupBox *basicPropsGroup =
        new QGroupBox(tr("Basic Properties"));
    QFormLayout *basicPropsLayout =
        new QFormLayout(basicPropsGroup);

    // Container ID
    idEdit = new QLineEdit(container->getContainerID());
    basicPropsLayout->addRow(tr("Container ID:"), idEdit);

    // Container Size
    sizeCombo = new QComboBox();
    sizeCombo->addItem(tr("20ft Standard"),
                       ContainerCore::Container::twentyFT);
    sizeCombo->addItem(
        tr("20ft High Cube"),
        ContainerCore::Container::twentyFT_HighCube);
    sizeCombo->addItem(tr("40ft Standard"),
                       ContainerCore::Container::fourtyFT);
    sizeCombo->addItem(
        tr("40ft High Cube"),
        ContainerCore::Container::fourtyFT_HighCube);
    sizeCombo->addItem(
        tr("45ft Standard"),
        ContainerCore::Container::fortyFiveFT);
    sizeCombo->addItem(
        tr("45ft High Cube"),
        ContainerCore::Container::fortyFiveFT_HighCube);
    sizeCombo->addItem(tr("10ft Standard"),
                       ContainerCore::Container::tenFT);
    sizeCombo->addItem(tr("30ft Standard"),
                       ContainerCore::Container::thirtyFT);
    sizeCombo->addItem(
        tr("48ft Standard"),
        ContainerCore::Container::fortyEightFT);
    sizeCombo->addItem(
        tr("53ft Standard"),
        ContainerCore::Container::fiftyThreeFT);
    sizeCombo->addItem(tr("60ft Standard"),
                       ContainerCore::Container::sixtyFT);

    int sizeIndex =
        sizeCombo->findData(container->getContainerSize());
    if (sizeIndex >= 0)
    {
        sizeCombo->setCurrentIndex(sizeIndex);
    }

    basicPropsLayout->addRow(tr("Container Size:"),
                             sizeCombo);

    // Next Destinations Group
    QGroupBox *destGroup =
        new QGroupBox(tr("Next Destinations"));
    QVBoxLayout *destLayout = new QVBoxLayout(destGroup);

    destinationsList = new QListWidget();

    // Add existing destinations
    QVector<QString> destinations =
        container->getContainerNextDestinations();
    for (const QString &dest : destinations)
    {
        destinationsList->addItem(dest);
    }

    // Add buttons for destinations
    QHBoxLayout *destButtonLayout = new QHBoxLayout();
    QPushButton *addDestBtn =
        new QPushButton(tr("Add Destination"));
    QPushButton *removeDestBtn =
        new QPushButton(tr("Remove Destination"));

    connect(addDestBtn, &QPushButton::clicked, this,
            &ContainerEditDialog::addDestination);
    connect(removeDestBtn, &QPushButton::clicked, this,
            &ContainerEditDialog::removeDestination);

    destButtonLayout->addWidget(addDestBtn);
    destButtonLayout->addWidget(removeDestBtn);

    destLayout->addWidget(destinationsList);
    destLayout->addLayout(destButtonLayout);

    // Custom Variables Group
    QGroupBox *varsGroup =
        new QGroupBox(tr("Custom Variables"));
    QVBoxLayout *varsLayout = new QVBoxLayout(varsGroup);

    // Custom Variables Table
    customVarsTable = new QTableWidget();
    customVarsTable->setColumnCount(2);
    customVarsTable->setHorizontalHeaderLabels(
        {tr("Key"), tr("Value")});
    customVarsTable->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::Stretch);

    // Load initial variables for the default hauler
    loadProperties();

    // Add/Remove buttons for custom variables
    QHBoxLayout *varsButtonLayout = new QHBoxLayout();
    QPushButton *addVarBtn =
        new QPushButton(tr("Add Variable"));
    QPushButton *deleteVarBtn =
        new QPushButton(tr("Delete Variable"));

    connect(addVarBtn, &QPushButton::clicked, this,
            &ContainerEditDialog::addCustomVariable);
    connect(deleteVarBtn, &QPushButton::clicked, this,
            &ContainerEditDialog::deleteCustomVariable);

    varsButtonLayout->addWidget(addVarBtn);
    varsButtonLayout->addWidget(deleteVarBtn);

    // varsLayout->addLayout(haulerLayout);
    varsLayout->addWidget(customVarsTable);
    varsLayout->addLayout(varsButtonLayout);

    // Dialog buttons
    QDialogButtonBox *dialogButtons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(dialogButtons, &QDialogButtonBox::accepted,
            this, &QDialog::accept);
    connect(dialogButtons, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    // Add all groups to main layout
    mainLayout->addWidget(basicPropsGroup);
    mainLayout->addWidget(destGroup);
    mainLayout->addWidget(varsGroup);
    mainLayout->addWidget(dialogButtons);
}

void ContainerEditDialog::loadProperties()
{

    ContainerCore::Container::HaulerType haulerType =
        ContainerCore::Container::noHauler;

    // Get variables for this hauler
    QVariantMap variables =
        m_container->getCustomVariablesForHauler(
            haulerType);

    // Clear and populate the table
    customVarsTable->setRowCount(variables.size());
    int row = 0;

    for (auto it = variables.begin(); it != variables.end();
         ++it, ++row)
    {
        customVarsTable->setItem(
            row, 0, new QTableWidgetItem(it.key()));
        customVarsTable->setItem(
            row, 1,
            new QTableWidgetItem(it.value().toString()));
    }
}

void ContainerEditDialog::addCustomVariable()
{
    int currentRow = customVarsTable->rowCount();
    customVarsTable->setRowCount(currentRow + 1);
    customVarsTable->setItem(currentRow, 0,
                             new QTableWidgetItem(""));
    customVarsTable->setItem(currentRow, 1,
                             new QTableWidgetItem(""));
}

void ContainerEditDialog::deleteCustomVariable()
{
    int currentRow = customVarsTable->currentRow();
    if (currentRow >= 0)
    {
        customVarsTable->removeRow(currentRow);
    }
}

void ContainerEditDialog::addDestination()
{
    bool    ok;
    QString destination = QInputDialog::getText(
        this, tr("Add Destination"), tr("Destination:"),
        QLineEdit::Normal, "", &ok);
    if (ok && !destination.isEmpty())
    {
        destinationsList->addItem(destination);
    }
}

void ContainerEditDialog::removeDestination()
{
    QListWidgetItem *current =
        destinationsList->currentItem();
    if (current)
    {
        delete destinationsList->takeItem(
            destinationsList->row(current));
    }
}

ContainerCore::Container *ContainerEditDialog::getContainer(
    TerminalItem *parent) const
{
    // Apply all changes to the container

    // Basic properties
    m_container->setContainerID(idEdit->text());
    m_container->setContainerSize(
        static_cast<
            ContainerCore::Container::ContainerSize>(
            sizeCombo->currentData().toInt()));
    m_container->setContainerCurrentLocation(
        parent->getID());

    // Next destinations
    QVector<QString> destinations;
    for (int i = 0; i < destinationsList->count(); ++i)
    {
        destinations.append(
            destinationsList->item(i)->text());
    }
    m_container->setContainerNextDestinations(destinations);

    ContainerCore::Container::HaulerType haulerType =
        ContainerCore::Container::noHauler;

    // Create new variables map
    QVariantMap variables;
    for (int row = 0; row < customVarsTable->rowCount();
         ++row)
    {
        QTableWidgetItem *keyItem =
            customVarsTable->item(row, 0);
        QTableWidgetItem *valueItem =
            customVarsTable->item(row, 1);

        if (keyItem && valueItem
            && !keyItem->text().trimmed().isEmpty())
        {
            variables[keyItem->text().trimmed()] =
                valueItem->text().trimmed();
        }
    }

    // Replace variables for this hauler type
    QMap<ContainerCore::Container::HaulerType, QVariantMap>
        allVariables = m_container->getCustomVariables();
    allVariables[haulerType] = variables;
    m_container->setCustomVariables(allVariables);

    return m_container;
}

// Implementation of GenerateContainersDialog

GenerateContainersDialog::GenerateContainersDialog(
    QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Generate Containers"));
    setMinimumWidth(300);

    QVBoxLayout *layout = new QVBoxLayout(this);

    // Number of containers input
    QHBoxLayout *numberLayout = new QHBoxLayout();
    QLabel      *numberLabel =
        new QLabel(tr("Number of Containers:"));
    numberSpin = new QSpinBox();
    numberSpin->setMinimum(1);
    numberSpin->setMaximum(1000);
    numberLayout->addWidget(numberLabel);
    numberLayout->addWidget(numberSpin);

    // Size selection
    QHBoxLayout *sizeLayout = new QHBoxLayout();
    QLabel *sizeLabel = new QLabel(tr("Container Size:"));
    sizeCombo         = new QComboBox();

    // Add container sizes
    sizeCombo->addItem(tr("20ft Standard"),
                       ContainerCore::Container::twentyFT);
    sizeCombo->addItem(
        tr("20ft High Cube"),
        ContainerCore::Container::twentyFT_HighCube);
    sizeCombo->addItem(tr("40ft Standard"),
                       ContainerCore::Container::fourtyFT);
    sizeCombo->addItem(
        tr("40ft High Cube"),
        ContainerCore::Container::fourtyFT_HighCube);
    sizeCombo->addItem(
        tr("45ft Standard"),
        ContainerCore::Container::fortyFiveFT);
    sizeCombo->addItem(
        tr("45ft High Cube"),
        ContainerCore::Container::fortyFiveFT_HighCube);
    sizeCombo->addItem(tr("10ft Standard"),
                       ContainerCore::Container::tenFT);
    sizeCombo->addItem(tr("30ft Standard"),
                       ContainerCore::Container::thirtyFT);
    sizeCombo->addItem(
        tr("48ft Standard"),
        ContainerCore::Container::fortyEightFT);
    sizeCombo->addItem(
        tr("53ft Standard"),
        ContainerCore::Container::fiftyThreeFT);
    sizeCombo->addItem(tr("60ft Standard"),
                       ContainerCore::Container::sixtyFT);

    sizeLayout->addWidget(sizeLabel);
    sizeLayout->addWidget(sizeCombo);

    // Dialog buttons
    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this,
            &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this,
            &QDialog::reject);

    // Add all layouts
    layout->addLayout(numberLayout);
    layout->addLayout(sizeLayout);
    // layout->addLayout(locationLayout);
    layout->addWidget(buttons);
}

QList<ContainerCore::Container *>
GenerateContainersDialog::getGeneratedContainers(
    TerminalItem *parent) const
{
    QList<ContainerCore::Container *> containers;
    int count = numberSpin->value();
    ContainerCore::Container::ContainerSize size =
        static_cast<
            ContainerCore::Container::ContainerSize>(
            sizeCombo->currentData().toInt());

    if (!parent)
    {
        return QList<ContainerCore::Container *>();
    }
    QString location = parent->getID();

    for (int i = 0; i < count; ++i)
    {
        QString containerId = QString("CONT-%1").arg(
            i + 1, 4, 10, QChar('0'));

        // Create new container
        ContainerCore::Container *container =
            new ContainerCore::Container(containerId, size,
                                         parent);

        // Set location if provided
        if (!location.isEmpty())
        {
            container->setContainerCurrentLocation(
                location);
        }

        containers.append(container);
    }

    return containers;
}

} // namespace GUI
} // namespace CargoNetSim
