#pragma once

#include <QSplashScreen>
#include <QPixmap>
#include <QProgressBar>
#include <QLabel>

namespace CargoNetSim {
namespace GUI {

/**
 * @brief Enhanced splash screen with loading progress
 * 
 * This class extends QSplashScreen to add a progress bar and status updates
 * for providing loading feedback to users during application startup.
 */
class SplashScreen : public QSplashScreen {
    Q_OBJECT
    Q_PROPERTY(int progress READ progress WRITE setProgress NOTIFY progressChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage WRITE setStatusMessage NOTIFY statusMessageChanged)

public:
    /**
     * @brief Default constructor
     * 
     * Creates a splash screen with the application logo and a progress bar
     */
    SplashScreen();
    
    /**
     * @brief Destructor
     */
    virtual ~SplashScreen();

    /**
     * @brief Get the current progress value
     * 
     * @return Current progress percentage (0-100)
     */
    int progress() const { return m_progress; }
    
    /**
     * @brief Get the current status message
     * 
     * @return Current status message text
     */
    QString statusMessage() const { return m_statusMessage; }

public slots:
    /**
     * @brief Set the progress value
     * 
     * @param progress New progress value (0-100)
     */
    void setProgress(int progress);
    
    /**
     * @brief Set the status message text
     * 
     * @param message New status message
     */
    void setStatusMessage(const QString& message);
    
    /**
     * @brief Show a message on the splash screen
     * 
     * Overrides QSplashScreen::showMessage to update internal status
     * 
     * @param message Message to display
     * @param alignment Text alignment flags
     * @param color Text color
     */
    void showMessage(const QString& message, int alignment = Qt::AlignLeft | Qt::AlignBottom, 
                     const QColor& color = Qt::black);

signals:
    /**
     * @brief Signal emitted when progress value changes
     * 
     * @param progress New progress value
     */
    void progressChanged(int progress);
    
    /**
     * @brief Signal emitted when status message changes
     * 
     * @param message New status message
     */
    void statusMessageChanged(const QString& message);
    
    /**
     * @brief Signal emitted when loading is complete
     */
    void loadingComplete();

protected:
    /**
     * @brief Draws the splash screen content
     * 
     * @param painter Painter to use for drawing
     */
    void drawContents(QPainter* painter) override;
    
    /**
     * @brief Handle resize events
     * 
     * @param event The resize event
     */
    void resizeEvent(QResizeEvent* event) override;

private:
    /**
     * @brief Initialize the UI components
     */
    void initUI();
    
    /**
     * @brief Update the layout of UI components
     */
    void updateLayout();

    QPixmap m_originalPixmap;     ///< Original splash image
    QProgressBar* m_progressBar;  ///< Progress bar widget
    QLabel* m_statusLabel;        ///< Status message label
    int m_progress;               ///< Current progress value
    QString m_statusMessage;      ///< Current status message
    bool m_isFinished;            ///< Flag indicating if splash is finished
};

} // namespace GUI
} // namespace CargoNetSim