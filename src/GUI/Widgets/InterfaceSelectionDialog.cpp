#include "InterfaceSelectionDialog.h"
#include <QDialogButtonBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace CargoNetSim
{
namespace GUI
{
InterfaceSelectionDialog::InterfaceSelectionDialog(
    const QSet<QString> &availableInterfaces,
    const QSet<QString> &visibleTerminalTypes,
    QWidget             *parent)
    : QDialog(parent)
{
    setWindowTitle("Select Interfaces to Connect");
    setMinimumWidth(450);
    setMaximumWidth(500);
    setMinimumHeight(500);
    setMaximumHeight(500);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Header with descriptive label
    QLabel *label = new QLabel(
        "Select which interfaces to connect:", this);
    label->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(label);

    // Create a scrollable area for interface checkboxes
    QScrollArea *interfaceScrollArea =
        new QScrollArea(this);
    interfaceScrollArea->setWidgetResizable(true);
    interfaceScrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *interfaceScrollContent =
        new QWidget(interfaceScrollArea);
    QVBoxLayout *checkboxLayout =
        new QVBoxLayout(interfaceScrollContent);
    checkboxLayout->setContentsMargins(0, 0, 0, 0);

    // Create a checkbox for each available interface
    for (const QString &interface : availableInterfaces)
    {
        QCheckBox *checkbox =
            new QCheckBox(interface, this);
        checkbox->setChecked(true); // Default to checked
        checkboxLayout->addWidget(checkbox);
        m_checkboxes[interface] = checkbox;
    }

    // Add some spacing at the bottom of the checkbox layout
    checkboxLayout->addStretch();

    interfaceScrollArea->setWidget(interfaceScrollContent);

    // Create button layout for interface selection
    QHBoxLayout *interfaceButtonLayout = new QHBoxLayout();
    QPushButton *selectAllInterfacesBtn =
        new QPushButton("Select All", this);
    QPushButton *deselectAllInterfacesBtn =
        new QPushButton("Deselect All", this);

    interfaceButtonLayout->addWidget(
        selectAllInterfacesBtn);
    interfaceButtonLayout->addWidget(
        deselectAllInterfacesBtn);

    // Create groupbox for interfaces section
    QGroupBox *interfacesBox =
        new QGroupBox("Available Interfaces:", this);
    QVBoxLayout *interfacesBoxLayout =
        new QVBoxLayout(interfacesBox);
    interfacesBoxLayout->addWidget(interfaceScrollArea);
    interfacesBoxLayout->addLayout(interfaceButtonLayout);

    mainLayout->addWidget(interfacesBox);

    // Only add terminal types section if we have visible
    // terminal types
    if (!visibleTerminalTypes.isEmpty())
    {
        // Create terminal types section
        QGroupBox *terminalTypesBox = new QGroupBox(
            "Terminal Types to Include:", this);
        QVBoxLayout *terminalTypesLayout =
            new QVBoxLayout(terminalTypesBox);

        // Add terminal type checkboxes - only for visible
        // types
        for (const QString &terminalType :
             visibleTerminalTypes)
        {
            QCheckBox *checkbox =
                new QCheckBox(terminalType, this);
            checkbox->setChecked(
                true); // Default to checked
            terminalTypesLayout->addWidget(checkbox);
            m_terminalTypeCheckboxes[terminalType] =
                checkbox;
        }

        // Create button layout for terminal type selection
        QHBoxLayout *terminalTypeButtonLayout =
            new QHBoxLayout();
        QPushButton *selectAllTerminalTypesBtn =
            new QPushButton("Select All", this);
        QPushButton *deselectAllTerminalTypesBtn =
            new QPushButton("Deselect All", this);

        terminalTypeButtonLayout->addWidget(
            selectAllTerminalTypesBtn);
        terminalTypeButtonLayout->addWidget(
            deselectAllTerminalTypesBtn);

        terminalTypesLayout->addLayout(
            terminalTypeButtonLayout);
        mainLayout->addWidget(terminalTypesBox);

        // Connect terminal type button signals
        connect(selectAllTerminalTypesBtn,
                &QPushButton::clicked, this,
                &InterfaceSelectionDialog::
                    selectAllTerminalTypes);
        connect(deselectAllTerminalTypesBtn,
                &QPushButton::clicked, this,
                &InterfaceSelectionDialog::
                    deselectAllTerminalTypes);
    }

    // Create a 2x2 grid layout for dialog buttons
    QGridLayout *buttonGrid = new QGridLayout();
    buttonGrid->setSpacing(10);

    QPushButton *okButton = new QPushButton("OK", this);
    QPushButton *cancelButton =
        new QPushButton("Cancel", this);

    // Style the OK button to make it stand out as the
    // primary action
    okButton->setDefault(true);

    // Add buttons to the grid layout (1x2 for consistency)
    buttonGrid->addWidget(okButton, 0, 0);
    buttonGrid->addWidget(cancelButton, 0, 1);

    // Add the button grid to the main layout
    mainLayout->addLayout(buttonGrid);

    // Connect button signals
    connect(selectAllInterfacesBtn, &QPushButton::clicked,
            this,
            &InterfaceSelectionDialog::selectAllInterfaces);
    connect(
        deselectAllInterfacesBtn, &QPushButton::clicked,
        this,
        &InterfaceSelectionDialog::deselectAllInterfaces);
    connect(okButton, &QPushButton::clicked, this,
            &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this,
            &QDialog::reject);
}

InterfaceSelectionDialog::~InterfaceSelectionDialog()
{
    // QObject parent takes care of widget deletion
}

QList<QString>
InterfaceSelectionDialog::getSelectedInterfaces() const
{
    QList<QString> selectedInterfaces;
    for (auto it = m_checkboxes.constBegin();
         it != m_checkboxes.constEnd(); ++it)
    {
        if (it.value()->isChecked())
        {
            selectedInterfaces.append(it.key());
        }
    }
    return selectedInterfaces;
}

QMap<QString, bool>
InterfaceSelectionDialog::getIncludedTerminalTypes() const
{
    QMap<QString, bool> includedTypes;
    for (auto it = m_terminalTypeCheckboxes.constBegin();
         it != m_terminalTypeCheckboxes.constEnd(); ++it)
    {
        includedTypes[it.key()] = it.value()->isChecked();
    }
    return includedTypes;
}

void InterfaceSelectionDialog::selectAllInterfaces()
{
    for (auto checkbox : m_checkboxes)
    {
        checkbox->setChecked(true);
    }
}

void InterfaceSelectionDialog::deselectAllInterfaces()
{
    for (auto checkbox : m_checkboxes)
    {
        checkbox->setChecked(false);
    }
}

void InterfaceSelectionDialog::selectAllTerminalTypes()
{
    for (auto checkbox : m_terminalTypeCheckboxes)
    {
        checkbox->setChecked(true);
    }
}

void InterfaceSelectionDialog::deselectAllTerminalTypes()
{
    for (auto checkbox : m_terminalTypeCheckboxes)
    {
        checkbox->setChecked(false);
    }
}
} // namespace GUI
} // namespace CargoNetSim
