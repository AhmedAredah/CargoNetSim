#pragma once
#include "GUI/Commons/NetworkType.h"
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtWidgets/qlabel.h>
namespace CargoNetSim
{
namespace GUI
{
class NetworkSelectionDialog : public QDialog
{
    Q_OBJECT
public:
    enum Mode
    {
        LinkMode,
        UnlinkMode
    };
    explicit NetworkSelectionDialog(
        QWidget *parent = nullptr, Mode mode = LinkMode);
    ~NetworkSelectionDialog() = default;
    QList<NetworkType> getSelectedNetworkTypes() const;
    void               setMode(Mode mode);

private:
    QCheckBox   *trainNetworkCheckBox;
    QCheckBox   *truckNetworkCheckBox;
    QPushButton *linkSelectedButton;
    QPushButton *linkAllVisibleButton;
    QPushButton *cancelButton;
    QLabel      *descriptionLabel;
    Mode         currentMode;
    void         updateButtonLabels();
private slots:
    void onCheckBoxStateChanged();
};
} // namespace GUI
} // namespace CargoNetSim
