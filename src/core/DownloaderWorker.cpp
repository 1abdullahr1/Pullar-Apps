#include "DownloaderWorker.h"
#include "AppSettings.h"
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTimer>

DownloaderWorker::DownloaderWorker(const DownloadTask &task, QObject *parent)
    : QObject(parent), m_task(task)
{
    setupProcess();
}

DownloaderWorker::~DownloaderWorker()
{
    cancel();
}

void DownloaderWorker::setupProcess()
{
    if (m_process) {
        m_process->disconnect(this);
        m_process->deleteLater();
        m_process = nullptr;
    }

    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &DownloaderWorker::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &DownloaderWorker::onReadyReadStandardError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &DownloaderWorker::onProcessFinished);
}

QString DownloaderWorker::taskId() const
{
    return m_task.id;
}

void DownloaderWorker::start()
{
    m_isCanceled = false;
    m_isPaused = false;
    m_lastError.clear();

    if (!m_process) {
        setupProcess();
    }

    QString ytDlp = AppSettings::findExecutable("yt-dlp");
    QString ffmpeg = AppSettings::findExecutable("ffmpeg");

    QDir targetDir(m_task.targetFolder);
    if (!targetDir.exists()) {
        targetDir.mkpath(".");
    }

    QString outputTemplate;
    if (m_task.isPlaylist) {
        outputTemplate = targetDir.filePath("%(playlist_title,playlist)s/%(playlist_index)02d - %(title)s.%(ext)s");
    } else {
        outputTemplate = targetDir.filePath("%(title)s.%(ext)s");
    }

    QStringList args;
    args << "--newline" << "--no-warnings";

    // Playlist handling: prevent runaway loops on single videos
    if (m_task.isPlaylist) {
        args << "--yes-playlist";
    } else {
        args << "--no-playlist";
    }

    // High-speed multi-connection segmented downloading scheme (IDM-style)
    args << "--concurrent-fragments" << "16";
    args << "--buffer-size" << "1024K";
    args << "--http-chunk-size" << "10M";
    args << "--retries" << "10";
    args << "--fragment-retries" << "10";
    args << "--file-access-retries" << "5";

    // Check if aria2c is installed for multi-connection HTTP downloads
    QString aria2 = AppSettings::findExecutable("aria2c");
    if (!aria2.isEmpty() && QFileInfo::exists(aria2)) {
        args << "--downloader" << "aria2c"
             << "--downloader-args" << "aria2c:-s 16 -x 16 -k 1M -j 16";
    }

    // Set ffmpeg location if found
    if (QFileInfo::exists(ffmpeg)) {
        args << "--ffmpeg-location" << QFileInfo(ffmpeg).absolutePath();
    }

    // Audio or Video mode
    if (m_task.formatId == "bestaudio/best") {
        args << "-x" << "--audio-format" << "mp3" << "--audio-quality" << "0";
    } else {
        args << "-f" << m_task.formatId << "--merge-output-format" << "mp4";
    }

    args << "-o" << outputTemplate << m_task.url;

    m_process->setWorkingDirectory(m_task.targetFolder);
    m_process->start(ytDlp, args);
}

void DownloaderWorker::cancel()
{
    m_isCanceled = true;

    if (m_process && m_process->state() != QProcess::NotRunning) {
        // Disconnect callbacks so termination never triggers onProcessFinished/failed signals
        m_process->disconnect(this);

#ifdef Q_OS_WIN
        qint64 pid = m_process->processId();
        if (pid > 0) {
            // Asynchronously kill entire process tree (/T) with force (/F)
            QProcess::startDetached("taskkill", QStringList() << "/F" << "/T" << "/PID" << QString::number(pid));
        }
#endif
        m_process->kill();

        // Release process asynchronously without blocking the UI thread
        QProcess *oldProcess = m_process;
        m_process = nullptr;
        connect(oldProcess, &QProcess::finished, oldProcess, &QObject::deleteLater);
        QTimer::singleShot(3000, oldProcess, &QObject::deleteLater);
    }

    emit downloadCanceled(m_task.id);
}

void DownloaderWorker::pause()
{
    m_isPaused = true;
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->disconnect(this);
#ifdef Q_OS_WIN
        qint64 pid = m_process->processId();
        if (pid > 0) {
            QProcess::startDetached("taskkill", QStringList() << "/F" << "/T" << "/PID" << QString::number(pid));
        }
#endif
        m_process->kill();
        QProcess *oldProcess = m_process;
        m_process = nullptr;
        connect(oldProcess, &QProcess::finished, oldProcess, &QObject::deleteLater);
        QTimer::singleShot(3000, oldProcess, &QObject::deleteLater);
    }
}

void DownloaderWorker::resume()
{
    if (m_isPaused) {
        m_isPaused = false;
        start();
    }
}

bool DownloaderWorker::isRunning() const
{
    return m_process && m_process->state() != QProcess::NotRunning;
}

void DownloaderWorker::onReadyReadStandardOutput()
{
    if (!m_process || m_isCanceled) return;

    while (m_process->canReadLine()) {
        QString line = QString::fromUtf8(m_process->readLine()).trimmed();
        parseProgressLine(line);
    }
}

void DownloaderWorker::onReadyReadStandardError()
{
    if (!m_process || m_isCanceled) return;

    QString err = QString::fromUtf8(m_process->readAllStandardError()).trimmed();
    if (!err.isEmpty()) {
        m_lastError = err;
    }
}

void DownloaderWorker::parseProgressLine(const QString &line)
{
    if (m_isCanceled) return;

    // Parse playlist item index if available: [download] Downloading video 3 of 15
    static const QRegularExpression itemRe("\\[download\\]\\s+Downloading\\s+(?:video|item)\\s+(\\d+)\\s+of\\s+(\\d+)");
    auto itemMatch = itemRe.match(line);
    if (itemMatch.hasMatch()) {
        m_currentPlaylistItem = itemMatch.captured(1).toInt();
        m_totalPlaylistItems = itemMatch.captured(2).toInt();
    }

    // Check destination filename
    if (line.contains("Destination:")) {
        QString path = line.section("Destination:", 1).trimmed();
        m_finalFilePath = QDir(m_task.targetFolder).filePath(path);
    } else if (line.contains("Merging formats into")) {
        static const QRegularExpression mergeRe("Merging formats into \"(.*)\"");
        auto match = mergeRe.match(line);
        if (match.hasMatch()) {
            m_finalFilePath = match.captured(1);
        }
        emit progressUpdated(m_task.id, 99.0, "Merging streams...", "00:01");
        return;
    }

    // Example line: [download]  45.2% of 85.34MiB at  4.21MiB/s ETA 00:11
    static const QRegularExpression progRe("\\[download\\]\\s+(\\d+(?:\\.\\d+)?)%\\s+of\\s+~?(\\S+)\\s+at\\s+(\\S+)\\s+ETA\\s+(\\S+)");
    auto match = progRe.match(line);
    if (match.hasMatch()) {
        double percent = match.captured(1).toDouble();
        QString speed = match.captured(3);
        QString eta = match.captured(4);

        if (m_totalPlaylistItems > 0) {
            speed = QString("[%1/%2] %3").arg(m_currentPlaylistItem).arg(m_totalPlaylistItems).arg(speed);
        }
        emit progressUpdated(m_task.id, percent, speed, eta);
    }
}

void DownloaderWorker::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (m_isPaused || m_isCanceled) {
        return;
    }

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        if (m_finalFilePath.isEmpty()) {
            m_finalFilePath = QDir(m_task.targetFolder).filePath(m_task.title + ".mp4");
        }
        emit downloadCompleted(m_task.id, m_finalFilePath);
    } else {
        QString msg = m_lastError.isEmpty() ? "Download process exited with an error." : m_lastError;
        emit downloadFailed(m_task.id, msg);
    }
}
