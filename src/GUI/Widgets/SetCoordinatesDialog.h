#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

namespace CargoNetSim
{
namespace GUI
{

/**
 * @brief Dialog for setting geographic coordinates for a
 * terminal
 *
 * This dialog allows users to input latitude and longitude
 * coordinates for a terminal's global position. It
 * validates the input and provides a clean interface for
 * coordinate editing.
 */
class SetCoordinatesDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param terminalName Name of the terminal being edited
     * @param currentLat Current latitude value
     * @param currentLon Current longitude value
     * @param parent Parent widget
     */
    explicit SetCoordinatesDialog(
        const QString &terminalName,
        double currentLat = 0.0, double currentLon = 0.0,
        QWidget *parent = nullptr);

    /**
     * @brief Get the coordinates entered by the user
     * @return Pair of latitude and longitude values
     */
    std::pair<double, double> getCoordinates() const;

signals:
    /**
     * @brief Signal emitted when coordinates are changed
     * @param lat New latitude value
     * @param lon New longitude value
     */
    void coordinatesChanged(double lat, double lon);

private slots:
    /**
     * @brief Handle value changes in the lat/lon inputs
     */
    void onCoordinatesChanged();

private:
    QLabel           *infoLabel;
    QDoubleSpinBox   *latInput;
    QDoubleSpinBox   *lonInput;
    QDialogButtonBox *buttonBox;
    QVBoxLayout      *mainLayout;
    QFormLayout      *formLayout;
};

} // namespace GUI
} // namespace CargoNetSim
