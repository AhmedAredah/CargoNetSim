#ifndef NETWORKMOVEDIALOG_H
#define NETWORKMOVEDIALOG_H
#include "GUI/Controllers/NetworkController.h"
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QVBoxLayout>
namespace CargoNetSim
{
namespace GUI
{
class NetworkMoveDialog : public QDialog
{
    Q_OBJECT
public:
    NetworkMoveDialog(MainWindow    *mainWindow,
                      const QString &regionName,
                      bool           isProjectedCoords,
                      QWidget       *parent = nullptr);
    ~NetworkMoveDialog();
    QPointF     getOffset() const;
    NetworkType getSelectedNetworkType() const;
    QString     getSelectedNetworkName() const;
    bool        hasNetworkSelected() const;
private slots:
    void updateNetworkSelectionUI();
    void onTrainGroupToggled(bool checked);
    void onTruckGroupToggled(bool checked);

private:
    QDoubleSpinBox *horizontalOffsetSpinBox;
    QDoubleSpinBox *verticalOffsetSpinBox;
    QLabel         *unitsLabel;
    bool            isProjected;
    // Network selection
    QComboBox  *trainNetworkCombo;
    QComboBox  *truckNetworkCombo;
    QGroupBox  *trainGroupBox;
    QGroupBox  *truckGroupBox;
    NetworkType selectedNetworkType;
    QString     selectedNetworkName;
    // Button box and OK button
    QDialogButtonBox *buttonBox;
    QPushButton      *okButton;
    // Reference to main window for accessing region data
    MainWindow *mainWindow;
    QString     regionName;
    void        populateNetworkLists();
};
} // namespace GUI
} // namespace CargoNetSim
#endif // NETWORKMOVEDIALOG_H
