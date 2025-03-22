#include "ContainerManagerWidget.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace CargoNetSim {
namespace GUI {

// Implementation of ContainerManagerWidget

ContainerManagerWidget::ContainerManagerWidget(
    const QMap<QString, QVariant> &containers,
    QWidget                       *parent)
    : QDialog(parent)
    , containers(containers) {
    setWindowTitle(tr("Container Management"));
    setMinimumWidth(400);

    // Main layout
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Container list
    containerList = new QTableWidget;
    containerList->setColumnCount(2);
    containerList->setHorizontalHeaderLabels(
        {tr("Container ID"), tr("Size")});

    // Make columns stretch to fill the space
    containerList->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Stretch);
    containerList->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::Stretch);

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

QMap<QString, QVariant>
ContainerManagerWidget::getContainers() const {
    return containers;
}

int ContainerManagerWidget::convertToContainerlibSize(
    ContainerSize customSize) {
    // Mapping to containerlib size constants
    // These values should match the ones defined in the
    // containerlib library
    switch (customSize) {
    case ContainerSize::TwentyFT:
        return 0;
    case ContainerSize::TwentyFT_HighCube:
        return 1;
    case ContainerSize::FourtyFT:
        return 2;
    case ContainerSize::FourtyFT_HighCube:
        return 3;
    case ContainerSize::FortyFiveFT:
        return 4;
    case ContainerSize::FortyFiveFT_HighCube:
        return 5;
    case ContainerSize::TenFT:
        return 6;
    case ContainerSize::ThirtyFT:
        return 7;
    case ContainerSize::FortyEightFT:
        return 8;
    case ContainerSize::FiftyThreeFT:
        return 9;
    case ContainerSize::SixtyFT:
        return 10;
    default:
        return 0; // Default to TwentyFT
    }
}

ContainerSize
ContainerManagerWidget::containerSizeFromString(
    const QString &sizeStr) {
    if (sizeStr == "TwentyFT")
        return ContainerSize::TwentyFT;
    if (sizeStr == "TwentyFT_HighCube")
        return ContainerSize::TwentyFT_HighCube;
    if (sizeStr == "FourtyFT")
        return ContainerSize::FourtyFT;
    if (sizeStr == "FourtyFT_HighCube")
        return ContainerSize::FourtyFT_HighCube;
    if (sizeStr == "FortyFiveFT")
        return ContainerSize::FortyFiveFT;
    if (sizeStr == "FortyFiveFT_HighCube")
        return ContainerSize::FortyFiveFT_HighCube;
    if (sizeStr == "TenFT")
        return ContainerSize::TenFT;
    if (sizeStr == "ThirtyFT")
        return ContainerSize::ThirtyFT;
    if (sizeStr == "FortyEightFT")
        return ContainerSize::FortyEightFT;
    if (sizeStr == "FiftyThreeFT")
        return ContainerSize::FiftyThreeFT;
    if (sizeStr == "SixtyFT")
        return ContainerSize::SixtyFT;
    return ContainerSize::TwentyFT; // Default
}

QString ContainerManagerWidget::containerSizeToString(
    ContainerSize size) {
    switch (size) {
    case ContainerSize::TwentyFT:
        return "TwentyFT";
    case ContainerSize::TwentyFT_HighCube:
        return "TwentyFT_HighCube";
    case ContainerSize::FourtyFT:
        return "FourtyFT";
    case ContainerSize::FourtyFT_HighCube:
        return "FourtyFT_HighCube";
    case ContainerSize::FortyFiveFT:
        return "FortyFiveFT";
    case ContainerSize::FortyFiveFT_HighCube:
        return "FortyFiveFT_HighCube";
    case ContainerSize::TenFT:
        return "TenFT";
    case ContainerSize::ThirtyFT:
        return "ThirtyFT";
    case ContainerSize::FortyEightFT:
        return "FortyEightFT";
    case ContainerSize::FiftyThreeFT:
        return "FiftyThreeFT";
    case ContainerSize::SixtyFT:
        return "SixtyFT";
    default:
        return "TwentyFT"; // Default
    }
}

QComboBox *ContainerManagerWidget::createSizeComboBox(
    const QString &currentSize) {
    QComboBox *combo = new QComboBox();

    // Add all container sizes with their display names
    combo->addItem(tr("20ft Standard"), "TwentyFT");
    combo->addItem(tr("20ft High Cube"),
                   "TwentyFT_HighCube");
    combo->addItem(tr("40ft Standard"), "FourtyFT");
    combo->addItem(tr("40ft High Cube"),
                   "FourtyFT_HighCube");
    combo->addItem(tr("45ft Standard"), "FortyFiveFT");
    combo->addItem(tr("45ft High Cube"),
                   "FortyFiveFT_HighCube");
    combo->addItem(tr("10ft Standard"), "TenFT");
    combo->addItem(tr("30ft Standard"), "ThirtyFT");
    combo->addItem(tr("48ft Standard"), "FortyEightFT");
    combo->addItem(tr("53ft Standard"), "FiftyThreeFT");
    combo->addItem(tr("60ft Standard"), "SixtyFT");

    // Set current selection
    int sizeIndex = combo->findData(currentSize);
    if (sizeIndex >= 0) {
        combo->setCurrentIndex(sizeIndex);
    }

    // Connect signal
    connect(
        combo,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this, combo]() { this->onSizeChanged(combo); });

    return combo;
}

void ContainerManagerWidget::onSizeChanged(
    QComboBox *combo) {
    int row = containerList->indexAt(combo->pos()).row();
    if (row >= 0) {
        QString containerId =
            containerList->item(row, 0)->text();
        QMap<QString, QVariant> containerData =
            containers[containerId].toMap();
        containerData["size"] =
            combo->currentData().toString();
        containers[containerId] = containerData;
    }
}

void ContainerManagerWidget::updateContainerList() {
    containerList->setRowCount(containers.size());
    int row = 0;

    for (auto it = containers.begin();
         it != containers.end(); ++it, ++row) {
        const QString          &containerId = it.key();
        QMap<QString, QVariant> containerData =
            it.value().toMap();

        // Container ID
        QTableWidgetItem *idItem =
            new QTableWidgetItem(containerId);
        containerList->setItem(row, 0, idItem);

        // Size (ComboBox)
        QString size =
            containerData.value("size", "TwentyFT")
                .toString();
        QComboBox *sizeCombo = createSizeComboBox(size);
        containerList->setCellWidget(row, 1, sizeCombo);
    }
}

void ContainerManagerWidget::addContainer() {
    QString containerId =
        QString::number(containers.size() + 1);

    QMap<QString, QVariant> containerData;
    containerData["size"] = "TwentyFT"; // Default size

    containers[containerId] = containerData;
    updateContainerList();

    // Select the new container and open properties dialog
    containerList->selectRow(containers.size() - 1);
}

void ContainerManagerWidget::editContainer() {
    int currentRow = containerList->currentRow();
    if (currentRow >= 0) {
        QString containerId =
            containerList->item(currentRow, 0)->text();
        QMap<QString, QVariant> containerData =
            containers[containerId].toMap();

        ContainerEditDialog dialog(containerData, this);
        if (dialog.exec() == QDialog::Accepted) {
            // Preserve the size while updating other
            // properties
            QString size = containerData["size"].toString();
            containerData = dialog.getContainerData();
            containerData["size"]   = size;
            containers[containerId] = containerData;
            updateContainerList();
        }
    }
}

void ContainerManagerWidget::deleteContainer() {
    int currentRow = containerList->currentRow();
    if (currentRow >= 0) {
        QString containerId =
            containerList->item(currentRow, 0)->text();
        containers.remove(containerId);
        updateContainerList();
    }
}

void ContainerManagerWidget::generateContainers() {
    GenerateContainersDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QMap<QString, QVariant> data =
            dialog.getGenerationData();
        int     count = data["count"].toInt();
        QString size  = data["size"].toString();

        int startId = containers.size() + 1;

        // Generate the containers
        for (int i = 0; i < count; ++i) {
            QString containerId =
                QString::number(startId + i);
            QMap<QString, QVariant> containerData;
            containerData["size"]   = size;
            containers[containerId] = containerData;
        }

        updateContainerList();

        // Select the first new container
        if (count > 0) {
            containerList->selectRow(startId - 1);
        }
    }
}

// Implementation of ContainerEditDialog

ContainerEditDialog::ContainerEditDialog(
    const QMap<QString, QVariant> &containerData,
    QWidget                       *parent)
    : QDialog(parent) {
    setWindowTitle(tr("Edit Container Properties"));
    setMinimumWidth(400);

    QVBoxLayout *layout = new QVBoxLayout(this);

    // Property table
    propTable = new QTableWidget();
    propTable->setColumnCount(2);
    propTable->setHorizontalHeaderLabels(
        {tr("Property"), tr("Value")});
    propTable->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);

    // Add/Delete buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addPropButton =
        new QPushButton(tr("Add Property"));
    connect(addPropButton, &QPushButton::clicked, this,
            &ContainerEditDialog::addProperty);

    QPushButton *deletePropButton =
        new QPushButton(tr("Delete Property"));
    connect(deletePropButton, &QPushButton::clicked, this,
            &ContainerEditDialog::deleteProperty);

    buttonLayout->addWidget(addPropButton);
    buttonLayout->addWidget(deletePropButton);

    // Dialog buttons
    QDialogButtonBox *dialogButtons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(dialogButtons, &QDialogButtonBox::accepted,
            this, &QDialog::accept);
    connect(dialogButtons, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    // Populate properties, excluding 'size' which is
    // handled separately
    QMap<QString, QVariant> props;
    for (auto it = containerData.begin();
         it != containerData.end(); ++it) {
        if (it.key() != "size") {
            props[it.key()] = it.value();
        }
    }
    loadProperties(props);

    // Add widgets to layout
    layout->addWidget(propTable);
    layout->addLayout(buttonLayout);
    layout->addWidget(dialogButtons);
}

void ContainerEditDialog::loadProperties(
    const QMap<QString, QVariant> &props) {
    propTable->setRowCount(props.size());
    int row = 0;

    for (auto it = props.begin(); it != props.end();
         ++it, ++row) {
        propTable->setItem(row, 0,
                           new QTableWidgetItem(it.key()));
        propTable->setItem(
            row, 1,
            new QTableWidgetItem(it.value().toString()));
    }
}

void ContainerEditDialog::addProperty() {
    int currentRow = propTable->rowCount();
    propTable->setRowCount(currentRow + 1);
    propTable->setItem(currentRow, 0,
                       new QTableWidgetItem(""));
    propTable->setItem(currentRow, 1,
                       new QTableWidgetItem(""));
}

void ContainerEditDialog::deleteProperty() {
    int currentRow = propTable->currentRow();
    if (currentRow >= 0) {
        propTable->removeRow(currentRow);
    }
}

QMap<QString, QVariant>
ContainerEditDialog::getContainerData() const {
    QMap<QString, QVariant> data;

    for (int row = 0; row < propTable->rowCount(); ++row) {
        QTableWidgetItem *keyItem = propTable->item(row, 0);
        QTableWidgetItem *valueItem =
            propTable->item(row, 1);

        if (keyItem && valueItem
            && !keyItem->text().trimmed().isEmpty()) {
            data[keyItem->text().trimmed()] =
                valueItem->text().trimmed();
        }
    }

    return data;
}

// Implementation of GenerateContainersDialog

GenerateContainersDialog::GenerateContainersDialog(
    QWidget *parent)
    : QDialog(parent) {
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
    sizeCombo->addItem(tr("20ft Standard"), "TwentyFT");
    sizeCombo->addItem(tr("20ft High Cube"),
                       "TwentyFT_HighCube");
    sizeCombo->addItem(tr("40ft Standard"), "FourtyFT");
    sizeCombo->addItem(tr("40ft High Cube"),
                       "FourtyFT_HighCube");
    sizeCombo->addItem(tr("45ft Standard"), "FortyFiveFT");
    sizeCombo->addItem(tr("45ft High Cube"),
                       "FortyFiveFT_HighCube");
    sizeCombo->addItem(tr("10ft Standard"), "TenFT");
    sizeCombo->addItem(tr("30ft Standard"), "ThirtyFT");
    sizeCombo->addItem(tr("48ft Standard"), "FortyEightFT");
    sizeCombo->addItem(tr("53ft Standard"), "FiftyThreeFT");
    sizeCombo->addItem(tr("60ft Standard"), "SixtyFT");

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
    layout->addWidget(buttons);
}

QMap<QString, QVariant>
GenerateContainersDialog::getGenerationData() const {
    QMap<QString, QVariant> data;
    data["count"] = numberSpin->value();
    data["size"]  = sizeCombo->currentData().toString();
    return data;
}

} // namespace GUI
} // namespace CargoNetSim
