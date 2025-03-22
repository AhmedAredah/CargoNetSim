#pragma once
#include <QBrush>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMap>
#include <QPainterPath>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

namespace CargoNetSim {
namespace GUI {

// Custom delegate for rendering the terminal path column
class TerminalPathDelegate : public QStyledItemDelegate {
public:
    explicit TerminalPathDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent) {}

    void paint(QPainter                   *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

class ShortestPathsTable : public QWidget {
    Q_OBJECT
public:
    // Structure to hold path data
    struct PathData {
        int              pathId;
        QVector<int>     terminalIds;
        QVector<QString> terminalNames;
        QVector<QString> transportationModes;
        QVector<QString> edgeIds;
        double           predictedCost;
        double           actualCost;
        bool             isVisible;
    };

    explicit ShortestPathsTable(QWidget *parent = nullptr);
    virtual ~ShortestPathsTable() = default;

    /**
     * @brief Adds a new path to the table
     * @param pathId Unique identifier for the path
     * @param terminalIds Vector of terminal IDs in the path
     * @param terminalNames Vector of terminal names in the
     * path
     * @param transportationModes Vector of transportation
     * modes between terminals
     * @param edgeIds Vector of edge IDs used in the path
     * @param predictedCost Predicted cost of the path
     *          (default: -1.0 for "waiting analysis")
     * @param actualCost Actual cost of the path
     *          (default: -1.0 for "waiting simulation")
     */
    void
    addPath(int pathId, const QVector<int> &terminalIds,
            const QVector<QString> &terminalNames,
            const QVector<QString> &transportationModes,
            const QVector<QString> &edgeIds,
            double                  predictedCost = -1.0,
            double                  actualCost    = -1.0);

    /**
     * @brief Updates the costs for an existing path
     * @param pathId The ID of the path to update
     * @param predictedCost New predicted cost (default:
     * -1.0 to not update)
     * @param actualCost New actual cost (default: -1.0 to
     * not update)
     */
    void updateCosts(int    pathId,
                     double predictedCost = -1.0,
                     double actualCost    = -1.0);

    /**
     * @brief Gets the path data for a specific path ID
     * @param pathId The ID of the path to retrieve
     * @return Pointer to the path data or nullptr if not
     * found
     */
    const PathData *getDataByPathId(int pathId) const;

    /**
     * @brief Gets the currently selected path ID
     * @return The selected path ID or -1 if none selected
     */
    int getSelectedPathId() const;

    /**
     * @brief Gets all path IDs that are currently checked
     * @return Vector of checked path IDs
     */
    QVector<int> getCheckedPathIds() const;

    /**
     * @brief Clears all data from the table
     */
    void clear();

signals:
    // Emitted when a path is selected in the table
    void pathSelected(int pathId);

    // Emitted when the show path button is clicked
    void showPathSignal(int pathId);

    // Emitted when a path checkbox state changes
    void checkboxChanged(int pathId, bool checked);

    // Emitted when path comparison is requested
    void
    pathComparisonRequested(const QVector<int> &pathIds);

    // Emitted when path export is requested
    void pathExportRequested(int pathId);

    // Emitted when export of all paths is requested
    void allPathsExportRequested();

private slots:
    // Slot called when selection in the table changes
    void onSelectionChanged();

    // Slot called when an item's checked state changes
    void onItemCheckedChanged(int row, int column);

    // Slot called when the compare button is clicked
    void onCompareButtonClicked();

    // Slot called when the export button is clicked
    void onExportButtonClicked();

    // Slot called when the export all button is clicked
    void onExportAllButtonClicked();

private:
    // Initializes the UI components
    void initUI();

    // Creates the table widget and configures it
    void createTableWidget();

    // Creates the path button panel
    void createPathButtonPanel();

    // Creates the export panel
    void createExportPanel();

    // Refreshes the table display with current path data
    void refreshTable();

    // Creates a widget containing the terminal
    // path visualization for a given path
    QWidget *createPathRow(int             pathId,
                           const PathData &pathData);

    // Creates an arrow pixmap with a label for a
    // transportation mode
    QPixmap createArrowPixmap(const QString &mode) const;

    QTableWidget       *m_table;    // Table widget
    QMap<int, PathData> m_pathData; // Storage for path data
                                    // indexed by path ID
    QPushButton *
        m_compareButton; // Button to compare selected paths
    QPushButton
        *m_exportButton; // Button to export selected path
    QPushButton
        *m_exportAllButton; // Button to export all paths
    bool m_updatingUI;      // Flag to prevent recursive UI
                            // updates
};

} // namespace GUI
} // namespace CargoNetSim
