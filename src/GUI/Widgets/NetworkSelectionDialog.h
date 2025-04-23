#pragma once

#include "GUI/Commons/NetworkType.h"
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace CargoNetSim
{
namespace GUI
{

class NetworkSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkSelectionDialog(
        QWidget *parent = nullptr);
    ~NetworkSelectionDialog() = default;

    QList<NetworkType> getSelectedNetworkTypes() const;

private:
    QCheckBox        *trainNetworkCheckBox;
    QCheckBox        *truckNetworkCheckBox;
    QPushButton      *linkButton;
    QPushButton      *cancelButton;
    QDialogButtonBox *buttonBox;
    QPushButton      *linkSelectedButton;
    QPushButton      *linkAllVisibleButton;

private slots:
    void onCheckBoxStateChanged();
};

} // namespace GUI
} // namespace CargoNetSim
