/**
 * @file CustomProgressBar.cpp
 * @brief This file contains the implementation of the
 * CustomProgressBar class.
 * @author Ahmed Aredah
 * @date 6/7/2023
 */

#include "CustomProgressBar.h"

namespace CargoNetSim
{
namespace GUI
{
/**
 * @brief Constructs a CustomProgressBar object.
 * @param parent The parent widget.
 */
CustomProgressBar::CustomProgressBar(QWidget *parent)
    : QProgressBar(parent)
{
    setMinimum(0);
    setMaximum(100);
    setTextVisible(false);
    hide();

    timer_ = new QTimer(this);
    timer_->setSingleShot(true);
    connect(timer_, &QTimer::timeout, this,
            &CustomProgressBar::hide);
}

/**
 * @brief Destroys the CustomProgressBar object.
 */
CustomProgressBar::~CustomProgressBar()
{
    delete timer_;
}

/**
 * @brief Starts the progress bar.
 * The progress bar is shown and the progressStarted()
 * signal is emitted.
 */
void CustomProgressBar::start()
{
    show();
    timer_->stop();
    emit progressStarted();
}

/**
 * @brief Stops the progress bar.
 * The progress bar is hidden and the progressStopped()
 * signal is emitted. After a timeout period of 5000
 * milliseconds (5 seconds), the progress bar automatically
 * stops and emits the progressStopped() signal.
 */
void CustomProgressBar::stop()
{
    timer_->start(5000);
    emit progressStopped();
}
} // namespace GUI
} // namespace CargoNetSim
