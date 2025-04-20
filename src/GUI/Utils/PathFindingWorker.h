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
    PathFindingWorker();
    void initialize(MainWindow *window, int count);
public slots:
    void process();
signals:
    void resultReady(const QList<Backend::Path *> &paths);
    void error(const QString &message);
    void finished();

private:
    MainWindow *mainWindow;
    int         pathsCount;

    // Changed to collect terminals instead of adding one by
    // one
    bool collectTerminals(ConnectionLine        *connection,
                          QList<TerminalItem *> &terminals,
                          QSet<QString> &terminalIds);

    // New method to create Terminal objects from
    // TerminalItems
    Backend::Terminal *
    createTerminalObject(TerminalItem *terminal);

    // Method to process all connections and collect route
    // segments
    bool processConnections(
        const QList<ConnectionLine *> &connections,
        QList<Backend::PathSegment *> &routes,
        QSet<QString> &processedConnectionIds);
};
} // namespace GUI
} // namespace CargoNetSim
