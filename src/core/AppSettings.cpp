#include "AppSettings.h"
#include <QSettings>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

AppSettings& AppSettings::instance()
{
    static AppSettings inst;
    return inst;
}

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
{
    load();
}

void AppSettings::load()
{
    QSettings s("AbdullahBhatti", "VideoDownloader");
    QString defDownloads = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    m_downloadFolder = s.value("downloadFolder", defDownloads).toString();
    m_maxConcurrentDownloads = s.value("maxConcurrentDownloads", 3).toInt();
    m_defaultQuality = s.value("defaultQuality", "1080p").toString();
    m_autoPasteClipboard = s.value("autoPasteClipboard", true).toBool();

    if (m_downloadFolder.isEmpty() || !QDir(m_downloadFolder).exists()) {
        m_downloadFolder = defDownloads;
    }
}

void AppSettings::save()
{
    QSettings s("AbdullahBhatti", "VideoDownloader");
    s.setValue("downloadFolder", m_downloadFolder);
    s.setValue("maxConcurrentDownloads", m_maxConcurrentDownloads);
    s.setValue("defaultQuality", m_defaultQuality);
    s.setValue("autoPasteClipboard", m_autoPasteClipboard);
}

QString AppSettings::downloadFolder() const
{
    return m_downloadFolder;
}

void AppSettings::setDownloadFolder(const QString &folder)
{
    if (m_downloadFolder != folder) {
        m_downloadFolder = folder;
        save();
        emit settingsChanged();
    }
}

int AppSettings::maxConcurrentDownloads() const
{
    return m_maxConcurrentDownloads;
}

void AppSettings::setMaxConcurrentDownloads(int count)
{
    if (m_maxConcurrentDownloads != count) {
        m_maxConcurrentDownloads = qBound(1, count, 5);
        save();
        emit settingsChanged();
    }
}

QString AppSettings::defaultQuality() const
{
    return m_defaultQuality;
}

void AppSettings::setDefaultQuality(const QString &quality)
{
    if (m_defaultQuality != quality) {
        m_defaultQuality = quality;
        save();
        emit settingsChanged();
    }
}

bool AppSettings::autoPasteClipboard() const
{
    return m_autoPasteClipboard;
}

void AppSettings::setAutoPasteClipboard(bool enable)
{
    if (m_autoPasteClipboard != enable) {
        m_autoPasteClipboard = enable;
        save();
        emit settingsChanged();
    }
}

QString AppSettings::findExecutable(const QString &exeName)
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString fileName = exeName.endsWith(".exe") ? exeName : exeName + ".exe";

    // 1. Same directory as application exe
    QString candidate1 = QDir(appDir).filePath(fileName);
    if (QFileInfo::exists(candidate1)) {
        return candidate1;
    }

    // 2. Subdirectory 'bin/'
    QString candidate2 = QDir(appDir).filePath("bin/" + fileName);
    if (QFileInfo::exists(candidate2)) {
        return candidate2;
    }

    // 3. System PATH
    QString sysPath = QStandardPaths::findExecutable(exeName);
    if (!sysPath.isEmpty()) {
        return sysPath;
    }

    return fileName; // Fallback to raw command
}
