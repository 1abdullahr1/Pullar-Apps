#include "MediaPlayerWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QUrl>
#include <QFileInfo>
#include <QTime>

MediaPlayerWidget::MediaPlayerWidget(QWidget *parent)
    : QFrame(parent)
{
    setObjectName("mediaPlayerPanel");
    setupUi();

    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    m_player->setVideoOutput(m_videoWidget);

    m_audioOutput->setVolume(0.8f);
    m_volumeSlider->setValue(80);

    connect(m_player, &QMediaPlayer::positionChanged, this, &MediaPlayerWidget::onPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &MediaPlayerWidget::onDurationChanged);
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, &MediaPlayerWidget::onPlaybackStateChanged);

    connect(m_playPauseBtn, &QPushButton::clicked, this, &MediaPlayerWidget::togglePlayPause);
    connect(m_stopBtn, &QPushButton::clicked, this, &MediaPlayerWidget::stopPlayback);
    connect(m_muteBtn, &QPushButton::clicked, this, &MediaPlayerWidget::toggleMute);
    connect(m_closeBtn, &QPushButton::clicked, this, [this]() {
        stopPlayback();
        hide();
        emit playerClosed();
    });

    connect(m_seekSlider, &QSlider::sliderMoved, this, &MediaPlayerWidget::onSliderMoved);
    connect(m_seekSlider, &QSlider::sliderReleased, this, [this]() {
        m_isSliderSeeking = false;
        m_player->setPosition(m_seekSlider->value());
    });

    connect(m_volumeSlider, &QSlider::valueChanged, this, &MediaPlayerWidget::onVolumeChanged);

    hide(); // Hidden until a file is played
}

MediaPlayerWidget::~MediaPlayerWidget()
{
    stopPlayback();
}

void MediaPlayerWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 12, 14, 12);
    mainLayout->setSpacing(10);

    // Top Header: Media title and Close button
    auto *topRow = new QHBoxLayout();
    m_mediaTitleLabel = new QLabel("Now Playing", this);
    m_mediaTitleLabel->setStyleSheet("font-weight: 700; font-size: 13.5px;");
    m_mediaTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_closeBtn = new QPushButton("Close Player", this);
    m_closeBtn->setFixedWidth(110);

    topRow->addWidget(m_mediaTitleLabel);
    topRow->addWidget(m_closeBtn);
    mainLayout->addLayout(topRow);

    // Display Area: Stacked Widget (Video Widget or Audio Placeholder)
    m_displayStack = new QStackedWidget(this);
    m_displayStack->setMinimumHeight(220);
    m_displayStack->setMaximumHeight(360);
    m_displayStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Page 0: Video Output Widget
    m_videoWidget = new QVideoWidget(this);
    m_videoWidget->setStyleSheet("background-color: #000000; border-radius: 6px;");
    m_videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_displayStack->addWidget(m_videoWidget);

    // Page 1: Audio Playback Widget
    m_audioPlaceholderWidget = new QWidget(this);
    auto *audioLayout = new QVBoxLayout(m_audioPlaceholderWidget);
    audioLayout->setAlignment(Qt::AlignCenter);

    auto *audioBadge = new QLabel("Audio Track", m_audioPlaceholderWidget);
    audioBadge->setStyleSheet("font-size: 18px; font-weight: 700; color: #2563eb;");
    audioBadge->setAlignment(Qt::AlignCenter);

    m_audioTitleLabel = new QLabel("Unknown Audio File", m_audioPlaceholderWidget);
    m_audioTitleLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #64748b;");
    m_audioTitleLabel->setAlignment(Qt::AlignCenter);
    m_audioTitleLabel->setWordWrap(true);

    audioLayout->addWidget(audioBadge);
    audioLayout->addWidget(m_audioTitleLabel);
    m_displayStack->addWidget(m_audioPlaceholderWidget);

    mainLayout->addWidget(m_displayStack, 1);

    // Seek Slider Row
    m_seekSlider = new QSlider(Qt::Horizontal, this);
    m_seekSlider->setRange(0, 0);
    m_seekSlider->setCursor(Qt::PointingHandCursor);
    mainLayout->addWidget(m_seekSlider);

    // Controls Row
    auto *controlsRow = new QHBoxLayout();
    controlsRow->setSpacing(8);

    m_playPauseBtn = new QPushButton("Play", this);
    m_playPauseBtn->setObjectName("primaryButton");
    m_playPauseBtn->setFixedWidth(80);

    m_stopBtn = new QPushButton("Stop", this);
    m_stopBtn->setFixedWidth(70);

    m_timeLabel = new QLabel("00:00 / 00:00", this);
    m_timeLabel->setStyleSheet("font-size: 12px; font-weight: 500;");
    m_timeLabel->setFixedWidth(120);

    controlsRow->addWidget(m_playPauseBtn);
    controlsRow->addWidget(m_stopBtn);
    controlsRow->addWidget(m_timeLabel);
    controlsRow->addStretch();

    m_muteBtn = new QPushButton("Mute", this);
    m_muteBtn->setFixedWidth(70);

    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setFixedWidth(100);
    m_volumeSlider->setCursor(Qt::PointingHandCursor);

    controlsRow->addWidget(m_muteBtn);
    controlsRow->addWidget(m_volumeSlider);

    mainLayout->addLayout(controlsRow);
}

void MediaPlayerWidget::playMedia(const QString &filePath, const QString &title, bool isAudioOnly)
{
    if (!QFileInfo::exists(filePath)) {
        return;
    }

    m_mediaTitleLabel->setText(title.isEmpty() ? QFileInfo(filePath).fileName() : title);
    m_audioTitleLabel->setText(m_mediaTitleLabel->text());

    QString ext = QFileInfo(filePath).suffix().toLower();
    bool isAudio = isAudioOnly || (ext == "mp3" || ext == "m4a" || ext == "wav" || ext == "flac" || ext == "aac" || ext == "ogg");

    if (isAudio) {
        m_displayStack->setCurrentIndex(1); // Audio mode
        m_displayStack->setMaximumHeight(140);
        m_displayStack->setMinimumHeight(120);
    } else {
        m_displayStack->setCurrentIndex(0); // Video mode
        m_displayStack->setMinimumHeight(220);
        m_displayStack->setMaximumHeight(360);
    }

    m_player->setSource(QUrl::fromLocalFile(filePath));
    m_player->play();

    show();
}

void MediaPlayerWidget::stopPlayback()
{
    if (m_player) {
        m_player->stop();
    }
    m_playPauseBtn->setText("Play");
    m_seekSlider->setValue(0);
    m_timeLabel->setText("00:00 / " + formatTime(m_player ? m_player->duration() : 0));
}

void MediaPlayerWidget::togglePlayPause()
{
    if (!m_player) return;

    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_player->pause();
    } else {
        m_player->play();
    }
}

void MediaPlayerWidget::onPlaybackStateChanged(QMediaPlayer::PlaybackState state)
{
    if (state == QMediaPlayer::PlayingState) {
        m_playPauseBtn->setText("Pause");
    } else {
        m_playPauseBtn->setText("Play");
    }
}

void MediaPlayerWidget::onPositionChanged(qint64 position)
{
    if (!m_isSliderSeeking) {
        m_seekSlider->setValue(static_cast<int>(position));
    }
    m_timeLabel->setText(formatTime(position) + " / " + formatTime(m_player->duration()));
}

void MediaPlayerWidget::onDurationChanged(qint64 duration)
{
    m_seekSlider->setRange(0, static_cast<int>(duration));
    m_timeLabel->setText(formatTime(m_player->position()) + " / " + formatTime(duration));
}

void MediaPlayerWidget::onSliderMoved(int position)
{
    m_isSliderSeeking = true;
    m_timeLabel->setText(formatTime(position) + " / " + formatTime(m_player->duration()));
}

void MediaPlayerWidget::onVolumeChanged(int value)
{
    float vol = static_cast<float>(value) / 100.0f;
    m_audioOutput->setVolume(vol);
    if (vol > 0.0f) {
        m_previousVolume = vol;
        m_muteBtn->setText("Mute");
    } else {
        m_muteBtn->setText("Unmute");
    }
}

void MediaPlayerWidget::toggleMute()
{
    if (m_audioOutput->volume() > 0.0f) {
        m_previousVolume = m_audioOutput->volume();
        m_audioOutput->setVolume(0.0f);
        m_volumeSlider->setValue(0);
        m_muteBtn->setText("Unmute");
    } else {
        float restored = m_previousVolume > 0.05f ? m_previousVolume : 0.8f;
        m_audioOutput->setVolume(restored);
        m_volumeSlider->setValue(static_cast<int>(restored * 100.0f));
        m_muteBtn->setText("Mute");
    }
}

QString MediaPlayerWidget::formatTime(qint64 ms) const
{
    qint64 totalSeconds = ms / 1000;
    qint64 seconds = totalSeconds % 60;
    qint64 minutes = (totalSeconds / 60) % 60;
    qint64 hours = totalSeconds / 3600;

    if (hours > 0) {
        return QString("%1:%2:%3")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    }

    return QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}
