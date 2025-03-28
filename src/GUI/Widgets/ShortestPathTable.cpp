#include "ShortestPathTable.h"
#include "../Utils/IconCreator.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QVariant>

Q_DECLARE_METATYPE(QWidget *)

namespace CargoNetSim
{
namespace GUI
{

// Terminal Path Delegate Implementation
void TerminalPathDelegate::paint(
    QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    // Check if we're in the terminals column
    if (index.column() == 2)
    {
        QWidget *widget = qvariant_cast<QWidget *>(
            index.data(Qt::UserRole));
        if (widget)
        {
            // Just render the widget in the cell
            QPixmap pixmap(widget->size());
            widget->render(&pixmap);
            painter->drawPixmap(option.rect.topLeft(),
                                pixmap);
            return;
        }
    }

    // Default rendering for other columns
    QStyledItemDelegate::paint(painter, option, index);
}

QSize TerminalPathDelegate::sizeHint(
    const QStyleOptionViewItem &option,
    const QModelIndex          &index) const
{
    if (index.column() == 2)
    {
        QWidget *widget = qvariant_cast<QWidget *>(
            index.data(Qt::UserRole));
        if (widget)
        {
            return widget->size();
        }
    }
    return QStyledItemDelegate::sizeHint(option, index);
}

ShortestPathsTable::ShortestPathsTable(QWidget *parent)
    : QWidget(parent)
    , m_updatingUI(false)
{
    initUI();
}

void ShortestPathsTable::initUI()
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    createTableWidget();
    layout->addWidget(m_table);

    auto buttonPanel = new QHBoxLayout();
    createPathButtonPanel();
    createExportPanel();

    buttonPanel->addStretch();
    buttonPanel->addWidget(m_compareButton);
    buttonPanel->addWidget(m_exportButton);
    buttonPanel->addWidget(m_exportAllButton);

    layout->addLayout(buttonPanel);
}

void ShortestPathsTable::createTableWidget()
{
    m_table = new QTableWidget(this);
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels(
        {"Select", "Path ID", "Terminal Path",
         "Predicted Cost", "Actual Cost"});

    m_table->setSelectionBehavior(QTableWidget::SelectRows);
    m_table->setSelectionMode(
        QTableWidget::SingleSelection);
    m_table->setEditTriggers(QTableWidget::NoEditTriggers);

    auto header = m_table->horizontalHeader();

    // Set column widths
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    m_table->setColumnWidth(0, 50); // Checkbox column

    header->setSectionResizeMode(
        1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::Stretch);
    header->setSectionResizeMode(
        3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(
        4, QHeaderView::ResizeToContents);

    // Set custom delegate for terminal path column
    m_table->setItemDelegate(
        new TerminalPathDelegate(m_table));

    // Connect selection signals
    connect(m_table, &QTableWidget::itemSelectionChanged,
            this, &ShortestPathsTable::onSelectionChanged);
}

void ShortestPathsTable::createPathButtonPanel()
{
    m_compareButton =
        new QPushButton(tr("Compare Paths"), this);
    m_compareButton->setToolTip(
        tr("Compare selected paths"));
    m_compareButton->setEnabled(false);
    connect(m_compareButton, &QPushButton::clicked, this,
            &ShortestPathsTable::onCompareButtonClicked);
}

void ShortestPathsTable::createExportPanel()
{
    m_exportButton =
        new QPushButton(tr("Export Path"), this);
    m_exportButton->setToolTip(
        tr("Export selected path data"));
    m_exportButton->setEnabled(false);
    connect(m_exportButton, &QPushButton::clicked, this,
            &ShortestPathsTable::onExportButtonClicked);

    m_exportAllButton =
        new QPushButton(tr("Export All"), this);
    m_exportAllButton->setToolTip(
        tr("Export all path data"));
    m_exportAllButton->setEnabled(false);
    connect(m_exportAllButton, &QPushButton::clicked, this,
            &ShortestPathsTable::onExportAllButtonClicked);
}

void ShortestPathsTable::addPath(
    int pathId, const QVector<int> &terminalIds,
    const QVector<QString> &terminalNames,
    const QVector<QString> &transportationModes,
    const QVector<QString> &edgeIds, double predictedCost,
    double actualCost)
{
    // Store path data
    PathData pathData = {
        pathId,        terminalIds,
        terminalNames, transportationModes,
        edgeIds,       predictedCost,
        actualCost,
        true // visible by default
    };
    m_pathData[pathId] = pathData;

    // Add to table
    refreshTable();

    // Enable export all button if we have data
    m_exportAllButton->setEnabled(!m_pathData.isEmpty());
}

QWidget *
ShortestPathsTable::createPathRow(int             pathId,
                                  const PathData &pathData)
{
    auto widget = new QWidget();
    auto layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    // Create a show button
    auto showButton = new QPushButton();
    showButton->setIcon(IconFactory::createShowEyeIcon());
    showButton->setFixedSize(24, 24);
    showButton->setToolTip(tr("Show this path on the map"));
    connect(
        showButton, &QPushButton::clicked,
        [this, pathId]() { emit showPathSignal(pathId); });
    layout->addWidget(showButton);

    // Add terminal names and transportation mode arrows
    for (int i = 0; i < pathData.terminalNames.size(); ++i)
    {
        // Add terminal name
        auto nameLabel =
            new QLabel(pathData.terminalNames[i]);
        nameLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(nameLabel);

        // Add arrow + transportation mode for all but the
        // last terminal
        if (i < pathData.terminalNames.size() - 1)
        {
            auto modeLabel = new QLabel();
            modeLabel->setAlignment(Qt::AlignCenter);

            // Create transportation mode pixmap
            QString modeText =
                pathData.transportationModes[i];
            QPixmap modePixmap =
                IconFactory::createTransportationModePixmap(
                    modeText);

            modeLabel->setPixmap(modePixmap);
            modeLabel->setToolTip(modeText);
            layout->addWidget(modeLabel);
        }
    }

    layout->addStretch();
    return widget;
}

QPixmap ShortestPathsTable::createArrowPixmap(
    const QString &mode) const
{
    // Create a pixmap for the arrow with the mode text
    QPixmap pixmap(64, 40);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Set color based on transportation mode
    QColor arrowColor = Qt::black;
    if (mode.contains("Truck", Qt::CaseInsensitive))
    {
        arrowColor =
            QColor(255, 0, 255); // Magenta for truck
    }
    else if (mode.contains("Rail", Qt::CaseInsensitive))
    {
        arrowColor =
            QColor(80, 80, 80); // Dark gray for rail
    }
    else if (mode.contains("Ship", Qt::CaseInsensitive))
    {
        arrowColor = QColor(0, 0, 255); // Blue for ship
    }

    // Draw the text
    painter.setPen(arrowColor);
    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRect(0, 0, pixmap.width(), 15),
                     Qt::AlignCenter, mode);

    // Draw the arrow
    QPen pen(arrowColor, 2);
    painter.setPen(pen);

    // Arrow shaft
    painter.drawLine(10, 25, 54, 25);

    // Arrow head
    QPolygon arrowHead;
    arrowHead << QPoint(48, 20) << QPoint(54, 25)
              << QPoint(48, 30);
    painter.setBrush(arrowColor);
    painter.drawPolygon(arrowHead);

    return pixmap;
}

void ShortestPathsTable::refreshTable()
{
    m_updatingUI = true;

    // Clear the table but preserve the headers
    m_table->setRowCount(0);

    // Add rows for each path
    for (auto it = m_pathData.begin();
         it != m_pathData.end(); ++it)
    {
        int             pathId   = it.key();
        const PathData &pathData = it.value();

        if (!pathData.isVisible)
        {
            continue;
        }

        int row = m_table->rowCount();
        m_table->insertRow(row);

        // Checkbox widget for select column
        auto checkboxWidget = new QWidget();
        auto checkboxLayout =
            new QHBoxLayout(checkboxWidget);
        checkboxLayout->setAlignment(Qt::AlignCenter);
        checkboxLayout->setContentsMargins(0, 0, 0, 0);

        auto checkbox = new QCheckBox();
        checkboxLayout->addWidget(checkbox);
        m_table->setCellWidget(row, 0, checkboxWidget);

        // Connect checkbox state change
        connect(checkbox, &QCheckBox::checkStateChanged,
                [this, pathId](Qt::CheckState state) {
                    emit checkboxChanged(
                        pathId, state == Qt::Checked);

                    // Enable compare button if we have at
                    // least 2 checked paths
                    m_compareButton->setEnabled(
                        getCheckedPathIds().size() >= 2);
                });

        // Path ID
        auto pathItem =
            new QTableWidgetItem(QString::number(pathId));
        m_table->setItem(row, 1, pathItem);

        // Terminal Path
        auto pathWidget = createPathRow(pathId, pathData);
        m_table->setCellWidget(row, 2, pathWidget);

        // Store widget pointer in user role for custom
        // delegate
        QVariant widgetPtr;
        widgetPtr.setValue(static_cast<quintptr>(
            reinterpret_cast<quintptr>(pathWidget)));
        m_table->model()->setData(
            m_table->model()->index(row, 2), widgetPtr,
            Qt::UserRole);

        // Predicted Cost
        QString predictedCostText =
            (pathData.predictedCost >= 0)
                ? QString::number(pathData.predictedCost,
                                  'f', 2)
                : tr("Waiting analysis");
        auto predictedItem =
            new QTableWidgetItem(predictedCostText);
        m_table->setItem(row, 3, predictedItem);

        // Actual Cost
        QString actualCostText =
            (pathData.actualCost >= 0)
                ? QString::number(pathData.actualCost, 'f',
                                  2)
                : tr("Waiting simulation");
        auto actualItem =
            new QTableWidgetItem(actualCostText);
        m_table->setItem(row, 4, actualItem);
    }

    m_updatingUI = false;

    // Update button states
    m_exportAllButton->setEnabled(!m_pathData.isEmpty());
}

void ShortestPathsTable::updateCosts(int    pathId,
                                     double predictedCost,
                                     double actualCost)
{
    if (!m_pathData.contains(pathId))
    {
        return;
    }

    // Update the stored data
    if (predictedCost >= 0)
    {
        m_pathData[pathId].predictedCost = predictedCost;
    }

    if (actualCost >= 0)
    {
        m_pathData[pathId].actualCost = actualCost;
    }

    // Update table display
    for (int row = 0; row < m_table->rowCount(); ++row)
    {
        auto idItem = m_table->item(row, 1);
        if (idItem && idItem->text().toInt() == pathId)
        {
            if (predictedCost >= 0)
            {
                m_table->item(row, 3)->setText(
                    QString::number(predictedCost, 'f', 2));
            }

            if (actualCost >= 0)
            {
                m_table->item(row, 4)->setText(
                    QString::number(actualCost, 'f', 2));
            }
            break;
        }
    }
}

const ShortestPathsTable::PathData *
ShortestPathsTable::getDataByPathId(int pathId) const
{
    auto it = m_pathData.find(pathId);
    if (it == m_pathData.end())
    {
        return nullptr;
    }
    return &(it.value());
}

int ShortestPathsTable::getSelectedPathId() const
{
    QList<QTableWidgetItem *> selectedItems =
        m_table->selectedItems();
    if (selectedItems.isEmpty())
    {
        return -1;
    }

    int  row    = selectedItems.first()->row();
    auto idItem = m_table->item(row, 1);
    return idItem ? idItem->text().toInt() : -1;
}

QVector<int> ShortestPathsTable::getCheckedPathIds() const
{
    QVector<int> checkedPaths;

    for (int row = 0; row < m_table->rowCount(); ++row)
    {
        auto checkboxWidget = m_table->cellWidget(row, 0);
        if (!checkboxWidget)
            continue;

        auto layout = checkboxWidget->layout();
        if (!layout)
            continue;

        auto checkBox = qobject_cast<QCheckBox *>(
            layout->itemAt(0)->widget());
        if (checkBox && checkBox->isChecked())
        {
            auto idItem = m_table->item(row, 1);
            if (idItem)
            {
                checkedPaths.append(idItem->text().toInt());
            }
        }
    }

    return checkedPaths;
}

void ShortestPathsTable::clear()
{
    m_pathData.clear();
    m_table->setRowCount(0);

    m_compareButton->setEnabled(false);
    m_exportButton->setEnabled(false);
    m_exportAllButton->setEnabled(false);
}

void ShortestPathsTable::onSelectionChanged()
{
    if (m_updatingUI)
    {
        return;
    }

    int pathId = getSelectedPathId();
    if (pathId >= 0)
    {
        emit pathSelected(pathId);
        m_exportButton->setEnabled(true);
    }
    else
    {
        m_exportButton->setEnabled(false);
    }
}

void ShortestPathsTable::onItemCheckedChanged(int row,
                                              int column)
{
    // This is handled by the checkbox state change
    // connections
}

void ShortestPathsTable::onCompareButtonClicked()
{
    QVector<int> checkedPaths = getCheckedPathIds();
    if (checkedPaths.size() >= 2)
    {
        emit pathComparisonRequested(checkedPaths);
    }
    else
    {
        QMessageBox::information(
            this, tr("Path Comparison"),
            tr("Please select at least "
               "two paths to compare."));
    }
}

void ShortestPathsTable::onExportButtonClicked()
{
    int pathId = getSelectedPathId();
    if (pathId >= 0)
    {
        emit pathExportRequested(pathId);
    }
}

void ShortestPathsTable::onExportAllButtonClicked()
{
    if (!m_pathData.isEmpty())
    {
        emit allPathsExportRequested();
    }
}

} // namespace GUI
} // namespace CargoNetSim
