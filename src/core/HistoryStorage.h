#ifndef HISTORYSTORAGE_H
#define HISTORYSTORAGE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QDateTime>
#include "DownloadTask.h"

struct HistoryItem {
    QString title;
    QString url;
    QString filePath;
    QString format;
    qint64 fileSizeBytes = 0;
    QDateTime completedAt;

    QString formattedDate() const {
        return completedAt.toString("yyyy-MM-dd hh:mm");
    }

    QString formattedSize() const {
        double mb = static_cast<double>(fileSizeBytes) / (1024.0 * 1024.0);
        return QString("%1 MB").arg(mb, 0, 'f', 1);
    }
};

class HistoryStorage : public QObject {
    Q_OBJECT

public:
    static HistoryStorage& instance();

    void recordCompleted(const DownloadTask &task);
    const QVector<HistoryItem>& items() const;
    void clear();
    void removeAt(int index);

signals:
    void historyUpdated();

private:
    explicit HistoryStorage(QObject *parent = nullptr);
    void load();
    void save();
    QString filePath() const;

    QVector<HistoryItem> m_items;
};

#endif // HISTORYSTORAGE_H
