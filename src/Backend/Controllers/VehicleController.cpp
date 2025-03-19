#include "VehicleController.h"


namespace CargoNetSim {
namespace Backend {

// Initialize static members
VehicleController* VehicleController::m_instance = nullptr;
QMutex VehicleController::m_mutex;

VehicleController* VehicleController::getInstance() {
    if (m_instance == nullptr) {
        QMutexLocker locker(&m_mutex);
        if (m_instance == nullptr) {
            m_instance = new VehicleController();
        }
    }
    return m_instance;
}

void VehicleController::cleanup() {
    QMutexLocker locker(&m_mutex);
    if (m_instance != nullptr) {
        delete m_instance;
        m_instance = nullptr;
    }
}

VehicleController::VehicleController(QObject* parent)
    : QObject(parent) {
}

VehicleController::~VehicleController() {
    clear();
}

// Ship Management Methods
bool VehicleController::loadShipsFromFile(const QString& filePath) {
    // Clear existing ships
    qDeleteAll(m_ships);
    m_ships.clear();
    
    // Load ships from file
    QVector<Ship*> loadedShips = ShipsReader::readShipsFile(filePath, this);
    if (loadedShips.isEmpty()) {
        return false;
    }
    
    // Add ships to map
    for (Ship* ship : loadedShips) {
        m_ships[ship->getUserId()] = ship;
        connect(ship, &Ship::shipChanged, this, [this, ship]() {
            emit shipUpdated(ship);
        });
    }
    
    emit shipsLoaded(loadedShips.size());
    return true;
}

Ship* VehicleController::getShip(const QString& shipId) const {
    return m_ships.value(shipId, nullptr);
}

QVector<Ship*> VehicleController::getAllShips() const {
    return m_ships.values().toVector();
}

bool VehicleController::addShip(Ship* ship) {
    if (!ship || m_ships.contains(ship->getUserId())) {
        return false;
    }
    
    // Take ownership of the ship
    ship->setParent(this);
    
    // Connect to signals
    connect(ship, &Ship::shipChanged, this, [this, ship]() {
        emit shipUpdated(ship);
    });
    
    // Add to map
    m_ships[ship->getUserId()] = ship;
    
    emit shipAdded(ship);
    return true;
}

bool VehicleController::removeShip(const QString& shipId) {
    if (!m_ships.contains(shipId)) {
        return false;
    }
    
    Ship* ship = m_ships.take(shipId);
    emit shipRemoved(shipId);
    
    delete ship;
    return true;
}

bool VehicleController::updateShip(Ship* ship) {
    if (!ship || !m_ships.contains(ship->getUserId())) {
        return false;
    }
    
    // Remove old ship
    Ship* oldShip = m_ships[ship->getUserId()];
    if (oldShip != ship) {
        delete oldShip;
    }
    
    // Add updated ship
    ship->setParent(this);
    m_ships[ship->getUserId()] = ship;
    
    // Connect to signals
    connect(ship, &Ship::shipChanged, this, [this, ship]() {
        emit shipUpdated(ship);
    }, Qt::UniqueConnection);
    
    emit shipUpdated(ship);
    return true;
}

bool VehicleController::updateShips(QVector<Ship*> ships) {
    // Create a set of new ship IDs for quick lookup
    QSet<QString> newShipIds;
    for (Ship* ship : ships) {
        newShipIds.insert(ship->getUserId());
    }

    // Find ships to remove (those in current map but not in new set)
    QVector<QString> shipsToRemove;
    for (auto it = m_ships.begin(); it != m_ships.end(); ++it) {
        if (!newShipIds.contains(it.key())) {
            shipsToRemove.append(it.key());
        }
    }

    // Remove ships that are no longer present
    for (const QString& shipId : shipsToRemove) {
        removeShip(shipId);
    }

    // Update or add new ships
    bool success = true;
    for (Ship* ship : ships) {
        if (m_ships.contains(ship->getUserId())) {
            success &= updateShip(ship);
        } else {
            success &= addShip(ship);
        }
    }

    return success;
}

int VehicleController::shipCount() const {
    return m_ships.size();
}

// Train Management Methods
bool VehicleController::loadTrainsFromFile(const QString& filePath) {
    // Clear existing trains
    qDeleteAll(m_trains);
    m_trains.clear();
    
    // Load trains from file
    QVector<Train*> loadedTrains = TrainsReader::readTrainsFile(filePath, this);
    if (loadedTrains.isEmpty()) {
        return false;
    }
    
    // Add trains to map
    for (Train* train : loadedTrains) {
        m_trains[train->getUserId()] = train;
        connect(train, &Train::trainChanged, this, [this, train]() {
            emit trainUpdated(train);
        });
    }
    
    emit trainsLoaded(loadedTrains.size());
    return true;
}

Train* VehicleController::getTrain(const QString& userId) const {
    return m_trains.value(userId, nullptr);
}

QVector<Train*> VehicleController::getAllTrains() const {
    return m_trains.values().toVector();
}

bool VehicleController::addTrain(Train* train) {
    if (!train || m_trains.contains(train->getUserId())) {
        return false;
    }
    
    // Take ownership of the train
    train->setParent(this);
    
    // Connect to signals
    connect(train, &Train::trainChanged, this, [this, train]() {
        emit trainUpdated(train);
    });
    
    // Add to map
    m_trains[train->getUserId()] = train;
    
    emit trainAdded(train);
    return true;
}

bool VehicleController::removeTrain(const QString& userId) {
    if (!m_trains.contains(userId)) {
        return false;
    }
    
    Train* train = m_trains.take(userId);
    emit trainRemoved(userId);
    
    delete train;
    return true;
}

bool VehicleController::updateTrain(Train* train) {
    if (!train || !m_trains.contains(train->getUserId())) {
        return false;
    }
    
    // Remove old train
    Train* oldTrain = m_trains[train->getUserId()];
    if (oldTrain != train) {
        delete oldTrain;
    }
    
    // Add updated train
    train->setParent(this);
    m_trains[train->getUserId()] = train;
    
    // Connect to signals
    connect(train, &Train::trainChanged, this, [this, train]() {
        emit trainUpdated(train);
    }, Qt::UniqueConnection);
    
    emit trainUpdated(train);
    return true;
}

bool VehicleController::updateTrains(QVector<Train*> trains) {
    // Create a set of new train IDs for quick lookup
    QSet<QString> newTrainIds;
    for (Train* train : trains) {
        newTrainIds.insert(train->getUserId());
    }

    // Find trains to remove (those in current map but not in new set)
    QVector<QString> trainsToRemove;
    for (auto it = m_trains.begin(); it != m_trains.end(); ++it) {
        if (!newTrainIds.contains(it.key())) {
            trainsToRemove.append(it.key());
        }
    }

    // Remove trains that are no longer present
    for (const QString& trainId : trainsToRemove) {
        removeTrain(trainId);
    }

    // Update or add new trains
    bool success = true;
    for (Train* train : trains) {
        if (m_trains.contains(train->getUserId())) {
            success &= updateTrain(train);
        } else {
            success &= addTrain(train);
        }
    }

    return success;
}

int VehicleController::trainCount() const {
    return m_trains.size();
}

// General operations
void VehicleController::clear() {
    // Clear ships
    qDeleteAll(m_ships);
    m_ships.clear();
    emit shipsCleared();
    
    // Clear trains
    qDeleteAll(m_trains);
    m_trains.clear();
    emit trainsCleared();
}

}// namespace Backend
}// namespace CargoNetSim
