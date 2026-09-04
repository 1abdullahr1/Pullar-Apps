#include "DownloaderWorker.h"
#include "AppSettings.h"
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

DownloaderWorker::DownloaderWorker(const DownloadTask &task, QObject *parent)
    : QObject(parent), m_task(task), m_process(new QProcess(this))
{
    connect(m_process, &QProcess::readyReadStandardOutput, this, &DownloaderWorker::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &DownloaderWorker::onReadyReadStandardError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &DownloaderWorker::onProcessFinished);
}

DownloaderWorker::~DownloaderWorker()
{
    cancel();
}

QString DownloaderWorker::taskId() const
{
    return m_task.id;
}

void DownloaderWorker::start()
{
    QString ytDlp = AppSettings::findExecutable("yt-dlp");
    QString ffmpeg = AppSettings::findExecutable("ffmpeg");

    QDir targetDir(m_task.targetFolder);
    if (!targetDir.exists()) {
        targetDir.mkpath(".");
    }

    QString outputTemplate = targetDir.filePath("%(title)s.%(ext)s");

    QStringList args;
    args << "--newline" << "--no-warnings";

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
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(500);
    }
}

void DownloaderWorker::pause()
{
    // Suspend or kill process gracefully; yt-dlp automatically resumes on re-start
    cancel();
    m_isPaused = true;
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
    return m_process->state() != QProcess::NotRunning;
}

void DownloaderWorker::onReadyReadStandardOutput()
{
    while (m_process->canReadLine()) {
        QString line = QString::fromUtf8(m_process->readLine()).trimmed();
        parseProgressLine(line);
    }
}

void DownloaderWorker::onReadyReadStandardError()
{
    QString err = QString::fromUtf8(m_process->readAllStandardError()).trimmed();
    if (!err.isEmpty()) {
        m_lastError = err;
    }
}

void DownloaderWorker::parseProgressLine(const QString &line)
{
    // Check destination filename
    if (line.contains("Destination:")) {
        QString path = line.section("Destination:", 1).trimmed();
        m_finalFilePath = QDir(m_task.targetFolder).filePath(path);
    } else if (line.contains("Merging formats into")) {
        static const QRegularExpression mergeRe(R"(Merging formats into "(.*)")");
        auto match = mergeRe.match(line);
        if (match.hasMatch()) {
            m_finalFilePath = match.captured(1);
        }
    }

    // Example line: [download]  45.2% of 85.34MiB at  4.21MiB/s ETA 00:11
    static const QRegularExpression progRe(R"(\[download\]\s+(\d+(?:\.\d+)?)%\s+of\s+~?(\S+)\s+at\s+(\S+)\s+ETA\s+(\S+))");
    auto match = progRe.match(line);
    if (match.hasMatch()) {
        double percent = match.captured(1).toDouble();
        QString speed = match.captured(3);
        QString eta = match.captured(4);
        emit progressUpdated(m_task.id, percent, speed, eta);
    }
}

void DownloaderWorker::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (m_isPaused) {
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
