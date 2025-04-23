// NetworkSelectionDialog.cpp
#include "NetworkSelectionDialog.h"
#include <QLabel>
#include <QMessageBox>

namespace CargoNetSim
{
namespace GUI
{

NetworkSelectionDialog::NetworkSelectionDialog(
    QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Select Network Types to Link");
    setMinimumWidth(350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Add description label
    QLabel *descriptionLabel = new QLabel(
        "Select the network type(s) to link terminals to:",
        this);
    descriptionLabel->setWordWrap(true);
    mainLayout->addWidget(descriptionLabel);

    // Add network type checkboxes
    trainNetworkCheckBox =
        new QCheckBox("Train Network", this);
    truckNetworkCheckBox =
        new QCheckBox("Truck Network", this);

    mainLayout->addWidget(trainNetworkCheckBox);
    mainLayout->addWidget(truckNetworkCheckBox);

    // Create button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // Create custom buttons
    linkSelectedButton =
        new QPushButton("Link Selected Terminals", this);
    linkAllVisibleButton =
        new QPushButton("Link All Visible Terminals", this);
    cancelButton = new QPushButton("Cancel", this);

    // Add buttons to layout
    buttonLayout->addWidget(linkSelectedButton);
    buttonLayout->addWidget(linkAllVisibleButton);
    buttonLayout->addWidget(cancelButton);

    // Add button layout to main layout
    mainLayout->addLayout(buttonLayout);

    // Disable buttons initially
    linkSelectedButton->setEnabled(false);
    linkAllVisibleButton->setEnabled(false);

    // Connect signals
    connect(
        trainNetworkCheckBox, &QCheckBox::stateChanged,
        this,
        &NetworkSelectionDialog::onCheckBoxStateChanged);
    connect(
        truckNetworkCheckBox, &QCheckBox::stateChanged,
        this,
        &NetworkSelectionDialog::onCheckBoxStateChanged);
    connect(linkSelectedButton, &QPushButton::clicked, this,
            &QDialog::accept);
    connect(linkAllVisibleButton, &QPushButton::clicked,
            [this]() {
                // Set a result code to distinguish between
                // link selected and link all
                this->done(QDialog::Accepted + 1);
            });
    connect(cancelButton, &QPushButton::clicked, this,
            &QDialog::reject);

    setLayout(mainLayout);
}

QList<NetworkType>
NetworkSelectionDialog::getSelectedNetworkTypes() const
{
    QList<NetworkType> types;

    if (trainNetworkCheckBox->isChecked())
    {
        types.append(NetworkType::Train);
    }

    if (truckNetworkCheckBox->isChecked())
    {
        types.append(NetworkType::Truck);
    }

    return types;
}

void NetworkSelectionDialog::onCheckBoxStateChanged()
{
    // Enable buttons only if at least one network type is
    // selected
    bool enable = trainNetworkCheckBox->isChecked()
                  || truckNetworkCheckBox->isChecked();
    linkSelectedButton->setEnabled(enable);
    linkAllVisibleButton->setEnabled(enable);
}

} // namespace GUI
} // namespace CargoNetSim
