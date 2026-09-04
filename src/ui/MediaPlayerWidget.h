#ifndef MEDIAPLAYERWIDGET_H
#define MEDIAPLAYERWIDGET_H

#include <QFrame>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QStackedWidget>

class MediaPlayerWidget : public QFrame {
    Q_OBJECT

public:
    explicit MediaPlayerWidget(QWidget *parent = nullptr);
    ~MediaPlayerWidget() override;

    void playMedia(const QString &filePath, const QString &title, bool isAudioOnly = false);
    void stopPlayback();

signals:
    void playerClosed();

private slots:
    void togglePlayPause();
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onSliderMoved(int position);
    void onVolumeChanged(int value);
    void toggleMute();
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);

private:
    void setupUi();
    QString formatTime(qint64 ms) const;

    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audioOutput = nullptr;
    QVideoWidget *m_videoWidget = nullptr;

    QStackedWidget *m_displayStack = nullptr;
    QWidget *m_audioPlaceholderWidget = nullptr;
    QLabel *m_audioTitleLabel = nullptr;
    QLabel *m_mediaTitleLabel = nullptr;
    QLabel *m_timeLabel = nullptr;

    QPushButton *m_playPauseBtn = nullptr;
    QPushButton *m_stopBtn = nullptr;
    QPushButton *m_muteBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;

    QSlider *m_seekSlider = nullptr;
    QSlider *m_volumeSlider = nullptr;

    bool m_isSliderSeeking = false;
    float m_previousVolume = 0.8f;
};

#endif // MEDIAPLAYERWIDGET_H
