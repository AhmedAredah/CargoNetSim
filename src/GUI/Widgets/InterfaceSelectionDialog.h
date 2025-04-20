#ifndef INTERFACESELECTIONDIALOG_H
#define INTERFACESELECTIONDIALOG_H
#include <QCheckBox>
#include <QDialog>
#include <QGridLayout>
#include <QList>
#include <QMap>
#include <QSet>
#include <QString>

namespace CargoNetSim
{
namespace GUI
{
class InterfaceSelectionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InterfaceSelectionDialog(
        const QSet<QString> &availableInterfaces,
        const QSet<QString> &visibleTerminalTypes,
        QWidget             *parent = nullptr);
    ~InterfaceSelectionDialog();

    QList<QString>      getSelectedInterfaces() const;
    QMap<QString, bool> getIncludedTerminalTypes() const;

private slots:
    void selectAllInterfaces();
    void deselectAllInterfaces();
    void selectAllTerminalTypes();
    void deselectAllTerminalTypes();

private:
    QMap<QString, QCheckBox *> m_checkboxes;
    QMap<QString, QCheckBox *> m_terminalTypeCheckboxes;
};
} // namespace GUI
} // namespace CargoNetSim
#endif // INTERFACESELECTIONDIALOG_H
