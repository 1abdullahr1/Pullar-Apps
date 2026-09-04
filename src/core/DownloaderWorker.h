#ifndef DOWNLOADERWORKER_H
#define DOWNLOADERWORKER_H

#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include "DownloadTask.h"

class DownloaderWorker : public QObject {
    Q_OBJECT

public:
    explicit DownloaderWorker(const DownloadTask &task, QObject *parent = nullptr);
    ~DownloaderWorker() override;

    QString taskId() const;
    void start();
    void cancel();
    void pause();
    void resume();
    bool isRunning() const;

signals:
    void progressUpdated(const QString &taskId, double percent, const QString &speed, const QString &eta);
    void downloadCompleted(const QString &taskId, const QString &finalFilePath);
    void downloadFailed(const QString &taskId, const QString &errorMessage);

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void parseProgressLine(const QString &line);

    DownloadTask m_task;
    QProcess *m_process;
    QString m_finalFilePath;
    QString m_lastError;
    bool m_isPaused = false;
};

#endif // DOWNLOADERWORKER_H
