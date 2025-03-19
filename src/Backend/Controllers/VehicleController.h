#pragma once

#include <QObject>
#include <QMap>
#include <QVector>
#include <QMutex>
#include "../Models/ShipSystem.h"
#include "../Models/TrainSystem.h"

namespace CargoNetSim {
namespace Backend {
class VehicleController : public QObject {
    Q_OBJECT

public:
    // Get the singleton instance
    static VehicleController* getInstance();
    
    /**
     * @brief Clean up the singleton instance
     *
     * This method should be called before application exit to properly
     * destroy the singleton instance and free all associated resources.
     */
    static void cleanup();
    
    // Ship Management
    bool loadShipsFromFile(const QString& filePath);
    Ship* getShip(const QString& shipId) const;
    QVector<Ship*> getAllShips() const;
    bool addShip(Ship* ship);
    bool removeShip(const QString& shipId);
    bool updateShip(Ship* ship);
    bool updateShips(QVector<Ship*> ships);
    int shipCount() const;
    
    // Train Management
    bool loadTrainsFromFile(const QString& filePath);
    Train* getTrain(const QString& userId) const;
    QVector<Train*> getAllTrains() const;
    bool addTrain(Train* train);
    bool removeTrain(const QString& userId);
    bool updateTrain(Train* train);
    bool updateTrains(QVector<Train*> trains);
    int trainCount() const;
    
    // General operations
    void clear();


    
signals:
    void shipAdded(Ship* ship);
    void shipRemoved(const QString& shipId);
    void shipUpdated(Ship* ship);
    void shipsLoaded(int count);
    void shipsCleared();
    
    void trainAdded(Train* train);
    void trainRemoved(const QString& userId);
    void trainUpdated(Train* train);
    void trainsLoaded(int count);
    void trainsCleared();

private:
    // Private constructor and destructor for singleton
    explicit VehicleController(QObject* parent = nullptr);
    ~VehicleController();
    
    // Disable copy constructor and assignment operator
    VehicleController(const VehicleController&) = delete;
    VehicleController& operator=(const VehicleController&) = delete;
    
    // The singleton instance
    static VehicleController* m_instance;
    static QMutex m_mutex;
    
    // Data members
    QMap<QString, Ship*> m_ships;
    QMap<QString, Train*> m_trains;
};

} // namespace Backend
} // namespace CargoNetSim
