#include "MainWindow.h"

#include <QApplication>
#include <QBuffer>
#include <QByteArray>
#include <QCloseEvent>
#include <QCursor>
#include <QDateTime>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSplashScreen>
#include <QSplitter>
#include <QStatusBar>
#include <QToolButton>
#include <QVBoxLayout>

#include "GUI/Items/MapLine.h"
#include "Widgets/GraphicsScene.h"
#include "Widgets/GraphicsView.h"
#include "Widgets/NetworkManagerDialog.h"
#include "Widgets/PropertiesPanel.h"
#include "Widgets/RegionManagerWidget.h"
#include "Widgets/SettingsWidget.h"
#include "Widgets/ShortestPathTable.h"

#include "Items/BackgroundPhotoItem.h"
#include "Items/ConnectionLabel.h"
#include "Items/ConnectionLine.h"
#include "Items/GlobalTerminalItem.h"
#include "Items/MapPoint.h"
#include "Items/RegionCenterPoint.h"
#include "Items/TerminalItem.h"

#include "Utils/IconCreator.h"

#include "Backend/Controllers/CargoNetSimController.h"
#include "Controllers/BasicButtonController.h"
#include "Controllers/NetworkController.h"
#include "Controllers/ToolbarController.h"
#include "Controllers/UtilityFunctions.h"
#include "Controllers/ViewController.h"

#include "Serializers/ProjectSerializer.h"

namespace CargoNetSim
{
namespace GUI
{

MainWindow *MainWindow::instance_ = nullptr;

MainWindow *MainWindow::getInstance()
{
    if (!instance_)
    {
        instance_ = new MainWindow();
    }
    return instance_;
}

MainWindow::MainWindow()
    : CustomMainWindow()
    , heartbeatController_(nullptr)
{
    // Initialize region management
    CargoNetSim::CargoNetSimController::getInstance()
        .getRegionDataController()
        ->addRegion("Default Region");
    CargoNetSim::CargoNetSimController::getInstance()
        .getRegionDataController()
        ->setRegionVariable("Default Region", "color",
                            QColor(Qt::green));
    CargoNetSim::CargoNetSimController::getInstance()
        .getRegionDataController()
        ->setCurrentRegion("DefaultRegion");

    selectedTerminal_ = nullptr;
    tableWasVisible_  = false;
    previousTabIndex_ = 0;

    // Setup UI components
    initializeUI();

    // Create default region center
    QColor regionColor =
        CargoNetSim::CargoNetSimController::getInstance()
            .getRegionDataController()
            ->getRegionVariableAs<QColor>("Default Region",
                                          "color");

    ViewController::createRegionCenter(
        this, "Default Region", regionColor, QPoint(0, 0),
        true);

    // Initialize heartbeat controller
    heartbeatController_ = new HeartbeatController(this);
    heartbeatController_->initialize();

    // Set window title
    setWindowTitle("CargoNetSim: Multimodal Freight "
                   "Operations Optimizer");
    resize(1000, 700);

    showStatusBarMessage("Ready.");

    // Update hte regions combo box
    BasicButtonController::updateRegionComboBox(this);

    // Setup the signals
    BasicButtonController::setupSignals(this);
}

MainWindow::~MainWindow()
{
    // Cleanup resources
    delete heartbeatController_;

    // Clear log timers
    if (logTimer_)
    {
        logTimer_->stop();
        delete logTimer_;
    }

    if (progressTimer_)
    {
        progressTimer_->stop();
        delete progressTimer_;
    }

    // Scene items will be cleaned up by Qt's parent-child
    // mechanism
}

void MainWindow::initializeUI()
{
    // Load the window icon
    QString imagePath      = ":/Logo25";
    auto    originalPixmap = QPixmap(imagePath);

    if (originalPixmap.isNull())
    {
        qWarning() << "Failed to load logo image:"
                   << imagePath;
        // Create a default splash pixmap
        originalPixmap = QPixmap(25, 25);
        originalPixmap.fill(Qt::white);

        QPainter painter(&originalPixmap);
        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", 5, QFont::Bold));
        painter.drawText(originalPixmap.rect(),
                         Qt::AlignCenter, "CNS");
    }

    QIcon appIcon(originalPixmap);
    if (!appIcon.isNull())
    {
        setWindowIcon(appIcon);
    }

    // Create tab widget for main view and global map
    tabWidget_ = new QTabWidget(this);
    tabWidget_->setTabsClosable(false);

    // Create main view tab
    QWidget     *mainViewTab = new QWidget();
    QVBoxLayout *mainViewLayout =
        new QVBoxLayout(mainViewTab);
    mainViewLayout->setContentsMargins(0, 0, 0, 0);

    // Setup scene and view
    setupRegionMapScene();
    mainViewLayout->addWidget(regionView_);

    // Create global map tab
    QWidget     *globalMapTab = new QWidget();
    QVBoxLayout *globalMapLayout =
        new QVBoxLayout(globalMapTab);
    globalMapLayout->setContentsMargins(0, 0, 0, 0);

    // Setup global map scene
    setupGlobalMapScene();
    globalMapLayout->addWidget(globalMapView_);

    // Create logging tab
    loggingTab_ = new QWidget();
    setupLoggingTab();

    // Add tabs to tab widget
    tabWidget_->addTab(mainViewTab, "Region View");
    tabWidget_->addTab(globalMapTab, "Global Map");
    tabWidget_->addTab(loggingTab_, "Servers Log");

    // Set tab widget as central widget
    centerSplitter->addWidget(tabWidget_);

    // Connect tab change signal
    connect(tabWidget_, &QTabWidget::currentChanged, this,
            &MainWindow::handleTabChange);

    // Setup connection menu
    connectionMenu_ = new QMenu(this);
    connectionTypes_ << "Truck" << "Rail" << "Ship";
    currentConnectionType_ = "Truck";

    // Add connection types to menu
    for (const QString &connType : connectionTypes_)
    {
        QAction *action =
            connectionMenu_->addAction(connType);
        action->setCheckable(true);
        connect(action, &QAction::triggered,
                [this, connType](bool checked) {
                    setConnectionType(connType);
                });
    }

    // Set initial checked state
    connectionMenu_->actions().at(0)->setChecked(true);

    // Setup docks
    setupDocks();

    // Setup toolbar
    ToolbarController::setupToolbar(this);

    // Setup status bar
    setupStatusBar();

    // Start queue processing
    startQueueProcessing();
}

void MainWindow::setupRegionMapScene()
{
    // Setup scene and view
    regionScene_ = new GraphicsScene(this);
    regionView_  = new GraphicsView(regionScene_);
    regionView_->setScene(regionScene_);

    // Add connection methods
    regionScene_->setIsInConnectMode(false);
    regionScene_->setConnectedFirstItem(QVariant());
}

void MainWindow::setupGlobalMapScene()
{
    globalMapScene_ = new GraphicsScene(this);
    globalMapView_  = new GraphicsView(globalMapScene_);

    // Force geodetic coordinates for global map
    globalMapView_->setUsingProjectedCoords(false);
    globalMapView_->setScene(globalMapScene_);

    // Add connection methods
    globalMapScene_->setIsInConnectMode(false);
    globalMapScene_->setConnectedFirstItem(QVariant());
}

void MainWindow::setupDocks()
{
    // Properties panel dock
    propertiesDock_  = new QDockWidget("Properties", this);
    propertiesPanel_ = new PropertiesPanel(this);
    propertiesDock_->setWidget(propertiesPanel_);
    addDockWidget(Qt::RightDockWidgetArea, propertiesDock_);
    propertiesDock_->hide(); // Start with properties hidden

    // Settings dock
    settingsDock_ =
        new QDockWidget("Simulation Settings", this);
    settingsWidget_ = new SettingsWidget(this);
    settingsDock_->setWidget(settingsWidget_);
    addDockWidget(Qt::RightDockWidgetArea, settingsDock_);

    // Tabify properties and settings docks
    tabifyDockWidget(propertiesDock_, settingsDock_);

    // Make settings visible by default instead of hiding
    // properties
    settingsDock_->raise();

    // Shortest paths table dock
    shortestPathTableDock_ =
        new QDockWidget("Shortest Paths Table", this);
    shortestPathTable_ = new ShortestPathsTable(this);
    shortestPathTableDock_->setWidget(shortestPathTable_);
    centerSplitter->addWidget(shortestPathTableDock_);
    shortestPathTableDock_
        ->hide(); // Start with shortest paths table hidden

    // Connect the settingsChanged signal
    connect(
        settingsWidget_, &SettingsWidget::settingsChanged,
        [this](const QMap<QString, QVariant> &settings) {
            showStatusBarMessage(
                "Simulation settings updated.", 2000);
        });

    // Terminal library dock
    setupTerminalLibrary();

    // Region manager dock
    setupRegionManager();

    // Network manager dock
    networkManagerDock_ = new NetworkManagerDialog(this);

    // Tabify the region manager and network manager docks
    tabifyDockWidget(regionManagerDock_,
                     networkManagerDock_);

    // Ensure region manager is visible by default
    regionManagerDock_->raise();
}

void MainWindow::setupTerminalLibrary()
{
    libraryDock_ =
        new QDockWidget("Terminal Library", this);
    libraryList_ = new QListWidget();
    libraryList_->setIconSize(QSize(32, 32));
    libraryList_->setDragEnabled(true);

    // Create terminal icons
    QMap<QString, QPixmap> terminalIcons =
        IconFactory::createTerminalIcons();

    // Add items with custom icons
    for (auto it = terminalIcons.constBegin();
         it != terminalIcons.constEnd(); ++it)
    {
        QListWidgetItem *item = new QListWidgetItem(
            QIcon(it.value()), it.key());
        item->setData(
            Qt::UserRole,
            it.value()); // Store pixmap for later use
        libraryList_->addItem(item);
    }

    libraryDock_->setWidget(libraryList_);
    addDockWidget(Qt::LeftDockWidgetArea, libraryDock_);
}

void MainWindow::setupRegionManager()
{
    regionManagerDock_ =
        new QDockWidget("Region Manager", this);
    regionManager_ = new RegionManagerWidget(this);
    regionManagerDock_->setWidget(regionManager_);
    addDockWidget(Qt::LeftDockWidgetArea,
                  regionManagerDock_);
    regionManagerDock_->resize(regionManagerDock_->width(),
                               200);
}

void MainWindow::setupLoggingTab()
{
    // Create main layout for logging tab
    QGridLayout *layout = new QGridLayout(loggingTab_);

    // Client names
    clientNames_ << "ShipClient" << "TrainClient"
                 << "TruckClient"
                 << "TerminalClient" << "CargoNetSim";

    // Create 2x2 grid of logging widgets for first 4
    // clients
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            int clientIndex = i * 2 + j;
            if (clientIndex >= clientNames_.size() - 1)
            {
                continue;
            }

            QString clientName = clientNames_[clientIndex];

            // Create group box for each client
            QGroupBox   *group = new QGroupBox(clientName);
            QVBoxLayout *groupLayout =
                new QVBoxLayout(group);

            // Progress bar and label
            QHBoxLayout *progressLayout = new QHBoxLayout();
            QProgressBar *progressBar = new QProgressBar();
            progressBar->setMaximum(100);
            progressBar->setValue(0);
            progressLayout->addWidget(progressBar);

            groupLayout->addLayout(progressLayout);

            // Text widget and scrollbar
            QTextEdit *textWidget = new QTextEdit();
            textWidget->setReadOnly(true);
            groupLayout->addWidget(textWidget);

            // Store references
            logTextWidgets_.append(textWidget);
            progressBars_.append(progressBar);

            // Add to main layout
            layout->addWidget(group, i, j);
        }
    }

    // Create general log section (CargoNetSim)
    QGroupBox *generalGroup =
        new QGroupBox(clientNames_.last());
    QVBoxLayout *generalLayout =
        new QVBoxLayout(generalGroup);

    // Progress bar and label for general log
    QHBoxLayout  *generalProgressLayout = new QHBoxLayout();
    QProgressBar *generalProgressBar = new QProgressBar();
    generalProgressBar->setMaximum(100);
    generalProgressLayout->addWidget(generalProgressBar);

    generalLayout->addLayout(generalProgressLayout);

    // Text widget for general log
    QTextEdit *generalText = new QTextEdit();
    generalText->setReadOnly(true);
    generalLayout->addWidget(generalText);

    // Store references
    logTextWidgets_.append(generalText);
    progressBars_.append(generalProgressBar);

    // Add general section to main layout
    layout->addWidget(generalGroup, 2, 0, 1, 2);
}

void MainWindow::startQueueProcessing()
{
    // Create timers for processing queues
    logTimer_ = new QTimer(this);
    connect(logTimer_, &QTimer::timeout, this,
            &MainWindow::processLogQueue);
    logTimer_->start(100);

    progressTimer_ = new QTimer(this);
    connect(progressTimer_, &QTimer::timeout, this,
            &MainWindow::processProgressQueue);
    progressTimer_->start(100);
}

void MainWindow::processLogQueue()
{
    // while (!ApplicationLogger::logQueueEmpty()) {
    //     ApplicationLogger::LogMessage message =
    //     ApplicationLogger::getNextLogMessage();

    //     // Add to appropriate log widget in the logging
    //     tab appendLog(message.text, message.widgetIndex,
    //     message.isError);

    //     // Also display important messages in the status
    //     bar if (message.isError) {
    //         // Show errors in the backend message area
    //         with red color
    //         updateBackendMessage(message.text, "error",
    //         8000);
    //     } else if
    //     (message.text.toLower().contains("created") ||
    //                message.text.toLower().contains("established")
    //                ||
    //                message.text.toLower().contains("success")
    //                ||
    //                message.text.toLower().contains("connected")
    //                ||
    //                message.text.toLower().contains("added"))
    //                {
    //         // Show success messages
    //         updateBackendMessage(message.text, "success",
    //         5000);
    //     } else if (message.text.startsWith("Message
    //     Sent:")) {
    //         // Show API calls
    //         updateBackendMessage(message.text, "info",
    //         3000);
    //     }
    // }
}

void MainWindow::processProgressQueue()
{
    // while (!ApplicationLogger::progressQueueEmpty()) {
    //     ApplicationLogger::ProgressInfo progress =
    //     ApplicationLogger::getNextProgressUpdate(); if
    //     (progress.clientIndex < progressBars_.size()) {
    //         progressBars_[progress.clientIndex]->setValue(progress.value);
    //     }
    // }
}

void MainWindow::appendLog(const QString &message,
                           int widgetIndex, bool isError)
{
    if (widgetIndex >= logTextWidgets_.size())
    {
        return;
    }

    QTextEdit  *textWidget = logTextWidgets_[widgetIndex];
    QTextCursor cursor     = textWidget->textCursor();
    cursor.movePosition(QTextCursor::End);
    textWidget->setTextCursor(cursor);

    // Create format for error messages
    if (isError)
    {
        QTextCharFormat format;
        format.setForeground(QColor("red"));
        cursor.setCharFormat(format);
    }

    cursor.insertText(message + "\n");
    textWidget->verticalScrollBar()->setValue(
        textWidget->verticalScrollBar()->maximum());
}

void MainWindow::setupStatusBar()
{
    // First, get the existing status bar
    QStatusBar *statusBar = this->statusBar();

    // Create a custom widget to fill the entire status bar
    QWidget     *mainContainer = new QWidget();
    QHBoxLayout *mainLayout =
        new QHBoxLayout(mainContainer);
    mainLayout->setContentsMargins(4, 0, 4, 0);
    mainLayout->setSpacing(6);

    // 1. LEFT SECTION - Status messages and spinner
    QWidget     *leftContainer = new QWidget();
    QHBoxLayout *leftLayout =
        new QHBoxLayout(leftContainer);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(6);

    // Add the spinner BEFORE the status label
    // Spinner widget
    statusSpinner_ = new SpinnerWidget();
    statusSpinner_->setFixedSize(16, 16);
    // Get the application palette's text color
    QColor textColor = palette().color(QPalette::Text);
    // Use that color for the spinner
    statusSpinner_->setSpinnerColor(textColor);
    statusSpinner_->setVisibleWhenIdle(false);
    leftLayout->addWidget(statusSpinner_);

    // Status label - add AFTER the spinner
    statusLabel_ = new QLabel("Ready.");
    statusLabel_->setMinimumWidth(300);
    statusLabel_->setMaximumWidth(400);
    leftLayout->addWidget(statusLabel_);

    mainLayout->addWidget(leftContainer);

    // 2. CENTER SECTION - Server indicators
    // Use a separate layout to ensure proper centering
    QWidget     *centerContainer = new QWidget();
    QHBoxLayout *centerLayout =
        new QHBoxLayout(centerContainer);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(8);
    centerLayout->setAlignment(Qt::AlignCenter);

    // Server indicators label
    QLabel *serverLabel = new QLabel("Servers:");
    centerLayout->addWidget(serverLabel);

    // Define servers with their display names
    QMap<QString, QMap<QString, QString>> serverInfo;
    serverInfo["NeTrainSim"] = {
        {"shortDesc", "Trains Server"},
        {"longDesc", "Open Source Large-Scale Network "
                     "Train Simulator"}};
    serverInfo["ShipNetSim"] = {
        {"shortDesc", "Ships Server"},
        {"longDesc", "Open Source Large-Scale Maritime "
                     "Transport Simulator"}};
    serverInfo["INTEGRATION"] = {
        {"shortDesc", "Trucks Server"},
        {"longDesc",
         "Free-ware Large-Scale Traffic Simulator"}};
    serverInfo["TerminalSim"] = {
        {"shortDesc", "Terminal Graph Server"},
        {"longDesc",
         "Intermodal Terminal Management System"}};

    // Add indicators for each server
    QMap<QString, QMap<QString, QVariant>> serverIndicators;

    for (auto it = serverInfo.constBegin();
         it != serverInfo.constEnd(); ++it)
    {
        QString serverId  = it.key();
        QString shortDesc = it.value()["shortDesc"];
        QString longDesc  = it.value()["longDesc"];

        // Create indicator container
        QWidget     *container = new QWidget();
        QHBoxLayout *layout    = new QHBoxLayout(container);
        layout->setContentsMargins(2, 0, 2, 0);
        layout->setSpacing(4);

        // Status indicator dot
        QLabel *indicator = new QLabel();
        indicator->setFixedSize(10, 10);
        indicator->setStyleSheet(
            "background-color: #808080; border-radius: "
            "5px;");
        indicator->setToolTip(shortDesc
                              + " - Disconnected");

        // Server label
        QLabel *label = new QLabel(serverId);
        label->setToolTip(longDesc);

        // Add widgets to layout
        layout->addWidget(indicator);
        layout->addWidget(label);

        // Add to center layout
        centerLayout->addWidget(container);

        // Store reference
        QMap<QString, QVariant> indicatorData;
        indicatorData["indicator"] =
            QVariant::fromValue<QLabel *>(indicator);
        indicatorData["label"] =
            QVariant::fromValue<QLabel *>(label);
        indicatorData["description"]          = shortDesc;
        indicatorData["detailed_description"] = longDesc;
        indicatorData["last_heartbeat"]       = 0;
        serverIndicators[serverId] = indicatorData;
    }

    // Add the center container with a stretch on both sides
    // to keep it centered
    mainLayout->addStretch(1);
    mainLayout->addWidget(centerContainer);
    mainLayout->addStretch(1);

    // 3. RIGHT SECTION - Backend message (fixed width)
    QWidget     *rightContainer = new QWidget();
    QHBoxLayout *rightLayout =
        new QHBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(4);

    // Backend message icon
    backendIcon_ = new QLabel();
    backendIcon_->setFixedSize(10, 10);
    backendIcon_->setStyleSheet(
        "background-color: #0066cc; border-radius: 5px;");
    backendIcon_->setVisible(false); // Hide initially
    rightLayout->addWidget(backendIcon_);

    // Backend message label
    backendReportLabel_ = new QLabel("");
    backendReportLabel_->setMinimumWidth(300);
    backendReportLabel_->setMaximumWidth(
        400); // Limit width
    rightLayout->addWidget(backendReportLabel_);

    // Add the right container
    mainLayout->addWidget(rightContainer);

    // Add our custom widget to take over the entire status
    // bar
    statusBar->addWidget(
        mainContainer,
        1); // Stretch factor 1 makes it fill the bar

    // Initialize message queue
    isProcessingMessageQueue_ = false;

    QTimer *messageQueueTimer_ = new QTimer(this);
    connect(messageQueueTimer_, &QTimer::timeout, this,
            &MainWindow::processMessageQueue);
    messageQueueTimer_->start(
        100); // Check queue every 100ms
}

void MainWindow::setConnectionType(
    const QString &connectionType)
{
    currentConnectionType_ = connectionType;

    // Uncheck all other actions
    for (QAction *action : connectionMenu_->actions())
    {
        action->setChecked(action->text()
                           == connectionType);
    }

    showStatusBarMessage(
        QString("Connection type set to: %1")
            .arg(connectionType),
        2000);
}

GraphicsView *MainWindow::getCurrentView() const
{
    if (tabWidget_->currentIndex() == 0)
    { // Main view tab
        return regionView_;
    }
    else if (tabWidget_->currentIndex() == 1)
    { // Global map tab
        return globalMapView_;
    }
    return nullptr;
}

GraphicsScene *MainWindow::getCurrentScene() const
{
    if (tabWidget_->currentIndex() == 0)
    { // Main view tab
        return regionScene_;
    }
    else if (tabWidget_->currentIndex() == 1)
    { // Global map tab
        return globalMapScene_;
    }
    return nullptr;
}

bool MainWindow::isGlobalViewActive() const
{
    return tabWidget_->currentIndex() == 1;
}

bool MainWindow::isRegionViewActive() const
{
    return tabWidget_->currentIndex() == 0;
}

void MainWindow::handleTabChange(int index)
{
    bool isGlobalMap =
        index == tabWidget_->indexOf(tabWidget_->widget(1));
    bool isLoggingTab =
        index == tabWidget_->indexOf(tabWidget_->widget(2));
    bool isMainView = !(isGlobalMap || isLoggingTab);

    // Common state reset
    regionScene_->setIsInConnectMode(false);
    globalMapScene_->setIsInConnectMode(false);

    // Reset measurement mode when changing tabs
    measureButton_->setChecked(false);
    BasicButtonController::resetOtherButtons(this);

    // Handle tool button visibility based on current tab
    for (auto it = toolsButtonsVisibility_.begin();
         it != toolsButtonsVisibility_.end(); ++it)
    {
        QWidget     *button     = it.key();
        QList<int>   tabIndices = it.value();
        button->setVisible(tabIndices.contains(index));
    }

    // Handle tab visibility in the toolbar
    for (auto it = tabsVisibility_.begin();
         it != tabsVisibility_.end(); ++it)
    {
        int        toolbarTabIndex = it.key();
        QList<int> tabIndices      = it.value();
        toolbar_->setTabVisible(toolbarTabIndex,
                                tabIndices.contains(index));
    }

    // Handle dock window visibility
    for (auto it = windowVisibility_.begin();
         it != windowVisibility_.end(); ++it)
    {
        QDockWidget            *dockWindow = it.key();
        QMap<QString, QVariant> config     = it.value();

        bool isTabAllowed =
            config["tabs"].toList().contains(index);
        bool isButtonChecked = config["button"]
                                   .value<QToolButton *>()
                                   ->isChecked();
        config["button"].value<QToolButton *>()->setEnabled(
            isTabAllowed);
        dockWindow->setVisible(isTabAllowed
                               && isButtonChecked);
    }

    // Update group visibility
    updateGroupVisibility(toolsGroup_, toolsButtons_);
    updateGroupVisibility(measurementsGroup_,
                          measurementsButtons_);
    updateGroupVisibility(regionGroup_, regionWidgets_);
    updateGroupVisibility(networkImportGroup_,
                          networkImportButtons_);
    updateGroupVisibility(navigationGroup_,
                          navigationButtons_);
    updateGroupVisibility(windowsGroup_, windowsButtons_);
    updateGroupVisibility(logsGroup_, logsButtons_);
    updateGroupVisibility(networkToolsGroup_,
                          networkToolsButtons_);
    updateGroupVisibility(projectGroup_, projectButtons_);
    updateGroupVisibility(simulationToolsGroup_,
                          simulationToolsButtons_);
    updateGroupVisibility(transportationVehiclesGroup_,
                          transportationVehiclesButtons_);
    updateGroupVisibility(visibilityGroup_,
                          visibilityButtons_);

    // Handle shortest paths table visibility
    if (isLoggingTab)
    {
        // Save current visibility state before hiding
        tableWasVisible_ =
            shortestPathTableDock_->isVisible();
        shortestPathTableDock_->hide();
    }
    else if (previousTabIndex_
             == tabWidget_->indexOf(tabWidget_->widget(2)))
    { // Coming from logging tab
        if (tableWasVisible_)
        {
            shortestPathTableDock_->show();
        }
    }

    // Store current tab index for next time
    previousTabIndex_ = index;
}

void MainWindow::updateGroupVisibility(
    QGroupBox *group, const QList<QWidget *> &buttons)
{
    int  currentTab         = tabWidget_->currentIndex();
    bool anyShouldBeVisible = false;

    for (QWidget *button : buttons)
    {
        // If button has tab visibility rules, check them
        if (toolsButtonsVisibility_.contains(button))
        {
            if (toolsButtonsVisibility_[button].contains(
                    currentTab))
            {
                anyShouldBeVisible = true;
                break;
            }
        }
        else
        {
            // If button has no visibility rules, assume it
            // should be visible
            anyShouldBeVisible = true;
            break;
        }
    }

    group->setVisible(anyShouldBeVisible);
}

void MainWindow::updateAllCoordinates()
{
    if (!regionScene_ || !regionView_)
        return;

    // Get all items in the scene
    QList<QGraphicsItem*> allItems = regionScene_->items();

    // Update the properties panel if it's showing coordinates
    if (propertiesPanel_ && propertiesPanel_->getCurrentItem()) {
        QGraphicsItem* currentItem = propertiesPanel_->getCurrentItem();
        if (dynamic_cast<RegionCenterPoint*>(currentItem) ||
            dynamic_cast<MapPoint*>(currentItem) ||
            dynamic_cast<TerminalItem*>(currentItem) ||
            dynamic_cast<BackgroundPhotoItem*>(currentItem)) {
            propertiesPanel_->displayProperties(currentItem);
        }
    }

    // Update the view
    regionView_->viewport()->update();
}

void MainWindow::showStatusBarMessage(QString message,
                                      int     timeout)
{
    // Add message to queue
    StatusMessage newMessage;
    newMessage.message = message;
    newMessage.timeout =
        timeout > 0
            ? timeout
            : 5000; // Default 5 seconds if not specified
    newMessage.timestamp = QDateTime::currentDateTime();
    newMessage.isError   = false;

    messageQueue_.append(newMessage);

    // Limit non-error messages to the latest 2
    int nonErrorCount = 0;
    for (int i = messageQueue_.size() - 1; i >= 0; i--)
    {
        if (!messageQueue_[i].isError)
        {
            nonErrorCount++;
            if (nonErrorCount > 2)
            {
                // Remove the older message
                messageQueue_.removeAt(i);
            }
        }
    }
}

void MainWindow::showStatusBarError(QString message,
                                    int     timeout)
{
    // Add error message to queue
    StatusMessage newMessage;
    newMessage.message = message;
    newMessage.timeout =
        timeout > 0
            ? timeout
            : 5000; // Default 5 seconds if not specified
    newMessage.timestamp = QDateTime::currentDateTime();
    newMessage.isError   = true;

    // Remove all non-error messages
    for (int i = messageQueue_.size() - 1; i >= 0; i--)
    {
        if (!messageQueue_[i].isError)
        {
            messageQueue_.removeAt(i);
        }
    }

    messageQueue_.append(newMessage);
}

void MainWindow::startStatusProgress()
{
    // Only start if we have a spinner
    if (statusSpinner_)
    {
        statusSpinner_->startSpinning();
    }
}

void MainWindow::stopStatusProgress()
{
    // Only stop if we have a spinner
    if (statusSpinner_)
    {
        statusSpinner_->stopSpinning();
    }
}

void MainWindow::processMessageQueue()
{
    // If already processing a message, do nothing
    if (isProcessingMessageQueue_)
    {
        return;
    }

    // If queue is empty, set default message based on
    // spinner state
    if (messageQueue_.isEmpty())
    {
        if (statusSpinner_ && statusSpinner_->isSpinning())
        {
            statusLabel_->setText("Processing...");
        }
        else
        {
            statusLabel_->setText("Ready.");
        }
        statusLabel_->setStyleSheet("");
        return;
    }

    // Mark as processing
    isProcessingMessageQueue_ = true;

    // Prioritize error messages - find the first error
    // message if any
    int messageIndex = 0;
    for (int i = 0; i < messageQueue_.size(); i++)
    {
        if (messageQueue_[i].isError)
        {
            messageIndex = i;
            break;
        }
    }

    // Get the prioritized message
    StatusMessage currentMessage =
        messageQueue_[messageIndex];

    // Display the message
    statusLabel_->setText(currentMessage.message);

    // Apply styling if it's an error message
    if (currentMessage.isError)
    {
        statusLabel_->setStyleSheet("color: red;");
    }
    else
    {
        statusLabel_->setStyleSheet("");
    }

    // Schedule message removal after timeout
    QTimer::singleShot(
        currentMessage.timeout, this,
        [this, messageIndex]() {
            // Remove the message from the queue
            if (messageIndex < messageQueue_.size())
            {
                messageQueue_.removeAt(messageIndex);
            }

            // Reset processing flag
            isProcessingMessageQueue_ = false;

            // Process next message immediately if available
            if (!messageQueue_.isEmpty())
            {
                processMessageQueue();
            }
            else
            {
                // Set text based on spinner state
                if (statusSpinner_
                    && statusSpinner_->isSpinning())
                {
                    statusLabel_->setText("Processing...");
                }
                else
                {
                    statusLabel_->setText("Ready.");
                }
                statusLabel_->setStyleSheet("");
            }
        });
}

void MainWindow::togglePanMode()
{
    // Toggle between pan modes
    GraphicsView *view = getCurrentView();
    if (!view)
        return;

    QString currentMode = view->getCurrentPanMode();
    QString newMode     = (currentMode == "middle_mouse")
                              ? "ctrl_left"
                              : "middle_mouse";

    // Update both views
    regionView_->setCurrentPanMode(newMode);
    globalMapView_->setCurrentPanMode(newMode);

    // Update button text
    QString buttonText = (newMode == "middle_mouse")
                             ? "Pan Mode:\nMiddle Mouse"
                             : "Pan Mode:\nCtrl + Left";
    panModeButton_->setText(buttonText);

    // Show status message
    QString modeText = (newMode == "middle_mouse")
                           ? "middle mouse button"
                           : "Ctrl + left mouse button";
    showStatusBarMessage(
        QString("Panning mode changed to %1").arg(modeText),
        2000);
}

void MainWindow::handleTerminalNodeLinking(
    QGraphicsItem *item)
{
    if (!regionScene_->isInLinkTerminalMode())
    {
        return;
    }

    TerminalItem *terminalItem =
        dynamic_cast<TerminalItem *>(item);
    if (terminalItem)
    {
        selectedTerminal_ = terminalItem;
        showStatusBarMessage(
            "Terminal selected. Now select a node to link "
            "it to...",
            2000);
        return;
    }

    MapPoint *mapPoint = dynamic_cast<MapPoint *>(item);
    if (mapPoint && selectedTerminal_)
    {
        UtilitiesFunctions::linkMapPointToTerminal(
            this, mapPoint, selectedTerminal_);

        // Exit linking mode
        linkTerminalButton_->setChecked(false);
        regionScene_->setIsInLinkTerminalMode(false);
        selectedTerminal_ = nullptr;
    }
    else if (!selectedTerminal_)
    {
        showStatusBarError("Please select a terminal first",
                           2000);
    }
}

void MainWindow::handleTerminalNodeUnlinking(
    QGraphicsItem *item)
{
    if (!regionScene_->isInUnlinkTerminalMode())
    {
        return;
    }

    MapPoint *mapPoint = dynamic_cast<MapPoint *>(item);
    if (mapPoint)
    {
        mapPoint->setLinkedTerminal(
            nullptr); // Unlink the terminal from the node

        // Update the properties panel if this item is
        // currently selected
        if (propertiesPanel_->getCurrentItem() == mapPoint)
        {
            propertiesPanel_->displayProperties(mapPoint);
        }

        // Exit unlinking mode
        unlinkTerminalButton_->setChecked(false);
        regionScene_->setIsInUnlinkTerminalMode(false);
        selectedTerminal_                = nullptr;
        showStatusBarMessage(
            "Terminal unlinked successfully", 2000);

        // Force a redraw of the MapPoint
        mapPoint->update();
    }
}

void MainWindow::toggleShortestPathsTable(bool show)
{
    // if (!show) {
    //     shortestPathTableDock_->hide();
    //     // Save the current sizes before hiding
    //     savedSplitterSizes_ = centerSplitter->sizes();
    //     // Give all space to the tab widget
    //     int totalHeight = savedSplitterSizes_.at(0) +
    //     savedSplitterSizes_.at(1);
    //     centerSplitter->setSizes({totalHeight, 0});
    // } else {
    //     shortestPathTableDock_->show();
    //     // Restore previous sizes if they were saved
    //     if (!savedSplitterSizes_.isEmpty()) {
    //         centerSplitter->setSizes(savedSplitterSizes_);
    //     } else {
    //         // Default split if no saved sizes
    //         int totalHeight = centerSplitter->height();
    //         centerSplitter->setSizes({int(totalHeight *
    //         0.7), int(totalHeight * 0.3)});
    //     }
    // }
}

void MainWindow::showError(const QString &errorText)
{
    QMessageBox msg;
    msg.setIcon(QMessageBox::Critical);
    msg.setText("An error occurred");
    msg.setDetailedText(errorText);
    msg.setWindowTitle("Error");
    msg.exec();
}

void MainWindow::updateServerHeartbeat(
    const QString &serverId, float timestamp)
{
    // This method no longer does anything, as we've removed
    // heartbeat functionality and rely exclusively on
    // consumer checks
}

void MainWindow::updateBackendMessage(
    const QString &message, const QString &status,
    int timeout)
{
    // Show the backend icon
    backendIcon_->setVisible(true);

    // Set style based on message type
    if (status.toLower() == "error"
        || message.toLower().contains("not exist")
        || message.toLower().contains("failed"))
    {
        backendReportLabel_->setStyleSheet(
            "color: #cc0000; font-weight: bold;"); // Red
                                                   // for
                                                   // errors
    }
    else if (status.toLower() == "success"
             || message.toLower().contains("success")
             || message.toLower().contains("created")
             || message.toLower().contains("established"))
    {
        backendReportLabel_->setStyleSheet(
            "color: #007700;"); // Green for success
    }
    else
    {
        backendReportLabel_->setStyleSheet(
            "color: #0066cc;"); // Blue for info
    }

    // Update the text
    backendReportLabel_->setText(message);

    // Auto-clear after a timeout if specified
    if (timeout > 0)
    {
        QTimer::singleShot(
            timeout, this,
            &MainWindow::clearBackendMessage);
    }
}

void MainWindow::clearBackendMessage()
{
    backendReportLabel_->setText("");
    backendIcon_->setVisible(false);
}

void MainWindow::shutdown()
{
    // Signal application shutdown
    QApplication::quit();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    CustomMainWindow::resizeEvent(event);
}

void MainWindow::assignSelectedToCurrentRegion()
{
    for (QGraphicsItem *item :
         regionScene_->selectedItems())
    {
        RegionCenterPoint *centerPoint =
            dynamic_cast<RegionCenterPoint *>(item);
        if (centerPoint)
        {
            showStatusBarError(
                "Region center point cannot be assigned to "
                "other regions.",
                3000);
            return;
        }

        QString currentRegion =
            CargoNetSim::CargoNetSimController::
                getInstance()
                    .getRegionDataController()
                    ->getCurrentRegion();

        // Handle items with a 'region' property
        if (item->type() == QGraphicsItem::UserType + 1)
        { // TerminalItem
            TerminalItem *terminal =
                static_cast<TerminalItem *>(item);
            terminal->setRegion(currentRegion);
        }
        else if (item->type()
                 == QGraphicsItem::UserType + 2)
        { // ConnectionLine
            ConnectionLine *connection =
                static_cast<ConnectionLine *>(item);
            connection->setRegion(currentRegion);
        }
        else if (item->type()
                 == QGraphicsItem::UserType + 4)
        { // BackgroundPhotoItem
            BackgroundPhotoItem *photo =
                static_cast<BackgroundPhotoItem *>(item);
            photo->setRegion(currentRegion);
        }
        else if (item->type()
                 == QGraphicsItem::UserType + 7)
        { // MapPoint
            MapPoint *point = static_cast<MapPoint *>(item);
            point->setRegion(currentRegion);
        }
        else if (item->type()
                 == QGraphicsItem::UserType + 8)
        { // MapLine
            MapLine *line = static_cast<MapLine *>(item);
            line->setRegion(currentRegion);
        }
    }

    // ViewController::updateSceneVisibility(this);
    // ViewController::updateGlobalMapScene(this);
    showStatusBarMessage(
        "Selected items assigned to current region.", 2000);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Ask for confirmation
    QMessageBox::StandardButton reply =
        QMessageBox::question(
            this, "Exit Application",
            "Are you sure you want to exit?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        // Save any application state if needed

        // Perform shutdown procedures
        shutdown();
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // Handle Delete key for removing selected items
    if (event->key() == Qt::Key_Delete
        || event->key() == Qt::Key_Backspace)
    {
        GraphicsScene *currentScene = getCurrentScene();
        if (!currentScene)
        {
            CustomMainWindow::keyPressEvent(event);
            return;
        }

        // Create a copy of the selected items list to
        // safely modify during iteration
        QList<QGraphicsItem *> selectedItems =
            currentScene->selectedItems();

        // Process ConnectionLabel items first to identify
        // parent lines to remove
        QList<ConnectionLine *> parentLinesToRemove;

        for (auto it = selectedItems.begin();
             it != selectedItems.end();)
        {
            ConnectionLabel *label =
                dynamic_cast<ConnectionLabel *>(*it);
            if (label)
            {
                ConnectionLine *parent =
                    dynamic_cast<ConnectionLine *>(
                        label->parentItem());
                if (parent)
                {
                    parentLinesToRemove.append(parent);
                }
                it = selectedItems.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // Add parent lines to our selection if not already
        // there
        for (ConnectionLine *parentLine :
             parentLinesToRemove)
        {
            if (!selectedItems.contains(parentLine))
            {
                selectedItems.append(parentLine);
            }
        }

        // Process terminal items first to handle their
        // associated items
        for (QGraphicsItem *item : selectedItems)
        {
            if (!item)
            {
                continue;
            }

            // Handle terminal items
            TerminalItem *terminal =
                dynamic_cast<TerminalItem *>(item);

            if (terminal)
            {
                // If in main view, we need to clean up the
                // global map as well
                if (currentScene == regionScene_)
                {
                    GlobalTerminalItem *globalItem =
                        terminal->getGlobalTerminalItem();

                    if (globalItem)
                    {
                        // Remove all connection lines in
                        // the global scene that involve
                        // this terminal
                        QList<ConnectionLine *>
                            linesToRemove;
                        for (QGraphicsItem *connection :
                             globalMapScene_->items())
                        {
                            ConnectionLine *line =
                                dynamic_cast<
                                    ConnectionLine *>(
                                    connection);
                            if (line
                                && (line->startItem()
                                        == globalItem
                                    || line->endItem()
                                           == globalItem))
                            {
                                linesToRemove.append(line);
                            }
                        }

                        // Remove the connections
                        for (ConnectionLine *line :
                             linesToRemove)
                        {
                            if (line)
                            {
                                globalMapScene_
                                    ->removeItemWithId<
                                        ConnectionLine>(
                                        line->getID());
                            }
                        }

                        // Remove the global item
                        if (globalItem)
                        {
                            globalMapScene_
                                ->removeItemWithId<
                                    GlobalTerminalItem>(
                                    globalItem->getID());
                        }
                    }
                }

                // Remove connection lines associated with
                // this terminal in the current scene
                for (ConnectionLine *line :
                     currentScene
                         ->getItemsByType<ConnectionLine>())
                {
                    if (line
                        && (line->startItem() == item
                            || line->endItem() == item))
                    {
                        currentScene->removeItemWithId<
                            ConnectionLine>(line->getID());
                    }
                }

                // Remove map point links to this terminal
                for (MapPoint *point :
                     currentScene
                         ->getItemsByType<MapPoint>())
                {
                    if (point
                        && point->getLinkedTerminal()
                               == terminal)
                    {
                        point->setLinkedTerminal(nullptr);
                    }
                }

                // Remove the terminal
                if (terminal)
                {
                    currentScene
                        ->removeItemWithId<TerminalItem>(
                            terminal->getID());
                }
            }
            // Handle connection lines
            else if (ConnectionLine *line =
                         dynamic_cast<ConnectionLine *>(
                             item))
            {
                if (line)
                {
                    currentScene
                        ->removeItemWithId<ConnectionLine>(
                            line->getID());
                }
            }
            // Handle background photos
            else if (BackgroundPhotoItem *photo =
                         dynamic_cast<
                             BackgroundPhotoItem *>(item))
            {
                if (photo)
                {
                    currentScene->removeItemWithId<
                        BackgroundPhotoItem>(
                        photo->getID());
                }
            }
            // Handle other item types as needed
            else if (MapPoint *point =
                         dynamic_cast<MapPoint *>(item))
            {
                if (point)
                {
                    QString id =
                        point->getProperty("NodeID")
                            .toString();
                    currentScene
                        ->removeItemWithId<MapPoint>(id);
                }
            }
            else if (MapLine *line =
                         dynamic_cast<MapLine *>(item))
            {
                if (line)
                {
                    QString id = line->getProperty("LinkID")
                                     .toString();
                    currentScene->removeItemWithId<MapLine>(
                        id);
                }
            }

            selectedItems.removeAll(nullptr);
        }

        showStatusBarMessage("Selected items deleted.",
                             2000);
        event->accept();
    }
    // Handle escape key to cancel current operations
    else if (event->key() == Qt::Key_Escape)
    {
        // Clear selection in the current scene
        GraphicsScene *currentScene = getCurrentScene();
        if (currentScene)
        {
            currentScene->clearSelection();
        }

        // Reset all toggle button states
        connectButton_->setChecked(false);
        linkTerminalButton_->setChecked(false);
        unlinkTerminalButton_->setChecked(false);
        measureButton_->setChecked(false);

        // Reset all scene modes
        regionScene_->setIsInConnectMode(false);
        regionScene_->setIsInLinkTerminalMode(false);
        regionScene_->setIsInUnlinkTerminalMode(false);
        regionScene_->setIsInMeasureMode(false);
        regionScene_->setConnectedFirstItem(QVariant());
        selectedTerminal_ = nullptr;

        // Reset cursor
        unsetCursor();

        event->accept();
    }
    else
    {
        CustomMainWindow::keyPressEvent(event);
    }
}

} // namespace GUI
} // namespace CargoNetSim
