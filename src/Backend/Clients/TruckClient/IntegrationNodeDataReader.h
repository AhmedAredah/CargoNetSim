/**
 * @file IntegrationNodeDataReader.h
 * @brief Defines reader for node data from file
 * @author [Your Name]
 * @date 2025-03-22
 */

 #pragma once

 #include <QObject>
 #include <QVector>
 #include <QString>
 #include "IntegrationNode.h"
 
 namespace CargoNetSim
 {
 namespace Backend
 {
 namespace TruckClient
 {
 
 /**
  * @class IntegrationNodeDataReader
  * @brief Reads and parses node data from file
  *
  * Handles reading and parsing of node data from formatted
  * text files for integration with transportation networks.
  */
 class IntegrationNodeDataReader : public QObject
 {
     Q_OBJECT
 
 public:
     /**
      * @brief Constructor
      * @param parent The parent QObject
      */
     explicit IntegrationNodeDataReader(QObject *parent = nullptr);
 
     /**
      * @brief Read node data from file
      * @param filename Path to the node file
      * @param parent Optional parent for created node objects
      * @return Vector of IntegrationNode pointers (caller takes ownership)
      * @throws std::runtime_error if the file cannot be read or is malformed
      */
     QVector<IntegrationNode*> readNodesFile(const QString &filename, QObject *parent = nullptr) const;
 };
 
 } // namespace TruckClient
 } // namespace Backend
 } // namespace CargoNetSim