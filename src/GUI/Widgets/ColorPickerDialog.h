#pragma once

#include <QColor>
#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QMap>
#include <QPushButton>
#include <QTabWidget>

namespace CargoNetSim {
namespace GUI {

class ColorPalette;

/**
 * @brief Dialog for selecting colors from either a
 * predefined palette or custom color picker
 *
 * The ColorPickerDialog provides a two-tab interface for
 * selecting colors:
 * 1. A list of predefined colors from the ColorPalette
 * 2. A custom color selection tab with a color dialog
 * button
 */
class ColorPickerDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Construct a new Color Picker Dialog
     *
     * @param currentColor Initial color to select (or null
     * if none)
     * @param parent Parent widget
     */
    explicit ColorPickerDialog(
        const QColor &currentColor = QColor(),
        QWidget      *parent       = nullptr);

    /**
     * @brief Get the selected color from the dialog
     *
     * @return QColor The selected color (invalid if no
     * color was selected)
     */
    QColor getSelectedColor() const;

private slots:
    /**
     * @brief Open the native color dialog when "Select
     * Custom Color" is clicked
     */
    void openColorDialog();

    /**
     * @brief Update the preview when selection changes
     *
     * @param item The newly selected list item (for
     * predefined colors)
     */
    void updatePreview(QListWidgetItem *item = nullptr);

    /**
     * @brief Handle tab changes to update preview
     * accordingly
     *
     * @param index The newly selected tab index
     */
    void onTabChanged(int index);

private:
    /**
     * @brief Set up the user interface
     */
    void setupUI();

    // UI Elements
    QTabWidget  *tabWidget;
    QListWidget *colorList;
    QPushButton *colorButton;
    QLabel      *customPreview;
    QLabel      *previewLabel;

    // State
    QColor currentColor; ///< Current selected color
    QColor
        customColor; ///< Current custom color if selected
};

} // namespace GUI
} // namespace CargoNetSim
