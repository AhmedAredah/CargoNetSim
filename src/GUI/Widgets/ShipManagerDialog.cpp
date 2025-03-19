#include "ShipManagerDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QToolButton>
#include <QIcon>
#include <QDialogButtonBox>
#include <QApplication>
#include <QStyle>

// Include the Ship class header
#include "../../Backend/Models/ShipSystem.h"
#include "../Utils/IconCreator.h"

namespace CargoNetSim {
namespace GUI {

ShipManagerDialog::ShipManagerDialog(QWidget* parent) 
    : QDialog(parent), m_ships() {
    setWindowTitle(tr("Ship Manager"));
    setMinimumSize(1000, 700);
    
    initUI();
}

void ShipManagerDialog::initUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // Create toolbar
    m_toolbar = new QToolBar();
    m_toolbar->setIconSize(QSize(32, 32));
    m_toolbar->setStyleSheet(
        "QToolButton {"
        "    padding: 6px;"
        "    icon-size: 32px;"
        "}"
        "QToolButton:hover {"
        "    background-color: #E5E5E5;"
        "}"
    );
    
    // Add ship action
    m_loadAction = new QAction(tr("Load Ships"), this);
    m_loadAction->setIcon(QIcon(IconFactory::createImportShipsIcon()));
    m_loadAction->setToolTip(tr("Load ships from DAT file"));
    connect(m_loadAction, &QAction::triggered, this, &ShipManagerDialog::loadShips);
    
    // Create QToolButton for load action
    QToolButton* loadButton = new QToolButton();
    loadButton->setDefaultAction(m_loadAction);
    loadButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    loadButton->setText(tr("Load\nShips"));
    m_toolbar->addWidget(loadButton);
    
    // Delete ship action
    m_deleteAction = new QAction(tr("Delete Ship"), this);
    m_deleteAction->setIcon(QIcon(IconFactory::createDeleteShipIcon()));
    m_deleteAction->setToolTip(tr("Delete selected ship"));
    connect(m_deleteAction, &QAction::triggered, this, &ShipManagerDialog::deleteShip);
    
    // Create QToolButton for delete action
    QToolButton* deleteButton = new QToolButton();
    deleteButton->setDefaultAction(m_deleteAction);
    deleteButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    deleteButton->setText(tr("Delete\nShip"));
    m_toolbar->addWidget(deleteButton);
    
    layout->addWidget(m_toolbar);
    
    // Create splitter for table and details
    m_splitter = new QSplitter(Qt::Vertical);
    
    // Create main table for ships overview
    m_table = new QTableWidget();
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({
        tr("Ship ID"),
        tr("Max Speed (knots)"),
        tr("Length (m)"),
        tr("Beam (m)"),
        tr("Draft (F/A) (m)"),
        tr("Displacement (m³)"),
        tr("Cargo Weight (t)"),
        tr("Propulsion")
    });
    
    // Set column stretch
    QHeaderView* header = m_table->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    for (int i = 1; i < 8; ++i) {
        header->setSectionResizeMode(i, QHeaderView::Stretch);
    }
    
    // Create details view
    m_detailsText = new QTextEdit();
    m_detailsText->setReadOnly(true);
    m_detailsText->setMinimumHeight(300);
    
    // Add widgets to splitter
    m_splitter->addWidget(m_table);
    m_splitter->addWidget(m_detailsText);
    
    // Set initial sizes for splitter (60% table, 40% details)
    m_splitter->setSizes({400, 300});
    
    layout->addWidget(m_splitter);
    
    // Connect selection change to update details
    connect(m_table, &QTableWidget::itemSelectionChanged, 
            this, &ShipManagerDialog::updateDetails);
    
    // Add Accept/Cancel buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}

void ShipManagerDialog::loadShips() {
    QString fileName = QFileDialog::getOpenFileName(
        this, 
        tr("Load Ships File"),
        QString(), 
        tr("DAT Files (*.dat);;All Files (*)")
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    try {
        QList<CargoNetSim::Backend::Ship*> loadedShips = Backend::ShipsReader::readShipsFile(fileName);
        
        if (loadedShips.isEmpty()) {
            QMessageBox::warning(this, tr("Warning"), 
                                tr("No valid ships found in the file."));
            return;
        }
        
        // Extend the existing ships list
        m_ships.append(loadedShips);
        updateTable();
        
        // Emit signal
        emit shipsLoaded(loadedShips.size());
        
        QMessageBox::information(this, tr("Ships Loaded"), 
                               tr("Successfully loaded %1 ships.").arg(loadedShips.size()));
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Error"), 
                            tr("Failed to load ships: %1").arg(e.what()));
    }
}

void ShipManagerDialog::deleteShip() {
    int currentRow = m_table->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, tr("Warning"), 
                            tr("Please select a ship to delete."));
        return;
    }
    
    QString shipId = m_table->item(currentRow, 0)->text();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        tr("Confirm Delete"),
        tr("Are you sure you want to delete ship '%1'?").arg(shipId),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        // Store the ship ID before removal
        emit shipDeleted(shipId);
        
        // Remove the ship
        m_ships.removeAt(currentRow);
        updateTable();
        m_detailsText->clear();
    }
}

void ShipManagerDialog::updateTable() {
    // Clear the table
    m_table->setRowCount(0);
    
    // Populate with ships
    for (const auto& ship : m_ships) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        
        // Ship ID
        m_table->setItem(row, 0, new QTableWidgetItem(ship->shipId()));
        
        // Max Speed
        m_table->setItem(row, 1, new QTableWidgetItem(
            QString::number(ship->maxSpeed(), 'f', 1)));
        
        // Length
        m_table->setItem(row, 2, new QTableWidgetItem(
            QString::number(ship->waterlineLength(), 'f', 1)));
        
        // Beam
        m_table->setItem(row, 3, new QTableWidgetItem(
            QString::number(ship->beam(), 'f', 1)));
        
        // Draft
        m_table->setItem(row, 4, new QTableWidgetItem(
            QString("%1/%2").arg(
                QString::number(ship->draftAtForward(), 'f', 1),
                QString::number(ship->draftAtAft(), 'f', 1)
            )
        ));
        
        // Displacement
        double disp = ship->volumetricDisplacement();
        QString dispText = disp > 0 ? QString::number(disp, 'f', 1) : tr("N/A");
        m_table->setItem(row, 5, new QTableWidgetItem(dispText));
        
        // Cargo Weight
        m_table->setItem(row, 6, new QTableWidgetItem(
            QString::number(ship->cargoWeight(), 'f', 1)));
        
        // Propulsion summary
        QString propText = QString("%1x %2m")
            .arg(ship->propellerCount())
            .arg(QString::number(ship->propellerDiameter(), 'f', 1));
        m_table->setItem(row, 7, new QTableWidgetItem(propText));
    }
}

void ShipManagerDialog::updateDetails() {
    int currentRow = m_table->currentRow();
    if (currentRow < 0 || currentRow >= m_ships.size()) {
        m_detailsText->clear();
        return;
    }
    
    auto ship = m_ships.at(currentRow);
    
    // Emit signal when a ship is selected
    emit shipSelected(ship->shipId());
    
    // Format and display the details
    m_detailsText->setHtml(formatShipDetails(ship));
}

QString ShipManagerDialog::formatShipDetails(const Backend::Ship &ship) const {
    QString details = QString(
        "<h2>Ship Details for ship ID: %1</h2>"
        
        "<h3>Physical Dimensions:</h3>"
        "<ul>"
        "    <li><b>Waterline Length:</b> %2 m</li>"
        "    <li><b>Length between Perpendiculars:</b> %3 m</li>"
        "    <li><b>Beam:</b> %4 m</li>"
        "    <li><b>Draft:</b> Forward %5 m, Aft %6 m</li>"
        "    <li><b>Displacement:</b> %7 m³</li>"
        "</ul>"
        
        "<h3>Hull Characteristics:</h3>"
        "<ul>"
        "    <li><b>Wetted Hull Surface:</b> %8 m²</li>"
        "    <li><b>Area Above Waterline:</b> %9 m²</li>"
        "    <li><b>Surface Roughness:</b> %10</li>"
        "    <li><b>Buoyancy Center:</b> %11</li>"
        "</ul>"
        
        "<h3>Coefficients:</h3>"
        "<ul>"
        "    <li><b>Block Coefficient:</b> %12</li>"
        "    <li><b>Prismatic Coefficient:</b> %13</li>"
        "    <li><b>Midship Section Coefficient:</b> %14</li>"
        "    <li><b>Waterplane Area Coefficient:</b> %15</li>"
        "</ul>"
        
        "<h3>Propulsion System:</h3>"
        "<ul>"
        "    <li><b>Propellers:</b> %16x Ø%17m</li>"
        "    <li><b>Propeller Pitch:</b> %18 m</li>"
        "    <li><b>Blades per Propeller:</b> %19</li>"
        "    <li><b>Engines per Propeller:</b> %20</li>"
        "    <li><b>Gearbox Ratio:</b> %21</li>"
        "    <li><b>System Efficiencies:</b>"
        "        <ul>"
        "            <li><b>Gearbox:</b> %22</li>"
        "            <li><b>Shaft:</b> %23</li>"
        "        </ul>"
        "    </li>"
        "</ul>"
        
        "<h3>Weights:</h3>"
        "<ul>"
        "    <li><b>Vessel Weight:</b> %24 t</li>"
        "    <li><b>Cargo Weight:</b> %25 t</li>"
        "</ul>"
        
        "<h3>Operational Parameters:</h3>"
        "<ul>"
        "    <li><b>Maximum Speed:</b> %26 knots</li>"
        "    <li><b>Maximum Rudder Angle:</b> %27°</li>"
        "    <li><b>Stop if No Energy:</b> %28</li>"
        "</ul>"
    )
    .arg(ship.shipId())
    .arg(QString::number(ship.waterlineLength(), 'f', 2))
    .arg(QString::number(ship.lengthBetweenPerpendiculars(), 'f', 2))
    .arg(QString::number(ship.beam(), 'f', 2))
    .arg(QString::number(ship.draftAtForward(), 'f', 2))
    .arg(QString::number(ship.draftAtAft(), 'f', 2))
    .arg(ship.volumetricDisplacement() > 0 ?
         QString::number(ship.volumetricDisplacement(), 'f', 2) : tr("N/A"))
    
    .arg(ship.wettedHullSurface() > 0 ?
         QString::number(ship.wettedHullSurface(), 'f', 2) : tr("N/A"))
    .arg(QString::number(ship.areaAboveWaterline(), 'f', 2))
    .arg(QString::number(ship.surfaceRoughness(), 'f', 4))
    .arg(QString::number(ship.buoyancyCenter(), 'f', 2))
    
    .arg(ship.blockCoef() > 0 ?
         QString::number(ship.blockCoef(), 'f', 4) : tr("N/A"))
    .arg(ship.prismaticCoef() > 0 ?
         QString::number(ship.prismaticCoef(), 'f', 4) : tr("N/A"))
    .arg(ship.midshipSectionCoef()> 0 ?
         QString::number(ship.midshipSectionCoef(), 'f', 4) : tr("N/A"))
    .arg(ship.waterplaneAreaCoef() > 0 ?
         QString::number(ship.waterplaneAreaCoef(), 'f', 4) : tr("N/A"))
    
    .arg(ship.propellerCount())
    .arg(QString::number(ship.propellerDiameter(), 'f', 2))
    .arg(QString::number(ship.propellerPitch(), 'f', 2))
    .arg(ship.propellerBladesCount())
    .arg(ship.enginesPerPropeller())
    .arg(QString::number(ship.gearboxRatio(), 'f', 3))
    
    .arg(QString::number(ship.gearboxEfficiency(), 'f', 3))
    .arg(QString::number(ship.shaftEfficiency(), 'f', 3))
    
    .arg(QString::number(ship.vesselWeight(), 'f', 2))
    .arg(QString::number(ship.cargoWeight(), 'f', 2))
    
    .arg(QString::number(ship.maxSpeed(), 'f', 1))
    .arg(ship.maxRudderAngle() > 0 ?
         QString::number(ship.maxRudderAngle(), 'f', 1) : tr("N/A"))
    .arg(ship.stopIfNoEnergy() ? tr("Yes") : tr("No"));
    
    return details;
}

QList<Backend::Ship*> ShipManagerDialog::getShips() const {
    return m_ships;
}

void ShipManagerDialog::setShips(const QList<Backend::Ship*>& ships) {
    m_ships = ships;
    updateTable();
}

} // namespace GUI
} // namespace CargoNetSim
