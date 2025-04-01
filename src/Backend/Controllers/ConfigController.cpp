#include "ConfigController.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>

namespace CargoNetSim
{
namespace Backend
{
ConfigController::ConfigController(
    const QString &configFile)
    : m_configFile(configFile)
{
    // If file doesn't exist, create it with default values
    QFile file(m_configFile);
    if (!file.exists())
    {
        createDefaultConfig();
        saveConfig();
    }
    else
    {
        loadConfig();
    }
}

bool ConfigController::loadConfig()
{
    try
    {
        QFile file(m_configFile);
        if (!file.open(QIODevice::ReadOnly
                       | QIODevice::Text))
        {
            qWarning() << "Could not open config file:"
                       << file.errorString();
            return false;
        }

        QDomDocument doc;
        QString      errorMsg;
        int          errorLine, errorColumn;

        if (!doc.setContent(&file, &errorMsg, &errorLine,
                            &errorColumn))
        {
            file.close();
            qWarning() << "Failed to parse XML:" << errorMsg
                       << " at line " << errorLine
                       << ", column " << errorColumn;
            return false;
        }
        file.close();

        // Get the root element
        QDomElement root = doc.documentElement();
        if (root.tagName() != "config")
        {
            qWarning() << "Invalid root element in config "
                          "file, expected 'config'";
            return false;
        }

        // Clear existing config
        m_config.clear();

        // Process each child element of the root
        QDomElement element = root.firstChildElement();
        while (!element.isNull())
        {
            QString tagName = element.tagName();

            if (tagName == "simulation"
                || tagName == "fuel_energy"
                || tagName == "fuel_carbon_content"
                || tagName == "fuel_prices"
                || tagName == "carbon_taxes")
            {

                m_config[tagName] =
                    parseXmlElement(element);
            }
            else if (tagName == "transport_modes")
            {
                QVariantMap transportModes;

                QDomElement modeElement =
                    element.firstChildElement();
                while (!modeElement.isNull())
                {
                    QString modeName =
                        modeElement.tagName();
                    transportModes[modeName] =
                        parseXmlElement(modeElement);
                    modeElement =
                        modeElement.nextSiblingElement();
                }

                m_config["transport_modes"] =
                    transportModes;
            }

            element = element.nextSiblingElement();
        }

        return true;
    }
    catch (const std::exception &e)
    {
        qWarning() << "An error occurred while loading the "
                      "config file:"
                   << e.what();
        return false;
    }
}

QVariantMap ConfigController::parseXmlElement(
    const QDomElement &element) const
{
    QVariantMap result;

    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        QString tagName     = child.tagName();
        QString textContent = child.text();

        // Convert to appropriate type
        if (textContent == "true" || textContent == "false")
        {
            result[tagName] = (textContent == "true");
        }
        else
        {
            bool   isDouble;
            double doubleValue =
                textContent.toDouble(&isDouble);

            if (isDouble)
            {
                result[tagName] = doubleValue;
            }
            else
            {
                // If it's not a number, keep it as a string
                result[tagName] = textContent;
            }
        }

        child = child.nextSiblingElement();
    }

    return result;
}

void ConfigController::createDefaultConfig()
{
    // Create a default configuration with the values from
    // the XML example
    QVariantMap simulation;
    simulation["time_step"]           = 15;
    simulation["time_value_of_money"] = 45;
    simulation["shortest_paths"]      = 3;
    m_config["simulation"]            = simulation;

    QVariantMap fuelEnergy;
    fuelEnergy["HFO"]       = 11.1;
    fuelEnergy["diesel_1"]  = 10.7;
    fuelEnergy["diesel_2"]  = 10.0;
    m_config["fuel_energy"] = fuelEnergy;

    QVariantMap fuelCarbonContent;
    fuelCarbonContent["HFO"]        = 3.15;
    fuelCarbonContent["diesel_1"]   = 2.68;
    fuelCarbonContent["diesel_2"]   = 2.68;
    m_config["fuel_carbon_content"] = fuelCarbonContent;

    QVariantMap fuelPrices;
    fuelPrices["HFO"]       = 580.0;
    fuelPrices["diesel_1"]  = 1.35;
    fuelPrices["diesel_2"]  = 1.35;
    m_config["fuel_prices"] = fuelPrices;

    QVariantMap carbonTaxes;
    carbonTaxes["rate"]             = 65;
    carbonTaxes["ship_multiplier"]  = 1.2;
    carbonTaxes["truck_multiplier"] = 1.1;
    carbonTaxes["train_multiplier"] = 1.1;
    m_config["carbon_taxes"]        = carbonTaxes;

    QVariantMap ship;
    ship["average_speed"]            = 20;
    ship["average_fuel_consumption"] = 50;
    ship["average_container_number"] = 5000;
    ship["risk_factor"]              = 0.025;
    ship["fuel_type"]                = "HFO";

    QVariantMap train;
    train["average_speed"]            = 40;
    train["average_fuel_consumption"] = 20;
    train["average_container_number"] = 400;
    train["risk_factor"]              = 0.006;
    train["use_network"]              = true;
    train["fuel_type"]                = "diesel_1";

    QVariantMap truck;
    truck["average_speed"]            = 70;
    truck["average_fuel_consumption"] = 15;
    truck["average_container_number"] = 1;
    truck["risk_factor"]              = 0.012;
    truck["use_network"]              = false;
    truck["fuel_type"]                = "diesel_2";

    QVariantMap transportModes;
    transportModes["ship"]      = ship;
    transportModes["train"]     = train;
    transportModes["truck"]     = truck;
    m_config["transport_modes"] = transportModes;
}

QVariantMap ConfigController::getAllParams() const
{
    return m_config;
}

QVariantMap ConfigController::getSimulationParams() const
{
    return m_config.value("simulation").toMap();
}

QVariantMap ConfigController::getFuelEnergy() const
{
    return m_config.value("fuel_energy").toMap();
}

QVariantMap ConfigController::getFuelCarbonContent() const
{
    return m_config.value("fuel_carbon_content").toMap();
}

QVariantMap ConfigController::getFuelPrices() const
{
    return m_config.value("fuel_prices").toMap();
}

QVariantMap ConfigController::getCarbonTaxes() const
{
    return m_config.value("carbon_taxes").toMap();
}

QVariantMap ConfigController::getTransportModes() const
{
    return m_config.value("transport_modes").toMap();
}

void ConfigController::updateConfig(
    const QVariantMap &newConfig)
{
    m_config = newConfig;
}

bool ConfigController::saveConfig()
{
    try
    {
        QDomDocument doc;
        QDomElement  root = doc.createElement("config");
        doc.appendChild(root);

        // Add a comment
        QDomComment comment = doc.createComment(
            "Configuration parameters for CargoNetSim");
        root.appendChild(comment);

        // Add simulation section
        QVariantMap simulation = getSimulationParams();
        variantMapToXmlElement(doc, root, simulation,
                               "simulation");

        // Add fuel_energy section
        QVariantMap fuelEnergy = getFuelEnergy();
        variantMapToXmlElement(doc, root, fuelEnergy,
                               "fuel_energy");

        // Add fuel_carbon_content section
        QVariantMap fuelCarbonContent =
            getFuelCarbonContent();
        variantMapToXmlElement(doc, root, fuelCarbonContent,
                               "fuel_carbon_content");

        // Add fuel_prices section
        QVariantMap fuelPrices = getFuelPrices();
        variantMapToXmlElement(doc, root, fuelPrices,
                               "fuel_prices");

        // Add carbon_taxes section
        QVariantMap carbonTaxes = getCarbonTaxes();
        variantMapToXmlElement(doc, root, carbonTaxes,
                               "carbon_taxes");

        // Add transport_modes section
        QVariantMap transportModes = getTransportModes();
        QDomElement transportModesElement =
            doc.createElement("transport_modes");
        root.appendChild(transportModesElement);

        // Add ship section
        QVariantMap ship =
            transportModes.value("ship").toMap();
        variantMapToXmlElement(doc, transportModesElement,
                               ship, "ship");

        // Add train section
        QVariantMap train =
            transportModes.value("train").toMap();
        variantMapToXmlElement(doc, transportModesElement,
                               train, "train");

        // Add truck section
        QVariantMap truck =
            transportModes.value("truck").toMap();
        variantMapToXmlElement(doc, transportModesElement,
                               truck, "truck");

        // Write to file
        QFile file(m_configFile);
        if (!file.open(QIODevice::WriteOnly
                       | QIODevice::Text))
        {
            qWarning() << "Could not open file for writing:"
                       << file.errorString();
            return false;
        }

        QTextStream stream(&file);
        stream << doc.toString(4); // 4-space indentation
        file.close();

        return true;
    }
    catch (const std::exception &e)
    {
        qWarning() << "Error saving config:" << e.what();
        return false;
    }
}

void ConfigController::variantMapToXmlElement(
    QDomDocument &doc, QDomElement &parentElement,
    const QVariantMap &map,
    const QString     &sectionName) const
{
    QDomElement sectionElement =
        doc.createElement(sectionName);
    parentElement.appendChild(sectionElement);

    for (auto it = map.constBegin(); it != map.constEnd();
         ++it)
    {
        QDomElement element = doc.createElement(it.key());
        QDomText    text;

        switch (it.value().type())
        {
        case QVariant::Bool:
            text = doc.createTextNode(
                it.value().toBool() ? "true" : "false");
            break;

        case QVariant::Int:
            text = doc.createTextNode(
                QString::number(it.value().toInt()));
            break;

        case QVariant::Double:
            // Use fixed-point notation for doubles with
            // precision of 6
            text = doc.createTextNode(QString::number(
                it.value().toDouble(), 'f', 6));
            break;

        default:
            text =
                doc.createTextNode(it.value().toString());
            break;
        }

        element.appendChild(text);
        sectionElement.appendChild(element);
    }
}

} // namespace Backend
} // namespace CargoNetSim
