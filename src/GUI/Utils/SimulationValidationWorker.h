#pragma once

#include "Backend/Controllers/CargoNetSimController.h"
#include "GUI/MainWindow.h"
#include <QObject>
#include <QString>

namespace CargoNetSim
{
namespace GUI
{

// Forward declarations
class TerminalItem;
class ConnectionLine;
class GlobalTerminalItem;

class SimulationValidationWorker : public QObject
{
    Q_OBJECT

public:
    SimulationValidationWorker();
    void initialize(MainWindow *window);

public slots:
    void process();

signals:
    void statusMessage(const QString &message);
    void errorMessage(const QString &message);
    void finished();

private:
    MainWindow *mainWindow;

    // Nested structs to hold simulation data
    struct ShipSimData
    {
        Backend::Ship                    *ship;
        QList<ContainerCore::Container *> containers;
        QString destinationTerminal;
    };

    struct TrainSimData
    {
        Backend::Train                   *train;
        QList<ContainerCore::Container *> containers;
    };

    struct TruckSimData
    {
        QString                           tripId;
        int                               originNode;
        int                               destinationNode;
        QList<ContainerCore::Container *> containers;
    };

    // Helper methods
    bool validateConfiguration();
    bool validateTerminals();
    bool processSelectedPaths();
    bool setupSimulationData(
        QMap<QString, QList<ShipSimData>>
            &shipSimulationData,
        QMap<QString, QList<TrainSimData>>
            &trainSimulationData,
        QMap<QString, QList<TruckSimData>>
            &truckSimulationData,
        QMap<QString,
             Backend::TrainClient::NeTrainSimNetwork *>
            &trainNetworks,
        QMap<QString,
             Backend::TruckClient::IntegrationNetwork *>
            &truckNetworks);
    bool runSimulations(
        const QMap<QString, QList<ShipSimData>>
            &shipSimulationData,
        const QMap<QString, QList<TrainSimData>>
            &trainSimulationData,
        const QMap<QString, QList<TruckSimData>>
                          &truckSimulationData,
        const QMap<QString,
                   Backend::TrainClient::NeTrainSimNetwork
                       *> &trainNetworks,
        const QMap<QString,
                   Backend::TruckClient::IntegrationNetwork
                       *> &truckNetworks);
};

} // namespace GUI
} // namespace CargoNetSim
