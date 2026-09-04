#include "DownloadCardWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>

DownloadCardWidget::DownloadCardWidget(const DownloadTask &task, QWidget *parent)
    : QFrame(parent), m_task(task)
{
    setObjectName("downloadCard");
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
    mainLayout->setSpacing(8);

    // Row 1: Title + Badges
    auto *topRow = new QHBoxLayout();
    m_titleLabel = new QLabel(m_task.title.isEmpty() ? "Downloading Video..." : m_task.title, this);
    m_titleLabel->setStyleSheet("font-size: 13.5px; font-weight: 600; color: #0f172a;");
    m_titleLabel->setWordWrap(true);

    m_formatBadge = new QLabel(m_task.formatName, this);
    m_formatBadge->setObjectName("qualityBadge");

    m_statusBadge = new QLabel(m_task.statusString(), this);
    m_statusBadge->setObjectName("statusDownloading");

    topRow->addWidget(m_titleLabel, 1);
    topRow->addWidget(m_formatBadge);
    topRow->addWidget(m_statusBadge);
    mainLayout->addLayout(topRow);

    // Row 2: Progress Bar
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(static_cast<int>(m_task.progressPercent));
    m_progressBar->setTextVisible(false);
    mainLayout->addWidget(m_progressBar);

    // Row 3: Metrics (Progress %, Speed, ETA) + Actions
    auto *bottomRow = new QHBoxLayout();
    m_metricsLabel = new QLabel(QString("0.0% • %1 • ETA %2").arg(m_task.speedText, m_task.etaText), this);
    m_metricsLabel->setStyleSheet("font-size: 12px; color: #64748b; font-weight: 500;");
    bottomRow->addWidget(m_metricsLabel, 1);

    m_pauseResumeBtn = new QPushButton("Pause", this);
    m_cancelBtn = new QPushButton("Cancel", this);
    m_cancelBtn->setObjectName("dangerButton");

    m_openFileBtn = new QPushButton("Open File", this);
    m_openFileBtn->setObjectName("primaryButton");
    m_openFileBtn->setVisible(false);

    m_openFolderBtn = new QPushButton("Show in Folder", this);
    m_openFolderBtn->setVisible(false);

    bottomRow->addWidget(m_pauseResumeBtn);
    bottomRow->addWidget(m_cancelBtn);
    bottomRow->addWidget(m_openFileBtn);
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
        emit cancelRequested(m_task.id);
    });

    connect(m_openFileBtn, &QPushButton::clicked, this, &DownloadCardWidget::openFile);
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
        m_metricsLabel->setText("Download completed successfully.");
        m_metricsLabel->setStyleSheet("color: #15803d; font-weight: 500; font-size: 12px;");
        m_pauseResumeBtn->setVisible(false);
        m_cancelBtn->setVisible(false);
        m_openFileBtn->setVisible(true);
        m_openFolderBtn->setVisible(true);
    } else if (status == TaskStatus::Paused) {
        m_statusBadge->setObjectName("qualityBadge");
        m_pauseResumeBtn->setText("Resume");
        m_metricsLabel->setText("Download paused.");
    } else if (status == TaskStatus::Downloading) {
        m_statusBadge->setObjectName("statusDownloading");
        m_pauseResumeBtn->setText("Pause");
    } else if (status == TaskStatus::Failed) {
        m_statusBadge->setObjectName("statusFailed");
        m_metricsLabel->setText(m_task.errorMessage.isEmpty() ? "Download failed." : m_task.errorMessage);
        m_metricsLabel->setStyleSheet("color: #dc2626; font-size: 12px;");
        m_pauseResumeBtn->setVisible(false);
        m_cancelBtn->setText("Dismiss");
    } else if (status == TaskStatus::Canceled) {
        m_statusBadge->setObjectName("statusFailed");
        m_metricsLabel->setText("Download canceled.");
        m_pauseResumeBtn->setVisible(false);
        m_cancelBtn->setText("Dismiss");
    }

    // Refresh styling on status badge
    m_statusBadge->style()->unpolish(m_statusBadge);
    m_statusBadge->style()->polish(m_statusBadge);
}

void DownloadCardWidget::openFile()
{
    if (!m_task.targetFilePath.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_task.targetFilePath));
    }
}

void DownloadCardWidget::openFolder()
{
    if (!m_task.targetFilePath.isEmpty()) {
        QString folder = QFileInfo(m_task.targetFilePath).absolutePath();
        QDesktopServices::openUrl(QUrl::fromLocalFile(folder));
    } else if (!m_task.targetFolder.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_task.targetFolder));
    }
}
