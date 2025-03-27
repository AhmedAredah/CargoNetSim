#include "NetworkManagerDialog.h"

#include <QAction>
#include <QColorDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QListWidget>
#include <QMessageBox>
#include <QPainter>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include "../Controllers/NetworkController.h"
#include "../Items/MapLine.h"
#include "../Items/MapPoint.h"
#include "../MainWindow.h"
#include "../Utils/ColorUtils.h"
#include "../Widgets/ColorPickerDialog.h"
#include "Backend/Controllers/CargoNetSimController.h"

// External services - these would be properly included in
// your project #include
// "service_clients/network_handlers/train_network/train_network_manager.h"
// #include
// "service_clients/network_handlers/truck_network/truck_network_manager.h"

namespace CargoNetSim
{
namespace GUI
{

NetworkManagerDialog::NetworkManagerDialog(QWidget *parent)
    : QDockWidget("Network Manager", parent)
{
    mainWindow = qobject_cast<MainWindow *>(parent);
    if (!mainWindow)
    {
        // Fallback if not directly parented to MainWindow
        mainWindow = qobject_cast<MainWindow *>(parent->window());
    }

    setObjectName("NetworkManagerDock");

    // Create main widget and layout
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);

    // Create tab widget
    QTabWidget *tabWidget = new QTabWidget(mainWidget);

    // Create NeTrainSim tab
    QWidget *netrainsimTab = createNetworkTab("Train Network");
    tabWidget->addTab(netrainsimTab, "Train Network");

    // Create INTEGRATION tab
    QWidget *integrationTab = createNetworkTab("Truck Network");
    tabWidget->addTab(integrationTab, "Truck Network");

    mainLayout->addWidget(tabWidget);
    setWidget(mainWidget);

    // Initialize network registries
    updateNetworkList("Train Network");
    updateNetworkList("Truck Network");

    // Connect to region change signals if MainWindow provides them
    if (mainWindow)
    {
        connect(mainWindow, &MainWindow::regionChanged,
                this, [this](const QString &region) {
                    updateNetworkListForChangedRegion(region);
                });
    }
}

QWidget *NetworkManagerDialog::createNetworkTab(
    const QString &networkType)
{
    QWidget     *tab    = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);

    // Create network list
    QListWidget *listWidget = new QListWidget(tab);
    listWidget->setObjectName(
        networkType.toLower().replace(" ", "_") + "_list");

    // Connect selection changed signal
    connect(listWidget, &QListWidget::itemSelectionChanged,
            this, [this, networkType]() {
                onSelectionChanged(networkType);
            });
    layout->addWidget(listWidget);

    // Create grid layout for buttons (2x2)
    QGridLayout *buttonLayout = new QGridLayout();
    buttonLayout->setColumnStretch(
        0, 1); // Make columns stretch equally
    buttonLayout->setColumnStretch(1, 1);

    // Add network button
    QPushButton *addButton = new QPushButton("Add", tab);
    connect(
        addButton, &QPushButton::clicked, this,
        [this, networkType]() { addNetwork(networkType); });
    buttonLayout->addWidget(addButton, 0,
                            0); // Row 0, Column 0

    // Rename network button
    QPushButton *renameButton =
        new QPushButton("Rename", tab);
    renameButton->setEnabled(false); // Initially disabled
    connect(renameButton, &QPushButton::clicked, this,
            [this, networkType]() {
                renameNetwork(networkType);
            });
    buttonLayout->addWidget(renameButton, 0,
                            1); // Row 0, Column 1

    // Delete network button
    QPushButton *deleteButton =
        new QPushButton("Delete", tab);
    deleteButton->setEnabled(false); // Initially disabled
    connect(deleteButton, &QPushButton::clicked, this,
            [this, networkType]() {
                deleteNetwork(networkType);
            });
    buttonLayout->addWidget(deleteButton, 1,
                            0); // Row 1, Column 0

    // Change color button
    QPushButton *colorButton =
        new QPushButton("Change Color", tab);
    colorButton->setEnabled(false); // Initially disabled
    connect(colorButton, &QPushButton::clicked, this,
            [this, networkType]() {
                changeNetworkColor(networkType);
            });
    buttonLayout->addWidget(colorButton, 1,
                            1); // Row 1, Column 1

    // Store button references
    QMap<QString, QPushButton *> buttons;
    buttons["rename"]           = renameButton;
    buttons["delete"]           = deleteButton;
    buttons["color"]            = colorButton;
    networkButtons[networkType] = buttons;

    layout->addLayout(buttonLayout);
    return tab;
}

void NetworkManagerDialog::onSelectionChanged(
    const QString &networkType)
{
    QListWidget *listWidget = findChild<QListWidget *>(
        networkType.toLower().replace(" ", "_") + "_list");

    if (!listWidget)
        return;

    bool hasSelection =
        listWidget->currentItem() != nullptr;

    // Get the appropriate button dictionary
    QMap<QString, QPushButton *> buttons =
        networkButtons[networkType];

    // Enable/disable buttons based on selection
    for (auto it = buttons.begin(); it != buttons.end();
         ++it)
    {
        it.value()->setEnabled(hasSelection);
    }
}

void NetworkManagerDialog::addNetwork(
    const QString &networkType)
{
    if (!mainWindow)
        return;

    Backend::RegionData *regionData =
        CargoNetSim::CargoNetSimController::getInstance()
            .getRegionDataController()
            ->getCurrentRegionData();

    if (!regionData)
        return;

    // Map network types to their corresponding methods
    QStringList networkNames;
    if (networkType == "Train Network")
    {
        networkNames = regionData->getTrainNetworks();
    }
    else if (networkType == "Truck Network")
    {
        networkNames = regionData->getTruckNetworks();
    }
    else
    {
        return;
    }

    // Check for existing network
    if (!networkNames.isEmpty())
    {
        QMessageBox::warning(
            this, "Warning",
            QString("One %1 is allowed for region '%2'")
                .arg(networkType.toLower())
                .arg(regionData->getRegion()));
        return;
    }

    // Import the new network
    try
    {
        QString networkName;

        if (networkType == "Train Network")
        {
            networkName = NetworkController::importNetwork(
                mainWindow, NetworkType::Train, regionData);
        }
        else
        {
            networkName = NetworkController::importNetwork(
                mainWindow, NetworkType::Truck, regionData);
        }

        updateNetworkList(networkType);
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(
            this, "Error",
            QString("Failed to import %1: %2")
                .arg(networkType.toLower())
                .arg(e.what()));
    }
}

void NetworkManagerDialog::deleteNetwork(
    const QString &networkType)
{
    QListWidget *listWidget = findChild<QListWidget *>(
        networkType.toLower().replace(" ", "_") + "_list");

    if (!listWidget || !mainWindow)
        return;

    QListWidgetItem *currentItem =
        listWidget->currentItem();

    if (!currentItem)
    {
        QMessageBox::warning(
            this, "Warning",
            "Please select a network to delete.");
        return;
    }

    QString networkName = currentItem->text();

    QMessageBox::StandardButton reply =
        QMessageBox::question(
            this, "Confirm Delete",
            QString("Are you sure you want to delete the "
                    "network '%1'?")
                .arg(networkName),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        try
        {
            Backend::RegionData *regionData =
                CargoNetSim::CargoNetSimController::
                    getInstance()
                        .getRegionDataController()
                        ->getCurrentRegionData();

            NetworkType type =
                (networkType == "Train Network")
                    ? NetworkType::Train
                    : NetworkType::Truck;
            NetworkController::removeNetwork(
                mainWindow, type, networkName, regionData);

            updateNetworkList(networkType);
        }
        catch (const std::exception &e)
        {
            QMessageBox::critical(
                this, "Error",
                QString("Failed to delete network: %1")
                    .arg(e.what()));
        }
    }
}

void NetworkManagerDialog::renameNetwork(
    const QString &networkType)
{
    QListWidget *listWidget = findChild<QListWidget *>(
        networkType.toLower().replace(" ", "_") + "_list");

    if (!listWidget || !mainWindow)
        return;

    QListWidgetItem *currentItem =
        listWidget->currentItem();

    if (!currentItem)
    {
        QMessageBox::warning(
            this, "Warning",
            "Please select a network to rename.");
        return;
    }

    QString              oldName = currentItem->text();
    Backend::RegionData *regionData =
        CargoNetSim::CargoNetSimController::getInstance()
            .getRegionDataController()
            ->getRegionData(
                CargoNetSim::CargoNetSimController::
                    getInstance()
                        .getRegionDataController()
                        ->getCurrentRegion());

    while (true)
    {
        bool    ok;
        QString newName = QInputDialog::getText(
            this, "Rename Network",
            "Enter new network name:", QLineEdit::Normal,
            oldName, &ok);

        if (!ok || newName.isEmpty())
        {
            return;
        }

        QString prefix = (networkType == "Train Network")
                             ? "train_"
                             : "truck_";
        QString oldNameStore = prefix + oldName;
        QString newNameStore = prefix + newName;

        // Check if network exists using appropriate method
        bool networkExists = false;
        if (networkType == "Train Network")
        {
            networkExists = regionData->trainNetworkExists(
                newNameStore);
        }
        else
        {
            networkExists = regionData->truckNetworkExists(
                newNameStore);
        }

        if (networkExists && newNameStore != oldNameStore)
        {
            QMessageBox::warning(
                this, "Name Already Exists",
                QString(
                    "A network named '%1' already exists. "
                    "Please choose a different name.")
                    .arg(newName));
            continue;
        }

        try
        {
            // Call appropriate rename method
            if (networkType == "Train Network")
            {
                regionData->renameTrainNetwork(
                    oldNameStore, newNameStore);
            }
            else
            {
                regionData->renameTruckNetwork(
                    oldNameStore, newNameStore);
            }

            updateNetworkList(networkType);

            // TODO
            // Rename all scene items (MapPoints and
            // MapLines) groups if (mainWindow->getScene())
            // {
            //     QGraphicsScene* scene =
            //     mainWindow->getScene(); for
            //     (QGraphicsItem* item : scene->items()) {
            //         if (MapPoint* mapPoint =
            //         dynamic_cast<MapPoint*>(item)) {
            //             if (mapPoint->getGroup() ==
            //             oldNameStore) {
            //                 mapPoint->setProperty("group",
            //                 newNameStore);
            //                 mapPoint->setRegion(newNameStore);
            //             }
            //         } else if (MapLine* mapLine =
            //         dynamic_cast<MapLine*>(item)) {
            //             if (mapLine->getGroup() ==
            //             oldNameStore) {
            //                 mapLine->setProperty("group",
            //                 newNameStore);
            //                 mapLine->setRegion(newNameStore);
            //             }
            //         }
            //     }
            // }
            break;
        }
        catch (const std::exception &e)
        {
            QMessageBox::critical(
                this, "Error",
                QString("Failed to rename network: %1")
                    .arg(e.what()));
            return;
        }
    }
}

void NetworkManagerDialog::changeNetworkColor(
    const QString &networkType)
{
    QListWidget *listWidget = findChild<QListWidget *>(
        networkType.toLower().replace(" ", "_") + "_list");

    if (!listWidget)
        return;

    QListWidgetItem *currentItem =
        listWidget->currentItem();

    if (!currentItem)
    {
        QMessageBox::warning(
            this, "Warning",
            "Please select a network to change color.");
        return;
    }

    ColorPickerDialog colorDialog(nullptr, this);
    if (!colorDialog.exec())
    {
        return;
    }

    QColor newColor = colorDialog.getSelectedColor();
    if (!newColor.isValid())
    {
        return;
    }

    QString networkName = currentItem->text();

    // Store color in network properties based on network
    // type if (networkType == "Train Network") {
    //     networkName =
    //     QString("train_%1").arg(networkName); auto*
    //     network =
    //     get_NeTrainSim_network_registry().getNetwork(networkName);
    //     if (network) {
    //         network->addVariable("color",
    //         newColor.name());
    //     }
    // } else {
    //     networkName =
    //     QString("truck_%1").arg(networkName); auto*
    //     network =
    //     get_INTEGRATION_network_registry().getConfig(networkName);
    //     if (network) {
    //         network->addVariable("color",
    //         newColor.name());
    //     }
    // }

    // Update the color pixmap for the list item
    currentItem->setIcon(
        QIcon(createColorPixmap(newColor)));

    // TODO
    // // Update color of all network items in the scene
    // if (mainWindow && mainWindow->getScene()) {
    //     QGraphicsScene* scene = mainWindow->getScene();
    //     QColor darkerColor = newColor.darker(150);  //
    //     150% darker for MapPoints

    //     for (QGraphicsItem* item : scene->items()) {
    //         if (MapLine* mapLine =
    //         dynamic_cast<MapLine*>(item)) {
    //             if (mapLine->getGroup() == networkName) {
    //                 mapLine->setPen(QPen(newColor, 1));
    //                 mapLine->update();
    //             }
    //         } else if (MapPoint* mapPoint =
    //         dynamic_cast<MapPoint*>(item)) {
    //             if (mapPoint->getGroup() == networkName)
    //             {
    //                 mapPoint->setColor(darkerColor);
    //                 mapPoint->update();
    //             }
    //         }
    //     }
    // }
}

void NetworkManagerDialog::updateNetworkList(
    const QString &networkType)
{
    QListWidget *listWidget = findChild<QListWidget *>(
        networkType.toLower().replace(" ", "_") + "_list");

    if (!listWidget)
        return;

    // Store current checkbox states before clearing
    QMap<QString, Qt::CheckState> checkboxStates;
    for (int i = 0; i < listWidget->count(); ++i)
    {
        QListWidgetItem *item        = listWidget->item(i);
        checkboxStates[item->text()] = item->checkState();
    }
    listWidget->clear();

    // Get registry and functions based on network type
    QStringList networkNames;
    QString     prefix;
    // if (networkType == "Train Network") {
    //     auto& registry =
    //     get_NeTrainSim_network_registry(); networkNames =
    //     registry.listNetworks(); prefix = "train_";
    // } else {
    //     auto& registry =
    //     get_INTEGRATION_network_registry(); networkNames
    //     = registry.listConfigs(); prefix = "truck_";
    // }

    // Get the current region
    QString currentRegion;
    if (mainWindow)
    {
        currentRegion = CargoNetSim::CargoNetSimController::
                            getInstance()
                                .getRegionDataController()
                                ->getCurrentRegion();
    }

    // Add network items to list
    for (const QString &networkName : networkNames)
    {
        QColor  color;
        QString region;

        // Get network object and region
        // if (networkType == "Train Network") {
        //     auto* network =
        //     get_NeTrainSim_network_registry().getNetwork(networkName);
        //     if (network) {
        //         region = network->getVariable("region");
        //         color =
        //         QColor(network->getVariable("color"));
        //     }
        // } else {
        //     auto* config =
        //     get_INTEGRATION_network_registry().getConfig(networkName);
        //     if (config) {
        //         region = config->getVariable("region");
        //         color =
        //         QColor(config->getVariable("color"));
        //     }
        // }

        // Only show networks for the current region
        if (region != currentRegion)
        {
            continue;
        }

        // Create list item
        QString displayName = networkName;
        if (displayName.startsWith(prefix))
        {
            displayName = displayName.mid(prefix.length());
        }

        QListWidgetItem *item =
            new QListWidgetItem(displayName);
        item->setFlags(item->flags()
                       | Qt::ItemIsUserCheckable);

        // Restore previous check state or default to
        // checked
        Qt::CheckState checkState =
            checkboxStates.value(displayName, Qt::Checked);
        item->setCheckState(checkState);

        // Set color icon
        if (color.isValid())
        {
            item->setIcon(QIcon(createColorPixmap(color)));
        }

        listWidget->addItem(item);
    }

    // Connect the item changed signal (only connect once)
    listWidget->disconnect(
        SIGNAL(itemChanged(QListWidgetItem *)));
    connect(listWidget, &QListWidget::itemChanged, this,
            [this, networkType](QListWidgetItem *item) {
                onItemCheckedChanged(item, networkType);
            });
}

void NetworkManagerDialog::
    updateNetworkListForChangedRegion(
        const QString &regionName)
{
    updateNetworkList("Train Network");
    updateNetworkList("Truck Network");
}

void NetworkManagerDialog::onItemCheckedChanged(
    QListWidgetItem *item, const QString &networkType)
{
    if (!item || !mainWindow)
        return;

    QString networkName = item->text();
    bool    isVisible   = item->checkState() == Qt::Checked;

    // Add prefix based on network type
    if (networkType == "Train Network")
    {
        if (!networkName.startsWith("train_"))
        {
            networkName = "train_" + networkName;
        }
    }
    else if (networkType == "Truck Network")
    {
        if (!networkName.startsWith("truck_"))
        {
            networkName = "truck_" + networkName;
        }
    }

    // TODO
    // Update visibility of scene items
    // if (mainWindow->getScene()) {
    //     QGraphicsScene* scene = mainWindow->getScene();
    //     for (QGraphicsItem* item : scene->items()) {
    //         if (MapPoint* mapPoint =
    //         dynamic_cast<MapPoint*>(item)) {
    //             if (mapPoint->getGroup() == networkName)
    //             {
    //                 mapPoint->setVisible(isVisible);
    //             }
    //         } else if (MapLine* mapLine =
    //         dynamic_cast<MapLine*>(item)) {
    //             if (mapLine->getGroup() == networkName) {
    //                 mapLine->setVisible(isVisible);
    //             }
    //         }
    //     }
    // }
}

QPixmap
NetworkManagerDialog::createColorPixmap(const QColor &color,
                                        int           size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(QBrush(color));
    painter.drawRect(0, 0, size - 1, size - 1);
    painter.end();

    return pixmap;
}

void NetworkManagerDialog::clear()
{
    // Clear both list widgets
    QListWidget *trainList =
        findChild<QListWidget *>("train_network_list");
    QListWidget *truckList =
        findChild<QListWidget *>("truck_network_list");

    if (trainList)
    {
        trainList->clear();
        // Disconnect any existing signals
        trainList->disconnect(
            SIGNAL(itemChanged(QListWidgetItem *)));
    }

    if (truckList)
    {
        truckList->clear();
        truckList->disconnect(
            SIGNAL(itemChanged(QListWidgetItem *)));
    }

    // Remove all networks from canvas
    // if (mainWindow) {
    //     for (const QString& networkName :
    //     get_NeTrainSim_network_registry().listNetworks())
    //     {
    //         NetworkController::removeFromCanvas(mainWindow,
    //         networkName);
    //     }

    //     for (const QString& networkName :
    //     get_INTEGRATION_network_registry().listConfigs())
    //     {
    //         NetworkController::removeFromCanvas(mainWindow,
    //         networkName);
    //     }
    // }
}

} // namespace GUI
} // namespace CargoNetSim
