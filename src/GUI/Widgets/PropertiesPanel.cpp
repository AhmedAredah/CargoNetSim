#include "PropertiesPanel.h"
#include "../Controllers/ViewController.h"
#include "../Items/BackgroundPhotoItem.h"
#include "../Items/ConnectionLine.h"
#include "../Items/MapPoint.h"
#include "../Items/RegionCenterPoint.h"
#include "../Items/TerminalItem.h"
#include "../MainWindow.h"
#include "../Utils/IconCreator.h"
#include "Backend/Controllers/RegionDataController.h"
#include "ContainerManagerWidget.h"
#include "GraphicsView.h"

#include <QApplication>
#include <QDebug>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QVBoxLayout>

namespace CargoNetSim {
namespace GUI {

PropertiesPanel::PropertiesPanel(QWidget *parent)
    : QWidget(parent)
    , currentItem(nullptr) {
    mainWindow = qobject_cast<MainWindow *>(window());

    // Create main layout for the widget
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Create a QScrollArea
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);

    // Create container widget for the scroll area
    container = new QWidget();
    layout    = new QFormLayout(container);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    // Save button
    saveButton = new QPushButton(tr("Save"));
    connect(saveButton, &QPushButton::clicked, this,
            &PropertiesPanel::saveProperties);
    layout->addRow(saveButton);

    // Set the container as the widget for the scroll area
    scrollArea->setWidget(container);

    // Add the scroll area to the main layout
    mainLayout->addWidget(scrollArea);

    // Set the default height to half the screen height
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        setMinimumWidth(250);
    }
}

void PropertiesPanel::displayMapProperties() {
    if (!mainWindow) {
        mainWindow = qobject_cast<MainWindow *>(window());
        if (!mainWindow) {
            return;
        }
    }

    GraphicsView *view = mainWindow->getCurrentView();
    if (!view) {
        return;
    }

    currentItem = nullptr;
    clearLayout();

    // Create coordinate system selection
    QGroupBox *coordGroup =
        new QGroupBox(tr("Coordinate System"));
    QVBoxLayout *coordLayout = new QVBoxLayout();

    QComboBox *coordCombo = new QComboBox();
    coordCombo->addItems(
        {tr("WGS84 (Geodetic)"),
         tr("WGS84 World Mercator (Projected)")});
    coordCombo->setCurrentIndex(
        view->useProjectedCoords ? 1 : 0);
    connect(
        coordCombo,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &PropertiesPanel::onCoordSystemChanged);

    QLabel *descLabel =
        new QLabel(tr("WGS84: Lat/Lon in degrees\nWGS84 "
                      "World Mercator: X/Y in meters"));
    descLabel->setWordWrap(true);

    coordLayout->addWidget(coordCombo);
    coordLayout->addWidget(descLabel);
    coordGroup->setLayout(coordLayout);

    layout->addRow(coordGroup);
    layout->addRow(saveButton);

    emit requestRefresh();
}

void PropertiesPanel::displayProperties(
    QGraphicsItem *item) {
    clearLayout();

    if (!item) {
        return;
    }

    currentItem = item;
    editFields.clear();

    // Dispatch to appropriate handler based on item type
    if (MapPoint *mapPoint =
            dynamic_cast<MapPoint *>(item)) {
        displayMapPointProperties(mapPoint);
    } else if (RegionCenterPoint *regionCenter =
                   dynamic_cast<RegionCenterPoint *>(
                       item)) {
        displayRegionCenterProperties(regionCenter);
    } else if (ConnectionLine *connection =
                   dynamic_cast<ConnectionLine *>(item)) {
        displayConnectionProperties(connection);
    } else if (TerminalItem *terminal =
                   dynamic_cast<TerminalItem *>(item)) {
        displayTerminalProperties(terminal);
    } else {
        displayGenericProperties(item);
    }

    layout->addRow(saveButton);
}

void PropertiesPanel::clearLayout() {
    // Clear all widgets except save button from layout
    for (int i = layout->count() - 1; i >= 0; --i) {
        QLayoutItem *item = layout->itemAt(i);
        QWidget *widget   = item ? item->widget() : nullptr;
        if (widget && widget != saveButton) {
            layout->removeWidget(widget);
            delete widget;
        } else if (widget == saveButton) {
            layout->removeWidget(widget);
        }
    }
}

void PropertiesPanel::displayMapPointProperties(
    MapPoint *item) {
    // Display properties for MapPoint items
    for (auto it = item->getProperties().constBegin();
         it != item->getProperties().constEnd(); ++it) {
        if (it.key() == "x" || it.key() == "y") {
            // Skip position properties as they're handled
            // by the GraphicsItem position
            continue;
        }
        QLabel *label = new QLabel(it.value().toString());
        layout->addRow(it.key() + ":", label);
    }
}

void PropertiesPanel::displayRegionCenterProperties(
    RegionCenterPoint *item) {
    if (!mainWindow) {
        mainWindow = qobject_cast<MainWindow *>(window());
        if (!mainWindow) {
            return;
        }
    }

    GraphicsView *view = mainWindow->getCurrentView();
    if (!view) {
        return;
    }

    const QMap<QString, QVariant> &props =
        item->getProperties();
    for (auto it = props.constBegin();
         it != props.constEnd(); ++it) {
        if (it.key() == "Type" || it.key() == "Region") {
            continue;
        }

        if (it.key() == "Latitude"
            || it.key() == "Longitude") {
            addCoordinateField(it.key(), it.value(), view,
                               item);
        } else if (it.key() == "Shared Latitude"
                   || it.key() == "Shared Longitude") {
            QLineEdit *lineEdit =
                new QLineEdit(it.value().toString());
            editFields[it.key()] = lineEdit;
            layout->addRow(QString("%1:°").arg(it.key()),
                           lineEdit);
        } else {
            addGenericField(it.key(), it.value());
        }
    }
}

void PropertiesPanel::addCoordinateField(
    const QString &key, const QVariant &value,
    GraphicsView *view, RegionCenterPoint *item) {
    QDoubleValidator *validator =
        new QDoubleValidator(this);

    QString label;
    QString valueStr;

    if (view->useProjectedCoords) {
        double lat =
            item->getProperties()["Latitude"].toDouble();
        double lon =
            item->getProperties()["Longitude"].toDouble();
        QPointF projected = view->convertCoordinates(
            QPointF(lon, lat), "to_projected");

        label = (key == "Longitude") ? tr("X Position (m)")
                                     : tr("Y Position (m)");
        valueStr =
            (key == "Longitude")
                ? QString::number(projected.x(), 'f', 2)
                : QString::number(projected.y(), 'f', 2);
    } else {
        label    = QString("%1 (°)").arg(key);
        valueStr = value.toString();
    }

    QLineEdit *lineEdit = new QLineEdit(valueStr);
    lineEdit->setValidator(validator);
    editFields[key] = lineEdit;
    layout->addRow(QString("%1:").arg(label), lineEdit);
}

void PropertiesPanel::displayConnectionProperties(
    ConnectionLine *item) {
    QDoubleValidator *validator =
        new QDoubleValidator(this);
    validator->setBottom(0.0); // Ensure non-negative values

    static const QMap<QString, QPair<QString, QString>>
        propertiesWithUnits = {
            {"cost", {tr("Cost (USD)"), ""}},
            {"travelTime", {tr("Travel Time (Hours)"), ""}},
            {"distance", {tr("Distance (Km)"), ""}},
            {"carbonEmissions",
             {tr("Carbon Emissions (ton CO₂)"), ""}},
            {"risk", {tr("Risk (%)"), ""}},
            {"energyConsumption",
             {tr("Energy Consumption (kWh)"), ""}}};

    const QMap<QString, QVariant> &props =
        item->getProperties();
    for (auto it = propertiesWithUnits.constBegin();
         it != propertiesWithUnits.constEnd(); ++it) {
        QLineEdit *lineEdit = new QLineEdit(
            props.value(it.key(), "0.0").toString());
        lineEdit->setValidator(validator);
        editFields[it.key()] = lineEdit;
        layout->addRow(it.value().first + ":", lineEdit);
    }
}

void PropertiesPanel::displayTerminalProperties(
    TerminalItem *item) {
    displayGenericProperties(
        item,
        {"ID", "Type", "capacity", "cost", "dwell_time",
         "customs", "Available Interfaces", "Containers"});

    // Only allow editing interfaces for Origin and
    // Destination terminals
    QMap<QString, bool> isEditable;
    if (item->getTerminalType() == "Origin"
        || item->getTerminalType() == "Destination") {
        isEditable = {{"land_side", true},
                      {"sea_side", true}};
    } else if (item->getTerminalType()
               == "Sea Port Terminal") {
        isEditable = {{"land_side", true},
                      {"sea_side", false}};
    } else {
        isEditable = {{"land_side", false},
                      {"sea_side", false}};
    }

    addInterfacesSection(item, isEditable);
    addCapacitySection(item);
    addCostSection(item);
    addDwellTimeSection(item);
    addCustomsSection(item);

    if (item->getTerminalType() == "Origin") {
        addContainerManagement(item);
    }
}

void PropertiesPanel::addInterfacesSection(
    TerminalItem              *item,
    const QMap<QString, bool> &isEditable) {
    QGroupBox *interfacesGroup =
        new QGroupBox(tr("Available Interfaces"));
    QVBoxLayout *interfacesLayout = new QVBoxLayout();

    // Get interfaces from properties
    const QMap<QString, QVariant> &properties =
        item->getProperties();
    QMap<QString, QVariant> interfaces =
        properties["Available Interfaces"].toMap();

    // Land-side interfaces
    QStringList currentLand =
        interfaces["land_side"].toStringList();
    QLayout *landLayout = createInterfaceLayout(
        tr("Land-side:"),
        {{tr("Truck"), "truck"}, {tr("Train"), "train"}},
        currentLand, "land",
        isEditable.value("land_side", false));

    // Sea-side interfaces
    QStringList currentSea =
        interfaces["sea_side"].toStringList();
    QLayout *seaLayout = createInterfaceLayout(
        tr("Sea-side:"), {{tr("Ship"), "ship"}}, currentSea,
        "sea", isEditable.value("sea_side", false));

    interfacesLayout->addLayout(landLayout);
    interfacesLayout->addLayout(seaLayout);
    interfacesGroup->setLayout(interfacesLayout);
    layout->addRow(interfacesGroup);
}

QLayout *PropertiesPanel::createInterfaceLayout(
    const QString                        &label,
    const QList<QPair<QString, QString>> &options,
    const QStringList &currentValues, const QString &side,
    bool isEditable) {
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(new QLabel(label));

    QHBoxLayout *checkboxLayout = new QHBoxLayout();
    for (const auto &option : options) {
        QCheckBox *checkbox = new QCheckBox(option.first);
        checkbox->setChecked(
            currentValues.contains(option.first));
        checkbox->setEnabled(isEditable);
        editFields[QString("interfaces.%1.%2")
                       .arg(side)
                       .arg(option.second)] = checkbox;
        checkboxLayout->addWidget(checkbox);
    }

    layout->addLayout(checkboxLayout);
    return layout;
}

void PropertiesPanel::addNestedPropertiesSection(
    TerminalItem *item, const QString &sectionName,
    const QString &propertiesKey) {
    const QMap<QString, QVariant> &properties =
        item->getProperties();
    if (!properties.contains(propertiesKey)) {
        return;
    }

    QGroupBox   *group = new QGroupBox(sectionName);
    QFormLayout *propertyLayout = new QFormLayout();

    // Define units and validators for different property
    // types
    static const QMap<
        QString,
        QMap<QString, QPair<QString, QDoubleValidator *>>>
        propertyConfigs = {
            {"capacity",
             {{"storage",
               {tr("Storage Capacity (TEU)"),
                new QDoubleValidator()}},
              {"processing",
               {tr("Processing Capacity (TEU/day)"),
                new QDoubleValidator()}}}},
            {"cost",
             {{"fixed",
               {tr("Fixed Cost (USD/year)"),
                new QDoubleValidator()}},
              {"variable",
               {tr("Variable Cost (USD/TEU)"),
                new QDoubleValidator()}},
              {"penalty",
               {tr("Penalty Cost (USD/day)"),
                new QDoubleValidator()}}}},
            {"customs",
             {{"processing_time",
               {tr("Processing Time (hours)"),
                new QDoubleValidator()}},
              {"cost",
               {tr("Cost (USD/TEU)"),
                new QDoubleValidator()}}}}};

    QMap<QString, QVariant> nestedProperties =
        properties[propertiesKey].toMap();
    const QMap<QString, QPair<QString, QDoubleValidator *>>
        &configs = propertyConfigs[propertiesKey];

    for (auto it = nestedProperties.constBegin();
         it != nestedProperties.constEnd(); ++it) {
        QLineEdit *lineEdit =
            new QLineEdit(it.value().toString());

        QString           label     = it.key();
        QDoubleValidator *validator = nullptr;

        if (configs.contains(it.key())) {
            label     = configs[it.key()].first;
            validator = configs[it.key()].second;
            if (validator) {
                validator->setBottom(
                    0.0); // Ensure non-negative values
                lineEdit->setValidator(validator);
            }
        }

        editFields[QString("%1.%2")
                       .arg(propertiesKey)
                       .arg(it.key())] = lineEdit;
        propertyLayout->addRow(label + ":", lineEdit);
    }

    group->setLayout(propertyLayout);
    layout->addRow(group);
}

void PropertiesPanel::addCapacitySection(
    TerminalItem *item) {
    addNestedPropertiesSection(item, tr("Capacity"),
                               "capacity");
}

void PropertiesPanel::addCostSection(TerminalItem *item) {
    addNestedPropertiesSection(item, tr("Cost"), "cost");
}

void PropertiesPanel::addCustomsSection(
    TerminalItem *item) {
    addNestedPropertiesSection(item, tr("Customs"),
                               "customs");
}

void PropertiesPanel::addDwellTimeSection(
    TerminalItem *item) {
    const QMap<QString, QVariant> &properties =
        item->getProperties();
    if (!properties.contains("dwell_time")) {
        return;
    }

    QMap<QString, QVariant> dwellTime =
        properties["dwell_time"].toMap();

    QGroupBox *dwellGroup = new QGroupBox(tr("Dwell Time"));
    QVBoxLayout *dwellLayout = new QVBoxLayout();

    // Method selection
    QHBoxLayout *methodLayout = new QHBoxLayout();
    QLabel      *methodLabel  = new QLabel(tr("Method:"));
    QComboBox   *methodCombo  = new QComboBox();
    methodCombo->addItems(
        {"normal", "gamma", "exponential", "lognormal"});

    QString currentMethod = dwellTime["method"].toString();
    if (currentMethod.isEmpty()) {
        currentMethod = "normal";
    }
    methodCombo->setCurrentText(currentMethod);
    editFields["dwell_time.method"] = methodCombo;

    methodLayout->addWidget(methodLabel);
    methodLayout->addWidget(methodCombo);
    dwellLayout->addLayout(methodLayout);

    // Parameters
    QMap<QString, QVariant> currentParams =
        dwellTime["parameters"].toMap();
    auto [paramLayout, paramFields] =
        createDwellTimeParameters(currentMethod,
                                  currentParams);
    dwellLayout->addLayout(paramLayout);

    for (auto it = paramFields.constBegin();
         it != paramFields.constEnd(); ++it) {
        editFields[QString("dwell_time.parameters.%1")
                       .arg(it.key())] = it.value();
    }

    connect(methodCombo, &QComboBox::currentTextChanged,
            [this, dwellGroup](const QString &text) {
                onDwellMethodChanged(text, dwellGroup);
            });

    dwellGroup->setLayout(dwellLayout);
    layout->addRow(dwellGroup);
}

void PropertiesPanel::addContainerManagement(
    TerminalItem *item) {
    QPushButton *containerButton =
        new QPushButton(tr("Manage Containers"));
    connect(containerButton, &QPushButton::clicked,
            [this, item]() { openContainerManager(item); });
    layout->addRow(containerButton);
}

void PropertiesPanel::displayGenericProperties(
    QGraphicsItem     *item,
    const QStringList &skipProperties) {
    QStringList defaultSkip = {"Type"};
    QStringList allSkip     = defaultSkip + skipProperties;

    // Need to convert to proper item type to get properties
    TerminalItem *terminal =
        dynamic_cast<TerminalItem *>(item);
    BackgroundPhotoItem *background =
        dynamic_cast<BackgroundPhotoItem *>(item);
    RegionCenterPoint *regionCenter =
        dynamic_cast<RegionCenterPoint *>(item);
    MapPoint *mapPoint = dynamic_cast<MapPoint *>(item);
    ConnectionLine *connection =
        dynamic_cast<ConnectionLine *>(item);

    QMap<QString, QVariant> properties;

    if (terminal) {
        properties = terminal->getProperties();
    } else if (background) {
        properties = background->getProperties();
    } else if (regionCenter) {
        properties = regionCenter->getProperties();
    } else if (mapPoint) {
        properties = mapPoint->getProperties();
    } else if (connection) {
        properties = connection->getProperties();
    } else {
        return; // No supported item type
    }

    for (auto it = properties.constBegin();
         it != properties.constEnd(); ++it) {
        if (allSkip.contains(it.key())) {
            continue;
        }

        if (it.key() == "Show on Global Map" && terminal) {
            QCheckBox *checkbox = new QCheckBox();
            checkbox->setChecked(
                it.value().toString().toLower() == "true");
            editFields[it.key()] = checkbox;
            layout->addRow(it.key() + ":", checkbox);
        } else if (it.key() == "Region") {
            QComboBox *combo = new QComboBox();
            combo->addItems(
                Backend::RegionDataController::getInstance()
                    .getAllRegionNames());
            combo->setCurrentText(it.value().toString());
            editFields[it.key()] = combo;
            layout->addRow(it.key() + ":", combo);
        } else {
            addGenericField(it.key(), it.value());
        }
    }
}

void PropertiesPanel::addGenericField(
    const QString &key, const QVariant &value) {
    QLineEdit *lineEdit = new QLineEdit(value.toString());
    editFields[key]     = lineEdit;
    layout->addRow(key + ":", lineEdit);
}

QPair<QLayout *, QMap<QString, QWidget *>>
PropertiesPanel::createDwellTimeParameters(
    const QString                 &method,
    const QMap<QString, QVariant> &currentParams) {
    QFormLayout *paramLayout = new QFormLayout();
    QMap<QString, QWidget *> paramFields;

    if (method == "gamma") {
        // Shape parameter
        QLineEdit *shape = new QLineEdit(
            currentParams.value("shape", "2.0").toString());
        paramFields["shape"] = shape;
        paramLayout->addRow(tr("Shape (k):"), shape);

        // Scale parameter
        QLineEdit *scale = new QLineEdit(
            currentParams.value("scale", "1440")
                .toString());
        paramFields["scale"] = scale;
        paramLayout->addRow(tr("Scale (θ) minutes:"),
                            scale);
    } else if (method == "exponential") {
        // Scale parameter
        QLineEdit *scale = new QLineEdit(
            currentParams.value("scale", "2880")
                .toString());
        paramFields["scale"] = scale;
        paramLayout->addRow(tr("Scale (λ) minutes:"),
                            scale);
    } else if (method == "normal") {
        // Mean parameter
        QLineEdit *mean = new QLineEdit(
            currentParams.value("mean", "2880").toString());
        paramFields["mean"] = mean;
        paramLayout->addRow(tr("Mean (minutes):"), mean);

        // Standard deviation parameter
        QLineEdit *stdDev = new QLineEdit(
            currentParams.value("std_dev", "720")
                .toString());
        paramFields["std_dev"] = stdDev;
        paramLayout->addRow(tr("Std Dev (minutes):"),
                            stdDev);
    } else if (method == "lognormal") {
        // Mean parameter (log-scale)
        QLineEdit *mean = new QLineEdit(
            currentParams.value("mean", "3.45").toString());
        paramFields["mean"] = mean;
        paramLayout->addRow(tr("Mean (log-scale):"), mean);

        // Sigma parameter
        QLineEdit *sigma = new QLineEdit(
            currentParams.value("sigma", "0.25")
                .toString());
        paramFields["sigma"] = sigma;
        paramLayout->addRow(tr("Sigma:"), sigma);
    }

    return {paramLayout, paramFields};
}

void PropertiesPanel::onDwellMethodChanged(
    const QString &method, QGroupBox *dwellGroup) {
    // Remove old parameter fields
    QFormLayout *oldParamLayout = nullptr;
    for (int i = 0; i < dwellGroup->layout()->count();
         ++i) {
        QLayoutItem *item = dwellGroup->layout()->itemAt(i);
        if (QFormLayout *formLayout =
                qobject_cast<QFormLayout *>(
                    item->layout())) {
            oldParamLayout = formLayout;
            break;
        }
    }

    if (oldParamLayout) {
        // Get the current parameters if they exist
        QMap<QString, QVariant> currentParams;
        for (auto it = editFields.constBegin();
             it != editFields.constEnd(); ++it) {
            if (it.key().startsWith(
                    "dwell_time.parameters.")) {
                QString paramName = it.key().mid(
                    21); // Remove "dwell_time.parameters."
                if (QLineEdit *lineEdit =
                        qobject_cast<QLineEdit *>(
                            it.value())) {
                    currentParams[paramName] =
                        lineEdit->text();
                }
            }
        }

        // Remove old parameter fields from editFields
        for (auto it = editFields.begin();
             it != editFields.end();) {
            if (it.key().startsWith(
                    "dwell_time.parameters.")) {
                it = editFields.erase(it);
            } else {
                ++it;
            }
        }

        // Remove the old layout
        while (oldParamLayout->count()) {
            QLayoutItem *item = oldParamLayout->takeAt(0);
            if (item->widget()) {
                delete item->widget();
            }
            delete item;
        }
        QLayout *groupLayout = dwellGroup->layout();
        groupLayout->removeItem(oldParamLayout);
        delete oldParamLayout;
    }

    // Create and add new parameter fields
    auto [paramLayout, paramFields] =
        createDwellTimeParameters(method);
    dwellGroup->layout()->addItem(paramLayout);

    // Update editFields with new parameter fields
    for (auto it = paramFields.constBegin();
         it != paramFields.constEnd(); ++it) {
        editFields[QString("dwell_time.parameters.%1")
                       .arg(it.key())] = it.value();
    }
}

void PropertiesPanel::updatePositionFields(
    const QPointF &pos) {
    if (currentItem && !editFields.isEmpty()) {
        if (editFields.contains("X Position")) {
            QLineEdit *xField = qobject_cast<QLineEdit *>(
                editFields["X Position"]);
            if (xField) {
                xField->setText(
                    QString::number(pos.x(), 'f', 2));
            }
        }
        if (editFields.contains("Y Position")) {
            QLineEdit *yField = qobject_cast<QLineEdit *>(
                editFields["Y Position"]);
            if (yField) {
                yField->setText(
                    QString::number(pos.y(), 'f', 2));
            }
        }
    }
}

void PropertiesPanel::updateCoordinateFields(double lat,
                                             double lon) {
    if (currentItem && !editFields.isEmpty()) {
        if (editFields.contains("Latitude")) {
            QLineEdit *latField = qobject_cast<QLineEdit *>(
                editFields["Latitude"]);
            if (latField) {
                latField->setText(
                    QString::number(lat, 'f', 6));
            }
        }
        if (editFields.contains("Longitude")) {
            QLineEdit *lonField = qobject_cast<QLineEdit *>(
                editFields["Longitude"]);
            if (lonField) {
                lonField->setText(
                    QString::number(lon, 'f', 6));
            }
        }
    }
}

void PropertiesPanel::saveProperties() {
    if (!currentItem) {
        return;
    }

    // Create new properties dictionary
    QMap<QString, QVariant> newProperties;

    // Handle all properties
    for (auto it = editFields.constBegin();
         it != editFields.constEnd(); ++it) {
        if (it.key().contains(
                '.')) { // Handle nested properties
            QStringList parts = it.key().split('.');
            QMap<QString, QVariant> *currentDict =
                &newProperties;

            // Special handling for interfaces
            if (parts[0] == "interfaces") {
                if (!newProperties.contains(
                        "Available Interfaces")) {
                    newProperties["Available Interfaces"] =
                        QVariantMap{
                            {"land_side", QVariantList()},
                            {"sea_side", QVariantList()}};
                }

                // Get the current lists
                QVariantMap interfaces =
                    newProperties["Available Interfaces"]
                        .toMap();
                QStringList landSide =
                    interfaces["land_side"].toStringList();
                QStringList seaSide =
                    interfaces["sea_side"].toStringList();

                // Add to appropriate side if checked
                if (QCheckBox *checkbox =
                        qobject_cast<QCheckBox *>(
                            it.value())) {
                    if (checkbox->isChecked()) {
                        if (parts[1] == "land") {
                            QString mode = parts[2];
                            mode[0] =
                                mode[0]
                                    .toUpper(); // Capitalize
                                                // first
                                                // letter
                            if (!landSide.contains(mode)) {
                                landSide.append(mode);
                            }
                        } else if (parts[1] == "sea") {
                            QString mode = parts[2];
                            mode[0] =
                                mode[0]
                                    .toUpper(); // Capitalize
                                                // first
                                                // letter
                            if (!seaSide.contains(mode)) {
                                seaSide.append(mode);
                            }
                        }
                    }

                    // Update the interfaces in the map
                    interfaces["land_side"] = landSide;
                    interfaces["sea_side"]  = seaSide;
                    newProperties["Available Interfaces"] =
                        interfaces;

                    continue;
                }

                // Handle other nested properties
                QVariantMap nestedMap;
                for (int i = 0; i < parts.size() - 1; ++i) {
                    QString part = parts[i];
                    if (i == 0) {
                        if (!newProperties.contains(part)) {
                            newProperties[part] =
                                QVariantMap();
                        }
                        nestedMap =
                            newProperties[part].toMap();
                    } else {
                        if (!nestedMap.contains(part)) {
                            nestedMap[part] = QVariantMap();
                        }
                        nestedMap = nestedMap[part].toMap();
                    }
                }

                // Set the value in the nested map
                QString finalKey = parts.last();
                if (QComboBox *combo =
                        qobject_cast<QComboBox *>(
                            it.value())) {
                    nestedMap[finalKey] =
                        combo->currentText();
                } else if (QLineEdit *lineEdit =
                               qobject_cast<QLineEdit *>(
                                   it.value())) {
                    nestedMap[finalKey] = lineEdit->text();
                }

                // Update the nested properties in the
                // parent map
                if (parts.size() > 1) {
                    QVariantMap parentMap =
                        newProperties[parts.first()]
                            .toMap();
                    if (parts.size() == 2) {
                        parentMap[parts.last()] =
                            nestedMap[parts.last()];
                    } else if (parts.size() == 3) {
                        QVariantMap middleMap =
                            parentMap[parts[1]].toMap();
                        middleMap[parts.last()] =
                            nestedMap[parts.last()];
                        parentMap[parts[1]] = middleMap;
                    }
                    newProperties[parts.first()] =
                        parentMap;
                }
            } else { // Handle top-level properties
                if (QCheckBox *checkbox =
                        qobject_cast<QCheckBox *>(
                            it.value())) {
                    newProperties[it.key()] =
                        checkbox->isChecked() ? "True"
                                              : "False";
                } else if (QComboBox *combo =
                               qobject_cast<QComboBox *>(
                                   it.value())) {
                    newProperties[it.key()] =
                        combo->currentText();
                } else if (QLineEdit *lineEdit =
                               qobject_cast<QLineEdit *>(
                                   it.value())) {
                    newProperties[it.key()] =
                        lineEdit->text();
                }
            }
        }

        // Get main window reference
        if (!mainWindow) {
            mainWindow =
                qobject_cast<MainWindow *>(window());
        }

        // Handle region change if present - skip for
        // RegionCenterPoint
        RegionCenterPoint *regionCenter =
            dynamic_cast<RegionCenterPoint *>(currentItem);
        if (!regionCenter
            && newProperties.contains("Region")) {
            TerminalItem *terminal =
                dynamic_cast<TerminalItem *>(currentItem);
            if (terminal
                && terminal->getRegion()
                       != newProperties["Region"]
                              .toString()) {
                QString newRegionName =
                    newProperties["Region"].toString();
                QString oldRegionName =
                    terminal->getRegion();

                // Get the new region's center point
                RegionCenterPoint *newRegionCenter =
                    nullptr;
                RegionCenterPoint *oldRegionCenter =
                    nullptr;

                if (mainWindow) {
                    const auto &centers =
                        Backend::RegionDataController::
                            getInstance()
                                .getAllRegionVariableAs<
                                    RegionCenterPoint *>(
                                    "regionCenterPoint");
                    newRegionCenter =
                        centers.value(newRegionName);
                    oldRegionCenter =
                        centers.value(oldRegionName);
                }

                if (newRegionCenter && oldRegionCenter) {
                    // Calculate item's offset from old
                    // region center
                    QPointF oldOffset(
                        terminal->pos().x()
                            - oldRegionCenter->pos().x(),
                        terminal->pos().y()
                            - oldRegionCenter->pos().y());

                    // Apply offset to new region center
                    QPointF newPos(
                        newRegionCenter->pos().x()
                            + oldOffset.x(),
                        newRegionCenter->pos().y()
                            + oldOffset.y());

                    // Update item position
                    terminal->setPos(newPos);

                    // Update region property
                    terminal->setRegion(
                        newProperties["Region"].toString());
                }
            }
        }

        // Handle coordinate changes for RegionCenterPoint
        if (regionCenter) {
            try {
                // Get new lat/lon values
                double newLat =
                    newProperties.value("Latitude", "0.0")
                        .toDouble();
                double newLon =
                    newProperties.value("Longitude", "0.0")
                        .toDouble();

                // Convert to scene coordinates and update
                // position
                if (mainWindow) {
                    GraphicsView *view =
                        mainWindow->getCurrentView();
                    if (view) {
                        QPointF newPos = view->wgs84ToScene(
                            newLat, newLon);
                        regionCenter->setPos(newPos);
                    }
                }
            } catch (const std::exception &e) {
                if (mainWindow) {
                    mainWindow->showStatusBarMessage(
                        tr("Invalid coordinate values: %1")
                            .arg(e.what()),
                        3000);
                }
                return;
            }
        }

        // Handle coordinate changes for BackgroundPhotoItem
        BackgroundPhotoItem *bgPhoto =
            dynamic_cast<BackgroundPhotoItem *>(
                currentItem);
        if (bgPhoto) {
            try {
                // Get new lat/lon values
                double newLat =
                    newProperties.value("Latitude", "0.0")
                        .toDouble();
                double newLon =
                    newProperties.value("Longitude", "0.0")
                        .toDouble();

                // Get and validate new scale value
                double newScale =
                    newProperties.value("Scale", "1.0")
                        .toDouble();
                if (newScale <= 0) {
                    throw std::invalid_argument(
                        "Scale must be greater than 0");
                }

                // Update position using WGS84 coordinates
                bgPhoto->setFromWGS84(newLat, newLon);

                // Update scale and trigger redraw
                bgPhoto->getProperties()["Scale"] =
                    QString::number(newScale);
                bgPhoto->updateScale();
            } catch (const std::exception &e) {
                if (mainWindow) {
                    mainWindow->showStatusBarMessage(
                        tr("Invalid coordinate or scale "
                           "values: %1")
                            .arg(e.what()),
                        3000);
                }
                return;
            }
        }

        // Update the item's properties
        if (TerminalItem *terminal =
                dynamic_cast<TerminalItem *>(currentItem)) {
            terminal->updateProperties(newProperties);
        } else if (BackgroundPhotoItem *background =
                       dynamic_cast<BackgroundPhotoItem *>(
                           currentItem)) {
            background->updateProperties(newProperties);
        } else if (RegionCenterPoint *regionCenter =
                       dynamic_cast<RegionCenterPoint *>(
                           currentItem)) {
            regionCenter->updateProperties(newProperties);
        } else if (MapPoint *mapPoint =
                       dynamic_cast<MapPoint *>(
                           currentItem)) {
            mapPoint->updateProperties(newProperties);
        } else if (ConnectionLine *connection =
                       dynamic_cast<ConnectionLine *>(
                           currentItem)) {
            connection->updateProperties(newProperties);
        }

        // Emit the properties changed signal
        emit propertiesChanged(currentItem, newProperties);

        // Show status bar message
        if (mainWindow) {
            mainWindow->showStatusBarMessage(
                tr("Properties updated successfully"),
                2000);

            // TODO: Update visibility and global map
            // ViewController::updateSceneVisibility(mainWindow);
            // ViewController::updateGlobalMapScene(mainWindow);
        }
    }
}

void PropertiesPanel::openContainerManager(
    TerminalItem *item) {
    if (!item) {
        return;
    }

    QMap<QString, QVariant> containers =
        item->getProperties().value("Containers").toMap();
    ContainerManagerWidget dialog(containers, this);

    if (dialog.exec() == QDialog::Accepted) {
        QMap<QString, QVariant> updatedContainers =
            dialog.getContainers();
        QMap<QString, QVariant> props =
            item->getProperties();
        props["Containers"] = updatedContainers;
        item->updateProperties(props);

        // Emit the properties changed signal
        emit propertiesChanged(item, props);
    }
}

void PropertiesPanel::onCoordSystemChanged(int index) {
    if (!mainWindow) {
        mainWindow = qobject_cast<MainWindow *>(window());
    }

    if (!mainWindow) {
        return;
    }

    GraphicsView *view = mainWindow->getCurrentView();
    if (!view) {
        return;
    }

    view->useProjectedCoords = (index == 1);
    mainWindow->updateAllCoordinates();
}

} // namespace GUI
} // namespace CargoNetSim
