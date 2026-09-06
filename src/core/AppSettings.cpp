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
    QSettings s("Pullar", "Pullar");
    QString defDownloads = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    m_downloadFolder = s.value("downloadFolder", "").toString();
    if (m_downloadFolder.isEmpty()) {
        QSettings legacy("OphiraLabs", "VideoDownloader");
        m_downloadFolder = legacy.value("downloadFolder", defDownloads).toString();
    }
    m_maxConcurrentDownloads = s.value("maxConcurrentDownloads", 3).toInt();
    m_defaultQuality = s.value("defaultQuality", "1080p").toString();
    m_autoPasteClipboard = s.value("autoPasteClipboard", true).toBool();
    m_themeMode = s.value("themeMode", "dark").toString();

    if (m_themeMode != "light" && m_themeMode != "dark") {
        m_themeMode = "dark";
    }

    if (m_downloadFolder.isEmpty() || !QDir(m_downloadFolder).exists()) {
        m_downloadFolder = defDownloads;
    }
}

void AppSettings::save()
{
    QSettings s("Pullar", "Pullar");
    s.setValue("downloadFolder", m_downloadFolder);
    s.setValue("maxConcurrentDownloads", m_maxConcurrentDownloads);
    s.setValue("defaultQuality", m_defaultQuality);
    s.setValue("autoPasteClipboard", m_autoPasteClipboard);
    s.setValue("themeMode", m_themeMode);
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

QString AppSettings::themeMode() const
{
    return m_themeMode;
}

void AppSettings::setThemeMode(const QString &theme)
{
    if (m_themeMode != theme && (theme == "light" || theme == "dark")) {
        m_themeMode = theme;
        save();
        emit themeChanged(m_themeMode);
        emit settingsChanged();
    }
}

QString AppSettings::findExecutable(const QString &exeName)
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString filename = exeName;
#ifdef Q_OS_WIN
    if (!filename.endsWith(".exe", Qt::CaseInsensitive)) {
        filename += ".exe";
    }
#endif

    // 1. Same folder as app executable
    QString localPath = QDir(appDir).filePath(filename);
    if (QFileInfo::exists(localPath)) {
        return localPath;
    }

    // 2. Subdirectory tools/ or bin/
    QString binPath = QDir(appDir).filePath("bin/" + filename);
    if (QFileInfo::exists(binPath)) {
        return binPath;
    }

    // 3. Search in system PATH
    return QStandardPaths::findExecutable(exeName);
}
