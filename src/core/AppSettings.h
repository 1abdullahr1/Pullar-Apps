#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QString>
#include <QObject>

class AppSettings : public QObject {
    Q_OBJECT

public:
    static AppSettings& instance();

    QString downloadFolder() const;
    void setDownloadFolder(const QString &folder);

    int maxConcurrentDownloads() const;
    void setMaxConcurrentDownloads(int count);

    QString defaultQuality() const;
    void setDefaultQuality(const QString &quality);

    bool autoPasteClipboard() const;
    void setAutoPasteClipboard(bool enable);

    QString themeMode() const;
    void setThemeMode(const QString &theme);

    static QString findExecutable(const QString &exeName);

signals:
    void settingsChanged();
    void themeChanged(const QString &theme);

private:
    explicit AppSettings(QObject *parent = nullptr);
    void load();
    void save();

    QString m_downloadFolder;
    int m_maxConcurrentDownloads = 3;
    QString m_defaultQuality = "1080p";
    bool m_autoPasteClipboard = true;
    QString m_themeMode = "light";
};

#endif // APPSETTINGS_H
