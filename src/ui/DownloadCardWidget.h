#ifndef DOWNLOADCARDWIDGET_H
#define DOWNLOADCARDWIDGET_H

#include <QFrame>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include "DownloadTask.h"

class DownloadCardWidget : public QFrame {
    Q_OBJECT

public:
    explicit DownloadCardWidget(const DownloadTask &task, QWidget *parent = nullptr);

    QString taskId() const;
    TaskStatus status() const { return m_task.status; }
    void updateProgress(double percent, const QString &speed, const QString &eta);
    void setStatus(TaskStatus status, const QString &finalPath = QString(), const QString &error = QString());

signals:
    void pauseRequested(const QString &taskId);
    void resumeRequested(const QString &taskId);
    void cancelRequested(const QString &taskId);
    void removeRequested(const QString &taskId);
    void playRequested(const QString &filePath, const QString &title, bool isAudioOnly);

private slots:
    void handlePlayClicked();
    void openFolder();

private:
    void setupUi();

    DownloadTask m_task;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_formatBadge = nullptr;
    QLabel *m_statusBadge = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QLabel *m_metricsLabel = nullptr;

    QPushButton *m_pauseResumeBtn = nullptr;
    QPushButton *m_cancelBtn = nullptr;
    QPushButton *m_playBtn = nullptr;
    QPushButton *m_openFolderBtn = nullptr;
};

#endif // DOWNLOADCARDWIDGET_H
