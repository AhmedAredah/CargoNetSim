#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

namespace CargoNetSim {
namespace Backend {

class Locomotive : public QObject {
    Q_OBJECT

public:
    Locomotive(QObject* parent = nullptr);
    Locomotive(
        float power,
        float transmissionEff,
        float length,
        float airDragCoeff,
        float frontalArea,
        float grossWeight,
        int noOfAxles,
        int locoType,
        int count,
        QObject* parent = nullptr
        );
    
    // Constructor from JSON
    Locomotive(const QJsonObject& json, QObject* parent = nullptr);
    
    QJsonObject toJson() const;
    Locomotive* copy() const;

    // Getters
    float power() const { return m_power; }
    float transmissionEff() const { return m_transmissionEff; }
    float length() const { return m_length; }
    float airDragCoeff() const { return m_airDragCoeff; }
    float frontalArea() const { return m_frontalArea; }
    float grossWeight() const { return m_grossWeight; }
    int noOfAxles() const { return m_noOfAxles; }
    int locoType() const { return m_locoType; }
    int count() const { return m_count; }

    // Setters
    void setPower(float power);
    void setTransmissionEff(float transmissionEff);
    void setLength(float length);
    void setAirDragCoeff(float airDragCoeff);
    void setFrontalArea(float frontalArea);
    void setGrossWeight(float grossWeight);
    void setNoOfAxles(int noOfAxles);
    void setLocoType(int locoType);
    void setCount(int count);

signals:
    void locomotiveChanged();

private:
    float m_power;
    float m_transmissionEff;
    float m_length;
    float m_airDragCoeff;
    float m_frontalArea;
    float m_grossWeight;
    int m_noOfAxles;
    int m_locoType;
    int m_count;
};

class Car : public QObject {
    Q_OBJECT

public:
    Car(QObject* parent = nullptr);
    Car(
        float length,
        float airDragCoeff,
        float frontalArea,
        float tareWeight,
        float grossWeight,
        int noOfAxles,
        int carType,
        int count,
        QObject* parent = nullptr
        );
    
    // Constructor from JSON
    Car(const QJsonObject& json, QObject* parent = nullptr);
    
    QJsonObject toJson() const;
    Car* copy() const;

    // Getters
    float length() const { return m_length; }
    float airDragCoeff() const { return m_airDragCoeff; }
    float frontalArea() const { return m_frontalArea; }
    float tareWeight() const { return m_tareWeight; }
    float grossWeight() const { return m_grossWeight; }
    int noOfAxles() const { return m_noOfAxles; }
    int carType() const { return m_carType; }
    int count() const { return m_count; }

    // Setters
    void setLength(float length);
    void setAirDragCoeff(float airDragCoeff);
    void setFrontalArea(float frontalArea);
    void setTareWeight(float tareWeight);
    void setGrossWeight(float grossWeight);
    void setNoOfAxles(int noOfAxles);
    void setCarType(int carType);
    void setCount(int count);

signals:
    void carChanged();

private:
    float m_length;
    float m_airDragCoeff;
    float m_frontalArea;
    float m_tareWeight;
    float m_grossWeight;
    int m_noOfAxles;
    int m_carType;
    int m_count;
};

class Train : public QObject {
    Q_OBJECT

public:
    Train(QObject* parent = nullptr);
    Train(
        const QString& userId,
        const QVector<int>& trainPathOnNodeIds,
        float loadTime,
        float frictionCoef,
        const QVector<Locomotive*>& locomotives,
        const QVector<Car*>& cars,
        bool optimize = false,
        QObject* parent = nullptr
        );
    
    // Constructor from JSON
    Train(const QJsonObject& json, QObject* parent = nullptr);
    
    ~Train();
    
    QJsonObject toJson() const;
    Train* copy() const;

    // Getters
    QString userId() const { return m_userId; }
    QVector<int> trainPathOnNodeIds() const { return m_trainPathOnNodeIds; }
    float loadTime() const { return m_loadTime; }
    float frictionCoef() const { return m_frictionCoef; }
    QVector<Locomotive*> locomotives() const { return m_locomotives; }
    QVector<Car*> cars() const { return m_cars; }
    bool optimize() const { return m_optimize; }

    // Setters
    void setUserId(const QString& userId);
    void setTrainPathOnNodeIds(const QVector<int>& trainPathOnNodeIds);
    void setLoadTime(float loadTime);
    void setFrictionCoef(float frictionCoef);
    void setLocomotives(const QVector<Locomotive*>& locomotives);
    void setCars(const QVector<Car*>& cars);
    void setOptimize(bool optimize);

signals:
    void trainChanged();
    void locomotivesChanged();
    void carsChanged();

private:
    QString m_userId;
    QVector<int> m_trainPathOnNodeIds;
    float m_loadTime;
    float m_frictionCoef;
    QVector<Locomotive*> m_locomotives;
    QVector<Car*> m_cars;
    bool m_optimize;
};

class TrainsReader : public QObject {
    Q_OBJECT

public:
    static QVector<Train*> readTrainsFile(const QString& filePath,
                                           QObject* parent = nullptr);

private:
    static QVector<Locomotive*> parseLocomotives(const QString& locomotivesStr,
                                                  QObject* parent);
    static QVector<Car*> parseCars(const QString& carsStr, QObject* parent);
    static QVector<int> splitStringToIntList(const QString& string);
};

} // namespace Backend
} // namespace CargoNetSim
