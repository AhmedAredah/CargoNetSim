#ifndef TERMINALSELECTIONDIALOG_H
#define TERMINALSELECTIONDIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QScrollArea>
#include <QSet>
#include <QSplitter>
#include <QStringList>
#include <QVBoxLayout>

namespace CargoNetSim
{
namespace GUI
{

class MainWindow;

class TerminalSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TerminalSelectionDialog(
        MainWindow *mainWindow, QWidget *parent = nullptr);
    ~TerminalSelectionDialog();

    QStringList getSelectedTerminalNames() const;
    QStringList getSelectedConnectionTypes() const;

private slots:
    void updateFilter();
    void validateSelections();
    void filterTerminalList(const QString &text);
    void selectAllTerminals(bool checked);
    void selectAllConnectionTypes(bool checked);

private:
    MainWindow *mainWindow_;

    QLineEdit         *searchField_;
    QListWidget       *terminalListWidget_;
    QCheckBox         *selectAllTerminalsCheckBox_;
    QWidget           *connectionTypesWidget_;
    QList<QCheckBox *> connectionTypeCheckBoxes_;
    QCheckBox         *selectAllConnectionTypesCheckBox_;
    QPushButton       *okButton_;
    QPushButton       *cancelButton_;

    QStringList originalTerminalNames_;
    QStringList availableConnectionTypes_;

    void populateTerminalNames();
    void populateConnectionTypes();
    void setupUI();
    void createConnections();
};

} // namespace GUI
} // namespace CargoNetSim

#endif // TERMINALSELECTIONDIALOG_H
