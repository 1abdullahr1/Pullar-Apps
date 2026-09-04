#include "SettingsPage.h"
#include "AppSettings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    refreshEngineStatus();
}

void SettingsPage::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(16);

    // Header Card
    auto *headerCard = new QFrame(this);
    headerCard->setObjectName("cardFrame");
    auto *headerLayout = new QVBoxLayout(headerCard);
    headerLayout->setContentsMargins(16, 12, 16, 12);

    auto *title = new QLabel("Application Settings", headerCard);
    title->setStyleSheet("font-size: 16px; font-weight: 700; color: #0f172a;");
    auto *sub = new QLabel("Configure default save paths, concurrency limits, and tool integrations", headerCard);
    sub->setStyleSheet("font-size: 12px; color: #64748b;");
    headerLayout->addWidget(title);
    headerLayout->addWidget(sub);
    rootLayout->addWidget(headerCard);

    // Download Preferences Card
    auto *prefCard = new QFrame(this);
    prefCard->setObjectName("cardFrame");
    auto *prefLayout = new QVBoxLayout(prefCard);
    prefLayout->setContentsMargins(18, 16, 18, 16);
    prefLayout->setSpacing(14);

    auto *prefTitle = new QLabel("Download Preferences", prefCard);
    prefTitle->setStyleSheet("font-size: 14px; font-weight: 700; color: #0f172a;");
    prefLayout->addWidget(prefTitle);

    auto *grid = new QGridLayout();
    grid->setSpacing(12);

    // 1. Download Folder
    auto *folderLabel = new QLabel("Default Download Directory:", prefCard);
    folderLabel->setStyleSheet("font-weight: 500; color: #334155;");
    m_folderEdit = new QLineEdit(AppSettings::instance().downloadFolder(), prefCard);
    m_browseFolderBtn = new QPushButton("Browse...", prefCard);
    m_browseFolderBtn->setFixedWidth(90);

    auto *folderRow = new QHBoxLayout();
    folderRow->addWidget(m_folderEdit, 1);
    folderRow->addWidget(m_browseFolderBtn);

    grid->addWidget(folderLabel, 0, 0);
    grid->addLayout(folderRow, 0, 1);

    // 2. Max Concurrent Downloads
    auto *concurrentLabel = new QLabel("Maximum Simultaneous Downloads:", prefCard);
    concurrentLabel->setStyleSheet("font-weight: 500; color: #334155;");
    m_concurrentSpin = new QSpinBox(prefCard);
    m_concurrentSpin->setRange(1, 5);
    m_concurrentSpin->setValue(AppSettings::instance().maxConcurrentDownloads());
    m_concurrentSpin->setFixedWidth(100);

    grid->addWidget(concurrentLabel, 1, 0);
    grid->addWidget(m_concurrentSpin, 1, 1, Qt::AlignLeft);

    // 3. Preferred Quality
    auto *qualityLabel = new QLabel("Preferred Default Quality:", prefCard);
    qualityLabel->setStyleSheet("font-weight: 500; color: #334155;");
    m_qualityCombo = new QComboBox(prefCard);
    m_qualityCombo->addItems({"Best Available (1080p/4K)", "720p HD", "480p SD", "Audio Only (MP3)"});
    m_qualityCombo->setFixedWidth(240);
    grid->addWidget(qualityLabel, 2, 0);
    grid->addWidget(m_qualityCombo, 2, 1, Qt::AlignLeft);

    prefLayout->addLayout(grid);

    // 4. Auto-paste clipboard
    m_autoPasteCheck = new QCheckBox("Automatically detect and paste media links from Windows clipboard", prefCard);
    m_autoPasteCheck->setChecked(AppSettings::instance().autoPasteClipboard());
    prefLayout->addWidget(m_autoPasteCheck);

    rootLayout->addWidget(prefCard);

    // Backend Tools Card
    auto *toolsCard = new QFrame(this);
    toolsCard->setObjectName("cardFrame");
    auto *toolsLayout = new QVBoxLayout(toolsCard);
    toolsLayout->setContentsMargins(18, 16, 18, 16);
    toolsLayout->setSpacing(12);

    auto *toolsTitle = new QLabel("Media Engine Integration", toolsCard);
    toolsTitle->setStyleSheet("font-size: 14px; font-weight: 700; color: #0f172a;");
    toolsLayout->addWidget(toolsTitle);

    auto *toolsGrid = new QGridLayout();
    toolsGrid->setSpacing(10);

    toolsGrid->addWidget(new QLabel("Extraction Core (yt-dlp):", toolsCard), 0, 0);
    m_ytDlpStatus = new QLabel("Detecting...", toolsCard);
    toolsGrid->addWidget(m_ytDlpStatus, 0, 1);

    toolsGrid->addWidget(new QLabel("Media Transcoder (FFmpeg):", toolsCard), 1, 0);
    m_ffmpegStatus = new QLabel("Detecting...", toolsCard);
    toolsGrid->addWidget(m_ffmpegStatus, 1, 1);

    toolsLayout->addLayout(toolsGrid);
    rootLayout->addWidget(toolsCard);

    rootLayout->addStretch();

    // Connections
    connect(m_browseFolderBtn, &QPushButton::clicked, this, &SettingsPage::browseDownloadFolder);
    connect(m_folderEdit, &QLineEdit::textChanged, this, &SettingsPage::onFolderChanged);
    connect(m_concurrentSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsPage::onConcurrencyChanged);
    connect(m_qualityCombo, &QComboBox::currentTextChanged, this, &SettingsPage::onQualityChanged);
    connect(m_autoPasteCheck, &QCheckBox::toggled, this, &SettingsPage::onAutoPasteToggled);
}

void SettingsPage::browseDownloadFolder()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Default Download Directory", m_folderEdit->text());
    if (!dir.isEmpty()) {
        m_folderEdit->setText(dir);
        AppSettings::instance().setDownloadFolder(dir);
    }
}

void SettingsPage::onFolderChanged(const QString &text)
{
    AppSettings::instance().setDownloadFolder(text);
}

void SettingsPage::onConcurrencyChanged(int val)
{
    AppSettings::instance().setMaxConcurrentDownloads(val);
}

void SettingsPage::onQualityChanged(const QString &val)
{
    AppSettings::instance().setDefaultQuality(val);
}

void SettingsPage::onAutoPasteToggled(bool checked)
{
    AppSettings::instance().setAutoPasteClipboard(checked);
}

void SettingsPage::refreshEngineStatus()
{
    QString ytDlp = AppSettings::findExecutable("yt-dlp");
    if (QFileInfo::exists(ytDlp)) {
        m_ytDlpStatus->setText("✓ Ready (" + ytDlp + ")");
        m_ytDlpStatus->setStyleSheet("color: #16a34a; font-weight: 600;");
    } else {
        m_ytDlpStatus->setText("⚠ Not found (Will be bundled with installer)");
        m_ytDlpStatus->setStyleSheet("color: #d97706;");
    }

    QString ffmpeg = AppSettings::findExecutable("ffmpeg");
    if (QFileInfo::exists(ffmpeg)) {
        m_ffmpegStatus->setText("✓ Ready (" + ffmpeg + ")");
        m_ffmpegStatus->setStyleSheet("color: #16a34a; font-weight: 600;");
    } else {
        m_ffmpegStatus->setText("⚠ Not found (Will be bundled with installer)");
        m_ffmpegStatus->setStyleSheet("color: #d97706;");
    }
}
