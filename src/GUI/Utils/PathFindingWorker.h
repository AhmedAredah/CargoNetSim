#pragma once

#include "Backend/Controllers/CargoNetSimController.h"
#include "GUI/Items/ConnectionLine.h"
#include "GUI/MainWindow.h"
#include <QObject>
#include <QString>

namespace CargoNetSim
{
namespace GUI
{

class PathFindingWorker : public QObject
{
    Q_OBJECT
public:
    PathFindingWorker(MainWindow *window, int count);

public slots:
    void process();

signals:
    void resultReady(const QList<Backend::Path *> &paths);
    void error(const QString &message);
    void finished();

private:
    MainWindow *mainWindow;
    int         pathsCount;

    bool processConnectionAndTerminals(
        ConnectionLine *connection,
        CargoNetSim::Backend::TerminalSimulationClient
                      *terminalClient,
        QSet<QString> &addedTerminalIds,
        MainWindow    *mainWindow);

    bool addTerminalToServer(
        TerminalItem *terminal,
        CargoNetSim::Backend::TerminalSimulationClient
            *terminalClient);
};

} // namespace GUI
} // namespace CargoNetSim
