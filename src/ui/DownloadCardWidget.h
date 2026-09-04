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
    void updateProgress(double percent, const QString &speed, const QString &eta);
    void setStatus(TaskStatus status, const QString &finalPath = QString(), const QString &error = QString());

signals:
    void pauseRequested(const QString &taskId);
    void resumeRequested(const QString &taskId);
    void cancelRequested(const QString &taskId);
    void removeRequested(const QString &taskId);

private slots:
    void openFile();
    void openFolder();

private:
    void setupUi();

    DownloadTask m_task;
    QLabel *m_titleLabel;
    QLabel *m_formatBadge;
    QLabel *m_statusBadge;
    QProgressBar *m_progressBar;
    QLabel *m_metricsLabel;

    QPushButton *m_pauseResumeBtn;
    QPushButton *m_cancelBtn;
    QPushButton *m_openFileBtn;
    QPushButton *m_openFolderBtn;
};

#endif // DOWNLOADCARDWIDGET_H
