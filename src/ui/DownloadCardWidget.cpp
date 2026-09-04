#include "DownloadCardWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QStyle>

DownloadCardWidget::DownloadCardWidget(const DownloadTask &task, QWidget *parent)
    : QFrame(parent), m_task(task)
{
    setObjectName("downloadCard");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setupUi();
}

QString DownloadCardWidget::taskId() const
{
    return m_task.id;
}

void DownloadCardWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 12, 14, 12);
    mainLayout->setSpacing(10);

    // Top Row: Title, Format Tag, Status Badge
    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(8);

    m_titleLabel = new QLabel(m_task.title.isEmpty() ? m_task.url : m_task.title, this);
    m_titleLabel->setStyleSheet("font-weight: 600; font-size: 13.5px;");
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_formatBadge = new QLabel(m_task.formatLabel, this);
    m_formatBadge->setObjectName("qualityBadge");

    m_statusBadge = new QLabel(m_task.statusString(), this);
    m_statusBadge->setObjectName("statusDownloading");

    topRow->addWidget(m_titleLabel, 1);
    topRow->addWidget(m_formatBadge);
    topRow->addWidget(m_statusBadge);
    mainLayout->addLayout(topRow);

    // Middle Row: Progress Bar
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(static_cast<int>(m_task.progressPercent));
    m_progressBar->setTextVisible(false);
    m_progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mainLayout->addWidget(m_progressBar);

    // Bottom Row: Metrics on the left, Control Buttons on the right
    auto *bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(8);

    m_metricsLabel = new QLabel(this);
    m_metricsLabel->setStyleSheet("font-size: 12px; color: #64748b; font-weight: 500;");
    m_metricsLabel->setText("Initializing...");
    m_metricsLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    bottomRow->addWidget(m_metricsLabel, 1);

    m_pauseResumeBtn = new QPushButton("Pause", this);
    m_pauseResumeBtn->setFixedWidth(80);

    m_cancelBtn = new QPushButton("Cancel", this);
    m_cancelBtn->setObjectName("dangerButton");
    m_cancelBtn->setFixedWidth(80);

    m_playBtn = new QPushButton("Play", this);
    m_playBtn->setObjectName("primaryButton");
    m_playBtn->setFixedWidth(80);
    m_playBtn->setVisible(false);

    m_openFolderBtn = new QPushButton("Show in Folder", this);
    m_openFolderBtn->setFixedWidth(115);
    m_openFolderBtn->setVisible(false);

    bottomRow->addWidget(m_pauseResumeBtn);
    bottomRow->addWidget(m_cancelBtn);
    bottomRow->addWidget(m_playBtn);
    bottomRow->addWidget(m_openFolderBtn);

    mainLayout->addLayout(bottomRow);

    // Signals
    connect(m_pauseResumeBtn, &QPushButton::clicked, this, [this]() {
        if (m_task.status == TaskStatus::Downloading) {
            emit pauseRequested(m_task.id);
        } else if (m_task.status == TaskStatus::Paused) {
            emit resumeRequested(m_task.id);
        }
    });

    connect(m_cancelBtn, &QPushButton::clicked, this, [this]() {
        if (m_task.status == TaskStatus::Completed || m_task.status == TaskStatus::Failed || m_task.status == TaskStatus::Canceled) {
            emit removeRequested(m_task.id);
        } else {
            emit cancelRequested(m_task.id);
        }
    });

    connect(m_playBtn, &QPushButton::clicked, this, &DownloadCardWidget::handlePlayClicked);
    connect(m_openFolderBtn, &QPushButton::clicked, this, &DownloadCardWidget::openFolder);
}

void DownloadCardWidget::updateProgress(double percent, const QString &speed, const QString &eta)
{
    m_task.progressPercent = percent;
    m_task.speedText = speed;
    m_task.etaText = eta;

    m_progressBar->setValue(static_cast<int>(percent));
    m_metricsLabel->setText(QString("%1% • %2 • ETA %3")
                           .arg(percent, 0, 'f', 1)
                           .arg(speed)
                           .arg(eta));
}

void DownloadCardWidget::setStatus(TaskStatus status, const QString &finalPath, const QString &error)
{
    m_task.status = status;
    if (!finalPath.isEmpty()) m_task.targetFilePath = finalPath;
    if (!error.isEmpty()) m_task.errorMessage = error;

    m_statusBadge->setText(m_task.statusString());

    if (status == TaskStatus::Completed) {
        m_statusBadge->setObjectName("statusCompleted");
        m_progressBar->setValue(100);
        m_metricsLabel->setText("Saved: " + QFileInfo(m_task.targetFilePath).fileName());
        m_pauseResumeBtn->setVisible(false);
        m_cancelBtn->setVisible(false);
        m_playBtn->setVisible(true);
        m_openFolderBtn->setVisible(true);
    } else if (status == TaskStatus::Downloading) {
        m_statusBadge->setObjectName("statusDownloading");
        m_pauseResumeBtn->setText("Pause");
        m_pauseResumeBtn->setVisible(true);
        m_cancelBtn->setText("Cancel");
        m_cancelBtn->setVisible(true);
        m_playBtn->setVisible(false);
        m_openFolderBtn->setVisible(false);
    } else if (status == TaskStatus::Paused) {
        m_statusBadge->setObjectName("statusPaused");
        m_pauseResumeBtn->setText("Resume");
        m_pauseResumeBtn->setVisible(true);
        m_cancelBtn->setText("Cancel");
        m_cancelBtn->setVisible(true);
        m_playBtn->setVisible(false);
        m_openFolderBtn->setVisible(false);
    } else if (status == TaskStatus::Failed) {
        m_statusBadge->setObjectName("statusFailed");
        m_metricsLabel->setText(m_task.errorMessage.isEmpty() ? "Download failed." : m_task.errorMessage);
        m_metricsLabel->setStyleSheet("color: #dc2626; font-size: 12px;");
        m_pauseResumeBtn->setVisible(false);
        m_cancelBtn->setText("Dismiss");
        m_cancelBtn->setVisible(true);
        m_playBtn->setVisible(false);
        m_openFolderBtn->setVisible(false);
    } else if (status == TaskStatus::Canceled) {
        m_statusBadge->setObjectName("statusFailed");
        m_metricsLabel->setText("Download canceled.");
        m_pauseResumeBtn->setVisible(false);
        m_cancelBtn->setText("Dismiss");
        m_cancelBtn->setVisible(true);
        m_playBtn->setVisible(false);
        m_openFolderBtn->setVisible(false);
    }

    // Refresh styling on status badge
    m_statusBadge->style()->unpolish(m_statusBadge);
    m_statusBadge->style()->polish(m_statusBadge);
}

void DownloadCardWidget::handlePlayClicked()
{
    if (!m_task.targetFilePath.isEmpty() && QFileInfo::exists(m_task.targetFilePath)) {
        emit playRequested(m_task.targetFilePath, m_task.title, m_task.isAudioOnly());
    }
}

void DownloadCardWidget::openFolder()
{
    if (!m_task.targetFilePath.isEmpty()) {
        QFileInfo fi(m_task.targetFilePath);
        QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absolutePath()));
    }
}
