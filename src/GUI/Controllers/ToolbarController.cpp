// ToolbarController.cpp
#include "ToolbarController.h"

#include <QAction>
#include <QHBoxLayout>
#include <QStyle>
#include <QVBoxLayout>
#include <QWidgetAction>

#include "../Controllers/BasicButtonController.h"
#include "../Controllers/NetworkController.h"
#include "../Controllers/UtilityFunctions.h"
#include "../MainWindow.h"
#include "../Utils/ColorUtils.h"
#include "../Utils/IconCreator.h"
#include "../Widgets/ShipManagerDialog.h"
#include "../Widgets/TrainManagerDialog.h"
#include "Backend/Controllers/VehicleController.h"
#include "GUI/Controllers/ViewController.h"

namespace CargoNetSim
{
namespace GUI
{

void ToolbarController::setupToolbar(MainWindow *mainWindow)
{
    // Create ribbon-style toolbar
    mainWindow->ribbon_ = new QTabWidget();

    // Add style for centered group box titles
    mainWindow->ribbon_->setStyleSheet(
        "QGroupBox {"
        "   margin-top: 0px;    /* Remove space above the "
        "GroupBox */"
        "   margin-bottom: 15px; /* Add space below the "
        "GroupBox */"
        "   padding-top: 0px;"
        "   padding-right: 2px;"
        "   padding-bottom: 10px;"
        "   padding-left: 2px;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: bottom center;"
        "   padding: 0 5px;"
        "   bottom: 7px;"
        "}"
        "QToolButton {"
        "   icon-size: 32px;"
        "}");

    mainWindow->addToolBarBreak();
    QToolBar *toolbar = new QToolBar();
    toolbar->setAllowedAreas(
        Qt::ToolBarArea::TopToolBarArea);
    toolbar->setMovable(false);
    mainWindow->addToolBar(toolbar);
    toolbar->addWidget(mainWindow->ribbon_);

    // Create Home tab
    QWidget     *homeTab    = new QWidget();
    QHBoxLayout *homeLayout = new QHBoxLayout(homeTab);
    homeLayout->setSpacing(4);
    homeLayout->setContentsMargins(4, 4, 4, 4);

    // Create Project group
    mainWindow->projectGroup_ = new QGroupBox("Project");
    QHBoxLayout *projectLayout =
        new QHBoxLayout(mainWindow->projectGroup_);
    projectLayout->setSpacing(4);
    projectLayout->setContentsMargins(8, 12, 8, 8);

    // New project button
    QToolButton *newProjectButton = new QToolButton();
    newProjectButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    newProjectButton->setText("New\nProject");
    newProjectButton->setIcon(
        QIcon(IconFactory::createNewProjectIcon()));
    // QObject::connect(newProjectButton,
    // &QToolButton::clicked,
    //                  [mainWindow]() {
    //                  BasicButtonController::newProject(mainWindow);
    //                  });
    projectLayout->addWidget(newProjectButton);

    // Open project button
    QToolButton *openProjectButton = new QToolButton();
    openProjectButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    openProjectButton->setText("Open\nProject");
    openProjectButton->setIcon(
        QIcon(IconFactory::createOpenProjectIcon()));
    // QObject::connect(openProjectButton,
    // &QToolButton::clicked,
    //                  [mainWindow]() {
    //                  BasicButtonController::openProject(mainWindow);
    //                  });
    projectLayout->addWidget(openProjectButton);

    // Save project button
    QToolButton *saveProjectButton = new QToolButton();
    saveProjectButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    saveProjectButton->setText("Save\nProject");
    saveProjectButton->setIcon(
        QIcon(IconFactory::createSaveProjectIcon()));
    // QObject::connect(saveProjectButton,
    // &QToolButton::clicked,
    //                  [mainWindow]() {
    //                  BasicButtonController::saveProject(mainWindow);
    //                  });
    projectLayout->addWidget(saveProjectButton);

    mainWindow->projectButtons_ = {newProjectButton,
                                   openProjectButton,
                                   saveProjectButton};
    homeLayout->addWidget(mainWindow->projectGroup_);

    // Create Tools group
    mainWindow->toolsGroup_ = new QGroupBox("Basic Tools");
    QHBoxLayout *toolsLayout =
        new QHBoxLayout(mainWindow->toolsGroup_);
    toolsLayout->setSpacing(4);
    toolsLayout->setContentsMargins(8, 12, 8, 8);

    // Add connection button with dropdown menu
    QToolButton *connectButton = new QToolButton();
    connectButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    connectButton->setCheckable(true);
    connectButton->setText("Connect\nTerminals");
    connectButton->setIcon(
        QIcon(IconFactory::createConnectTerminalsPixmap()));
    connectButton->setMenu(mainWindow->connectionMenu_);
    connectButton->setPopupMode(
        QToolButton::ToolButtonPopupMode::MenuButtonPopup);
    // QObject::connect(connectButton,
    // &QToolButton::clicked,
    //                  [mainWindow](bool checked) {
    //                  BasicButtonController::toggleConnectMode(mainWindow,
    //                  checked); });
    mainWindow->connectButton_ = connectButton;
    toolsLayout->addWidget(connectButton);

    // Add link terminal button to tools group
    QToolButton *linkTerminalButton = new QToolButton();
    linkTerminalButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    linkTerminalButton->setText("Link Terminal\nto Node");
    linkTerminalButton->setIcon(
        QIcon(IconFactory::createLinkTerminalIcon()));
    linkTerminalButton->setCheckable(true);
    // QObject::connect(linkTerminalButton,
    // &QToolButton::clicked,
    //                  [mainWindow](bool checked) {
    //                  BasicButtonController::toggleLinkTerminalMode(mainWindow,
    //                  checked); });
    toolsLayout->addWidget(linkTerminalButton);
    mainWindow->linkTerminalButton_ = linkTerminalButton;

    QToolButton *unlinkTerminalButton = new QToolButton();
    unlinkTerminalButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    unlinkTerminalButton->setText("Unlink\nTerminal");
    unlinkTerminalButton->setIcon(
        QIcon(IconFactory::createUnlinkTerminalIcon()));
    unlinkTerminalButton->setCheckable(true);
    // QObject::connect(unlinkTerminalButton,
    // &QToolButton::clicked,
    //                  [mainWindow](bool checked) {
    //                  BasicButtonController::toggleUnlinkTerminalMode(mainWindow,
    //                  checked); });
    toolsLayout->addWidget(unlinkTerminalButton);
    mainWindow->unlinkTerminalButton_ =
        unlinkTerminalButton;

    // Add set global position button
    QToolButton *setGlobalPositionButton =
        new QToolButton();
    setGlobalPositionButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    setGlobalPositionButton->setText(
        "Set Terminal\nGlobal Position");
    setGlobalPositionButton->setIcon(
        QIcon(IconFactory::createSetGlobalPositionIcon()));
    setGlobalPositionButton->setCheckable(true);
    // QObject::connect(setGlobalPositionButton,
    // &QToolButton::clicked,
    //                  [mainWindow](bool checked) {
    //                  BasicButtonController::toggleSetGlobalPositionMode(mainWindow,
    //                  checked); });
    setGlobalPositionButton->setVisible(false);
    toolsLayout->addWidget(setGlobalPositionButton);
    mainWindow->setGlobalPositionButton_ =
        setGlobalPositionButton;

    mainWindow->toolsButtons_ = {
        connectButton, linkTerminalButton,
        unlinkTerminalButton, setGlobalPositionButton};
    homeLayout->addWidget(mainWindow->toolsGroup_);

    // Create Measurements group
    mainWindow->measurementsGroup_ =
        new QGroupBox("Measurements");
    QHBoxLayout *measurementsLayout =
        new QHBoxLayout(mainWindow->measurementsGroup_);
    measurementsLayout->setSpacing(4);
    measurementsLayout->setContentsMargins(8, 12, 8, 8);

    // Add measure distance button
    QToolButton *measureButton = new QToolButton();
    measureButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    measureButton->setText("Measure\nDistance");
    measureButton->setIcon(
        QIcon(IconFactory::createMeasureDistancePixmap()));
    measureButton->setCheckable(true);
    // QObject::connect(measureButton,
    // &QToolButton::clicked,
    //                  [mainWindow](bool checked) {
    //                  BasicButtonController::toggleMeasureMode(mainWindow,
    //                  checked); });
    measurementsLayout->addWidget(measureButton);
    mainWindow->measureButton_ = measureButton;

    // Add clear measurements button
    QToolButton *clearMeasureButton = new QToolButton();
    clearMeasureButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    clearMeasureButton->setText("Clear\nMeasurements");
    clearMeasureButton->setIcon(QIcon(
        IconFactory::createClearMeasurementsPixmap()));
    // QObject::connect(clearMeasureButton,
    // &QToolButton::clicked,
    //                  [mainWindow]() {
    //                  BasicButtonController::clearMeasurements(mainWindow);
    //                  });
    measurementsLayout->addWidget(clearMeasureButton);

    mainWindow->measurementsButtons_ = {measureButton,
                                        clearMeasureButton};
    homeLayout->addWidget(mainWindow->measurementsGroup_);

    // Create Region group
    mainWindow->regionGroup_ = new QGroupBox("Region");
    QVBoxLayout *regionLayout =
        new QVBoxLayout(mainWindow->regionGroup_);
    regionLayout->setContentsMargins(8, 12, 8, 8);

    // Add region selector
    QWidget     *regionWidget = new QWidget();
    QVBoxLayout *regionInnerLayout =
        new QVBoxLayout(regionWidget);
    regionInnerLayout->addWidget(
        new QLabel("Active Region:"));

    // Setup region combo box (placeholder)
    mainWindow->regionCombo_ = new QComboBox();
    // Add regions (this would be populated from
    // RegionDataController)
    // QObject::connect(mainWindow->regionCombo_,
    // QOverload<int>::of(&QComboBox::currentIndexChanged),
    //                  [mainWindow](int index) {
    //                  BasicButtonController::changeRegion(mainWindow,
    //                  index); });
    regionInnerLayout->addWidget(mainWindow->regionCombo_);
    regionLayout->addWidget(regionWidget);

    mainWindow->regionButtons_.clear();
    mainWindow->regionButtons_.append(
        dynamic_cast<QToolButton *>(regionWidget));

    homeLayout->addWidget(mainWindow->regionGroup_);

    // Create Network Tools group
    mainWindow->networkToolsGroup_ =
        new QGroupBox("Network Tools");
    QHBoxLayout *networkToolsLayout =
        new QHBoxLayout(mainWindow->networkToolsGroup_);
    networkToolsLayout->setContentsMargins(8, 12, 8, 8);

    // Add connect visible terminals button
    QToolButton *connectVisibleTerminalsButton =
        new QToolButton();
    connectVisibleTerminalsButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    connectVisibleTerminalsButton->setText(
        "Auto Connect\nTerminals By Networks");
    connectVisibleTerminalsButton->setIcon(QIcon(
        IconFactory::createAutoConnectTerminalsIcon()));

    // Create a menu for the button
    QMenu *autoConnectMenu = new QMenu();

    // Create a custom QToolButton for the menu
    QToolButton *menuButton = new QToolButton();
    menuButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    menuButton->setText(
        "Auto Connect\nTerminals By Interfaces");
    menuButton->setIcon(
        QIcon(IconFactory::createConnectByInterfaceIcon()));
    menuButton->setIconSize(
        QSize(32, 32)); // Match the toolbar icon size

    // Create a QWidgetAction to hold the custom button
    QWidgetAction *widgetAction =
        new QWidgetAction(mainWindow);
    widgetAction->setDefaultWidget(menuButton);

    autoConnectMenu->addAction(widgetAction);

    connectVisibleTerminalsButton->setMenu(autoConnectMenu);
    connectVisibleTerminalsButton->setPopupMode(
        QToolButton::ToolButtonPopupMode::MenuButtonPopup);
    // QObject::connect(connectVisibleTerminalsButton,
    // &QToolButton::clicked,
    //                  [mainWindow]() {
    //                  UtilityFunctions::connectVisibleTerminalsByNetworks(mainWindow,
    //                  mainWindow->scene_); });

    // Connect the menu button's click signal
    // QObject::connect(menuButton, &QToolButton::clicked,
    //                  [mainWindow]() {
    //                  UtilityFunctions::connectVisibleTerminalsByInterfaces(mainWindow);
    //                  });

    networkToolsLayout->addWidget(
        connectVisibleTerminalsButton);

    // Add disconnect all terminals button
    QToolButton *disconnectAllTerminalsButton =
        new QToolButton();
    disconnectAllTerminalsButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    disconnectAllTerminalsButton->setText(
        "Disconnect Visible\nTerminals");
    disconnectAllTerminalsButton->setIcon(
        QIcon(IconFactory::createUnconnectTerminalsIcon()));
    // QObject::connect(disconnectAllTerminalsButton,
    // &QToolButton::clicked,
    //                  [mainWindow]() {
    //                      BasicButtonController::disconnectAllTerminals(
    //                          mainWindow,
    //                          mainWindow->getCurrentScene(),
    //                          (mainWindow->isRegionViewActive()
    //                          ? mainWindow->currentRegion_
    //                          : "*")
    //                          );
    //                  });
    networkToolsLayout->addWidget(
        disconnectAllTerminalsButton);

    QToolButton *checkNetworkButton = new QToolButton();
    checkNetworkButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    checkNetworkButton->setText("Check Region\nTerminals");
    checkNetworkButton->setIcon(
        QIcon(IconFactory::createCheckNetworkIcon()));
    // QObject::connect(checkNetworkButton,
    // &QToolButton::clicked,
    //                  [mainWindow]() {
    //                  BasicButtonController::checkNetwork(mainWindow,
    //                  mainWindow->scene_); });
    networkToolsLayout->addWidget(checkNetworkButton);

    mainWindow->networkToolsButtons_ = {
        checkNetworkButton, connectVisibleTerminalsButton,
        disconnectAllTerminalsButton};

    homeLayout->addWidget(mainWindow->networkToolsGroup_);

    // Create Simulation Tools group
    mainWindow->simulationToolsGroup_ =
        new QGroupBox("Simulation Tools");
    QHBoxLayout *simulationToolsLayout =
        new QHBoxLayout(mainWindow->simulationToolsGroup_);
    simulationToolsLayout->setSpacing(4);
    simulationToolsLayout->setContentsMargins(8, 12, 8, 8);

    // Add find shortest paths button
    QToolButton *shortestPathsButton = new QToolButton();
    shortestPathsButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    shortestPathsButton->setText(
        "Find Top Heuristic\nShortest Paths");
    shortestPathsButton->setIcon(
        QIcon(IconFactory::createShortestPathsIcon()));
    // QObject::connect(shortestPathsButton,
    // &QToolButton::clicked,
    //                  [mainWindow]() {
    //                      NetworkController::getTopShortestPaths(
    //                          mainWindow,
    //                          mainWindow->scene_,
    //                          mainWindow->globalMapScene_
    //                          );
    //                  });
    simulationToolsLayout->addWidget(shortestPathsButton);

    // Add verify by simulation button
    QToolButton *verifySimulationButton = new QToolButton();
    verifySimulationButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    verifySimulationButton->setText(
        "Verify by\nSimulation");
    verifySimulationButton->setIcon(
        QIcon(IconFactory::createVerifySimulationIcon()));
    // QObject::connect(verifySimulationButton,
    // &QToolButton::clicked,
    //                  [mainWindow]() {
    //                      NetworkController::validatePath(
    //                          mainWindow,
    //                          mainWindow->scene_,
    //                          mainWindow->globalMapScene_
    //                          );
    //                  });
    simulationToolsLayout->addWidget(
        verifySimulationButton);

    // Store the simulation tools buttons
    mainWindow->simulationToolsButtons_ = {
        shortestPathsButton, verifySimulationButton};
    homeLayout->addWidget(
        mainWindow->simulationToolsGroup_);

    // Create Logs group
    mainWindow->logsGroup_ = new QGroupBox("Logs");
    QHBoxLayout *logsLayout =
        new QHBoxLayout(mainWindow->logsGroup_);
    logsLayout->setSpacing(4);
    logsLayout->setContentsMargins(8, 12, 8, 8);

    // Add save logs button to the logs group
    QToolButton *saveLogsButton = new QToolButton();
    saveLogsButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    saveLogsButton->setText(
        "Export Servers\nCommunication Log");
    saveLogsButton->setIcon(QIcon::fromTheme(
        "document-save",
        QIcon(mainWindow->style()->standardIcon(
            QStyle::StandardPixmap::SP_DialogSaveButton))));
    // QObject::connect(saveLogsButton,
    // &QToolButton::clicked,
    //                  [mainWindow]() {
    //                  BasicButtonController::exportLog(mainWindow);
    //                  });
    logsLayout->addWidget(saveLogsButton);

    // Hide both the button and group initially
    saveLogsButton->hide();
    mainWindow->logsGroup_->hide();

    // Store the logs buttons
    mainWindow->logsButtons_ = {saveLogsButton};
    homeLayout->addWidget(mainWindow->logsGroup_);

    homeLayout->addStretch();
    int homeTabIndex =
        mainWindow->ribbon_->addTab(homeTab, "Home");

    // Create Import tab
    QWidget     *importTab    = new QWidget();
    QHBoxLayout *importLayout = new QHBoxLayout(importTab);
    importLayout->setSpacing(4);
    importLayout->setContentsMargins(4, 4, 4, 4);

    // Create View Import group
    mainWindow->viewImportGroup_ =
        new QGroupBox("View Import");
    QHBoxLayout *viewImportLayout =
        new QHBoxLayout(mainWindow->viewImportGroup_);
    viewImportLayout->setSpacing(4);
    viewImportLayout->setContentsMargins(8, 12, 8, 8);

    // Add background photo button
    QToolButton *bgPhotoButton = new QToolButton();
    bgPhotoButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    bgPhotoButton->setText("Import\nBackground Photo");
    bgPhotoButton->setIcon(QIcon(
        IconFactory::createSetBackgroundColorPixmap()));
    QObject::connect(bgPhotoButton, &QToolButton::clicked,
                     mainWindow, [&mainWindow]() {
                         ViewController::addBackgroundPhoto(
                             mainWindow);
                     });
    viewImportLayout->addWidget(bgPhotoButton);

    mainWindow->viewImportButtons_ = {bgPhotoButton};
    importLayout->addWidget(mainWindow->viewImportGroup_);

    // Create Network Import group
    mainWindow->networkImportGroup_ =
        new QGroupBox("Network Import");
    QHBoxLayout *networkImportLayout =
        new QHBoxLayout(mainWindow->networkImportGroup_);
    networkImportLayout->setSpacing(4);
    networkImportLayout->setContentsMargins(8, 12, 8, 8);

    // Train network import button
    QToolButton *trainImportButton = new QToolButton();
    trainImportButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    trainImportButton->setText("Import Train\nNetwork");
    trainImportButton->setIcon(
        QIcon(IconFactory::createFreightTrainIcon()));
    // QObject::connect(trainImportButton,
    // &QToolButton::clicked,
    //                  [mainWindow]() {
    //                      NetworkController::importTrainNetwork(
    //                          mainWindow,
    //                          mainWindow->currentRegion_,
    //                          ColorUtils::getRandomColor()
    //                          );
    //                  });
    networkImportLayout->addWidget(trainImportButton);

    // Truck network import button
    QToolButton *truckImportButton = new QToolButton();
    truckImportButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    truckImportButton->setText("Import Truck\nNetwork");
    truckImportButton->setIcon(
        QIcon(IconFactory::createFreightTruckIcon()));
    // QObject::connect(truckImportButton,
    // &QToolButton::clicked,
    //                  [mainWindow]() {
    //                      NetworkController::importTruckNetwork(
    //                          mainWindow,
    //                          mainWindow->currentRegion_,
    //                          ColorUtils::getRandomColor()
    //                          );
    //                  });
    networkImportLayout->addWidget(truckImportButton);

    mainWindow->networkImportButtons_ = {trainImportButton,
                                         truckImportButton};
    importLayout->addWidget(
        mainWindow->networkImportGroup_);

    // Create Transportation Vehicles group
    mainWindow->transportationVehiclesGroup_ =
        new QGroupBox("Transportation Vehicles");
    QHBoxLayout *transportationVehiclesLayout =
        new QHBoxLayout(
            mainWindow->transportationVehiclesGroup_);
    transportationVehiclesLayout->setSpacing(4);
    transportationVehiclesLayout->setContentsMargins(8, 12,
                                                     8, 8);

    // Train management button
    QToolButton *trainManagerButton = new QToolButton();
    trainManagerButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    trainManagerButton->setText("Train\nManager");
    trainManagerButton->setIcon(
        QIcon(IconFactory::createTrainManagerIcon()));
    QObject::connect(
        trainManagerButton, &QToolButton::clicked,
        [mainWindow]() {
            BasicButtonController::showTrainManager(
                mainWindow);
        });
    transportationVehiclesLayout->addWidget(
        trainManagerButton);

    // Ship management button
    QToolButton *shipManagerButton = new QToolButton();
    shipManagerButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    shipManagerButton->setText("Ship\nManager");
    shipManagerButton->setIcon(
        QIcon(IconFactory::createShipManagerIcon()));
    QObject::connect(
        shipManagerButton, &QToolButton::clicked,
        [mainWindow]() {
            BasicButtonController::showShipManager(
                mainWindow);
        });
    transportationVehiclesLayout->addWidget(
        shipManagerButton);

    // Store the transportation vehicles buttons
    mainWindow->transportationVehiclesButtons_ = {
        trainManagerButton, shipManagerButton};
    importLayout->addWidget(
        mainWindow->transportationVehiclesGroup_);

    importLayout->addStretch();
    int importTabIndex =
        mainWindow->ribbon_->addTab(importTab, "Import");

    // Create View tab
    QWidget     *viewTab    = new QWidget();
    QHBoxLayout *viewLayout = new QHBoxLayout(viewTab);
    viewLayout->setSpacing(4);
    viewLayout->setContentsMargins(4, 4, 4, 4);

    // Create Navigation group
    mainWindow->navigationGroup_ =
        new QGroupBox("Navigation");
    QHBoxLayout *navigationLayout =
        new QHBoxLayout(mainWindow->navigationGroup_);
    navigationLayout->setSpacing(4);
    navigationLayout->setContentsMargins(8, 12, 8, 8);

    // Add grid toggle button
    QToolButton *gridButton = new QToolButton();
    gridButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    gridButton->setIcon(
        QIcon(IconFactory::createShowHideGridIcon()));
    gridButton->setCheckable(true);
    gridButton->setChecked(true);
    gridButton->setText("Hide\nGrid");
    // QObject::connect(gridButton, &QToolButton::clicked,
    //                  [mainWindow](bool checked) {
    //                  BasicButtonController::toggleGrid(mainWindow,
    //                  checked); });
    navigationLayout->addWidget(gridButton);

    // Add pan mode toggle button
    QToolButton *panModeButton = new QToolButton();
    panModeButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    panModeButton->setIcon(
        QIcon(IconFactory::createPanModeIcon()));
    panModeButton->setText("Pan Mode:\nCtrl + Left");
    QObject::connect(panModeButton, &QToolButton::clicked,
                     mainWindow,
                     &MainWindow::togglePanMode);
    navigationLayout->addWidget(panModeButton);
    mainWindow->panModeButton_ = panModeButton;

    mainWindow->navigationButtons_ = {gridButton,
                                      panModeButton};
    viewLayout->addWidget(mainWindow->navigationGroup_);

    // Create Windows group
    mainWindow->windowsGroup_ = new QGroupBox("Windows");
    QHBoxLayout *windowsLayout =
        new QHBoxLayout(mainWindow->windowsGroup_);
    windowsLayout->setSpacing(4);
    windowsLayout->setContentsMargins(8, 12, 8, 8);

    // Add toggle buttons for dock widgets
    QToolButton *terminalLibraryButton = new QToolButton();
    terminalLibraryButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    terminalLibraryButton->setIcon(QIcon(
        IconFactory::createFreightTerminalLibraryIcon()));
    terminalLibraryButton->setCheckable(true);
    terminalLibraryButton->setChecked(true);
    terminalLibraryButton->setText(
        "Hide\nTerminal Library");
    QObject::connect(
        terminalLibraryButton, &QToolButton::clicked,
        [mainWindow, terminalLibraryButton](bool checked) {
            BasicButtonController::toggleDockWidget(
                checked, mainWindow->libraryDock_,
                terminalLibraryButton, "Terminal Library");
        });
    windowsLayout->addWidget(terminalLibraryButton);

    QToolButton *regionManagerButton = new QToolButton();
    regionManagerButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    regionManagerButton->setIcon(
        QIcon(IconFactory::createRegionManagerIcon()));
    regionManagerButton->setCheckable(true);
    regionManagerButton->setChecked(true);
    regionManagerButton->setText("Hide\nRegion Manager");
    QObject::connect(
        regionManagerButton, &QToolButton::clicked,
        [mainWindow, regionManagerButton](bool checked) {
            BasicButtonController::toggleDockWidget(
                checked, mainWindow->regionManagerDock_,
                regionManagerButton, "Region Manager");
        });
    windowsLayout->addWidget(regionManagerButton);

    QToolButton *regionNetworksButton = new QToolButton();
    regionNetworksButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    regionNetworksButton->setIcon(
        QIcon(IconFactory::createNetworkManagerIcon()));
    regionNetworksButton->setCheckable(true);
    regionNetworksButton->setChecked(true);
    regionNetworksButton->setText("Hide\nNetwork Manager");
    // QObject::connect(regionNetworksButton,
    // &QToolButton::clicked,
    //                  [mainWindow,
    //                  regionNetworksButton](bool checked)
    //                  {
    //                      ToolbarController::toggleDockWidget(
    //                          checked,
    //                          mainWindow->networkManagerDock_,
    //                          regionNetworksButton,
    //                          "Network Manager"
    //                          );
    //                  });
    windowsLayout->addWidget(regionNetworksButton);

    QToolButton *shortestPathsTableButton =
        new QToolButton();
    shortestPathsTableButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    shortestPathsTableButton->setIcon(
        QIcon(IconFactory::createShowHidePathsTableIcon()));
    shortestPathsTableButton->setCheckable(true);
    shortestPathsTableButton->setChecked(
        false); // Start unchecked since table is hidden
    shortestPathsTableButton->setText(
        "Show\nShortest Paths");
    QObject::connect(
        shortestPathsTableButton, &QToolButton::clicked,
        [mainWindow,
         shortestPathsTableButton](bool checked) {
            BasicButtonController::toggleDockWidget(
                checked, mainWindow->shortestPathTableDock_,
                shortestPathsTableButton, "Shortest Paths");
        });
    windowsLayout->addWidget(shortestPathsTableButton);

    QToolButton *propertiesButton = new QToolButton();
    propertiesButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    propertiesButton->setIcon(
        QIcon(IconFactory::createPropertiesIcon()));
    propertiesButton->setCheckable(true);
    propertiesButton->setChecked(true);
    propertiesButton->setText("Hide\nProperties");
    QObject::connect(
        propertiesButton, &QToolButton::clicked,
        [mainWindow, propertiesButton](bool checked) {
            BasicButtonController::toggleDockWidget(
                checked, mainWindow->propertiesDock_,
                propertiesButton, "Properties");
        });
    windowsLayout->addWidget(propertiesButton);

    QToolButton *settingsButton = new QToolButton();
    settingsButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    settingsButton->setIcon(
        QIcon(IconFactory::createSimulationSettingsIcon()));
    settingsButton->setCheckable(true);
    settingsButton->setChecked(true);
    settingsButton->setText("Hide\nSettings");
    QObject::connect(
        settingsButton, &QToolButton::clicked,
        [mainWindow, settingsButton](bool checked) {
            BasicButtonController::toggleDockWidget(
                checked, mainWindow->settingsDock_,
                settingsButton, "Settings");
        });
    windowsLayout->addWidget(settingsButton);

    // Create Visibility group
    mainWindow->visibilityGroup_ =
        new QGroupBox("Visibility");
    QHBoxLayout *visibilityLayout =
        new QHBoxLayout(mainWindow->visibilityGroup_);
    visibilityLayout->setSpacing(4);
    visibilityLayout->setContentsMargins(8, 12, 8, 8);

    // Add connection lines visibility button
    QToolButton *connectionLinesButton = new QToolButton();
    connectionLinesButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    connectionLinesButton->setText("Hide\nConnections");
    connectionLinesButton->setIcon(QIcon(
        IconFactory::createShowHideConnectionsIcon()));
    connectionLinesButton->setCheckable(true);
    connectionLinesButton->setChecked(true);
    // QObject::connect(connectionLinesButton,
    // &QToolButton::clicked,
    //                  [mainWindow](bool checked) {
    //                  BasicButtonController::toggleConnectionLines(mainWindow,
    //                  checked); });
    visibilityLayout->addWidget(connectionLinesButton);

    // Add terminals visibility button
    QToolButton *terminalsButton = new QToolButton();
    terminalsButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    terminalsButton->setText("Hide\nTerminals");
    terminalsButton->setIcon(
        QIcon(IconFactory::createShowHideTerminalsIcon()));
    terminalsButton->setCheckable(true);
    terminalsButton->setChecked(true);
    // QObject::connect(terminalsButton,
    // &QToolButton::clicked,
    //                  [mainWindow](bool checked) {
    //                  BasicButtonController::toggleTerminals(mainWindow,
    //                  checked); });
    visibilityLayout->addWidget(terminalsButton);

    // Store buttons for visibility management
    mainWindow->visibilityButtons_ = {connectionLinesButton,
                                      terminalsButton};
    viewLayout->addWidget(mainWindow->visibilityGroup_);

    mainWindow->windowsButtons_ = {
        regionManagerButton,  terminalLibraryButton,
        propertiesButton,     settingsButton,
        regionNetworksButton, shortestPathsTableButton};
    viewLayout->addWidget(mainWindow->windowsGroup_);
    viewLayout->addStretch();
    int viewTabIndex =
        mainWindow->ribbon_->addTab(viewTab, "View");

    mainWindow->windowVisibility_.clear();

    // Add each entry individually:
    mainWindow
        ->windowVisibility_[mainWindow->regionManagerDock_]
                           ["button"] =
        QVariant::fromValue(regionManagerButton);
    mainWindow
        ->windowVisibility_[mainWindow->regionManagerDock_]
                           ["tabs"] =
        QVariant::fromValue(QList<int>{0, 1});

    mainWindow->windowVisibility_[mainWindow->libraryDock_]
                                 ["button"] =
        QVariant::fromValue(terminalLibraryButton);
    mainWindow->windowVisibility_[mainWindow->libraryDock_]
                                 ["tabs"] =
        QVariant::fromValue(QList<int>{0});

    mainWindow
        ->windowVisibility_[mainWindow->propertiesDock_]
                           ["button"] =
        QVariant::fromValue(propertiesButton);
    mainWindow
        ->windowVisibility_[mainWindow->propertiesDock_]
                           ["tabs"] =
        QVariant::fromValue(QList<int>{0, 1});

    mainWindow->windowVisibility_[mainWindow->settingsDock_]
                                 ["button"] =
        QVariant::fromValue(settingsButton);
    mainWindow->windowVisibility_[mainWindow->settingsDock_]
                                 ["tabs"] =
        QVariant::fromValue(QList<int>{0, 1});

    mainWindow->windowVisibility_
        [mainWindow->shortestPathTableDock_]["button"] =
        QVariant::fromValue(shortestPathsTableButton);
    mainWindow->windowVisibility_
        [mainWindow->shortestPathTableDock_]["tabs"] =
        QVariant::fromValue(QList<int>{0, 1});

    mainWindow->networkManagerVisibility_.clear();

    mainWindow->networkManagerVisibility_
        [mainWindow->networkManagerDock_]["button"] =
        QVariant::fromValue(regionNetworksButton);
    mainWindow->networkManagerVisibility_
        [mainWindow->networkManagerDock_]["tabs"] =
        QVariant::fromValue(QList<int>{0});

    // Store tools button visibility configuration
    mainWindow->toolsButtonsVisibility_.clear();

    // Add each entry individually using QList<int> for the
    // values:
    mainWindow->toolsButtonsVisibility_[trainImportButton] =
        QList<int>{0};
    mainWindow->toolsButtonsVisibility_[truckImportButton] =
        QList<int>{0};
    mainWindow->toolsButtonsVisibility_[connectButton] =
        QList<int>{0, 1};
    mainWindow
        ->toolsButtonsVisibility_[linkTerminalButton] =
        QList<int>{0};
    mainWindow
        ->toolsButtonsVisibility_[unlinkTerminalButton] =
        QList<int>{0};
    mainWindow
        ->toolsButtonsVisibility_[setGlobalPositionButton] =
        QList<int>{1};
    mainWindow->toolsButtonsVisibility_[bgPhotoButton] =
        QList<int>{0, 1};

    // For regionWidget, if it's not a QToolButton, you'll
    // need a different approach Perhaps store it separately
    // or cast it if it's safe to do so
    if (QToolButton *toolButton =
            dynamic_cast<QToolButton *>(regionWidget))
    {
        mainWindow->toolsButtonsVisibility_[toolButton] =
            QList<int>{0};
    }

    mainWindow->toolsButtonsVisibility_[measureButton] =
        QList<int>{0, 1};
    mainWindow
        ->toolsButtonsVisibility_[clearMeasureButton] =
        QList<int>{0, 1};
    mainWindow
        ->toolsButtonsVisibility_[checkNetworkButton] =
        QList<int>{0, 1};
    mainWindow->toolsButtonsVisibility_
        [connectVisibleTerminalsButton] = QList<int>{0, 1};
    mainWindow->toolsButtonsVisibility_
        [disconnectAllTerminalsButton] = QList<int>{0, 1};
    mainWindow->toolsButtonsVisibility_[panModeButton] =
        QList<int>{0, 1};
    mainWindow->toolsButtonsVisibility_[saveLogsButton] =
        QList<int>{2};
    mainWindow->toolsButtonsVisibility_[newProjectButton] =
        QList<int>{0, 1};
    mainWindow->toolsButtonsVisibility_[openProjectButton] =
        QList<int>{0, 1};
    mainWindow->toolsButtonsVisibility_[saveProjectButton] =
        QList<int>{0, 1};
    mainWindow
        ->toolsButtonsVisibility_[shortestPathsButton] =
        QList<int>{0, 1};
    mainWindow
        ->toolsButtonsVisibility_[verifySimulationButton] =
        QList<int>{0, 1};
    mainWindow
        ->toolsButtonsVisibility_[trainManagerButton] =
        QList<int>{0};
    mainWindow->toolsButtonsVisibility_[shipManagerButton] =
        QList<int>{0};

    // Store tabs visibility configuration
    mainWindow->tabsVisibility_ = {
        {homeTabIndex, {0, 1, 2}},
        {importTabIndex, {0, 1}},
        {viewTabIndex, {0, 1}}};
}

} // namespace GUI
} // namespace CargoNetSim
