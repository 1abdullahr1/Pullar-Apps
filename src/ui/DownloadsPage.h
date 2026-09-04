#ifndef DOWNLOADSPAGE_H
#define DOWNLOADSPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMap>
#include "DownloadCardWidget.h"
#include "DownloadTask.h"
#include "MediaPlayerWidget.h"

class DownloadsPage : public QWidget {
    Q_OBJECT

public:
    explicit DownloadsPage(QWidget *parent = nullptr);

public slots:
    void addTask(const DownloadTask &task);
    void updateTaskProgress(const QString &taskId, double percent, const QString &speed, const QString &eta);
    void updateTaskStatus(const QString &taskId, TaskStatus status, const QString &path = QString(), const QString &err = QString());

private slots:
    void clearFinished();
    void updateHeader();

private:
    void setupUi();

    QVBoxLayout *m_cardsLayout = nullptr;
    QWidget *m_emptyLabelWidget = nullptr;
    QLabel *m_countLabel = nullptr;
    QPushButton *m_clearCompletedBtn = nullptr;
    QMap<QString, DownloadCardWidget*> m_cards;

    MediaPlayerWidget *m_playerWidget = nullptr;
};

#endif // DOWNLOADSPAGE_H
