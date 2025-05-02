/**
 * @file ScrollableToolBar.h
 * @author Ahmed Aredah
 * @brief Header file for the ScrollableToolBar class
 * @details This file contains the declaration of the
 * ScrollableToolBar class, which extends QToolBar to
 * provide scrolling capabilities and a ribbon interface.
 */

#pragma once

#include <QTabWidget>
#include <QToolBar>
#include <QtWidgets/qtoolbutton.h>

// Forward declarations to reduce header dependencies
class QScrollArea;
class QHBoxLayout;
class QResizeEvent;

namespace CargoNetSim
{
namespace GUI
{

/**
 * @class ScrollableToolBar
 * @brief A custom toolbar with scrolling capabilities and a
 * ribbon interface
 *
 * The ScrollableToolBar extends QToolBar to provide
 * horizontal scrolling functionality when the toolbar
 * content exceeds the available width. It also includes a
 * ribbon interface (QTabWidget) for organizing toolbar
 * content into tabs.
 */
class ScrollableToolBar : public QToolBar
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for ScrollableToolBar
     * @param title The title of the toolbar
     * @param parent The parent widget
     *
     * Creates a new ScrollableToolBar with the specified
     * title and parent. Initializes the internal container
     * widget, layout, ribbon, and scroll area.
     */
    explicit ScrollableToolBar(
        const QString &title  = QString(),
        QWidget       *parent = nullptr);

    /**
     * @brief Get the pointer to the ribbon widget
     * @return QTabWidget* Pointer to the ribbon tab widget
     */
    QTabWidget *getRibbon();

    /**
     * @brief Add a tab to the ribbon
     * @param widget The widget to be added as a tab
     * @param label The label for the tab
     * @return int Index of the newly added tab
     */
    int addTab(QWidget *widget, const QString &label);

    /**
     * @brief Set the visibility of a tab
     * @param index The index of the tab
     * @param visible True to make the tab visible, false to
     * hide it
     */
    void setTabVisible(int index, bool visible);

    /**
     * @brief Add a widget to the toolbar
     * @param widget The widget to add
     * @return QAction* An action representing the added
     * widget
     *
     * Overrides QToolBar::addWidget to add widgets to the
     * container layout instead of directly to the toolbar.
     */
    QAction *addWidget(QWidget *widget);

    /**
     * @brief Add an action with text to the toolbar
     * @param text The text for the action
     * @return QAction* The newly created action
     *
     * Overrides QToolBar::addAction to add actions to the
     * container layout.
     */
    QAction *addAction(const QString &text);

    /**
     * @brief Add an action with icon and text to the
     * toolbar
     * @param icon The icon for the action
     * @param text The text for the action
     * @return QAction* The newly created action
     *
     * Overrides QToolBar::addAction to add actions to the
     * container layout.
     */
    QAction *addAction(const QIcon   &icon,
                       const QString &text);

    /**
     * @brief Add a separator to the toolbar
     * @return QAction* An action representing the separator
     *
     * Overrides QToolBar::addSeparator to add separators to
     * the container layout.
     */
    QAction *addSeparator();

    /**
     * @brief Find all interactive widgets in the toolbar
     * and its containers
     * @return QList<QWidget*> List of all interactive
     * widgets found
     */
    QList<QWidget *> findAllInteractiveWidgets();

protected:
    /**
     * @brief Handle resize events
     * @param event The resize event
     *
     * Overrides QWidget::resizeEvent to resize the scroll
     * area when the toolbar is resized.
     */
    void resizeEvent(QResizeEvent *event) override;

private:
    /**
     * @brief Container widget for the toolbar content
     *
     * This widget holds all the toolbar content and is
     * placed inside the scroll area.
     */
    QWidget *containerWidget;

    /**
     * @brief Layout for the container widget
     *
     * Horizontal layout that organizes the toolbar content.
     */
    QHBoxLayout *containerLayout;

    /**
     * @brief Scroll area for horizontal scrolling
     *
     * Provides scrolling functionality when the toolbar
     * content exceeds the available width.
     */
    QScrollArea *scrollArea;

    /**
     * @brief Ribbon interface
     *
     * Tab widget that serves as a ribbon interface for
     * organizing toolbar content.
     */
    QTabWidget *ribbon_;

    /**
     * @brief Create a widget for an action
     * @param action The action to create a widget for
     * @return QWidget* A widget representing the action
     *
     * Creates a QToolButton for the specified action with
     * text under icon style.
     */
    QWidget *createWidgetForAction(QAction *action);
};

} // namespace GUI
} // namespace CargoNetSim
