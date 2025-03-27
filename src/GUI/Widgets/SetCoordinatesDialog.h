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
        const QString &terminalName, QPointF geoPoint,
        QWidget *parent = nullptr);

    /**
     * @brief Get the coordinates entered by the user
     * @return QPointF Geodetic coordinates (long, lat)
     */
    QPointF getCoordinates() const;

signals:
    /**
     * @brief Signal emitted when coordinates are changed
     * @param geoPoint New geodetic coordinates (long, lat)
     */
    void coordinatesChanged(QPointF geoPoint);

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
