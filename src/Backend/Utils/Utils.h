#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QString>

namespace CargoNetSim
{
namespace Backend
{
namespace Utils
{

static QString findConfigFilePath()
{
    // First, try to find the config file in the directory
    // beside the executable
    QDir execDir(QCoreApplication::applicationDirPath());
    if (execDir.exists("config/config.xml"))
    {
        return execDir.filePath("config/config.xml");
    }

    // Next, try one directory up (in case the executable is
    // in a bin/ subdirectory)
    QDir parentDir = execDir;
    if (parentDir.cdUp()
        && parentDir.exists("config/config.xml"))
    {
        return parentDir.filePath("config/config.xml");
    }

    // For development: try to find it relative to the
    // source directory
    QDir repoDir(QCoreApplication::applicationDirPath());
    while (!repoDir.exists("config/config.xml")
           && repoDir.cdUp())
    {
        // Keep going up until we find the config directory
        // or hit the root
    }

    if (repoDir.exists("config/config.xml"))
    {
        return repoDir.filePath("config/config.xml");
    }

    // Fallback - create config in user's config location
    QString fallbackPath =
        QStandardPaths::writableLocation(
            QStandardPaths::AppConfigLocation)
        + "/config.xml";

    // Ensure the directory exists
    QDir configDir = QFileInfo(fallbackPath).dir();
    if (!configDir.exists())
    {
        configDir.mkpath(".");
    }

    qWarning()
        << "Config file not found, will create new one at:"
        << fallbackPath;
    return fallbackPath;
}

} // namespace Utils
} // namespace Backend
} // namespace CargoNetSim
