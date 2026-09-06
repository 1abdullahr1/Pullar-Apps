#ifndef VIDEOMETADATA_H
#define VIDEOMETADATA_H

#include <QString>
#include <QVector>

struct VideoFormat {
    QString formatId;       // e.g. "best", "1080p", "137+140"
    QString resolution;     // e.g. "1080p", "720p", "Audio Only"
    QString extension;      // e.g. "mp4", "mp3", "m4a"
    qint64 fileSizeBytes = 0;
    QString note;
    bool isAudioOnly = false;

    QString displayName() const {
        QString name = resolution;
        if (!extension.isEmpty()) {
            name += " (" + extension.toUpper() + ")";
        }
        if (!note.isEmpty()) {
            name += " • " + note;
        }
        if (fileSizeBytes > 0) {
            double mb = static_cast<double>(fileSizeBytes) / (1024.0 * 1024.0);
            name += QString(" [~%1 MB]").arg(mb, 0, 'f', 1);
        }
        return name;
    }
};

struct VideoMetadata {
    QString url;
    QString title;
    QString uploader;
    int durationSeconds = 0;
    QString thumbnailUrl;
    QString platform;
    QVector<VideoFormat> formats;
    bool isPlaylist = false;
    int playlistCount = 0;

    QString formattedDuration() const {
        if (isPlaylist) {
            return QString("%1 videos").arg(playlistCount > 0 ? QString::number(playlistCount) : "Playlist");
        }
        if (durationSeconds <= 0) return QString();
        int hours = durationSeconds / 3600;
        int mins = (durationSeconds % 3600) / 60;
        int secs = durationSeconds % 60;
        if (hours > 0) {
            return QString("%1:%2:%3")
                .arg(hours)
                .arg(mins, 2, 10, QChar('0'))
                .arg(secs, 2, 10, QChar('0'));
        }
        return QString("%1:%2")
            .arg(mins)
            .arg(secs, 2, 10, QChar('0'));
    }

    QString durationString() const {
        return formattedDuration();
    }
};

#endif // VIDEOMETADATA_H
