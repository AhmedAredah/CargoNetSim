/**
 * @file IntegrationNodeDataReader.cpp
 * @brief Implements reader for node data from file
 * @author [Your Name]
 * @date 2025-03-22
 */

 #include "IntegrationNodeDataReader.h"
 #include <QFile>
 #include <QTextStream>
 #include <QRegularExpression>
 #include <stdexcept>
 
 namespace CargoNetSim
 {
 namespace Backend
 {
 namespace TruckClient
 {
 
 IntegrationNodeDataReader::IntegrationNodeDataReader(QObject *parent)
     : QObject(parent)
 {
 }
 
 QVector<IntegrationNode*> IntegrationNodeDataReader::readNodesFile(const QString &filename, QObject *parent) const
 {
     try
     {
         QFile file(filename);
         if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
         {
             throw std::runtime_error(QString("Cannot open file: %1").arg(filename).toStdString());
         }
 
         QTextStream stream(&file);
         QStringList lines;
         
         // Read all lines and filter out control characters
         while (!stream.atEnd())
         {
             QString line = stream.readLine().trimmed();
             // Remove control characters (like 0x1A)
             line.remove(QRegularExpression("[\\x00-\\x1F\\x7F]"));
             if (!line.isEmpty())
             {
                 lines.append(line);
             }
         }
         
         file.close();
 
         if (lines.isEmpty())
         {
             throw std::runtime_error("Nodes file is empty");
         }
 
         // Parse scale information from second line
         QStringList scales = lines[1].split(QRegularExpression("\\s+"));
         if (scales.size() < 3)
         {
             throw std::runtime_error("Bad nodes file structure: invalid scale information");
         }
 
         bool convOk;
         float scaleX = scales[1].toFloat(&convOk);
         if (!convOk)
         {
             throw std::runtime_error("Invalid X scale value");
         }
         
         float scaleY = scales[2].toFloat(&convOk);
         if (!convOk)
         {
             throw std::runtime_error("Invalid Y scale value");
         }
 
         // Process node records starting from line 3
         QVector<IntegrationNode*> nodes;
         for (int i = 2; i < lines.size(); i++)
         {
             QStringList values = lines[i].split(QRegularExpression("\\s+"));
             if (values.size() < 6)
             {
                 // Ensure at least the required fields are present
                 continue;
             }
 
             // Extract description (which might contain spaces)
             QString description;
             if (values.size() > 6)
             {
                 description = values.mid(6).join(" ");
             }
 
             bool ok;
             int nodeId = values[0].toInt(&ok);
             if (!ok) continue;
             
             float xCoord = values[1].toFloat(&ok);
             if (!ok) continue;
             
             float yCoord = values[2].toFloat(&ok);
             if (!ok) continue;
             
             int nodeType = values[3].toInt(&ok);
             if (!ok) continue;
             
             int macroZoneCluster = values[4].toInt(&ok);
             if (!ok) continue;
             
             int infoAvailability = values[5].toInt(&ok);
             if (!ok) continue;
 
             // Create a node object
             IntegrationNode* node = new IntegrationNode(
                 nodeId,
                 xCoord,
                 yCoord,
                 nodeType,
                 macroZoneCluster,
                 infoAvailability,
                 description,
                 scaleX,
                 scaleY,
                 parent
             );
             
             nodes.append(node);
         }
 
         return nodes;
     }
     catch (const std::exception &e)
     {
         // Clean up any created nodes before rethrowing
         qCritical() << "Error reading nodes file:" << e.what();
         throw;
     }
 }
 
 } // namespace TruckClient
 } // namespace Backend
 } // namespace CargoNetSim