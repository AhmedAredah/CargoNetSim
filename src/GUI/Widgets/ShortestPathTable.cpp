/**
 * @file ShortestPathTable.cpp
 * @brief Implementation of the ShortestPathsTable widget
 * for path visualization and comparison
 * @author Ahmed Aredah
 * @date April 5, 2025
 *
 * This file contains the implementation of the
 * ShortestPathsTable class, which provides a UI component
 * for displaying, comparing, and exporting path data in the
 * CargoNetSim system.
 */

#include "ShortestPathTable.h"
#include "../Utils/IconCreator.h" // For icon creation utilities
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QVariant>
#include <stdexcept>

// Register meta-type for storing widgets in QVariant
Q_DECLARE_METATYPE(QWidget *)

namespace CargoNetSim
{
namespace GUI
{

//------------------------------------------------------------------------------
// TerminalPathDelegate Implementation
//------------------------------------------------------------------------------

/**
 * @brief Custom paint implementation for the terminal path
 * delegate
 *
 * Renders the custom widget in the terminal path column, or
 * falls back to standard delegate rendering for other
 * columns.
 */
void TerminalPathDelegate::paint(
    QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    // Check if we're in the terminals column (column index
    // 2)
    if (index.column() == 2)
    {
        // Try to extract the widget from user data
        QWidget *widget = qvariant_cast<QWidget *>(
            index.data(Qt::UserRole));

        if (widget)
        {
            // Render the widget directly to the cell
            QPixmap pixmap(widget->size());
            widget->render(&pixmap);
            painter->drawPixmap(option.rect.topLeft(),
                                pixmap);
            return;
        }
    }

    // Fall back to default rendering for other columns or
    // if widget extraction failed
    QStyledItemDelegate::paint(painter, option, index);
}

/**
 * @brief Size hint implementation for the terminal path
 * delegate
 *
 * Returns the size of the custom widget for the terminal
 * path column, or falls back to standard delegate size for
 * other columns.
 */
QSize TerminalPathDelegate::sizeHint(
    const QStyleOptionViewItem &option,
    const QModelIndex          &index) const
{
    // Check if we're in the terminals column
    if (index.column() == 2)
    {
        // Try to extract the widget from user data
        QWidget *widget = qvariant_cast<QWidget *>(
            index.data(Qt::UserRole));

        if (widget)
        {
            // Use the widget's size for the cell
            return widget->size();
        }
    }

    // Fall back to default size hint for other columns
    return QStyledItemDelegate::sizeHint(option, index);
}

//------------------------------------------------------------------------------
// ShortestPathsTable Implementation
//------------------------------------------------------------------------------

/**
 * @brief Constructs a ShortestPathsTable widget
 * @param parent The parent widget
 *
 * Initializes the UI components and sets up the table
 * structure.
 */
ShortestPathsTable::ShortestPathsTable(QWidget *parent)
    : QWidget(parent)
    , m_updatingUI(false) // Initialize to prevent recursive
                          // UI updates during setup
{
    // Set up the user interface components
    initUI();
}

ShortestPathsTable::~ShortestPathsTable()
{
    // Clean up all PathData objects
    for (auto &pathDataPtr : m_pathData)
    {
        delete pathDataPtr;
    }
    m_pathData.clear();
}

/**
 * @brief Initializes the UI components
 *
 * Creates and lays out all UI elements including the table
 * and buttons.
 */
void ShortestPathsTable::initUI()
{
    // Create main layout with minimal margins
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(
        2); // Compact spacing between elements

    // Create and add the table widget
    createTableWidget();
    layout->addWidget(m_table);

    // Create panel for buttons
    auto buttonPanel = new QHBoxLayout();

    // Initialize button groups
    createPathButtonPanel();
    createExportPanel();

    // Add buttons to panel with right alignment
    buttonPanel->addStretch(); // Push buttons to the right
    buttonPanel->addWidget(m_compareButton);
    buttonPanel->addWidget(m_exportButton);
    buttonPanel->addWidget(m_exportAllButton);

    // Add button panel to main layout
    layout->addLayout(buttonPanel);
}

/**
 * @brief Creates and configures the table widget
 *
 * Sets up columns, headers, and behavior for the path
 * table.
 */
void ShortestPathsTable::createTableWidget()
{
    // Create table widget
    m_table = new QTableWidget(this);

    // Configure table structure
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({
        tr("Select"),  // Column 0: Checkbox for selection
        tr("Path ID"), // Column 1: Path identifier
        tr("Terminal Path"),  // Column 2: Visual
                              // representation of the path
        tr("Predicted Cost"), // Column 3: Analysis-based
                              // cost prediction
        tr("Actual Cost")     // Column 4: Simulation-based
                              // actual cost
    });

    // Configure selection behavior
    m_table->setSelectionBehavior(QTableWidget::SelectRows);
    m_table->setSelectionMode(
        QTableWidget::SingleSelection);
    m_table->setEditTriggers(
        QTableWidget::NoEditTriggers); // Read-only table
    m_table->verticalHeader()->setDefaultSectionSize(
        50); // 50 pixels high rows

    // Configure header appearance and behavior
    auto header = m_table->horizontalHeader();

    // Set column sizing policies
    header->setSectionResizeMode(
        0, QHeaderView::Fixed); // Fixed width for checkbox
    m_table->setColumnWidth(
        0, 50); // 50 pixels for checkbox column

    header->setSectionResizeMode(
        1, QHeaderView::ResizeToContents); // Auto-size for
                                           // Path ID
    header->setSectionResizeMode(
        2, QHeaderView::Stretch); // Stretch terminal path
                                  // column
    header->setSectionResizeMode(
        3, QHeaderView::ResizeToContents); // Auto-size for
                                           // costs
    header->setSectionResizeMode(
        4, QHeaderView::ResizeToContents); // Auto-size for
                                           // costs

    // Set custom delegate for terminal path visualization
    m_table->setItemDelegate(
        new TerminalPathDelegate(m_table));

    // Connect selection signal to update UI state when
    // selection changes
    connect(m_table, &QTableWidget::itemSelectionChanged,
            this, &ShortestPathsTable::onSelectionChanged);
}

/**
 * @brief Creates the path manipulation button panel
 *
 * Initializes buttons for operations on paths, such as
 * comparison.
 */
void ShortestPathsTable::createPathButtonPanel()
{
    // Create compare button for path comparison
    m_compareButton =
        new QPushButton(tr("Compare Paths"), this);
    m_compareButton->setToolTip(
        tr("Compare selected paths"));
    m_compareButton->setEnabled(
        false); // Disabled until paths are selected

    // Connect button click to comparison handler
    connect(m_compareButton, &QPushButton::clicked, this,
            &ShortestPathsTable::onCompareButtonClicked);
}

/**
 * @brief Creates the export button panel
 *
 * Initializes buttons for exporting path data.
 */
void ShortestPathsTable::createExportPanel()
{
    // Create button for exporting a single selected path
    m_exportButton =
        new QPushButton(tr("Export Path"), this);
    m_exportButton->setToolTip(
        tr("Export selected path data"));
    m_exportButton->setEnabled(
        false); // Disabled until a path is selected

    // Connect button click to export handler
    connect(m_exportButton, &QPushButton::clicked, this,
            &ShortestPathsTable::onExportButtonClicked);

    // Create button for exporting all paths
    m_exportAllButton =
        new QPushButton(tr("Export All"), this);
    m_exportAllButton->setToolTip(
        tr("Export all path data"));
    m_exportAllButton->setEnabled(
        false); // Disabled until paths are added

    // Connect button click to export all handler
    connect(m_exportAllButton, &QPushButton::clicked, this,
            &ShortestPathsTable::onExportAllButtonClicked);
}

/**
 * @brief Adds multiple paths to the table
 * @param paths List of Path pointers to add
 *
 * Processes each Path in the list, creating a PathData
 * object and adding it to the internal data structure.
 */
void ShortestPathsTable::addPaths(
    const QList<Backend::Path *> &paths)
{
    // Process each path from the list
    for (Backend::Path *path : paths)
    {
        if (!path)
        {
            // Skip null paths
            qWarning() << "Skipping null path in addPaths";
            continue;
        }

        try
        {
            // Create a new PathData pointer
            PathData *pathDataPtr = new PathData();
            pathDataPtr->path =
                path; // PathData now owns the Path
            pathDataPtr->m_totalSimulationPathCost  = -1.0;
            pathDataPtr->m_totalSimulationEdgeCosts = -1.0;
            pathDataPtr->m_totalSimulationTerminalCosts =
                -1.0;
            pathDataPtr->isVisible = true;

            // Store the pointer in the map
            m_pathData.insert(path->getPathId(),
                              pathDataPtr);
        }
        catch (const std::exception &ex)
        {
            // Handle any exceptions during path processing
            qWarning() << "Error adding path:" << ex.what();
            delete path; // Clean up on error
        }
    }

    // Refresh the table with the new paths
    refreshTable();

    // Enable export all button if we have data
    m_exportAllButton->setEnabled(!m_pathData.isEmpty());
}

/**
 * @brief Updates the prediction costs for an existing path
 * @param pathId The ID of the path to update
 * @param totalCost New predicted total cost
 * @param edgeCost New predicted edge cost
 * @param terminalCost New predicted terminal cost
 *
 * Updates the Path object's cost fields if the path exists
 * and refreshes the table display.
 */
void ShortestPathsTable::updatePredictionCosts(
    int pathId, double totalCost, double edgeCost,
    double terminalCost)
{
    // Check if the path exists in our data
    if (!m_pathData.contains(pathId))
    {
        qWarning()
            << "Path ID" << pathId
            << "not found for prediction cost update";
        return;
    }

    // Get reference to the path data
    PathData *pathData = m_pathData[pathId];

    // Validate path pointer
    if (!pathData->path)
    {
        qWarning() << "Path object is null for path ID"
                   << pathId;
        return;
    }

    // Update costs if values are valid (>= 0)
    try
    {
        // Use the new setter methods to update the Path
        // object
        if (totalCost >= 0)
        {
            pathData->path->setTotalPathCost(totalCost);
        }

        if (edgeCost >= 0)
        {
            pathData->path->setTotalEdgeCosts(edgeCost);
        }

        if (terminalCost >= 0)
        {
            pathData->path->setTotalTerminalCosts(
                terminalCost);
        }

        // Update the table display
        for (int row = 0; row < m_table->rowCount(); ++row)
        {
            // Get the path ID for this row
            auto idItem = m_table->item(row, 1);
            if (idItem && idItem->text().toInt() == pathId)
            {
                // Update the predicted cost cell if total
                // cost was updated
                if (totalCost >= 0)
                {
                    m_table->item(row, 3)->setText(
                        QString::number(totalCost, 'f', 2));
                }
                break;
            }
        }
    }
    catch (const std::exception &ex)
    {
        qWarning()
            << "Error updating prediction costs for path"
            << pathId << ":" << ex.what();
    }
}

/**
 * @brief Updates the simulation costs for an existing path
 * @param pathId The ID of the path to update
 * @param simulationTotalCost New simulation total cost
 * @param simulationEdgeCost New simulation edge cost
 * @param simulationTerminalCost New simulation terminal
 * cost
 *
 * Updates the PathData's simulation cost fields if the path
 * exists and refreshes the table display.
 */
void ShortestPathsTable::updateSimulationCosts(
    int pathId, double simulationTotalCost,
    double simulationEdgeCost,
    double simulationTerminalCost)
{
    // Check if the path exists in our data
    if (!m_pathData.contains(pathId))
    {
        qWarning()
            << "Path ID" << pathId
            << "not found for simulation cost update";
        return;
    }

    // Get reference to the path data
    PathData *pathData = m_pathData[pathId];

    // Update simulation costs if values are valid (>= 0)
    if (simulationTotalCost >= 0)
    {
        pathData->m_totalSimulationPathCost =
            simulationTotalCost;
    }

    if (simulationEdgeCost >= 0)
    {
        pathData->m_totalSimulationEdgeCosts =
            simulationEdgeCost;
    }

    if (simulationTerminalCost >= 0)
    {
        pathData->m_totalSimulationTerminalCosts =
            simulationTerminalCost;
    }

    // Update the table display for this path
    for (int row = 0; row < m_table->rowCount(); ++row)
    {
        // Get the path ID for this row
        auto idItem = m_table->item(row, 1);
        if (idItem && idItem->text().toInt() == pathId)
        {
            // Update the actual cost cell if simulation
            // total cost was updated
            if (simulationTotalCost >= 0)
            {
                m_table->item(row, 4)->setText(
                    QString::number(simulationTotalCost,
                                    'f', 2));
            }
            break;
        }
    }
}

/**
 * @brief Creates a visualization widget for a path
 * @param pathId ID of the path to visualize
 * @param pathData PathData object containing path details
 * @return Widget containing visual representation of the
 * path
 *
 * Generates a custom widget showing terminals and
 * transportation modes for a path, including a button to
 * show it on the map.
 */
QWidget *
ShortestPathsTable::createPathRow(int             pathId,
                                  const PathData *pathData)
{
    // Check if path pointer is valid
    if (!pathData->path)
    {
        qWarning()
            << "Cannot create path row: path is null for ID"
            << pathId;
        return new QWidget(); // Return empty widget on
                              // error
    }

    // Create container widget for the path visualization
    auto widget = new QWidget();
    auto layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(
        4); // Compact spacing between elements

    // Create show button to visualize the path on the map
    auto showButton = new QPushButton();
    showButton->setIcon(IconFactory::createShowEyeIcon());
    showButton->setFixedSize(24, 24);
    showButton->setToolTip(tr("Show this path on the map"));

    // Connect button to show path signal
    connect(
        showButton, &QPushButton::clicked, this,
        [this, pathId]() { emit showPathSignal(pathId); });

    layout->addWidget(showButton);

    // Get terminals and segments from the path
    const QList<QJsonObject> &terminals =
        pathData->path->getTerminalsInPath();
    const QList<Backend::PathSegment *> &segments =
        pathData->path->getSegments();

    // Validate terminal and segment data
    if (terminals.isEmpty())
    {
        qWarning() << "No terminals for path ID" << pathId;
        layout->addWidget(
            new QLabel(tr("No terminal data")));
        layout->addStretch();
        return widget;
    }

    // Add terminal names and transportation mode indicators
    for (int i = 0; i < terminals.size(); ++i)
    {
        // Extract terminal name from the JSON object
        QString terminalName =
            terminals[i]["name"].toString();
        if (terminalName.isEmpty())
        {
            terminalName = tr("Terminal %1")
                               .arg(i + 1); // Fallback name
        }

        // Add terminal name label
        auto nameLabel = new QLabel(terminalName);
        nameLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(nameLabel);

        // Add transportation mode arrow for all but the
        // last terminal
        if (i < terminals.size() - 1 && i < segments.size())
        {
            auto modeLabel = new QLabel();
            modeLabel->setAlignment(Qt::AlignCenter);

            // Get transportation mode from the segment
            Backend::TransportationTypes::TransportationMode
                    mode = segments[i]->getMode();
            QString modeText =
                Backend::TransportationTypes::toString(
                    mode);

            // Create transportation mode pixmap
            QPixmap modePixmap =
                IconFactory::createTransportationModePixmap(
                    modeText);

            modeLabel->setPixmap(modePixmap);
            modeLabel->setToolTip(
                modeText); // Show mode name on hover
            layout->addWidget(modeLabel);
        }
    }

    // Add stretch to ensure left alignment of path
    // visualization
    layout->addStretch();
    return widget;
}

/**
 * @brief Creates a transportation mode arrow pixmap
 * @param mode Transportation mode text
 * @return Pixmap containing the arrow and mode
 * visualization
 *
 * Generates a visual representation of a transportation
 * mode with appropriate coloring and labeling.
 */
QPixmap ShortestPathsTable::createArrowPixmap(
    const QString &mode) const
{
    // Create a pixmap for the arrow with the mode text
    QPixmap pixmap(64, 40); // Fixed size for consistency
    pixmap.fill(Qt::transparent); // Start with transparent
                                  // background

    QPainter painter(&pixmap);
    painter.setRenderHint(
        QPainter::Antialiasing); // Smooth rendering

    // Set color based on transportation mode
    QColor arrowColor = Qt::black; // Default color

    // Choose color based on transportation mode
    if (mode.contains("Truck", Qt::CaseInsensitive))
    {
        arrowColor =
            QColor(255, 0, 255); // Magenta for truck
    }
    else if (mode.contains("Rail", Qt::CaseInsensitive)
             || mode.contains("Train", Qt::CaseInsensitive))
    {
        arrowColor =
            QColor(80, 80, 80); // Dark gray for rail
    }
    else if (mode.contains("Ship", Qt::CaseInsensitive)
             || mode.contains("Water", Qt::CaseInsensitive))
    {
        arrowColor = QColor(0, 0, 255); // Blue for ship
    }

    // Draw the mode text
    painter.setPen(arrowColor);
    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRect(0, 0, pixmap.width(), 15),
                     Qt::AlignCenter, mode);

    // Draw the arrow
    QPen pen(arrowColor, 2); // 2 pixel width
    painter.setPen(pen);

    // Draw arrow shaft
    painter.drawLine(10, 25, 54, 25);

    // Draw arrow head
    QPolygon arrowHead;
    arrowHead << QPoint(48, 20) << QPoint(54, 25)
              << QPoint(48, 30);
    painter.setBrush(arrowColor);
    painter.drawPolygon(arrowHead);

    return pixmap;
}

/**
 * @brief Refreshes the table display with current path data
 *
 * Rebuilds the table contents based on the current state of
 * the PathData objects.
 */
void ShortestPathsTable::refreshTable()
{
    // Set flag to prevent recursive UI updates during
    // refresh
    m_updatingUI = true;

    // Clear the table while preserving header
    m_table->setRowCount(0);

    // Add rows for each visible path
    for (auto it = m_pathData.begin();
         it != m_pathData.end(); ++it)
    {
        int             pathId   = it.key();
        const PathData *pathData = it.value();

        // Skip paths marked as not visible
        if (!pathData->isVisible || !pathData->path)
        {
            continue;
        }

        // Add a new row at the end of the table
        int row = m_table->rowCount();
        m_table->insertRow(row);

        // Create checkbox widget for the select column
        auto checkboxWidget = new QWidget();
        auto checkboxLayout =
            new QHBoxLayout(checkboxWidget);
        checkboxLayout->setAlignment(Qt::AlignCenter);
        checkboxLayout->setContentsMargins(0, 0, 0, 0);

        auto checkbox = new QCheckBox();
        checkboxLayout->addWidget(checkbox);
        m_table->setCellWidget(row, 0, checkboxWidget);

        // Connect checkbox state change to update
        // compare button state
        connect(checkbox, &QCheckBox::checkStateChanged,
                this, [this, pathId](Qt::CheckState state) {
                    // Emit signal that checkbox state
                    // changed
                    emit checkboxChanged(
                        pathId, state == Qt::Checked);

                    // Enable compare button if at least
                    // 2 paths are checked
                    m_compareButton->setEnabled(
                        getCheckedPathIds().size() >= 2);
                });

        // Add Path ID cell
        auto pathItem =
            new QTableWidgetItem(QString::number(pathId));
        m_table->setItem(row, 1, pathItem);

        // Create and add terminal path visualization
        auto pathWidget = createPathRow(pathId, pathData);
        m_table->setCellWidget(row, 2, pathWidget);

        // Store widget pointer in user role for custom
        // delegate
        QVariant widgetPtr;
        widgetPtr.setValue(pathWidget);
        m_table->model()->setData(
            m_table->model()->index(row, 2), widgetPtr,
            Qt::UserRole);

        // Add Predicted Cost cell
        QString predictedCostText;
        if (pathData->path->getTotalPathCost() >= 0)
        {
            predictedCostText = QString::number(
                pathData->path->getTotalPathCost(), 'f', 2);
        }
        else
        {
            predictedCostText = tr("Waiting analysis");
        }
        auto predictedItem =
            new QTableWidgetItem(predictedCostText);
        m_table->setItem(row, 3, predictedItem);

        // Add Actual Cost cell
        QString actualCostText;
        if (pathData->m_totalSimulationPathCost >= 0)
        {
            actualCostText = QString::number(
                pathData->m_totalSimulationPathCost, 'f',
                2);
        }
        else
        {
            actualCostText = tr("Waiting simulation");
        }
        auto actualItem =
            new QTableWidgetItem(actualCostText);
        m_table->setItem(row, 4, actualItem);
    }

    // Clear UI update flag
    m_updatingUI = false;

    // Update button states
    m_exportAllButton->setEnabled(!m_pathData.isEmpty());
}

/**
 * @brief Retrieves path data for a specific path ID
 * @param pathId The ID of the path to retrieve
 * @return Pointer to the path data or nullptr if not found
 */
const ShortestPathsTable::PathData *
ShortestPathsTable::getDataByPathId(int pathId) const
{
    // Look up the path in the map
    auto it = m_pathData.find(pathId);
    if (it == m_pathData.end())
    {
        return nullptr; // Path not found
    }

    // Return pointer to the found path data
    return it.value();
}

/**
 * @brief Gets the currently selected path ID
 * @return The selected path ID or -1 if none selected
 */
int ShortestPathsTable::getSelectedPathId() const
{
    // Get all selected items
    QList<QTableWidgetItem *> selectedItems =
        m_table->selectedItems();
    if (selectedItems.isEmpty())
    {
        return -1; // No selection
    }

    // Get the row of the first selected item
    int row = selectedItems.first()->row();

    // Get the Path ID from column 1
    auto idItem = m_table->item(row, 1);
    return idItem ? idItem->text().toInt() : -1;
}

/**
 * @brief Gets all path IDs that are currently checked
 * @return Vector of checked path IDs
 */
QVector<int> ShortestPathsTable::getCheckedPathIds() const
{
    QVector<int> checkedPaths;

    // Iterate through all rows in the table
    for (int row = 0; row < m_table->rowCount(); ++row)
    {
        // Get the checkbox widget in the first column
        auto checkboxWidget = m_table->cellWidget(row, 0);
        if (!checkboxWidget)
        {
            continue; // Skip rows without checkbox widget
        }

        // Get the layout from the widget
        auto layout = checkboxWidget->layout();
        if (!layout)
        {
            continue; // Skip if no layout
        }

        // Get the checkbox from the layout
        auto checkBox = qobject_cast<QCheckBox *>(
            layout->itemAt(0)->widget());
        if (checkBox && checkBox->isChecked())
        {
            // If checkbox exists and is checked, add the
            // path ID
            auto idItem = m_table->item(row, 1);
            if (idItem)
            {
                checkedPaths.append(idItem->text().toInt());
            }
        }
    }

    return checkedPaths;
}

/**
 * @brief Clears all data from the table
 *
 * Removes all paths and resets the UI state.
 */
void ShortestPathsTable::clear()
{
    // Clear all path data, which will delete the owned Path
    // pointers
    m_pathData.clear();

    // Clear the table
    m_table->setRowCount(0);

    // Disable buttons that require paths
    m_compareButton->setEnabled(false);
    m_exportButton->setEnabled(false);
    m_exportAllButton->setEnabled(false);
}

/**
 * @brief Slot called when selection in the table changes
 *
 * Updates the UI state based on the new selection and
 * emits signals to notify listeners.
 */
void ShortestPathsTable::onSelectionChanged()
{
    // Ignore selection changes during UI updates
    if (m_updatingUI)
    {
        return;
    }

    // Get the currently selected path ID
    int pathId = getSelectedPathId();
    if (pathId >= 0)
    {
        // A path is selected
        emit pathSelected(pathId);
        m_exportButton->setEnabled(true);
    }
    else
    {
        // No path is selected
        m_exportButton->setEnabled(false);
    }
}

/**
 * @brief Slot called when an item's checked state changes
 * @param row The row of the changed item
 * @param column The column of the changed item
 *
 * Note: This slot is not directly connected but is provided
 * for completeness and potential future use.
 */
void ShortestPathsTable::onItemCheckedChanged(int row,
                                              int column)
{
    // This functionality is handled by the checkbox state
    // change connections established during table refresh
}

/**
 * @brief Slot called when the compare button is clicked
 *
 * Gathers checked paths and emits comparison signal if at
 * least two paths are selected.
 */
void ShortestPathsTable::onCompareButtonClicked()
{
    // Get IDs of all checked paths
    QVector<int> checkedPaths = getCheckedPathIds();

    // Verify we have enough paths to compare
    if (checkedPaths.size() >= 2)
    {
        // Emit signal to request path comparison
        emit pathComparisonRequested(checkedPaths);
    }
    else
    {
        // Show error if not enough paths are selected
        QMessageBox::information(
            this, tr("Path Comparison"),
            tr("Please select at least two paths to "
               "compare."));
    }
}

/**
 * @brief Slot called when the export button is clicked
 *
 * Emits signal to export the currently selected path.
 */
void ShortestPathsTable::onExportButtonClicked()
{
    // Get the currently selected path ID
    int pathId = getSelectedPathId();
    if (pathId >= 0)
    {
        // Emit signal to request path export
        emit pathExportRequested(pathId);
    }
}

/**
 * @brief Slot called when the export all button is clicked
 *
 * Emits signal to export all paths currently in the table.
 */
void ShortestPathsTable::onExportAllButtonClicked()
{
    // Verify we have paths to export
    if (!m_pathData.isEmpty())
    {
        // Emit signal to request export of all paths
        emit allPathsExportRequested();
    }
}

} // namespace GUI
} // namespace CargoNetSim
