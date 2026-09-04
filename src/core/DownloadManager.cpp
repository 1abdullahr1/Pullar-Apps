#include "DownloadManager.h"
#include "AppSettings.h"
#include "HistoryStorage.h"

DownloadManager& DownloadManager::instance()
{
    static DownloadManager inst;
    return inst;
}

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent)
{
}

void DownloadManager::enqueueTask(const DownloadTask &task)
{
    m_tasks.append(task);
    emit taskAdded(task);
    emit queueStatusChanged(activeCount(), totalCount());
    processQueue();
}

int DownloadManager::findTaskIndex(const QString &taskId) const
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id == taskId) {
            return i;
        }
    }
    return -1;
}

void DownloadManager::processQueue()
{
    int maxActive = AppSettings::instance().maxConcurrentDownloads();
    int currentActive = activeCount();

    if (currentActive >= maxActive) {
        return;
    }

    for (DownloadTask &task : m_tasks) {
        if (task.status == TaskStatus::Queued && !m_workers.contains(task.id)) {
            task.status = TaskStatus::Downloading;
            auto *worker = new DownloaderWorker(task, this);

            connect(worker, &DownloaderWorker::progressUpdated, this, &DownloadManager::onWorkerProgress);
            connect(worker, &DownloaderWorker::downloadCompleted, this, &DownloadManager::onWorkerCompleted);
            connect(worker, &DownloaderWorker::downloadFailed, this, &DownloadManager::onWorkerFailed);

            m_workers[task.id] = worker;
            worker->start();

            currentActive++;
            emit queueStatusChanged(activeCount(), totalCount());

            if (currentActive >= maxActive) {
                break;
            }
        }
    }
}

void DownloadManager::pauseTask(const QString &taskId)
{
    int idx = findTaskIndex(taskId);
    if (idx >= 0 && m_workers.contains(taskId)) {
        m_workers[taskId]->pause();
        m_workers[taskId]->deleteLater();
        m_workers.remove(taskId);
        m_tasks[idx].status = TaskStatus::Paused;
        m_tasks[idx].speedText = "Paused";
        emit queueStatusChanged(activeCount(), totalCount());
    }
}

void DownloadManager::resumeTask(const QString &taskId)
{
    int idx = findTaskIndex(taskId);
    if (idx >= 0 && m_tasks[idx].status == TaskStatus::Paused) {
        m_tasks[idx].status = TaskStatus::Queued;
        emit queueStatusChanged(activeCount(), totalCount());
        processQueue();
    }
}

void DownloadManager::cancelTask(const QString &taskId)
{
    int idx = findTaskIndex(taskId);
    if (idx >= 0) {
        if (m_workers.contains(taskId)) {
            m_workers[taskId]->cancel();
            m_workers[taskId]->deleteLater();
            m_workers.remove(taskId);
        }
        m_tasks[idx].status = TaskStatus::Canceled;
        m_tasks[idx].speedText = "Canceled";
        emit queueStatusChanged(activeCount(), totalCount());
        processQueue();
    }
}

void DownloadManager::removeTask(const QString &taskId)
{
    cancelTask(taskId);
    int idx = findTaskIndex(taskId);
    if (idx >= 0) {
        m_tasks.removeAt(idx);
        emit queueStatusChanged(activeCount(), totalCount());
    }
}

void DownloadManager::clearCompleted()
{
    for (int i = m_tasks.size() - 1; i >= 0; --i) {
        if (m_tasks[i].status == TaskStatus::Completed || m_tasks[i].status == TaskStatus::Canceled) {
            m_tasks.removeAt(i);
        }
    }
    emit queueStatusChanged(activeCount(), totalCount());
}

const QVector<DownloadTask>& DownloadManager::tasks() const
{
    return m_tasks;
}

int DownloadManager::activeCount() const
{
    int count = 0;
    for (const DownloadTask &t : m_tasks) {
        if (t.status == TaskStatus::Downloading) {
            count++;
        }
    }
    return count;
}

int DownloadManager::totalCount() const
{
    return m_tasks.size();
}

void DownloadManager::onWorkerProgress(const QString &taskId, double percent, const QString &speed, const QString &eta)
{
    int idx = findTaskIndex(taskId);
    if (idx >= 0) {
        m_tasks[idx].progressPercent = percent;
        m_tasks[idx].speedText = speed;
        m_tasks[idx].etaText = eta;
        emit taskProgress(taskId, percent, speed, eta);
    }
}

void DownloadManager::onWorkerCompleted(const QString &taskId, const QString &finalPath)
{
    int idx = findTaskIndex(taskId);
    if (idx >= 0) {
        m_tasks[idx].status = TaskStatus::Completed;
        m_tasks[idx].progressPercent = 100.0;
        m_tasks[idx].targetFilePath = finalPath;
        m_tasks[idx].speedText = "Completed";
        m_tasks[idx].etaText = "00:00";

        // Save to History
        HistoryStorage::instance().recordCompleted(m_tasks[idx]);

        emit taskCompleted(m_tasks[idx]);
    }

    if (m_workers.contains(taskId)) {
        m_workers[taskId]->deleteLater();
        m_workers.remove(taskId);
    }

    emit queueStatusChanged(activeCount(), totalCount());
    processQueue();
}

void DownloadManager::onWorkerFailed(const QString &taskId, const QString &error)
{
    int idx = findTaskIndex(taskId);
    if (idx >= 0) {
        m_tasks[idx].status = TaskStatus::Failed;
        m_tasks[idx].errorMessage = error;
        m_tasks[idx].speedText = "Failed";
        emit taskFailed(taskId, error);
    }

    if (m_workers.contains(taskId)) {
        m_workers[taskId]->deleteLater();
        m_workers.remove(taskId);
    }

    emit queueStatusChanged(activeCount(), totalCount());
    processQueue();
}
