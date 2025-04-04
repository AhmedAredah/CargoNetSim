/**
 * @file IntegrationLinkDataReader.h
 * @brief Defines reader for link data from file
 * @author [Your Name]
 * @date 2025-03-22
 */

 #pragma once

 #include <QObject>
 #include <QVector>
 #include <QString>
 #include "IntegrationLink.h"
 
 namespace CargoNetSim
 {
 namespace Backend
 {
 namespace TruckClient
 {
 
 /**
  * @class IntegrationLinkDataReader
  * @brief Reads and parses link data from file
  *
  * Handles reading and parsing of link data from formatted
  * text files for integration with transportation networks.
  */
 class IntegrationLinkDataReader : public QObject
 {
     Q_OBJECT
 
 public:
     /**
      * @brief Constructor
      * @param parent The parent QObject
      */
     explicit IntegrationLinkDataReader(QObject *parent = nullptr);
 
     /**
      * @brief Read link data from file
      * @param filename Path to the link file
      * @param parent Optional parent for created link objects
      * @return Vector of IntegrationLink pointers (caller takes ownership)
      * @throws std::runtime_error if the file cannot be read or is malformed
      */
     QVector<IntegrationLink*> readLinksFile(const QString &filename, QObject *parent = nullptr) const;
 };
 
 } // namespace TruckClient
 } // namespace Backend
 } // namespace CargoNetSim