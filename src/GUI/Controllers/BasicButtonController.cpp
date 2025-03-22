#include "BasicButtonController.h"

#include <QComboBox>
#include <QCursor>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QTextEdit>
#include <QToolButton>

#include "../Items/ConnectionLine.h"
#include "../Items/DistanceMeasurementTool.h"
#include "../Items/TerminalItem.h"
#include "../MainWindow.h"
#include "../Widgets/GraphicsScene.h"
#include "../Widgets/GraphicsView.h"
// #include "Controllers/ViewController.h"
#include "Backend/Controllers/RegionDataController.h"
// #include "Controllers/UtilityFunctions.h"
// #include "Serializers/ProjectSerializer.h"
#include "../Widgets/SetCoordinatesDialog.h"
#include "Backend/Controllers/NetworkController.h"

#include "../Controllers/NetworkController.h"
#include "../Controllers/UtilityFunctions.h"
#include "../Widgets/ShipManagerDialog.h"
#include "../Widgets/TrainManagerDialog.h"
#include "Backend/Controllers/VehicleController.h"

namespace CargoNetSim {
namespace GUI {

void BasicButtonController::resetOtherButtons(
    MainWindow *mainWindow, QToolButton *activeButton) {
    try {
        QList<QToolButton *> toggleButtons = {
            mainWindow->connectButton_,
            mainWindow->linkTerminalButton_,
            mainWindow->unlinkTerminalButton_,
            mainWindow->measureButton_};

        for (QToolButton *button : toggleButtons) {
            if (button != activeButton) {
                button->setChecked(false);
            }
        }

        // Reset associated modes in the scene
        mainWindow->scene_->connectMode        = false;
        mainWindow->scene_->linkTerminalMode   = false;
        mainWindow->scene_->unlinkTerminalMode = false;
        mainWindow->scene_->measureMode        = false;
        mainWindow->scene_->connectFirstItem   = QVariant();
        mainWindow->selectedTerminal_          = nullptr;
    } catch (const std::exception &e) {
        qCritical() << "Error in resetOtherButtons:"
                    << e.what();
        QMessageBox::critical(
            mainWindow, "Error",
            QString("Failed to reset buttons: %1")
                .arg(e.what()));
    }
}

void BasicButtonController::toggleGrid(
    MainWindow *mainWindow, bool checked) {
    try {
        // Update grid state for both views
        mainWindow->regionView_->setGridVisibility(checked);
        mainWindow->globalMapView_->setGridVisibility(
            checked);

        // Update button text
        QToolButton *sender = qobject_cast<QToolButton *>(
            mainWindow->sender());
        if (sender) {
            sender->setText(
                QString("%1\nGrid")
                    .arg(checked ? "Hide" : "Show"));
        }

        // Force update of both viewports
        mainWindow->regionView_->viewport()->update();
        mainWindow->globalMapView_->viewport()->update();

        mainWindow->statusBar()->showMessage(
            QString("Grid %1").arg(checked ? "enabled"
                                           : "disabled"),
            2000);
    } catch (const std::exception &e) {
        qCritical() << "Error in toggleGrid:" << e.what();
        QMessageBox::critical(
            mainWindow, "Error",
            QString("Failed to toggle grid: %1")
                .arg(e.what()));
    }
}

void BasicButtonController::toggleConnectMode(
    MainWindow *mainWindow, bool checked) {
    try {
        if (checked) {
            resetOtherButtons(mainWindow,
                              mainWindow->connectButton_);

            GraphicsScene *currentScene =
                mainWindow->tabWidget_->currentIndex() == 0
                    ? mainWindow->scene_
                    : mainWindow->globalMapScene_;

            currentScene->connectMode      = true;
            currentScene->connectFirstItem = QVariant();
            mainWindow->statusBar()->showMessage(
                "Click on two terminals to connect them...",
                3000);
        } else {
            mainWindow->scene_->connectMode = false;
            mainWindow->scene_->connectFirstItem =
                QVariant();
            mainWindow->globalMapScene_->connectMode =
                false;
            mainWindow->globalMapScene_->connectFirstItem =
                QVariant();
            mainWindow->connectButton_->setChecked(false);
            mainWindow->statusBar()->showMessage(
                "Connect mode disabled", 2000);
        }
    } catch (const std::exception &e) {
        qCritical() << "Error in toggleConnectMode:"
                    << e.what();
        QMessageBox::critical(
            mainWindow, "Error",
            QString("Failed to toggle connect mode: %1")
                .arg(e.what()));
    }
}

void BasicButtonController::toggleLinkTerminalMode(
    MainWindow *mainWindow, bool checked) {
    try {
        if (checked) {
            resetOtherButtons(
                mainWindow,
                mainWindow->linkTerminalButton_);

            // Disable other modes when entering link mode
            mainWindow->scene_->connectMode      = false;
            mainWindow->scene_->linkTerminalMode = true;
            mainWindow->selectedTerminal_        = nullptr;
            mainWindow->statusBar()->showMessage(
                "Select a terminal, then select a node to "
                "link them...",
                3000);
        } else {
            mainWindow->scene_->linkTerminalMode = false;
            mainWindow->selectedTerminal_        = nullptr;
            mainWindow->linkTerminalButton_->setChecked(
                false);
            mainWindow->statusBar()->showMessage(
                "Link terminal mode disabled", 2000);
        }
    } catch (const std::exception &e) {
        qCritical() << "Error in toggleLinkTerminalMode:"
                    << e.what();
        QMessageBox::critical(
            mainWindow, "Error",
            QString(
                "Failed to toggle link terminal mode: %1")
                .arg(e.what()));
    }
}

void BasicButtonController::toggleUnlinkTerminalMode(
    MainWindow *mainWindow, bool checked) {
    try {
        if (checked) {
            resetOtherButtons(
                mainWindow,
                mainWindow->unlinkTerminalButton_);

            // Disable other modes when entering unlink mode
            mainWindow->scene_->connectMode        = false;
            mainWindow->scene_->linkTerminalMode   = false;
            mainWindow->scene_->unlinkTerminalMode = true;
            mainWindow->selectedTerminal_ = nullptr;
            mainWindow->statusBar()->showMessage(
                "Select a terminal, then select a node to "
                "unlink them...",
                3000);
        } else {
            mainWindow->scene_->unlinkTerminalMode = false;
            mainWindow->selectedTerminal_ = nullptr;
            mainWindow->unlinkTerminalButton_->setChecked(
                false);
            mainWindow->statusBar()->showMessage(
                "Unlink terminal mode disabled", 2000);
        }
    } catch (const std::exception &e) {
        qCritical() << "Error in toggleUnlinkTerminalMode:"
                    << e.what();
        QMessageBox::critical(
            mainWindow, "Error",
            QString(
                "Failed to toggle unlink terminal mode: %1")
                .arg(e.what()));
    }
}

void BasicButtonController::toggleMeasureMode(
    MainWindow *mainWindow, bool checked) {
    try {
        if (checked) {
            resetOtherButtons(mainWindow,
                              mainWindow->measureButton_);
        }

        GraphicsView *currentView =
            mainWindow->tabWidget_->currentIndex() == 0
                ? mainWindow->regionView_
                : mainWindow->globalMapView_;

        GraphicsScene *currentScene =
            dynamic_cast<GraphicsScene *>(
                currentView->scene());

        if (!currentScene) {
            return;
        }

        currentView->measureMode  = checked;
        currentScene->measureMode = checked;
        currentScene->measurementTool =
            nullptr; // Reset measurement tool

        if (!checked) {
            if (currentView->measurementTool) {
                if (currentView->measurementTool->scene()) {
                    currentView->scene()->removeItem(
                        currentView->measurementTool);
                }
                currentView->measurementTool = nullptr;
            }
            currentView
                ->unsetCursor(); // Restore default cursor
            mainWindow->measureButton_->setChecked(false);
        } else {
            currentView->setCursor(
                QCursor(Qt::CrossCursor));
        }

        if (checked) {
            mainWindow->statusBar()->showMessage(
                "Click to set start point, click again to "
                "measure distance",
                3000);
        } else {
            mainWindow->statusBar()->showMessage(
                "Measurement mode disabled", 2000);
        }
    } catch (const std::exception &e) {
        qCritical() << "Error in toggleMeasureMode:"
                    << e.what();

        // Reset measure mode to safe state
        mainWindow->measureButton_->setChecked(false);

        GraphicsView *currentView =
            mainWindow->tabWidget_->currentIndex() == 0
                ? mainWindow->regionView_
                : mainWindow->globalMapView_;

        if (currentView) {
            currentView->measureMode = false;

            GraphicsScene *currentScene =
                dynamic_cast<GraphicsScene *>(
                    currentView->scene());

            if (currentScene) {
                currentScene->measureMode = false;
            }

            if (currentView->measurementTool
                && currentView->measurementTool->scene()) {
                currentView->scene()->removeItem(
                    currentView->measurementTool);
                currentView->measurementTool = nullptr;
            }

            currentView->unsetCursor();
        }

        QMessageBox::critical(
            mainWindow, "Error",
            QString("Error in measure mode: %1")
                .arg(e.what()));
    }
}

void BasicButtonController::clearMeasurements(
    MainWindow *mainWindow) {
    try {
        // Get current scene based on active tab
        GraphicsScene *currentScene =
            mainWindow->tabWidget_->currentIndex() == 0
                ? mainWindow->scene_
                : mainWindow->globalMapScene_;

        // Remove all measurement tools from the scene
        QList<QGraphicsItem *> itemsToRemove;
        for (QGraphicsItem *item : currentScene->items()) {
            if (dynamic_cast<DistanceMeasurementTool *>(
                    item)) {
                itemsToRemove.append(item);
            }
        }

        // Remove items separately to avoid modifying
        // collection during iteration
        for (QGraphicsItem *item : itemsToRemove) {
            currentScene->removeItem(item);
        }

        mainWindow->statusBar()->showMessage(
            "All measurements cleared", 2000);
    } catch (const std::exception &e) {
        qCritical() << "Error in clearMeasurements:"
                    << e.what();
        QMessageBox::critical(
            mainWindow, "Error",
            QString("Failed to clear measurements: %1")
                .arg(e.what()));
    }
}

void BasicButtonController::changeRegion(
    MainWindow *mainWindow, int index) {
    try {
        QString currentRegion =
            mainWindow->regionCombo_->currentText();
        Backend::RegionDataController::getInstance()
            .setCurrentRegion(currentRegion);
        // ViewController::updateSceneVisibility(mainWindow);
        // // TODO
        emit mainWindow->regionChanged(currentRegion);
    } catch (const std::exception &e) {
        qCritical() << "Error in changeRegion:" << e.what();
        QMessageBox::critical(
            mainWindow, "Error",
            QString("Failed to change region: %1")
                .arg(e.what()));
    }
}

void BasicButtonController::exportLog(
    MainWindow *mainWindow) {
    try {
        QString filePath = QFileDialog::getSaveFileName(
            mainWindow, "Save Log File", "",
            "Text Files (*.txt);;All Files (*.*)");

        if (!filePath.isEmpty()) {
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly
                          | QIODevice::Text)) {
                QTextStream stream(&file);

                // Write general log first
                int generalIndex =
                    mainWindow->clientNames_.size() - 1;
                stream << "--- "
                       << mainWindow
                              ->clientNames_[generalIndex]
                       << " ---\n";
                stream
                    << mainWindow
                           ->logTextWidgets_[generalIndex]
                           ->toPlainText();
                stream << "\n--------------------\n\n";

                // Write client logs
                for (int index = 0; index < generalIndex;
                     ++index) {
                    stream
                        << "--- "
                        << mainWindow->clientNames_[index]
                        << " ---\n";
                    stream << mainWindow
                                  ->logTextWidgets_[index]
                                  ->toPlainText();
                    stream << "\n--------------------\n\n";
                }

                file.close();
                mainWindow->statusBar()->showMessage(
                    QString("Log exported to %1")
                        .arg(filePath),
                    2000);
            } else {
                throw std::runtime_error(
                    QString("Could not open file for "
                            "writing: %1")
                        .arg(file.errorString())
                        .toStdString());
            }
        }
    } catch (const std::exception &e) {
        qCritical() << "Error in exportLog:" << e.what();
        QMessageBox::critical(
            mainWindow, "Error",
            QString("Failed to export log: %1")
                .arg(e.what()));
    }
}

void BasicButtonController::checkNetwork(
    MainWindow *mainWindow, GraphicsScene *scene) {
    // TODO
    // try {
    //     QList<TerminalItem*> allRegionTerminals =
    //     UtilityFunctions::getTerminalItems(
    //         scene,
    //         mainWindow->currentRegion_,
    //         "*",      // terminal_type
    //         nullptr,  // connected_only
    //         nullptr   // linked_only
    //         );

    //     QList<TerminalItem*> notConnectedTerminals =
    //     UtilityFunctions::getTerminalItems(
    //         scene,
    //         mainWindow->currentRegion_,
    //         "*",     // terminal_type
    //         false,   // connected_only
    //         nullptr  // linked_only
    //         );

    //     ViewController::flashTerminalItems(notConnectedTerminals,
    //     true);

    //     if (!notConnectedTerminals.isEmpty()) {
    //         mainWindow->statusBar()->showMessage(
    //             "There are terminals that are not
    //             connected to any map point.", 3000);
    //     }
    //     else {
    //         if (allRegionTerminals.isEmpty()) {
    //             mainWindow->statusBar()->showMessage(
    //                 "There are no terminals in the
    //                 current region.", 3000);
    //         }
    //         else {
    //             mainWindow->statusBar()->showMessage("All
    //             terminals are connected", 2000);
    //         }
    //     }
    // }
    // catch (const std::exception& e) {
    //     qCritical() << "Error in checkNetwork:" <<
    //     e.what(); QMessageBox::critical(mainWindow,
    //     "Error",
    //                           QString("Failed to check
    //                           network:
    //                           %1").arg(e.what()));
    // }
}

void BasicButtonController::disconnectAllTerminals(
    MainWindow *mainWindow, GraphicsScene *scene,
    const QString &region) {
    try {
        QList<QGraphicsItem *> itemsToRemove;
        for (QGraphicsItem *item : scene->items()) {
            ConnectionLine *connection =
                dynamic_cast<ConnectionLine *>(item);
            if (connection
                && (region == "*"
                    || connection->getRegion() == region)) {
                itemsToRemove.append(connection);
            }
        }

        // Remove items separately to avoid modifying
        // collection during iteration
        for (QGraphicsItem *item : itemsToRemove) {
            scene->removeItem(item);
        }

        mainWindow->showStatusBarMessage(
            "All terminals disconnected", 2000);
    } catch (const std::exception &e) {
        qCritical() << "Error in disconnectAllTerminals:"
                    << e.what();
        QMessageBox::critical(
            mainWindow, "Error",
            QString("Failed to disconnect terminals: %1")
                .arg(e.what()));
    }
}

void BasicButtonController::toggleConnectionLines(
    MainWindow *mainWindow, bool checked) {
    try {
        // Update button text
        QToolButton *sender = qobject_cast<QToolButton *>(
            mainWindow->sender());
        if (sender) {
            sender->setText(
                QString("%1\nConnections")
                    .arg(checked ? "Hide" : "Show"));
        }

        // Update visibility of connection lines
        for (QGraphicsItem *item :
             mainWindow->scene_->items()) {
            ConnectionLine *connection =
                dynamic_cast<ConnectionLine *>(item);
            if (connection
                && connection->getRegion()
                       == Backend::RegionDataController::
                              getInstance()
                                  .getCurrentRegion()) {
                connection->setVisible(checked);
            }
        }

        mainWindow->statusBar()->showMessage(
            QString("Connection lines %1")
                .arg(checked ? "shown" : "hidden"),
            2000);
    } catch (const std::exception &e) {
        qCritical() << "Error in toggleConnectionLines:"
                    << e.what();
        QMessageBox::critical(
            mainWindow, "Error",
            QString("Failed to toggle connection lines: %1")
                .arg(e.what()));
    }
}

void BasicButtonController::toggleTerminals(
    MainWindow *mainWindow, bool checked) {
    try {
        // Update button text
        QToolButton *sender = qobject_cast<QToolButton *>(
            mainWindow->sender());
        if (sender) {
            sender->setText(
                QString("%1\nTerminals")
                    .arg(checked ? "Hide" : "Show"));
        }

        // Update visibility of terminals
        for (QGraphicsItem *item :
             mainWindow->scene_->items()) {
            TerminalItem *terminal =
                dynamic_cast<TerminalItem *>(item);
            if (terminal
                && terminal->getRegion()
                       == Backend::RegionDataController::
                              getInstance()
                                  .getCurrentRegion()) {
                terminal->setVisible(checked);
            }
        }

        mainWindow->statusBar()->showMessage(
            QString("Terminals %1")
                .arg(checked ? "shown" : "hidden"),
            2000);
    } catch (const std::exception &e) {
        qCritical() << "Error in toggleTerminals:"
                    << e.what();
        QMessageBox::critical(
            mainWindow, "Error",
            QString("Failed to toggle terminals: %1")
                .arg(e.what()));
    }
}

void BasicButtonController::newProject(
    MainWindow *mainWindow) {
    try {
        QMessageBox::StandardButton reply =
            QMessageBox::question(
                mainWindow, "New Project",
                "Are you sure you want to start a new "
                "project? "
                "Any unsaved changes will be lost.",
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            // Clear current scene
            mainWindow->scene_->clear();

            // Reset Region Manager
            mainWindow->regionManager_->clearRegions();

            // Reset current region
            Backend::RegionDataController::getInstance()
                .setCurrentRegion("Default Region");

            // Reset network registries
            // TrainNetworkManager::getInstance()->clear();
            // TruckNetworkManager::getInstance()->clear();

            // Clear global scene
            for (QGraphicsItem *item :
                 mainWindow->globalMapScene_->items()) {
                mainWindow->globalMapScene_->removeItem(
                    item);
            }

            // Clear region centers
            // TODO
            // mainWindow->regionCenters_.clear();
            // RegionDataController::getInstance().clear();

            // // Create the default region
            // Backend::RegionDataController::getInstance().addRegion(
            //     "Default Region", QColor(Qt::green));

            // mainWindow->createRegionCenter(
            //     "Default Region",
            //     Backend::RegionDataController::getInstance()
            //         .getRegionData("Default
            //         Region")->getVariable("color")
            //     );

            // // Update UI
            // mainWindow->regionCombo_->clear();
            // mainWindow->regionCombo_->addItems(
            //     RegionDataController::getInstance().getAllRegionNames());
            // mainWindow->regionCombo_->setCurrentText(mainWindow->currentRegion_);

            // // Update views
            // ViewController::updateSceneVisibility(mainWindow);
            // ViewController::updateGlobalMapScene(mainWindow);

            mainWindow->statusBar()->showMessage(
                "New project created", 2000);
        }
    } catch (const std::exception &e) {
        qCritical() << "Error in newProject:" << e.what();
        QMessageBox::critical(
            mainWindow, "Error",
            QString("Failed to create new project: %1")
                .arg(e.what()));
    }
}

void BasicButtonController::openProject(
    MainWindow *mainWindow) {
    try {
        QString filePath = QFileDialog::getOpenFileName(
            mainWindow, "Open Project", "",
            "CargoNetSim Projects (*.cns);;All Files (*.*)",
            nullptr, QFileDialog::DontUseNativeDialog);

        if (!filePath.isEmpty()) {
            // if
            // (ProjectSerializer::loadProject(mainWindow,
            // filePath)) {
            //     mainWindow->currentProjectPath_ =
            //     filePath;
            //     mainWindow->statusBar()->showMessage(
            //         QString("Project loaded successfully
            //         from %1").arg(filePath), 2000);
            // }
            // else {
            //     throw std::runtime_error("Failed to load
            //     project. Check the console for
            //     details.");
            // }
        }
    } catch (const std::exception &e) {
        qCritical() << "Error in openProject:" << e.what();
        QMessageBox::critical(
            mainWindow, "Error",
            QString("Failed to open project: %1")
                .arg(e.what()));
    }
}

void BasicButtonController::saveProject(
    MainWindow *mainWindow) {
    try {
        // If project hasn't been saved before, show save
        // dialog
        if (!mainWindow->currentProjectPath_.length()) {
            QString filePath = QFileDialog::getSaveFileName(
                mainWindow, "Save Project", "",
                "CargoNetSim Projects (*.cns);;All Files "
                "(*.*)",
                nullptr, QFileDialog::DontUseNativeDialog);

            if (filePath.isEmpty()) {
                return;
            }

            // Add extension if not present
            if (!filePath.endsWith(".cns")) {
                filePath += ".cns";
            }

            mainWindow->currentProjectPath_ = filePath;
        }

        // if (ProjectSerializer::saveProject(mainWindow,
        // mainWindow->currentProjectPath_)) {
        //     mainWindow->statusBar()->showMessage(
        //         QString("Project saved successfully to
        //         %1").arg(mainWindow->currentProjectPath_),
        //         2000
        //         );
        // }
        // else {
        //     throw std::runtime_error("Failed to save
        //     project. Check the console for details.");
        // }
    } catch (const std::exception &e) {
        qCritical() << "Error in saveProject:" << e.what();
        QMessageBox::critical(
            mainWindow, "Error",
            QString("Failed to save project: %1")
                .arg(e.what()));
    }
}

void BasicButtonController::toggleSetGlobalPositionMode(
    MainWindow *mainWindow, bool checked) {
    try {
        // Force reset the scene mode to match the button
        // state
        mainWindow->globalMapScene_->setGlobalPositionMode =
            checked;

        if (checked) {
            resetOtherButtons(
                mainWindow,
                mainWindow->setGlobalPositionButton_);
            mainWindow->statusBar()->showMessage(
                "Click on a terminal to set its global "
                "position...",
                3000);
        } else {
            mainWindow->setGlobalPositionButton_
                ->setChecked(false);
            mainWindow->statusBar()->showMessage(
                "Set global position mode disabled", 2000);
        }
    } catch (const std::exception &e) {
        qCritical()
            << "Error in toggleSetGlobalPositionMode:"
            << e.what();
        QMessageBox::critical(
            mainWindow, "Error",
            QString(
                "Failed to toggle global position mode: %1")
                .arg(e.what()));
    }
}

bool BasicButtonController::setTerminalGlobalPosition(
    MainWindow *mainWindow, TerminalItem *terminal) {
    try {
        // // Get the terminal's current global position
        // QGraphicsItem* globalItem =
        // mainWindow->globalMapItems_.value(terminal,
        // nullptr); if (!globalItem) {
        //     mainWindow->statusBar()->showMessage("Terminal
        //     not found in global map", 2000); return
        //     false;
        // }

        // // Get the current position
        // QPointF globalPos = globalItem->pos() + QPointF(
        //                         globalItem->boundingRect().width()
        //                         / 2,
        //                         globalItem->boundingRect().height()
        //                         / 2
        //                         );

        // double currentLat, currentLon;
        // std::tie(currentLat, currentLon) =
        // mainWindow->globalMapView_->sceneToWGS84(globalPos);

        // // Show the dialog
        // SetCoordinatesDialog dialog(
        //     terminal->getProperties()["Name"].toString(),
        //     currentLat,
        //     currentLon,
        //     mainWindow
        //     );

        // if (dialog.exec() == QDialog::Accepted) {
        //     // Get the new coordinates
        //     double lat, lon;
        //     std::tie(lat, lon) = dialog.getCoordinates();

        //     // Set the terminal position
        //     if
        //     (ViewController::setTerminalGlobalPosition(mainWindow,
        //     terminal, lat, lon)) {
        //         mainWindow->statusBar()->showMessage("Terminal
        //         position updated", 2000); return true;
        //     }
        //     else {
        //         mainWindow->statusBar()->showMessage("Failed
        //         to update terminal position", 2000);
        //         return false;
        //     }
        // }

        // Dialog was cancelled
        return false;
    } catch (const std::exception &e) {
        qCritical() << "Error in setTerminalGlobalPosition:"
                    << e.what();
        QMessageBox::critical(
            mainWindow, "Error",
            QString("Failed to set terminal global "
                    "position: %1")
                .arg(e.what()));
        return false;
    }
}

void BasicButtonController::toggleDockWidget(
    bool checked, QDockWidget *dockWidget,
    QToolButton *button, const QString &widgetName) {
    dockWidget->setVisible(checked);
    button->setText(QString("%1\n%2")
                        .arg(checked ? "Hide" : "Show")
                        .arg(widgetName));
}

void BasicButtonController::showTrainManager(
    MainWindow *mainWindow) {
    TrainManagerDialog dialog(mainWindow);

    auto trains = Backend::VehicleController::getInstance()
                      ->getAllTrains();
    dialog.setTrains(trains);
    dialog.updateTable();

    if (dialog.exec() == QDialog::Accepted) {
        // Store trains
        auto newTrains = dialog.getTrains();
        Backend::VehicleController::getInstance()
            ->updateTrains(newTrains);
    }
}

void BasicButtonController::showShipManager(
    MainWindow *mainWindow) {
    ShipManagerDialog dialog(mainWindow);

    auto ships = Backend::VehicleController::getInstance()
                     ->getAllShips();
    dialog.setShips(ships);
    dialog.updateTable();

    if (dialog.exec() == QDialog::Accepted) {
        // Store ships
        auto newShips = dialog.getShips();
        Backend::VehicleController::getInstance()
            ->updateShips(newShips);
    }
}

void BasicButtonController::updateRegionComboBox(
    MainWindow *mainWindow) {
    // Store current selection
    QString currentRegion =
        mainWindow->regionCombo_->currentText();

    // Clear and repopulate
    mainWindow->regionCombo_->clear();

    // Get all region names from RegionDataController
    QStringList regionNames =
        Backend::RegionDataController::getInstance()
            .getAllRegionNames();
    mainWindow->regionCombo_->addItems(regionNames);

    // Restore selection if it still exists, otherwise
    // select first item
    int index =
        mainWindow->regionCombo_->findText(currentRegion);
    if (index >= 0) {
        mainWindow->regionCombo_->setCurrentIndex(index);
    } else if (mainWindow->regionCombo_->count() > 0) {
        mainWindow->regionCombo_->setCurrentIndex(0);
        // Update current region in controller
        if (!mainWindow->regionCombo_->currentText()
                 .isEmpty()) {
            Backend::RegionDataController::getInstance()
                .setCurrentRegion(mainWindow->regionCombo_
                                      ->currentText());
        }
    }
}

void BasicButtonController::setupSignals(
    MainWindow *mainWindow) {
    QObject::connect(
        &Backend::RegionDataController::getInstance(),
        &Backend::RegionDataController::regionAdded,
        mainWindow,
        [mainWindow](const QString &regionName) {
            updateRegionComboBox(mainWindow);
        });

    QObject::connect(
        &Backend::RegionDataController::getInstance(),
        &Backend::RegionDataController::regionRenamed,
        mainWindow,
        [mainWindow](const QString &oldName,
                     const QString &newName) {
            updateRegionComboBox(mainWindow);
        });

    QObject::connect(
        &Backend::RegionDataController::getInstance(),
        &Backend::RegionDataController::regionRemoved,
        mainWindow,
        [mainWindow](const QString &regionName) {
            updateRegionComboBox(mainWindow);
        });
}

} // namespace GUI
} // namespace CargoNetSim
