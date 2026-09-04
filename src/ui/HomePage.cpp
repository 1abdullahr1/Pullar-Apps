#include "HomePage.h"
#include "AppSettings.h"
#include "DownloadManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QClipboard>
#include <QGuiApplication>
#include <QScrollArea>
#include <QFileInfo>
#include <QUrl>

HomePage::HomePage(QWidget *parent)
    : QWidget(parent),
      m_extractor(new ExtractorEngine(this)),
      m_netManager(new QNetworkAccessManager(this))
{
    setupUi();

    connect(m_pasteBtn, &QPushButton::clicked, this, &HomePage::pasteFromClipboard);
    connect(m_analyzeBtn, &QPushButton::clicked, this, &HomePage::startAnalyze);
    connect(m_urlEdit, &QLineEdit::returnPressed, this, &HomePage::startAnalyze);
    connect(m_browseFolderBtn, &QPushButton::clicked, this, &HomePage::browseDownloadFolder);
    connect(m_downloadBtn, &QPushButton::clicked, this, &HomePage::startDownload);

    connect(m_extractor, &ExtractorEngine::analysisStarted, this, &HomePage::onAnalysisStarted);
    connect(m_extractor, &ExtractorEngine::metadataReady, this, &HomePage::onMetadataReady);
    connect(m_extractor, &ExtractorEngine::analysisFailed, this, &HomePage::onAnalysisFailed);

    connect(m_netManager, &QNetworkAccessManager::finished, this, &HomePage::onThumbnailDownloaded);
}

void HomePage::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *container = new QWidget();
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(20);

    // =========================================================================
    // 1. INPUT CARD
    // =========================================================================
    auto *inputCard = new QFrame(container);
    inputCard->setObjectName("cardFrame");
    auto *inputLayout = new QVBoxLayout(inputCard);
    inputLayout->setContentsMargins(20, 20, 20, 20);
    inputLayout->setSpacing(12);

    auto *inputTitle = new QLabel("Download Video or Audio", inputCard);
    inputTitle->setStyleSheet("font-size: 17px; font-weight: 700;");
    inputLayout->addWidget(inputTitle);

    auto *inputSubtitle = new QLabel("Paste a link from YouTube, X, TikTok, Instagram, Facebook, Vimeo, or a direct media stream:", inputCard);
    inputSubtitle->setStyleSheet("font-size: 13px; color: #64748b;");
    inputSubtitle->setWordWrap(true);
    inputLayout->addWidget(inputSubtitle);

    auto *urlRow = new QHBoxLayout();
    urlRow->setSpacing(8);

    m_urlEdit = new QLineEdit(inputCard);
    m_urlEdit->setPlaceholderText("https://www.youtube.com/watch?v=...");
    m_urlEdit->setClearButtonEnabled(true);
    m_urlEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_pasteBtn = new QPushButton("Paste", inputCard);
    m_pasteBtn->setFixedWidth(90);

    m_analyzeBtn = new QPushButton("Analyze", inputCard);
    m_analyzeBtn->setObjectName("primaryButton");
    m_analyzeBtn->setFixedWidth(110);

    urlRow->addWidget(m_urlEdit, 1);
    urlRow->addWidget(m_pasteBtn);
    urlRow->addWidget(m_analyzeBtn);
    inputLayout->addLayout(urlRow);

    // Platform Pills Row
    auto *pillsRow = new QHBoxLayout();
    pillsRow->setSpacing(6);
    QStringList platforms = {"YouTube", "X (Twitter)", "TikTok", "Instagram", "Facebook", "Vimeo", "Direct Streams"};
    for (const QString &p : platforms) {
        auto *pill = new QLabel(p, inputCard);
        pill->setObjectName("platformPill");
        pillsRow->addWidget(pill);
    }
    pillsRow->addStretch();
    inputLayout->addLayout(pillsRow);

    // Status / Message Label
    m_statusLabel = new QLabel(inputCard);
    m_statusLabel->setStyleSheet("font-size: 12.5px; font-weight: 500;");
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setVisible(false);
    inputLayout->addWidget(m_statusLabel);

    layout->addWidget(inputCard);

    // =========================================================================
    // 2. VIDEO PREVIEW & FORMAT SELECTION CARD
    // =========================================================================
    m_previewCard = new QFrame(container);
    m_previewCard->setObjectName("cardFrame");
    auto *previewLayout = new QVBoxLayout(m_previewCard);
    previewLayout->setContentsMargins(20, 20, 20, 20);
    previewLayout->setSpacing(14);

    auto *previewTopRow = new QHBoxLayout();
    previewTopRow->setSpacing(16);

    // Thumbnail Preview Box
    m_thumbLabel = new QLabel(m_previewCard);
    m_thumbLabel->setMinimumSize(220, 124);
    m_thumbLabel->setMaximumSize(280, 158);
    m_thumbLabel->setStyleSheet("background-color: #0f172a; border-radius: 8px; color: #94a3b8; font-weight: 500;");
    m_thumbLabel->setAlignment(Qt::AlignCenter);
    m_thumbLabel->setText("Preview");
    previewTopRow->addWidget(m_thumbLabel);

    // Video Info Box
    auto *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(6);

    auto *badgeRow = new QHBoxLayout();
    badgeRow->setSpacing(8);
    m_platformBadge = new QLabel("YouTube", m_previewCard);
    m_platformBadge->setObjectName("qualityBadge");
    m_durationBadge = new QLabel("00:00", m_previewCard);
    m_durationBadge->setObjectName("platformPill");
    badgeRow->addWidget(m_platformBadge);
    badgeRow->addWidget(m_durationBadge);
    badgeRow->addStretch();
    infoLayout->addLayout(badgeRow);

    m_titleLabel = new QLabel("Video Title", m_previewCard);
    m_titleLabel->setStyleSheet("font-size: 15px; font-weight: 700;");
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    infoLayout->addWidget(m_titleLabel);

    m_creatorLabel = new QLabel("Channel Name", m_previewCard);
    m_creatorLabel->setStyleSheet("font-size: 12.5px; color: #64748b; font-weight: 500;");
    infoLayout->addWidget(m_creatorLabel);

    infoLayout->addStretch();
    previewTopRow->addLayout(infoLayout, 1);
    previewLayout->addLayout(previewTopRow);

    // Separator line
    auto *sep = new QFrame(m_previewCard);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #e2e8f0;");
    previewLayout->addWidget(sep);

    // Format & Save Location Settings
    auto *settingsGrid = new QGridLayout();
    settingsGrid->setSpacing(10);

    auto *formatLabel = new QLabel("Quality & Format:", m_previewCard);
    formatLabel->setStyleSheet("font-weight: 600;");
    m_formatCombo = new QComboBox(m_previewCard);
    m_formatCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto *folderLabel = new QLabel("Download Folder:", m_previewCard);
    folderLabel->setStyleSheet("font-weight: 600;");

    auto *folderRow = new QHBoxLayout();
    folderRow->setSpacing(8);
    m_folderEdit = new QLineEdit(AppSettings::instance().downloadFolder(), m_previewCard);
    m_folderEdit->setReadOnly(true);
    m_folderEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_browseFolderBtn = new QPushButton("Browse...", m_previewCard);
    m_browseFolderBtn->setFixedWidth(90);
    folderRow->addWidget(m_folderEdit, 1);
    folderRow->addWidget(m_browseFolderBtn);

    settingsGrid->addWidget(formatLabel, 0, 0);
    settingsGrid->addWidget(m_formatCombo, 0, 1);
    settingsGrid->addWidget(folderLabel, 1, 0);
    settingsGrid->addLayout(folderRow, 1, 1);

    previewLayout->addLayout(settingsGrid);

    // Action Row
    auto *actionRow = new QHBoxLayout();
    actionRow->addStretch();

    m_downloadBtn = new QPushButton("Start Download", m_previewCard);
    m_downloadBtn->setObjectName("primaryButton");
    m_downloadBtn->setMinimumWidth(180);
    m_downloadBtn->setMinimumHeight(38);
    actionRow->addWidget(m_downloadBtn);

    previewLayout->addLayout(actionRow);
    m_previewCard->setVisible(false); // Initially hidden

    layout->addWidget(m_previewCard);
    layout->addStretch();

    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);
}

void HomePage::setUrl(const QString &url)
{
    m_urlEdit->setText(url);
    startAnalyze();
}

void HomePage::pasteFromClipboard()
{
    QClipboard *cb = QGuiApplication::clipboard();
    if (cb && !cb->text().trimmed().isEmpty()) {
        m_urlEdit->setText(cb->text().trimmed());
        startAnalyze();
    }
}

void HomePage::startAnalyze()
{
    QString url = m_urlEdit->text().trimmed();
    if (url.isEmpty()) {
        m_statusLabel->setText("Please enter or paste a valid video URL.");
        m_statusLabel->setStyleSheet("color: #dc2626;");
        m_statusLabel->setVisible(true);
        return;
    }

    m_statusLabel->setVisible(false);
    m_extractor->analyzeUrl(url);
}

void HomePage::onAnalysisStarted()
{
    m_analyzeBtn->setEnabled(false);
    m_analyzeBtn->setText("Analyzing...");
    m_statusLabel->setText("Fetching video metadata...");
    m_statusLabel->setStyleSheet("color: #2563eb;");
    m_statusLabel->setVisible(true);
    m_previewCard->setVisible(false);
}

void HomePage::onMetadataReady(const VideoMetadata &meta)
{
    m_currentMeta = meta;
    m_analyzeBtn->setEnabled(true);
    m_analyzeBtn->setText("Analyze");
    m_statusLabel->setVisible(false);

    m_titleLabel->setText(meta.title);
    m_creatorLabel->setText(meta.uploader.isEmpty() ? "Unknown Creator" : meta.uploader);
    m_platformBadge->setText(meta.platform);
    m_durationBadge->setText(meta.durationString());

    // Populate format dropdown
    m_formatCombo->clear();
    for (const auto &fmt : meta.formats) {
        QString label = fmt.resolution;
        if (!fmt.note.isEmpty()) {
            label += " (" + fmt.note + ")";
        }
        if (!fmt.extension.isEmpty()) {
            label += " • " + fmt.extension.toUpper();
        }
        m_formatCombo->addItem(label, fmt.formatId);
    }

    // Default to preferred quality
    QString defQ = AppSettings::instance().defaultQuality();
    for (int i = 0; i < m_formatCombo->count(); ++i) {
        if (m_formatCombo->itemText(i).contains(defQ)) {
            m_formatCombo->setCurrentIndex(i);
            break;
        }
    }

    // Reset and fetch thumbnail
    m_thumbLabel->setPixmap(QPixmap());
    m_thumbLabel->setText("Preview");
    if (!meta.thumbnailUrl.isEmpty()) {
        m_netManager->get(QNetworkRequest(QUrl(meta.thumbnailUrl)));
    }

    m_previewCard->setVisible(true);
}

void HomePage::onAnalysisFailed(const QString &errorMessage)
{
    m_analyzeBtn->setEnabled(true);
    m_analyzeBtn->setText("Analyze");
    m_statusLabel->setText(errorMessage);
    m_statusLabel->setStyleSheet("color: #dc2626;");
    m_statusLabel->setVisible(true);
    m_previewCard->setVisible(false);
}

void HomePage::onThumbnailDownloaded(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray imgBytes = reply->readAll();
        QPixmap pix;
        if (pix.loadFromData(imgBytes)) {
            m_thumbLabel->setPixmap(pix.scaled(m_thumbLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        }
    }
    reply->deleteLater();
}

void HomePage::browseDownloadFolder()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Download Folder", m_folderEdit->text());
    if (!dir.isEmpty()) {
        m_folderEdit->setText(dir);
        AppSettings::instance().setDownloadFolder(dir);
    }
}

void HomePage::startDownload()
{
    if (m_currentMeta.url.isEmpty()) return;

    DownloadTask task;
    task.id = DownloadTask::generateId();
    task.url = m_currentMeta.url;
    task.title = m_currentMeta.title;
    task.platform = m_currentMeta.platform;
    task.targetFolder = m_folderEdit->text();
    task.formatId = m_formatCombo->currentData().toString();
    task.formatLabel = m_formatCombo->currentText();

    DownloadManager::instance().enqueueTask(task);
    emit downloadStarted();
}
