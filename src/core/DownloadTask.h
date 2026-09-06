#ifndef DOWNLOADTASK_H
#define DOWNLOADTASK_H

#include <QString>
#include <QDateTime>
#include <QRandomGenerator>

enum class TaskStatus {
    Queued,
    Downloading,
    Paused,
    Completed,
    Failed,
    Canceled
};

struct DownloadTask {
    QString id;
    QString url;
    QString title;
    QString platform;
    QString uploader;
    QString thumbnailUrl;
    QString targetFolder;
    QString targetFilePath;
    QString formatId;
    QString formatLabel;
    QString formatName;
    bool isPlaylist = false;
    int playlistItemCount = 0;

    double progressPercent = 0.0;
    QString speedText = "0 KB/s";
    QString etaText = "--:--";
    qint64 downloadedBytes = 0;
    qint64 totalBytes = 0;

    TaskStatus status = TaskStatus::Queued;
    QString errorMessage;
    QDateTime createdAt;

    DownloadTask() : createdAt(QDateTime::currentDateTime()) {}

    static QString generateId() {
        return QString::number(QDateTime::currentMSecsSinceEpoch()) + "_" +
               QString::number(QRandomGenerator::global()->generate() % 10000);
    }

    bool isAudioOnly() const {
        return formatId.contains("bestaudio") ||
               formatLabel.contains("Audio", Qt::CaseInsensitive) ||
               targetFilePath.endsWith(".mp3", Qt::CaseInsensitive) ||
               targetFilePath.endsWith(".m4a", Qt::CaseInsensitive);
    }

    QString statusString() const {
        switch (status) {
            case TaskStatus::Queued:      return "Queued";
            case TaskStatus::Downloading: return "Downloading";
            case TaskStatus::Paused:      return "Paused";
            case TaskStatus::Completed:   return "Completed";
            case TaskStatus::Failed:      return "Failed";
            case TaskStatus::Canceled:    return "Canceled";
        }
        return QString();
    }

    QString formattedProgress() const {
        if (totalBytes > 0) {
            double dlMb = static_cast<double>(downloadedBytes) / (1024.0 * 1024.0);
            double totMb = static_cast<double>(totalBytes) / (1024.0 * 1024.0);
            return QString("%1 MB / %2 MB (%3%)")
                .arg(dlMb, 0, 'f', 1)
                .arg(totMb, 0, 'f', 1)
                .arg(progressPercent, 0, 'f', 1);
        }
        return QString("%1%").arg(progressPercent, 0, 'f', 1);
    }
};

#endif // DOWNLOADTASK_H
