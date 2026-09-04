#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include "DownloadTask.h"
#include "DownloaderWorker.h"

class DownloadManager : public QObject {
    Q_OBJECT

public:
    static DownloadManager& instance();

    void enqueueTask(const DownloadTask &task);
    void pauseTask(const QString &taskId);
    void resumeTask(const QString &taskId);
    void cancelTask(const QString &taskId);
    void removeTask(const QString &taskId);
    void clearCompleted();

    const QVector<DownloadTask>& tasks() const;
    int activeCount() const;
    int totalCount() const;

signals:
    void taskAdded(const DownloadTask &task);
    void taskProgress(const QString &taskId, double percent, const QString &speed, const QString &eta);
    void taskCompleted(const DownloadTask &task);
    void taskFailed(const QString &taskId, const QString &error);
    void queueStatusChanged(int activeCount, int totalCount);

private slots:
    void onWorkerProgress(const QString &taskId, double percent, const QString &speed, const QString &eta);
    void onWorkerCompleted(const QString &taskId, const QString &finalPath);
    void onWorkerFailed(const QString &taskId, const QString &error);

private:
    explicit DownloadManager(QObject *parent = nullptr);
    void processQueue();
    int findTaskIndex(const QString &taskId) const;

    QVector<DownloadTask> m_tasks;
    QMap<QString, DownloaderWorker*> m_workers;
};

#endif // DOWNLOADMANAGER_H
