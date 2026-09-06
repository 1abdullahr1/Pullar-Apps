#include "ExtractorEngine.h"
#include "AppSettings.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QFileInfo>
#include <QRegularExpression>

ExtractorEngine::ExtractorEngine(QObject *parent)
    : QObject(parent)
{
    m_watchdogTimer = new QTimer(this);
    m_watchdogTimer->setSingleShot(true);
    connect(m_watchdogTimer, &QTimer::timeout, this, &ExtractorEngine::onWatchdogTimeout);

    setupProcess();
}

ExtractorEngine::~ExtractorEngine()
{
    cancel();
}

void ExtractorEngine::setupProcess()
{
    if (m_process) {
        m_process->disconnect(this);
        m_process->deleteLater();
        m_process = nullptr;
    }

    m_process = new QProcess(this);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ExtractorEngine::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &ExtractorEngine::onProcessError);
}

void ExtractorEngine::analyzeUrl(const QString &url)
{
    cancel();

    m_isCanceled = false;
    m_currentUrl = url.trimmed();

    if (m_currentUrl.isEmpty()) {
        emit analysisFailed("Please enter or paste a valid video URL.");
        return;
    }

    emit analysisStarted();

    // Check if it's a direct media link
    QString lower = m_currentUrl.toLower();
    if (lower.endsWith(".mp4") || lower.endsWith(".mkv") || lower.endsWith(".webm") || lower.endsWith(".mp3")) {
        VideoMetadata meta;
        meta.url = m_currentUrl;
        meta.platform = "Direct Media";
        meta.title = QFileInfo(QUrl(m_currentUrl).path()).fileName();
        if (meta.title.isEmpty()) meta.title = "Direct Video Stream";
        meta.uploader = QUrl(m_currentUrl).host();

        VideoFormat bestFmt;
        bestFmt.formatId = "best";
        bestFmt.resolution = "Direct Stream Original";
        bestFmt.extension = QFileInfo(meta.title).suffix();
        meta.formats.append(bestFmt);

        emit metadataReady(meta);
        return;
    }

    if (!m_process) {
        setupProcess();
    }

    QString ytDlpPath = AppSettings::findExecutable("yt-dlp");
    QStringList args;
    args << "--dump-single-json" << "--no-warnings";

    bool isPurePlaylist = m_currentUrl.contains("/playlist") || 
                          (m_currentUrl.contains("list=") && !m_currentUrl.contains("v="));
    if (isPurePlaylist) {
        args << "--flat-playlist";
    } else {
        args << "--no-playlist";
    }

    args << "--skip-download" << "--socket-timeout" << "15" << m_currentUrl;

    m_process->start(ytDlpPath, args);
    m_watchdogTimer->start(25000); // 25 second timeout
}

void ExtractorEngine::cancel()
{
    m_isCanceled = true;
    if (m_watchdogTimer) {
        m_watchdogTimer->stop();
    }

    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->disconnect(this);

#ifdef Q_OS_WIN
        qint64 pid = m_process->processId();
        if (pid > 0) {
            QProcess::startDetached("taskkill", QStringList() << "/F" << "/T" << "/PID" << QString::number(pid));
        }
#endif
        m_process->kill();
        QProcess *oldProc = m_process;
        m_process = nullptr;
        connect(oldProc, &QProcess::finished, oldProc, &QObject::deleteLater);
        QTimer::singleShot(3000, oldProc, &QObject::deleteLater);
    }
}

bool ExtractorEngine::isRunning() const
{
    return m_process && m_process->state() != QProcess::NotRunning;
}

void ExtractorEngine::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (m_watchdogTimer) {
        m_watchdogTimer->stop();
    }

    if (m_isCanceled || !m_process) {
        return;
    }

    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        QString errStr = QString::fromUtf8(m_process->readAllStandardError());
        if (errStr.trimmed().isEmpty()) {
            errStr = "Failed to retrieve video information. Please verify the URL is public and accessible.";
        }
        emit analysisFailed(errStr);
        return;
    }

    QByteArray data = m_process->readAllStandardOutput();
    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseErr);

    if (doc.isNull() || !doc.isObject()) {
        emit analysisFailed("Could not parse media metadata from extractor.");
        return;
    }

    QJsonObject obj = doc.object();
    VideoMetadata meta;
    meta.url = m_currentUrl;
    meta.title = obj["title"].toString("Unknown Title");
    meta.uploader = obj["uploader"].toString(obj["channel"].toString("Unknown Creator"));
    meta.durationSeconds = obj["duration"].toInt(0);
    meta.thumbnailUrl = obj["thumbnail"].toString();
    meta.platform = detectPlatform(m_currentUrl);

    if (obj["_type"].toString() == "playlist" || obj.contains("entries")) {
        meta.isPlaylist = true;
        meta.platform = "YouTube Playlist";
        QJsonArray entries = obj["entries"].toArray();
        meta.playlistCount = entries.isEmpty() ? obj["playlist_count"].toInt(0) : entries.size();
        if (meta.thumbnailUrl.isEmpty() && !entries.isEmpty()) {
            meta.thumbnailUrl = entries.first().toObject()["thumbnail"].toString();
        }
        if (meta.title.isEmpty() || meta.title == "Unknown Title") {
            meta.title = "YouTube Playlist";
        }
    }

    // Standard preset formats
    VideoFormat best;
    best.formatId = "bestvideo+bestaudio/best";
    best.resolution = "Best Available Quality";
    best.extension = "mp4";
    best.note = "Highest Resolution";
    meta.formats.append(best);

    VideoFormat f1080;
    f1080.formatId = "bestvideo[height<=1080]+bestaudio/best[height<=1080]";
    f1080.resolution = "1080p Full HD";
    f1080.extension = "mp4";
    meta.formats.append(f1080);

    VideoFormat f720;
    f720.formatId = "bestvideo[height<=720]+bestaudio/best[height<=720]";
    f720.resolution = "720p HD";
    f720.extension = "mp4";
    meta.formats.append(f720);

    VideoFormat f480;
    f480.formatId = "bestvideo[height<=480]+bestaudio/best[height<=480]";
    f480.resolution = "480p Standard";
    f480.extension = "mp4";
    meta.formats.append(f480);

    VideoFormat mp3;
    mp3.formatId = "bestaudio/best";
    mp3.resolution = "Audio Only (MP3)";
    mp3.extension = "mp3";
    mp3.note = "320 kbps";
    mp3.isAudioOnly = true;
    meta.formats.append(mp3);

    emit metadataReady(meta);
}

void ExtractorEngine::onProcessError(QProcess::ProcessError err)
{
    if (m_isCanceled) return;

    if (m_watchdogTimer) {
        m_watchdogTimer->stop();
    }

    if (err == QProcess::FailedToStart) {
        emit analysisFailed("yt-dlp executable was not found. Please ensure yt-dlp is installed or in the application folder.");
    }
}

void ExtractorEngine::onWatchdogTimeout()
{
    if (isRunning()) {
        cancel();
        emit analysisFailed("Extraction timed out. The server or network connection took too long to respond.");
    }
}

QString ExtractorEngine::detectPlatform(const QString &url)
{
    QString u = url.toLower();
    if (u.contains("youtube.com") || u.contains("youtu.be")) return "YouTube";
    if (u.contains("twitter.com") || u.contains("x.com")) return "X (Twitter)";
    if (u.contains("tiktok.com")) return "TikTok";
    if (u.contains("instagram.com")) return "Instagram";
    if (u.contains("facebook.com") || u.contains("fb.watch")) return "Facebook";
    if (u.contains("vimeo.com")) return "Vimeo";
    if (u.contains("reddit.com")) return "Reddit";
    return "Web Media";
}
