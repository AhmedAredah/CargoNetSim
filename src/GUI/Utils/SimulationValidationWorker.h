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

    void extractResults();

    int getContainerCount(MainWindow *mainWindow);

    double calculateEdgeCosts(
        Backend::Path                       *path,
        const QList<Backend::PathSegment *> &segments,
        const QVariantMap &costFunctionWeights,
        const QVariantMap &transportModes,
        Backend::ShipClient::ShipSimulationClient
            *shipClient,
        Backend::TrainClient::TrainSimulationClient
            *trainClient,
        Backend::TruckClient::TruckSimulationManager
            *truckClient,
        int  containerCount);

    double calculateShipSegmentCost(
        Backend::Path *path, Backend::PathSegment *segment,
        int segmentCounter,
        Backend::ShipClient::ShipSimulationClient
                          *shipClient,
        const QVariantMap &modeWeights,
        const QVariantMap &transportModes,
        int                containerCount);

    double calculateTrainSegmentCost(
        Backend::Path *path, Backend::PathSegment *segment,
        int segmentCounter,
        Backend::TrainClient::TrainSimulationClient
                          *trainClient,
        const QVariantMap &modeWeights,
        const QVariantMap &transportModes,
        int                containerCount);

    double calculateTruckSegmentCost(
        Backend::Path *path, Backend::PathSegment *segment,
        int segmentCounter,
        Backend::TruckClient::TruckSimulationManager
                          *truckClient,
        const QVariantMap &modeWeights,
        const QVariantMap &transportModes,
        int                containerCount);

    double calculateTerminalCosts(
        const QList<Backend::PathSegment *> &segments,
        const QList<Backend::Terminal *>    &terminals,
        const QVariantMap &costFunctionWeights,
        int                containerCount);

    double calculateSingleTerminalCost(
        Backend::Terminal *terminal,
        const QVariantMap &costFunctionWeights,
        int                containerCount);

    double
    calculateTerminalDwellTime(const QJsonObject &config);

    bool calculateTerminalCustoms(const QJsonObject &config,
                                  double &customsDelay,
                                  double &customsCost);

    double
    calculateTerminalDirectCosts(const QJsonObject &config,
                                 bool customsApplied);

    void setSegmentActualDetails(
        Backend::PathSegment        *segment,
        const QMap<QString, double> &details,
        const QString               &underlyingKey);
};

} // namespace GUI
} // namespace CargoNetSim
