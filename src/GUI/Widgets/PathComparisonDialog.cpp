/**
 * @file PathComparisonDialog.cpp
 * @brief Implementation of the PathComparisonDialog for
 * path comparison
 * @author Ahmed Aredah
 * @date April 18, 2025
 *
 * This file contains the implementation of the
 * PathComparisonDialog class, which provides a UI component
 * for displaying and comparing multiple paths side-by-side
 * in the CargoNetSim system.
 */

#include "PathComparisonDialog.h"
#include "GUI/Utils/IconCreator.h" // For icon utilities
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QPainter>
#include <QTextStream>

namespace CargoNetSim
{
namespace GUI
{

PathComparisonDialog::PathComparisonDialog(
    const QList<const ShortestPathsTable::PathData *>
            &pathData,
    QWidget *parent)
    : QDialog(parent)
    , m_pathData(pathData)
    , m_tabWidget(nullptr)
    , m_exportButton(nullptr)
{
    // Set dialog properties
    setWindowTitle(tr("Path Comparison"));
    setMinimumSize(800, 600);

    // Initialize the UI
    initUI();
}

PathComparisonDialog::~PathComparisonDialog()
{
    // Qt automatically cleans up child widgets
}

void PathComparisonDialog::initUI()
{
    // Create main layout
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Create tab widget for organizing comparison views
    m_tabWidget = new QTabWidget(this);

    // Add tabs for different comparison aspects
    m_tabWidget->addTab(createSummaryTab(), tr("Summary"));
    m_tabWidget->addTab(createTerminalsTab(),
                        tr("Terminals"));
    m_tabWidget->addTab(createSegmentsTab(),
                        tr("Segments"));
    m_tabWidget->addTab(createCostsTab(), tr("Costs"));

    mainLayout->addWidget(m_tabWidget);

    // Create button panel
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    // Add export button
    m_exportButton =
        new QPushButton(tr("Export Comparison"), this);
    m_exportButton->setToolTip(
        tr("Export comparison data to CSV"));
    connect(m_exportButton, &QPushButton::clicked, this,
            &PathComparisonDialog::onExportButtonClicked);

    // Add close button
    auto closeButton = new QPushButton(tr("Close"), this);
    connect(closeButton, &QPushButton::clicked, this,
            &QDialog::accept);

    buttonLayout->addWidget(m_exportButton);
    buttonLayout->addWidget(closeButton);

    mainLayout->addLayout(buttonLayout);
}

QWidget *PathComparisonDialog::createSummaryTab()
{
    // Create container widget
    auto container = new QWidget(this);
    auto layout    = new QVBoxLayout(container);

    // Add a header
    auto headerLabel = new QLabel(
        tr("<h2>Path Comparison Summary</h2>"), this);
    headerLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(headerLabel);

    // Create column headers (Path ID 1, Path ID 2, etc.)
    QStringList headers;
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            headers << tr("Path %1").arg(
                path->path->getPathId());
        }
        else
        {
            headers << tr("Unknown Path");
        }
    }

    // Create row labels for the summary data
    QStringList rowLabels = {
        tr("Path ID"),        tr("Total Terminals"),
        tr("Total Segments"), tr("Predicted Cost"),
        tr("Actual Cost"),    tr("Start Terminal"),
        tr("End Terminal")};

    // Populate data for each path
    QList<QStringList> data;
    for (const auto &path : m_pathData)
    {
        QStringList pathData;

        if (path && path->path)
        {
            // Path ID
            pathData << QString::number(
                path->path->getPathId());

            // Total terminals
            pathData << QString::number(
                path->path->getTerminalsInPath().size());

            // Total segments
            pathData << QString::number(
                path->path->getSegments().size());

            // Predicted cost
            pathData << QString::number(
                path->path->getTotalPathCost(), 'f', 2);

            // Actual cost
            if (path->m_totalSimulationPathCost >= 0)
            {
                pathData << QString::number(
                    path->m_totalSimulationPathCost, 'f',
                    2);
            }
            else
            {
                pathData << tr("Not simulated");
            }

            // Start terminal
            QString startTerminal;
            try
            {
                startTerminal =
                    path->path->getStartTerminal();
            }
            catch (const std::exception &)
            {
                startTerminal = tr("Unknown");
            }
            pathData << startTerminal;

            // End terminal
            QString endTerminal;
            try
            {
                endTerminal = path->path->getEndTerminal();
            }
            catch (const std::exception &)
            {
                endTerminal = tr("Unknown");
            }
            pathData << endTerminal;
        }
        else
        {
            // Fill with placeholders if path data is
            // invalid
            for (int i = 0; i < rowLabels.size(); ++i)
            {
                pathData << tr("N/A");
            }
        }

        data.append(pathData);
    }

    // Transpose data for display (paths as columns)
    QList<QStringList> transposedData;
    for (int rowIdx = 0; rowIdx < rowLabels.size();
         ++rowIdx)
    {
        QStringList rowData;
        for (const auto &pathData : data)
        {
            if (rowIdx < pathData.size())
            {
                rowData << pathData[rowIdx];
            }
            else
            {
                rowData << tr("N/A");
            }
        }
        transposedData.append(rowData);
    }

    // Create and add table
    auto table = createComparisonTable(headers, rowLabels,
                                       transposedData);
    layout->addWidget(table);

    // Create path visualization
    auto visualizationLabel =
        new QLabel(tr("<h3>Path Visualization</h3>"), this);
    visualizationLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(visualizationLabel);

    auto visualizationContainer = new QWidget(this);
    createPathVisualization(visualizationContainer);

    auto scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(visualizationContainer);

    layout->addWidget(scrollArea);

    return container;
}

QWidget *PathComparisonDialog::createTerminalsTab()
{
    // Create container widget
    auto container = new QWidget(this);
    auto layout    = new QVBoxLayout(container);

    // Add a header
    auto headerLabel = new QLabel(
        tr("<h2>Terminal Comparison</h2>"), this);
    headerLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(headerLabel);

    // Create column headers (Path ID 1, Path ID 2, etc.)
    QStringList headers;
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            headers << tr("Path %1").arg(
                path->path->getPathId());
        }
        else
        {
            headers << tr("Unknown Path");
        }
    }

    // Find the maximum number of terminals across all paths
    int maxTerminals = 0;
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            maxTerminals = qMax(
                maxTerminals,
                path->path->getTerminalsInPath().size());
        }
    }

    // Create row labels for terminal indices
    QStringList rowLabels;
    for (int i = 0; i < maxTerminals; ++i)
    {
        rowLabels << tr("Terminal %1").arg(i + 1);
    }

    // Populate terminal data for each path
    QList<QStringList> data;
    for (const auto &path : m_pathData)
    {
        QStringList terminalData;

        if (path && path->path)
        {
            const auto &terminals =
                path->path->getTerminalsInPath();

            // Add terminal information
            for (int i = 0; i < maxTerminals; ++i)
            {
                if (i < terminals.size() && terminals[i])
                {
                    terminalData
                        << terminals[i]->getDisplayName();
                }
                else
                {
                    terminalData << tr("-");
                }
            }
        }
        else
        {
            // Fill with placeholders if path data is
            // invalid
            for (int i = 0; i < maxTerminals; ++i)
            {
                terminalData << tr("N/A");
            }
        }

        data.append(terminalData);
    }

    // Transpose data for display (paths as columns)
    QList<QStringList> transposedData;
    for (int rowIdx = 0; rowIdx < maxTerminals; ++rowIdx)
    {
        QStringList rowData;
        for (const auto &terminalData : data)
        {
            if (rowIdx < terminalData.size())
            {
                rowData << terminalData[rowIdx];
            }
            else
            {
                rowData << tr("-");
            }
        }
        transposedData.append(rowData);
    }

    // Create and add table
    auto table = createComparisonTable(headers, rowLabels,
                                       transposedData);
    layout->addWidget(table);

    return container;
}

QWidget *PathComparisonDialog::createSegmentsTab()
{
    // Create container widget
    auto container = new QWidget(this);
    auto layout    = new QVBoxLayout(container);

    // Add a header
    auto headerLabel =
        new QLabel(tr("<h2>Segment Comparison</h2>"), this);
    headerLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(headerLabel);

    // Create column headers (Path ID 1, Path ID 2, etc.)
    QStringList headers;
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            headers << tr("Path %1").arg(
                path->path->getPathId());
        }
        else
        {
            headers << tr("Unknown Path");
        }
    }

    // Find the maximum number of segments across all paths
    int maxSegments = 0;
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            maxSegments =
                qMax(maxSegments,
                     path->path->getSegments().size());
        }
    }

    // Create tab container for segments with attributes
    auto segmentTabWidget = new QTabWidget(this);

    // Create basic segment info tab
    auto basicInfoWidget = new QWidget(this);
    auto basicInfoLayout = new QVBoxLayout(basicInfoWidget);

    // Create row labels for segment indices
    QStringList rowLabels;
    for (int i = 0; i < maxSegments; ++i)
    {
        rowLabels << tr("Segment %1").arg(i + 1);
    }

    // Populate segment data for each path
    QList<QStringList> data;
    for (const auto &path : m_pathData)
    {
        QStringList segmentData;

        if (path && path->path)
        {
            const auto &segments =
                path->path->getSegments();

            // Add segment information
            for (int i = 0; i < maxSegments; ++i)
            {
                if (i < segments.size() && segments[i])
                {
                    // Format: "Start → End (Mode)"
                    QString segmentInfo =
                        QString("%1 → %2 (%3)")
                            .arg(segments[i]->getStart())
                            .arg(segments[i]->getEnd())
                            .arg(
                                Backend::TransportationTypes::
                                    toString(
                                        segments[i]
                                            ->getMode()));
                    segmentData << segmentInfo;
                }
                else
                {
                    segmentData << tr("-");
                }
            }
        }
        else
        {
            // Fill with placeholders if path data is
            // invalid
            for (int i = 0; i < maxSegments; ++i)
            {
                segmentData << tr("N/A");
            }
        }

        data.append(segmentData);
    }

    // Transpose data for display (paths as columns)
    QList<QStringList> transposedData;
    for (int rowIdx = 0; rowIdx < maxSegments; ++rowIdx)
    {
        QStringList rowData;
        for (const auto &segmentData : data)
        {
            if (rowIdx < segmentData.size())
            {
                rowData << segmentData[rowIdx];
            }
            else
            {
                rowData << tr("-");
            }
        }
        transposedData.append(rowData);
    }

    // Create and add table
    auto table = createComparisonTable(headers, rowLabels,
                                       transposedData);
    basicInfoLayout->addWidget(table);

    segmentTabWidget->addTab(basicInfoWidget,
                             tr("Basic Info"));

    // Create attribute tabs for each segment
    for (int segmentIdx = 0; segmentIdx < maxSegments;
         ++segmentIdx)
    {
        auto attributeWidget = new QWidget(this);
        auto attributeLayout =
            new QVBoxLayout(attributeWidget);

        // Create attribute headers
        QStringList attributeHeaders = headers;

        // Create row labels for attributes
        QStringList attributeRowLabels = {
            tr("Carbon Emissions (Predicted)"),
            tr("Carbon Emissions (Actual)"),
            tr("Cost (Predicted)"),
            tr("Cost (Actual)"),
            tr("Distance (Predicted)"),
            tr("Distance (Actual)"),
            tr("Energy Consumption (Predicted)"),
            tr("Energy Consumption (Actual)"),
            tr("Risk (Predicted)"),
            tr("Risk (Actual)"),
            tr("Travel Time (Predicted)"),
            tr("Travel Time (Actual)")};

        // Populate data for each attribute
        QList<QStringList> attributeData;

        for (const auto &path : m_pathData)
        {
            QStringList pathAttributeData;

            if (path && path->path)
            {
                const auto &segments =
                    path->path->getSegments();

                if (segmentIdx < segments.size()
                    && segments[segmentIdx])
                {
                    const QJsonObject &attributes =
                        segments[segmentIdx]
                            ->getAttributes();

                    // Extract estimated_values
                    QJsonObject estimatedValuesObj;
                    if (attributes.contains(
                            "estimated_values")
                        && attributes["estimated_values"]
                               .isObject())
                    {
                        estimatedValuesObj =
                            attributes["estimated_values"]
                                .toObject();
                    }

                    // Extract actual_values values
                    QJsonObject actualValuesObj;
                    if (attributes.contains("actual_values")
                        && attributes["actual_values"]
                               .isObject())
                    {
                        actualValuesObj =
                            attributes["actual_values"]
                                .toObject();
                    }

                    // Carbon Emissions
                    pathAttributeData
                        << (estimatedValuesObj.contains(
                                "carbonEmissions")
                                ? QString::number(
                                      estimatedValuesObj
                                          ["carbonEmission"
                                           "s"]
                                              .toDouble(),
                                      'f', 3)
                                : tr("N/A"));
                    pathAttributeData
                        << (actualValuesObj.contains(
                                "carbonEmissions")
                                ? QString::number(
                                      actualValuesObj
                                          ["carbonEmission"
                                           "s"]
                                              .toDouble(),
                                      'f', 3)
                                : tr("N/A"));

                    // Cost
                    pathAttributeData
                        << (estimatedValuesObj.contains(
                                "cost")
                                ? QString::number(
                                      estimatedValuesObj
                                          ["cost"]
                                              .toDouble(),
                                      'f', 2)
                                : tr("N/A"));
                    pathAttributeData
                        << (actualValuesObj.contains("cost")
                                ? QString::number(
                                      actualValuesObj
                                          ["cost"]
                                              .toDouble(),
                                      'f', 2)
                                : tr("N/A"));

                    // Distance
                    pathAttributeData
                        << (estimatedValuesObj.contains(
                                "distance")
                                ? QString::number(
                                      estimatedValuesObj
                                          ["distance"]
                                              .toDouble(),
                                      'f', 2)
                                : tr("N/A"));
                    pathAttributeData
                        << (actualValuesObj.contains(
                                "distance")
                                ? QString::number(
                                      actualValuesObj
                                          ["distance"]
                                              .toDouble(),
                                      'f', 2)
                                : tr("N/A"));

                    // Energy Consumption
                    pathAttributeData
                        << (estimatedValuesObj.contains(
                                "energyConsumption")
                                ? QString::number(
                                      estimatedValuesObj
                                          ["energyConsumpti"
                                           "on"]
                                              .toDouble(),
                                      'f', 2)
                                : tr("N/A"));
                    pathAttributeData
                        << (actualValuesObj.contains(
                                "energyConsumption")
                                ? QString::number(
                                      actualValuesObj
                                          ["energyConsumpti"
                                           "on"]
                                              .toDouble(),
                                      'f', 2)
                                : tr("N/A"));

                    // Risk
                    pathAttributeData
                        << (estimatedValuesObj.contains(
                                "risk")
                                ? QString::number(
                                      estimatedValuesObj
                                          ["risk"]
                                              .toDouble(),
                                      'f', 6)
                                : tr("N/A"));
                    pathAttributeData
                        << (actualValuesObj.contains("risk")
                                ? QString::number(
                                      actualValuesObj
                                          ["risk"]
                                              .toDouble(),
                                      'f', 6)
                                : tr("N/A"));

                    // Travel Time
                    pathAttributeData
                        << (estimatedValuesObj.contains(
                                "travelTime")
                                ? QString::number(
                                      estimatedValuesObj
                                          ["travelTime"]
                                              .toDouble(),
                                      'f', 2)
                                : tr("N/A"));
                    pathAttributeData
                        << (actualValuesObj.contains(
                                "travelTime")
                                ? QString::number(
                                      actualValuesObj
                                          ["travelTime"]
                                              .toDouble(),
                                      'f', 2)
                                : tr("N/A"));
                }
                else
                {
                    // Fill empty data for all attributes
                    for (int i = 0;
                         i < attributeRowLabels.size(); ++i)
                    {
                        pathAttributeData << tr("N/A");
                    }
                }
            }
            else
            {
                // Fill with placeholders for invalid path
                for (int i = 0;
                     i < attributeRowLabels.size(); ++i)
                {
                    pathAttributeData << tr("N/A");
                }
            }

            attributeData.append(pathAttributeData);
        }

        // Transpose attribute data
        QList<QStringList> transposedAttributeData;
        for (int rowIdx = 0;
             rowIdx < attributeRowLabels.size(); ++rowIdx)
        {
            QStringList rowData;
            for (const auto &attrData : attributeData)
            {
                if (rowIdx < attrData.size())
                {
                    rowData << attrData[rowIdx];
                }
                else
                {
                    rowData << tr("N/A");
                }
            }
            transposedAttributeData.append(rowData);
        }

        auto attributeTable = createComparisonTable(
            attributeHeaders, attributeRowLabels,
            transposedAttributeData);
        attributeLayout->addWidget(attributeTable);

        segmentTabWidget->addTab(attributeWidget,
                                 tr("Segment %1 Attributes")
                                     .arg(segmentIdx + 1));
    }

    layout->addWidget(segmentTabWidget);

    return container;
}

QWidget *PathComparisonDialog::createCostsTab()
{
    // Create container widget
    auto container = new QWidget(this);
    auto layout    = new QVBoxLayout(container);

    // Add a header
    auto headerLabel =
        new QLabel(tr("<h2>Cost Comparison</h2>"), this);
    headerLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(headerLabel);

    // Create tab widget for different cost breakdowns
    auto costTabWidget = new QTabWidget(this);

    // --- Create summary cost tab ---
    auto summaryWidget = new QWidget(this);
    auto summaryLayout = new QVBoxLayout(summaryWidget);

    // Create column headers (Path ID 1, Path ID 2, etc.)
    QStringList headers;
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            headers << tr("Path %1").arg(
                path->path->getPathId());
        }
        else
        {
            headers << tr("Unknown Path");
        }
    }

    // Create row labels for cost categories
    QStringList rowLabels = {
        tr("Predicted Total Cost"),
        tr("Predicted Edge Cost"),
        tr("Predicted Terminal Cost"),
        tr("Simulated Total Cost"),
        tr("Simulated Edge Cost"),
        tr("Simulated Terminal Cost"),
        tr("Cost Difference (%)") // Percentage difference
                                  // between predicted and
                                  // simulated
    };

    // Populate cost data for each path
    QList<QStringList> data;
    for (const auto &path : m_pathData)
    {
        QStringList costData;

        if (path && path->path)
        {
            // Add predicted costs
            costData << QString::number(
                path->path->getTotalPathCost(), 'f', 2);
            costData << QString::number(
                path->path->getTotalEdgeCosts(), 'f', 2);
            costData << QString::number(
                path->path->getTotalTerminalCosts(), 'f',
                2);

            // Add simulated costs
            if (path->m_totalSimulationPathCost >= 0)
            {
                costData << QString::number(
                    path->m_totalSimulationPathCost, 'f',
                    2);
            }
            else
            {
                costData << tr("Not simulated");
            }

            if (path->m_totalSimulationEdgeCosts >= 0)
            {
                costData << QString::number(
                    path->m_totalSimulationEdgeCosts, 'f',
                    2);
            }
            else
            {
                costData << tr("Not simulated");
            }

            if (path->m_totalSimulationTerminalCosts >= 0)
            {
                costData << QString::number(
                    path->m_totalSimulationTerminalCosts,
                    'f', 2);
            }
            else
            {
                costData << tr("Not simulated");
            }

            // Calculate percentage difference
            if (path->m_totalSimulationPathCost >= 0
                && path->path->getTotalPathCost() > 0)
            {
                double predictedCost =
                    path->path->getTotalPathCost();
                double simulatedCost =
                    path->m_totalSimulationPathCost;
                double difference =
                    ((simulatedCost - predictedCost)
                     / predictedCost)
                    * 100.0;

                // Format with + sign for positive
                // differences
                if (difference > 0)
                {
                    costData << QString("+%1%").arg(
                        difference, 0, 'f', 2);
                }
                else
                {
                    costData << QString("%1%").arg(
                        difference, 0, 'f', 2);
                }
            }
            else
            {
                costData << tr("N/A");
            }
        }
        else
        {
            // Fill with placeholders if path data is
            // invalid
            for (int i = 0; i < rowLabels.size(); ++i)
            {
                costData << tr("N/A");
            }
        }

        data.append(costData);
    }

    // Transpose data for display (paths as columns)
    QList<QStringList> transposedData;
    for (int rowIdx = 0; rowIdx < rowLabels.size();
         ++rowIdx)
    {
        QStringList rowData;
        for (const auto &costData : data)
        {
            if (rowIdx < costData.size())
            {
                rowData << costData[rowIdx];
            }
            else
            {
                rowData << tr("N/A");
            }
        }
        transposedData.append(rowData);
    }

    // Create and add table
    auto table = createComparisonTable(headers, rowLabels,
                                       transposedData);
    summaryLayout->addWidget(table);

    costTabWidget->addTab(summaryWidget, tr("Summary"));

    // --- Create detailed cost breakdown tab ---
    auto detailedWidget = new QWidget(this);
    auto detailedLayout = new QVBoxLayout(detailedWidget);

    // Create row labels for detailed cost breakdown
    QStringList detailedRowLabels = {
        tr("Carbon Emissions Cost (Predicted)"),
        tr("Carbon Emissions Cost (Actual)"),
        tr("Direct Cost (Predicted)"),
        tr("Direct Cost (Actual)"),
        tr("Distance-based Cost (Predicted)"),
        tr("Distance-based Cost (Actual)"),
        tr("Energy Consumption Cost (Predicted)"),
        tr("Energy Consumption Cost (Actual)"),
        tr("Risk-based Cost (Predicted)"),
        tr("Risk-based Cost (Actual)"),
        tr("Travel Time Cost (Predicted)"),
        tr("Travel Time Cost (Actual)")};

    // For each path, accumulate the cost breakdown across
    // all segments
    QList<QStringList> detailedData;

    for (const auto &path : m_pathData)
    {
        QStringList pathDetailedData;

        if (path && path->path)
        {
            const auto &segments =
                path->path->getSegments();

            // Initialize cost accumulators
            double predictedCarbonEmissionsCost = 0.0;
            double actualCarbonEmissionsCost    = 0.0;
            double predictedDirectCost          = 0.0;
            double actualDirectCost             = 0.0;
            double predictedDistanceCost        = 0.0;
            double actualDistanceCost           = 0.0;
            double predictedEnergyCost          = 0.0;
            double actualEnergyCost             = 0.0;
            double predictedRiskCost            = 0.0;
            double actualRiskCost               = 0.0;
            double predictedTimeCost            = 0.0;
            double actualTimeCost               = 0.0;

            bool hasActualData = false;

            // Sum up costs across all segments
            for (const auto &segment : segments)
            {
                if (segment)
                {
                    const QJsonObject &attributes =
                        segment->getAttributes();

                    // Extract estimated_values
                    if (attributes.contains(
                            "estimated_cost")
                        && attributes["estimated_cost"]
                               .isObject())
                    {
                        QJsonObject estimatedCostObj =
                            attributes["estimated_cost"]
                                .toObject();

                        // For predicted values
                        double carbonEmissions =
                            estimatedCostObj.contains(
                                "carbonEmissions")
                                ? estimatedCostObj
                                      ["carbonEmissions"]
                                          .toDouble()
                                : 0.0;
                        double directCost =
                            estimatedCostObj.contains(
                                "cost")
                                ? estimatedCostObj["cost"]
                                      .toDouble()
                                : 0.0;
                        double distance =
                            estimatedCostObj.contains(
                                "distance")
                                ? estimatedCostObj
                                      ["distance"]
                                          .toDouble()
                                : 0.0;
                        double energyConsumption =
                            estimatedCostObj.contains(
                                "energyConsumption")
                                ? estimatedCostObj
                                      ["energyConsumption"]
                                          .toDouble()
                                : 0.0;
                        double risk =
                            estimatedCostObj.contains(
                                "risk")
                                ? estimatedCostObj["risk"]
                                      .toDouble()
                                : 0.0;
                        double travelTime =
                            estimatedCostObj.contains(
                                "travelTime")
                                ? estimatedCostObj
                                      ["travelTime"]
                                          .toDouble()
                                : 0.0;

                        predictedCarbonEmissionsCost +=
                            carbonEmissions;
                        predictedDirectCost += directCost;
                        predictedDistanceCost += distance;
                        predictedEnergyCost +=
                            energyConsumption;
                        predictedRiskCost += risk;
                        predictedTimeCost += travelTime;
                    }
                    if (attributes.contains("actual_cost")
                        && attributes["actual_cost"]
                               .isObject())
                    {
                        QJsonObject actualCostObj =
                            attributes["actual_cost"]
                                .toObject();

                        // For actual values
                        double carbonEmissions =
                            actualCostObj.contains(
                                "carbonEmissions")
                                ? actualCostObj
                                      ["carbonEmissions"]
                                          .toDouble()
                                : 0.0;
                        double directCost =
                            actualCostObj.contains("cost")
                                ? actualCostObj["cost"]
                                      .toDouble()
                                : 0.0;
                        double distance =
                            actualCostObj.contains(
                                "distance")
                                ? actualCostObj["distance"]
                                      .toDouble()
                                : 0.0;
                        double energyConsumption =
                            actualCostObj.contains(
                                "energyConsumption")
                                ? actualCostObj
                                      ["energyConsumption"]
                                          .toDouble()
                                : 0.0;
                        double risk =
                            actualCostObj.contains("risk")
                                ? actualCostObj["risk"]
                                      .toDouble()
                                : 0.0;
                        double travelTime =
                            actualCostObj.contains(
                                "travelTime")
                                ? actualCostObj
                                      ["travelTime"]
                                          .toDouble()
                                : 0.0;

                        actualCarbonEmissionsCost +=
                            carbonEmissions;
                        actualDirectCost += directCost;
                        actualDistanceCost += distance;
                        actualEnergyCost +=
                            energyConsumption;
                        actualRiskCost += risk;
                        actualTimeCost += travelTime;

                        hasActualData = true;
                    }
                }
            }

            // Add predicted costs
            pathDetailedData << QString::number(
                predictedCarbonEmissionsCost, 'f', 2);
            pathDetailedData
                << (hasActualData
                        ? QString::number(
                              actualCarbonEmissionsCost,
                              'f', 2)
                        : tr("Not simulated"));

            pathDetailedData << QString::number(
                predictedDirectCost, 'f', 2);
            pathDetailedData
                << (hasActualData
                        ? QString::number(actualDirectCost,
                                          'f', 2)
                        : tr("Not simulated"));

            pathDetailedData << QString::number(
                predictedDistanceCost, 'f', 2);
            pathDetailedData
                << (hasActualData
                        ? QString::number(
                              actualDistanceCost, 'f', 2)
                        : tr("Not simulated"));

            pathDetailedData << QString::number(
                predictedEnergyCost, 'f', 2);
            pathDetailedData
                << (hasActualData
                        ? QString::number(actualEnergyCost,
                                          'f', 2)
                        : tr("Not simulated"));

            pathDetailedData << QString::number(
                predictedRiskCost, 'f', 2);
            pathDetailedData
                << (hasActualData
                        ? QString::number(actualRiskCost,
                                          'f', 2)
                        : tr("Not simulated"));

            pathDetailedData << QString::number(
                predictedTimeCost, 'f', 2);
            pathDetailedData
                << (hasActualData
                        ? QString::number(actualTimeCost,
                                          'f', 2)
                        : tr("Not simulated"));
        }
        else
        {
            // Fill with placeholders for invalid path
            for (int i = 0; i < detailedRowLabels.size();
                 ++i)
            {
                pathDetailedData << tr("N/A");
            }
        }

        detailedData.append(pathDetailedData);
    }

    // Transpose detailed data
    QList<QStringList> transposedDetailedData;
    for (int rowIdx = 0; rowIdx < detailedRowLabels.size();
         ++rowIdx)
    {
        QStringList rowData;
        for (const auto &detData : detailedData)
        {
            if (rowIdx < detData.size())
            {
                rowData << detData[rowIdx];
            }
            else
            {
                rowData << tr("N/A");
            }
        }
        transposedDetailedData.append(rowData);
    }

    auto detailedTable = createComparisonTable(
        headers, detailedRowLabels, transposedDetailedData);
    detailedLayout->addWidget(detailedTable);

    costTabWidget->addTab(detailedWidget,
                          tr("Cost Breakdown"));

    // --- Create segment-level cost breakdown tabs ---
    // Find the maximum number of segments across all paths
    int maxSegments = 0;
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            maxSegments =
                qMax(maxSegments,
                     path->path->getSegments().size());
        }
    }

    // Create a tab for each segment
    for (int segmentIdx = 0; segmentIdx < maxSegments;
         ++segmentIdx)
    {
        auto segmentWidget = new QWidget(this);
        auto segmentLayout = new QVBoxLayout(segmentWidget);

        // Row labels for segment costs
        QStringList segmentCostRowLabels = {
            tr("Carbon Emissions Cost (Predicted)"),
            tr("Carbon Emissions Cost (Actual)"),
            tr("Direct Cost (Predicted)"),
            tr("Direct Cost (Actual)"),
            tr("Distance-based Cost (Predicted)"),
            tr("Distance-based Cost (Actual)"),
            tr("Energy Consumption Cost (Predicted)"),
            tr("Energy Consumption Cost (Actual)"),
            tr("Risk-based Cost (Predicted)"),
            tr("Risk-based Cost (Actual)"),
            tr("Travel Time Cost (Predicted)"),
            tr("Travel Time Cost (Actual)")};

        // Populate data for each path's segment
        QList<QStringList> segmentCostData;

        for (const auto &path : m_pathData)
        {
            QStringList pathSegmentData;

            if (path && path->path)
            {
                const auto &segments =
                    path->path->getSegments();

                if (segmentIdx < segments.size()
                    && segments[segmentIdx])
                {
                    const QJsonObject &attributes =
                        segments[segmentIdx]
                            ->getAttributes();

                    // Extract estimated_cost
                    QJsonObject estimatedCostObj;
                    if (attributes.contains(
                            "estimated_cost")
                        && attributes["estimated_cost"]
                               .isObject())
                    {
                        estimatedCostObj =
                            attributes["estimated_cost"]
                                .toObject();
                    }

                    // Extract actual_cost
                    QJsonObject actualCostObj;
                    if (attributes.contains("actual_cost")
                        && attributes["actual_cost"]
                               .isObject())
                    {
                        actualCostObj =
                            attributes["actual_cost"]
                                .toObject();
                    }

                    // Carbon Emissions Cost
                    pathSegmentData
                        << (estimatedCostObj.contains(
                                "carbonEmissions")
                                ? QString::number(
                                      estimatedCostObj
                                          ["carbonEmission"
                                           "s"]
                                              .toDouble(),
                                      'f', 2)
                                : tr("N/A"));
                    pathSegmentData
                        << (actualCostObj.contains(
                                "carbonEmissions")
                                ? QString::number(
                                      actualCostObj
                                          ["carbonEmission"
                                           "s"]
                                              .toDouble(),
                                      'f', 2)
                                : tr("N/A"));

                    // Direct Cost
                    pathSegmentData
                        << (estimatedCostObj.contains(
                                "cost")
                                ? QString::number(
                                      estimatedCostObj
                                          ["cost"]
                                              .toDouble(),
                                      'f', 2)
                                : tr("N/A"));
                    pathSegmentData
                        << (actualCostObj.contains("cost")
                                ? QString::number(
                                      actualCostObj["cost"]
                                          .toDouble(),
                                      'f', 2)
                                : tr("N/A"));

                    // Distance-based Cost
                    pathSegmentData
                        << (estimatedCostObj.contains(
                                "distance")
                                ? QString::number(
                                      estimatedCostObj
                                          ["distance"]
                                              .toDouble(),
                                      'f', 2)
                                : tr("N/A"));
                    pathSegmentData
                        << (actualCostObj.contains(
                                "distance")
                                ? QString::number(
                                      actualCostObj
                                          ["distance"]
                                              .toDouble(),
                                      'f', 2)
                                : tr("N/A"));

                    // Energy Consumption Cost
                    pathSegmentData
                        << (estimatedCostObj.contains(
                                "energyConsumption")
                                ? QString::number(
                                      estimatedCostObj
                                          ["energyConsumpti"
                                           "on"]
                                              .toDouble(),
                                      'f', 2)
                                : tr("N/A"));
                    pathSegmentData
                        << (actualCostObj.contains(
                                "energyConsumption")
                                ? QString::number(
                                      actualCostObj
                                          ["energyConsumpti"
                                           "on"]
                                              .toDouble(),
                                      'f', 2)
                                : tr("N/A"));

                    // Risk Cost
                    pathSegmentData
                        << (estimatedCostObj.contains(
                                "risk")
                                ? QString::number(
                                      estimatedCostObj
                                          ["risk"]
                                              .toDouble(),
                                      'f', 6)
                                : tr("N/A"));
                    pathSegmentData
                        << (actualCostObj.contains("risk")
                                ? QString::number(
                                      actualCostObj["risk"]
                                          .toDouble(),
                                      'f', 6)
                                : tr("N/A"));

                    // Travel Time Cost
                    pathSegmentData
                        << (estimatedCostObj.contains(
                                "travelTime")
                                ? QString::number(
                                      estimatedCostObj
                                          ["travelTime"]
                                              .toDouble(),
                                      'f', 2)
                                : tr("N/A"));
                    pathSegmentData
                        << (actualCostObj.contains(
                                "travelTime")
                                ? QString::number(
                                      actualCostObj
                                          ["travelTime"]
                                              .toDouble(),
                                      'f', 2)
                                : tr("N/A"));
                }
                else
                {
                    // Fill with placeholders for missing
                    // segment
                    for (int i = 0;
                         i < segmentCostRowLabels.size();
                         ++i)
                    {
                        pathSegmentData << tr("-");
                    }
                }
            }
            else
            {
                // Fill with placeholders for invalid path
                for (int i = 0;
                     i < segmentCostRowLabels.size(); ++i)
                {
                    pathSegmentData << tr("N/A");
                }
            }

            segmentCostData.append(pathSegmentData);
        }

        // Transpose data for display
        QList<QStringList> transposedSegmentData;
        for (int rowIdx = 0;
             rowIdx < segmentCostRowLabels.size(); ++rowIdx)
        {
            QStringList rowData;
            for (const auto &segData : segmentCostData)
            {
                if (rowIdx < segData.size())
                {
                    rowData << segData[rowIdx];
                }
                else
                {
                    rowData << tr("N/A");
                }
            }
            transposedSegmentData.append(rowData);
        }

        // Create and add table
        auto segmentTable = createComparisonTable(
            headers, segmentCostRowLabels,
            transposedSegmentData);
        segmentLayout->addWidget(segmentTable);

        // Add segment info at the top of the tab
        QString segmentInfoText =
            tr("<h3>Segment %1 Costs</h3>")
                .arg(segmentIdx + 1);
        for (const auto &path : m_pathData)
        {
            if (path && path->path)
            {
                const auto &segments =
                    path->path->getSegments();
                if (segmentIdx < segments.size()
                    && segments[segmentIdx])
                {
                    QString segmentInfo =
                        QString("<p><b>Path %1:</b> %2 → "
                                "%3 (%4)</p>")
                            .arg(path->path->getPathId())
                            .arg(segments[segmentIdx]
                                     ->getStart())
                            .arg(segments[segmentIdx]
                                     ->getEnd())
                            .arg(
                                Backend::TransportationTypes::
                                    toString(
                                        segments[segmentIdx]
                                            ->getMode()));
                    segmentInfoText += segmentInfo;
                }
            }
        }

        auto infoLabel = new QLabel(segmentInfoText, this);
        infoLabel->setAlignment(Qt::AlignCenter);
        segmentLayout->insertWidget(0, infoLabel);

        costTabWidget->addTab(
            segmentWidget,
            tr("Segment %1").arg(segmentIdx + 1));
    }

    layout->addWidget(costTabWidget);

    return container;
}

QTableWidget *PathComparisonDialog::createComparisonTable(
    const QStringList        &headers,
    const QStringList        &rowLabels,
    const QList<QStringList> &data)
{
    // Create table widget
    auto table = new QTableWidget(this);

    // Set dimensions
    table->setRowCount(rowLabels.size());
    table->setColumnCount(headers.size()
                          + 1); // +1 for row labels

    // Set headers
    QStringList allHeaders;
    allHeaders << tr("Property"); // First column header
    allHeaders.append(headers);
    table->setHorizontalHeaderLabels(allHeaders);

    // Populate with data
    for (int row = 0; row < rowLabels.size(); ++row)
    {
        // Set row label
        auto labelItem =
            new QTableWidgetItem(rowLabels[row]);
        labelItem->setFlags(labelItem->flags()
                            & ~Qt::ItemIsEditable);

        // Make the text bold
        QFont font = labelItem->font();
        font.setBold(true);
        labelItem->setFont(font);
        table->setItem(row, 0, labelItem);

        // Set data cells
        if (row < data.size())
        {
            const QStringList &rowData = data[row];

            for (int col = 0; col < rowData.size()
                              && col < headers.size();
                 ++col)
            {
                auto dataItem =
                    new QTableWidgetItem(rowData[col]);
                dataItem->setFlags(dataItem->flags()
                                   & ~Qt::ItemIsEditable);

                // Color-code cost differences in the costs
                // tab
                if (rowLabels[row]
                        == tr("Cost Difference (%)")
                    && rowData[col] != tr("N/A"))
                {

                    // Extract numeric part
                    QString numStr = rowData[col];
                    numStr.remove('%');
                    numStr.remove('+');
                    bool   ok;
                    double value = numStr.toDouble(&ok);

                    if (ok)
                    {
                        // Green for savings (negative
                        // difference), red for overruns
                        // (positive difference)
                        if (value < 0)
                        {
                            // Darker green for bigger
                            // savings
                            int intensity = qMin(
                                255,
                                qMax(0,
                                     255
                                         - qAbs(static_cast<
                                                int>(
                                             value * 2))));
                            dataItem->setBackground(QBrush(
                                QColor(intensity, 255,
                                       intensity)));
                        }
                        else if (value > 0)
                        {
                            // Darker red for bigger
                            // overruns
                            int intensity = qMin(
                                255,
                                qMax(0,
                                     255
                                         - qAbs(static_cast<
                                                int>(
                                             value * 2))));
                            dataItem->setBackground(QBrush(
                                QColor(255, intensity,
                                       intensity)));
                        }
                    }
                }

                table->setItem(row, col + 1, dataItem);
            }
        }
    }

    // Configure table appearance
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    table->setEditTriggers(QTableWidget::NoEditTriggers);
    table->setSelectionBehavior(QTableWidget::SelectRows);

    return table;
}

void PathComparisonDialog::createPathVisualization(
    QWidget *container)
{
    // Create layout for visualization
    auto layout = new QVBoxLayout(container);

    // Create a grid layout to place path visualizations
    // side by side
    auto gridLayout = new QGridLayout();
    gridLayout->setSpacing(20);

    // Add each path visualization
    for (int pathIdx = 0; pathIdx < m_pathData.size();
         ++pathIdx)
    {
        const auto &pathData = m_pathData[pathIdx];

        if (pathData && pathData->path)
        {
            // Create a container for this path
            auto pathContainer = new QWidget(container);
            auto pathLayout =
                new QVBoxLayout(pathContainer);

            // Add path header
            auto pathHeader = new QLabel(
                tr("<h3>Path %1</h3>")
                    .arg(pathData->path->getPathId()),
                container);
            pathHeader->setAlignment(Qt::AlignCenter);
            pathLayout->addWidget(pathHeader);

            // Create a widget for terminal visualization
            auto terminalWidget = new QWidget(container);
            auto terminalLayout =
                new QHBoxLayout(terminalWidget);
            terminalLayout->setSpacing(4);

            // Get terminals and segments
            const auto &terminals =
                pathData->path->getTerminalsInPath();
            const auto &segments =
                pathData->path->getSegments();

            if (!terminals.isEmpty())
            {
                // Add terminals and transportation modes
                for (int i = 0; i < terminals.size(); ++i)
                {
                    if (!terminals[i])
                    {
                        continue;
                    }

                    // Add terminal name label
                    QString terminalName =
                        terminals[i]->getDisplayName();
                    if (terminalName.isEmpty())
                    {
                        terminalName =
                            tr("Terminal %1").arg(i + 1);
                    }

                    auto nameLabel =
                        new QLabel(terminalName, container);
                    nameLabel->setAlignment(
                        Qt::AlignCenter);
                    nameLabel->setMinimumWidth(120);
                    terminalLayout->addWidget(nameLabel);

                    // Add transportation mode arrow for all
                    // but the last terminal
                    if (i < terminals.size() - 1
                        && i < segments.size()
                        && segments[i])
                    {
                        auto modeLabel =
                            new QLabel(container);
                        modeLabel->setAlignment(
                            Qt::AlignCenter);

                        // Get transportation mode
                        Backend::TransportationTypes::
                            TransportationMode mode =
                                segments[i]->getMode();
                        QString modeText =
                            Backend::TransportationTypes::
                                toString(mode);

                        // Create mode pixmap
                        QPixmap modePixmap =
                            createTransportModePixmap(
                                modeText);
                        modeLabel->setPixmap(modePixmap);
                        modeLabel->setToolTip(modeText);
                        terminalLayout->addWidget(
                            modeLabel);
                    }
                }
            }
            else
            {
                terminalLayout->addWidget(new QLabel(
                    tr("No terminal data"), container));
            }

            // Add stretch to ensure elements are
            // left-aligned
            terminalLayout->addStretch();

            // Add terminal visualization to path layout
            pathLayout->addWidget(terminalWidget);

            // Add cost information
            QString costInfo =
                tr("Predicted: %1, Simulated: %2")
                    .arg(pathData->path->getTotalPathCost(),
                         0, 'f', 2)
                    .arg(
                        pathData->m_totalSimulationPathCost
                                >= 0
                            ? QString::number(
                                  pathData
                                      ->m_totalSimulationPathCost,
                                  'f', 2)
                            : tr("Not simulated"));

            auto costLabel =
                new QLabel(costInfo, container);
            costLabel->setAlignment(Qt::AlignCenter);
            pathLayout->addWidget(costLabel);

            // Add this path to the grid
            gridLayout->addWidget(pathContainer, 0,
                                  pathIdx);
        }
    }

    // Add grid to main layout
    layout->addLayout(gridLayout);
    layout->addStretch();
}

QPixmap PathComparisonDialog::createTransportModePixmap(
    const QString &mode)
{
    // Create a pixmap for the arrow with the mode text
    QPixmap pixmap(64, 40);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Set color based on transportation mode
    QColor arrowColor = Qt::black; // Default color

    if (mode.contains("Truck", Qt::CaseInsensitive))
    {
        arrowColor =
            QColor(255, 0, 255); // Magenta for truck
    }
    else if (mode.contains("Rail", Qt::CaseInsensitive)
             || mode.contains("Train", Qt::CaseInsensitive))
    {
        arrowColor =
            QColor(80, 80, 80); // Dark gray for rail
    }
    else if (mode.contains("Ship", Qt::CaseInsensitive)
             || mode.contains("Water", Qt::CaseInsensitive))
    {
        arrowColor = QColor(0, 0, 255); // Blue for ship
    }

    // Draw the mode text
    painter.setPen(arrowColor);
    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRect(0, 0, pixmap.width(), 15),
                     Qt::AlignCenter, mode);

    // Draw the arrow
    QPen pen(arrowColor, 2);
    painter.setPen(pen);

    // Draw arrow shaft
    painter.drawLine(10, 25, 54, 25);

    // Draw arrow head
    QPolygon arrowHead;
    arrowHead << QPoint(48, 20) << QPoint(54, 25)
              << QPoint(48, 30);
    painter.setBrush(arrowColor);
    painter.drawPolygon(arrowHead);

    return pixmap;
}

void PathComparisonDialog::onExportButtonClicked()
{
    // Ask user for file location
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Export Comparison Data"),
        QDir::homePath(),
        tr("CSV Files (*.csv);;All Files (*)"));

    if (fileName.isEmpty())
    {
        return; // User canceled
    }

    // Make sure file has .csv extension
    if (!fileName.endsWith(".csv", Qt::CaseInsensitive))
    {
        fileName += ".csv";
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(
            this, tr("Export Error"),
            tr("Could not open file for writing: %1")
                .arg(file.errorString()));
        return;
    }

    QTextStream out(&file);

    // Write header with path IDs
    // Write header with path IDs
    out << "Property";
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            out << ",Path " << path->path->getPathId();
        }
        else
        {
            out << ",Unknown Path";
        }
    }
    out << "\n";

    // Write summary data
    out << "Path ID";
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            out << "," << path->path->getPathId();
        }
        else
        {
            out << ",N/A";
        }
    }
    out << "\n";

    out << "Total Terminals";
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            out << ","
                << path->path->getTerminalsInPath().size();
        }
        else
        {
            out << ",N/A";
        }
    }
    out << "\n";

    out << "Total Segments";
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            out << "," << path->path->getSegments().size();
        }
        else
        {
            out << ",N/A";
        }
    }
    out << "\n";

    out << "Predicted Cost";
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            out << "," << path->path->getTotalPathCost();
        }
        else
        {
            out << ",N/A";
        }
    }
    out << "\n";

    out << "Actual Cost";
    for (const auto &path : m_pathData)
    {
        if (path && path->path
            && path->m_totalSimulationPathCost >= 0)
        {
            out << "," << path->m_totalSimulationPathCost;
        }
        else
        {
            out << ",Not simulated";
        }
    }
    out << "\n";

    out << "Start Terminal";
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            try
            {
                out << ","
                    << path->path->getStartTerminal();
            }
            catch (const std::exception &)
            {
                out << ",Unknown";
            }
        }
        else
        {
            out << ",N/A";
        }
    }
    out << "\n";

    out << "End Terminal";
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            try
            {
                out << "," << path->path->getEndTerminal();
            }
            catch (const std::exception &)
            {
                out << ",Unknown";
            }
        }
        else
        {
            out << ",N/A";
        }
    }
    out << "\n\n";

    // Write terminal data
    out << "Terminals:\n";

    // Find the maximum number of terminals across all paths
    int maxTerminals = 0;
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            maxTerminals = qMax(
                maxTerminals,
                path->path->getTerminalsInPath().size());
        }
    }

    // Write header for terminal data
    out << "Terminal Index";
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            out << ",Path " << path->path->getPathId();
        }
        else
        {
            out << ",Unknown Path";
        }
    }
    out << "\n";

    // Write terminal data for each index
    for (int i = 0; i < maxTerminals; ++i)
    {
        out << "Terminal " << (i + 1);

        for (const auto &path : m_pathData)
        {
            if (path && path->path)
            {
                const auto &terminals =
                    path->path->getTerminalsInPath();
                if (i < terminals.size() && terminals[i])
                {
                    out << ","
                        << terminals[i]->getDisplayName();
                }
                else
                {
                    out << ",-";
                }
            }
            else
            {
                out << ",N/A";
            }
        }
        out << "\n";
    }
    out << "\n";

    // Write segment data
    out << "Segments:\n";

    // Find the maximum number of segments across all paths
    int maxSegments = 0;
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            maxSegments =
                qMax(maxSegments,
                     path->path->getSegments().size());
        }
    }

    // Write header for segment data
    out << "Segment Index";
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            out << ",Path " << path->path->getPathId();
        }
        else
        {
            out << ",Unknown Path";
        }
    }
    out << "\n";

    // Write segment data for each index
    for (int i = 0; i < maxSegments; ++i)
    {
        out << "Segment " << (i + 1);

        for (const auto &path : m_pathData)
        {
            if (path && path->path)
            {
                const auto &segments =
                    path->path->getSegments();
                if (i < segments.size() && segments[i])
                {
                    QString segmentInfo =
                        QString("%1 to %2 (%3)")
                            .arg(segments[i]->getStart())
                            .arg(segments[i]->getEnd())
                            .arg(
                                Backend::TransportationTypes::
                                    toString(
                                        segments[i]
                                            ->getMode()));
                    out << "," << segmentInfo;
                }
                else
                {
                    out << ",-";
                }
            }
            else
            {
                out << ",N/A";
            }
        }
        out << "\n";
    }
    out << "\n";

    // Write cost data
    out << "Costs:\n";

    // Write header for cost data
    out << "Cost Type";
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            out << ",Path " << path->path->getPathId();
        }
        else
        {
            out << ",Unknown Path";
        }
    }
    out << "\n";

    // Write predicted costs
    out << "Predicted Total Cost";
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            out << "," << path->path->getTotalPathCost();
        }
        else
        {
            out << ",N/A";
        }
    }
    out << "\n";

    out << "Predicted Edge Cost";
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            out << "," << path->path->getTotalEdgeCosts();
        }
        else
        {
            out << ",N/A";
        }
    }
    out << "\n";

    out << "Predicted Terminal Cost";
    for (const auto &path : m_pathData)
    {
        if (path && path->path)
        {
            out << ","
                << path->path->getTotalTerminalCosts();
        }
        else
        {
            out << ",N/A";
        }
    }
    out << "\n";

    // Write simulated costs
    out << "Simulated Total Cost";
    for (const auto &path : m_pathData)
    {
        if (path && path->path
            && path->m_totalSimulationPathCost >= 0)
        {
            out << "," << path->m_totalSimulationPathCost;
        }
        else
        {
            out << ",Not simulated";
        }
    }
    out << "\n";

    out << "Simulated Edge Cost";
    for (const auto &path : m_pathData)
    {
        if (path && path->path
            && path->m_totalSimulationEdgeCosts >= 0)
        {
            out << "," << path->m_totalSimulationEdgeCosts;
        }
        else
        {
            out << ",Not simulated";
        }
    }
    out << "\n";

    out << "Simulated Terminal Cost";
    for (const auto &path : m_pathData)
    {
        if (path && path->path
            && path->m_totalSimulationTerminalCosts >= 0)
        {
            out << ","
                << path->m_totalSimulationTerminalCosts;
        }
        else
        {
            out << ",Not simulated";
        }
    }
    out << "\n";

    // Write cost difference percentage
    out << "Cost Difference (%)";
    for (const auto &path : m_pathData)
    {
        if (path && path->path
            && path->m_totalSimulationPathCost >= 0
            && path->path->getTotalPathCost() > 0)
        {
            double predictedCost =
                path->path->getTotalPathCost();
            double simulatedCost =
                path->m_totalSimulationPathCost;
            double difference =
                ((simulatedCost - predictedCost)
                 / predictedCost)
                * 100.0;

            // Format with + sign for positive differences
            if (difference > 0)
            {
                out << ",+" << difference;
            }
            else
            {
                out << "," << difference;
            }
        }
        else
        {
            out << ",N/A";
        }
    }
    out << "\n";

    file.close();

    QMessageBox::information(
        this, tr("Export Successful"),
        tr("Comparison data has been exported to:\n%1")
            .arg(fileName));
}

} // namespace GUI
} // namespace CargoNetSim
