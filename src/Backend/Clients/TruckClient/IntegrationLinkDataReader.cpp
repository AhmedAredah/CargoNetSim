/**
 * @file IntegrationLinkDataReader.cpp
 * @brief Implements reader for link data from file
 * @author [Your Name]
 * @date 2025-03-22
 */

 #include "IntegrationLinkDataReader.h"
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
 
 IntegrationLinkDataReader::IntegrationLinkDataReader(QObject *parent)
     : QObject(parent)
 {
 }
 
 QVector<IntegrationLink*> IntegrationLinkDataReader::readLinksFile(const QString &filename, QObject *parent) const
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
             throw std::runtime_error("Links file is empty");
         }
 
         // Parse scale information from second line
         QStringList scales = lines[1].split(QRegularExpression("\\s+"));
         if (scales.size() < 6)
         {
             throw std::runtime_error("Bad links file structure: invalid scale information");
         }
 
         bool convOk;
         float lengthScale = scales[1].toFloat(&convOk);
         if (!convOk) throw std::runtime_error("Invalid length scale value");
         
         float speedScale = scales[2].toFloat(&convOk);
         if (!convOk) throw std::runtime_error("Invalid speed scale value");
         
         float saturationFlowScale = scales[3].toFloat(&convOk);
         if (!convOk) throw std::runtime_error("Invalid saturation flow scale value");
         
         float speedAtCapacityScale = scales[4].toFloat(&convOk);
         if (!convOk) throw std::runtime_error("Invalid speed at capacity scale value");
         
         float jamDensityScale = scales[5].toFloat(&convOk);
         if (!convOk) throw std::runtime_error("Invalid jam density scale value");
 
         // Process link records starting from line 3
         QVector<IntegrationLink*> links;
         for (int i = 2; i < lines.size(); i++)
         {
             QStringList values = lines[i].split(QRegularExpression("\\s+"));
             if (values.size() < 20)
             {
                 // Ensure at least the required fields are present
                 continue;
             }
 
             // Extract description (which might contain spaces)
             QString description;
             if (values.size() > 20)
             {
                 description = values.mid(20).join(" ");
             }
 
             // Parse and validate all fields
             bool ok;
             int linkId = values[0].toInt(&ok);
             if (!ok) continue;
             
             int upstreamNodeId = values[1].toInt(&ok);
             if (!ok) continue;
             
             int downstreamNodeId = values[2].toInt(&ok);
             if (!ok) continue;
             
             float length = values[3].toFloat(&ok);
             if (!ok) continue;
             
             float freeSpeed = values[4].toFloat(&ok);
             if (!ok) continue;
             
             float saturationFlow = values[5].toFloat(&ok);
             if (!ok) continue;
             
             float lanes = values[6].toFloat(&ok);
             if (!ok) continue;
             
             float speedCoeffVariation = values[7].toFloat(&ok);
             if (!ok) continue;
             
             float speedAtCapacity = values[8].toFloat(&ok);
             if (!ok) continue;
             
             float jamDensity = values[9].toFloat(&ok);
             if (!ok) continue;
             
             int turnProhibition = values[10].toInt(&ok);
             if (!ok) continue;
             
             int prohibitionStart = values[11].toInt(&ok);
             if (!ok) continue;
             
             int prohibitionEnd = values[12].toInt(&ok);
             if (!ok) continue;
             
             int opposingLink1 = values[13].toInt(&ok);
             if (!ok) continue;
             
             int opposingLink2 = values[14].toInt(&ok);
             if (!ok) continue;
             
             int trafficSignal = values[15].toInt(&ok);
             if (!ok) continue;
             
             int phase1 = values[16].toInt(&ok);
             if (!ok) continue;
             
             int phase2 = values[17].toInt(&ok);
             if (!ok) continue;
             
             int vehicleClassProhibition = values[18].toInt(&ok);
             if (!ok) continue;
             
             int surveillanceLevel = values[19].toInt(&ok);
             if (!ok) continue;
 
             // Create link object
             IntegrationLink* link = new IntegrationLink(
                 linkId, 
                 upstreamNodeId,
                 downstreamNodeId, 
                 length, 
                 freeSpeed,
                 saturationFlow, 
                 lanes,
                 speedCoeffVariation, 
                 speedAtCapacity,
                 jamDensity, 
                 turnProhibition,
                 prohibitionStart, 
                 prohibitionEnd,
                 opposingLink1, 
                 opposingLink2,
                 trafficSignal, 
                 phase1, 
                 phase2,
                 vehicleClassProhibition, 
                 surveillanceLevel,
                 description, 
                 lengthScale,
                 speedScale, 
                 saturationFlowScale,
                 speedAtCapacityScale, 
                 jamDensityScale,
                 parent
             );
             
             links.append(link);
         }
 
         return links;
     }
     catch (const std::exception &e)
     {
         // Log error and rethrow
         qCritical() << "Error reading links file:" << e.what();
         throw;
     }
 }
 
 } // namespace TruckClient
 } // namespace Backend
 } // namespace CargoNetSim