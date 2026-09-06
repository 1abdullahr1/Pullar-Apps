#include "SettingsPage.h"
#include "AppSettings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QScrollArea>

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    refreshEngineStatus();
}

void SettingsPage::setupUi()
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *container = new QWidget();
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto *rootLayout = new QVBoxLayout(container);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(16);

    // Header Card
    auto *headerCard = new QFrame(container);
    headerCard->setObjectName("cardFrame");
    auto *headerLayout = new QVBoxLayout(headerCard);
    headerLayout->setContentsMargins(16, 12, 16, 12);

    auto *title = new QLabel("Application Settings", headerCard);
    title->setStyleSheet("font-size: 16px; font-weight: 700;");
    auto *sub = new QLabel("Configure default save paths, concurrency limits, theme appearance, and tool integrations", headerCard);
    sub->setStyleSheet("font-size: 12px; color: #64748b;");
    sub->setWordWrap(true);
    headerLayout->addWidget(title);
    headerLayout->addWidget(sub);
    rootLayout->addWidget(headerCard);

    // General Preferences Card
    auto *generalCard = new QFrame(container);
    generalCard->setObjectName("cardFrame");
    auto *generalLayout = new QVBoxLayout(generalCard);
    generalLayout->setContentsMargins(16, 16, 16, 16);
    generalLayout->setSpacing(14);

    auto *generalTitle = new QLabel("General Preferences", generalCard);
    generalTitle->setStyleSheet("font-size: 14px; font-weight: 700;");
    generalLayout->addWidget(generalTitle);

    auto *generalGrid = new QGridLayout();
    generalGrid->setSpacing(12);

    // 1. Download Folder
    generalGrid->addWidget(new QLabel("Default Download Path:", generalCard), 0, 0);
    auto *folderRow = new QHBoxLayout();
    folderRow->setSpacing(8);
    m_folderEdit = new QLineEdit(AppSettings::instance().downloadFolder(), generalCard);
    m_folderEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_browseFolderBtn = new QPushButton("Browse...", generalCard);
    m_browseFolderBtn->setFixedWidth(90);
    folderRow->addWidget(m_folderEdit, 1);
    folderRow->addWidget(m_browseFolderBtn);
    generalGrid->addLayout(folderRow, 0, 1);

    // 2. Concurrency
    generalGrid->addWidget(new QLabel("Max Concurrent Downloads:", generalCard), 1, 0);
    m_concurrentSpin = new QSpinBox(generalCard);
    m_concurrentSpin->setRange(1, 5);
    m_concurrentSpin->setValue(AppSettings::instance().maxConcurrentDownloads());
    m_concurrentSpin->setFixedWidth(90);
    generalGrid->addWidget(m_concurrentSpin, 1, 1, Qt::AlignLeft);

    // 3. Default Quality
    generalGrid->addWidget(new QLabel("Preferred Quality:", generalCard), 2, 0);
    m_qualityCombo = new QComboBox(generalCard);
    m_qualityCombo->addItems({"Best Quality", "1080p", "720p", "480p", "Audio Only (MP3)"});
    m_qualityCombo->setCurrentText(AppSettings::instance().defaultQuality());
    m_qualityCombo->setFixedWidth(180);
    generalGrid->addWidget(m_qualityCombo, 2, 1, Qt::AlignLeft);

    // 4. Theme Selection (Light / Dark)
    generalGrid->addWidget(new QLabel("Interface Appearance:", generalCard), 3, 0);
    m_themeCombo = new QComboBox(generalCard);
    m_themeCombo->addItem("Light Mode", "light");
    m_themeCombo->addItem("Dark Mode", "dark");
    if (AppSettings::instance().themeMode() == "dark") {
        m_themeCombo->setCurrentIndex(1);
    } else {
        m_themeCombo->setCurrentIndex(0);
    }
    m_themeCombo->setFixedWidth(180);
    generalGrid->addWidget(m_themeCombo, 3, 1, Qt::AlignLeft);

    // 5. Auto Paste
    m_autoPasteCheck = new QCheckBox("Automatically paste link from clipboard on window focus", generalCard);
    m_autoPasteCheck->setChecked(AppSettings::instance().autoPasteClipboard());
    generalGrid->addWidget(m_autoPasteCheck, 4, 0, 1, 2);

    generalLayout->addLayout(generalGrid);
    rootLayout->addWidget(generalCard);

    // Engine Diagnostic Card
    auto *toolsCard = new QFrame(container);
    toolsCard->setObjectName("cardFrame");
    auto *toolsLayout = new QVBoxLayout(toolsCard);
    toolsLayout->setContentsMargins(16, 16, 16, 16);
    toolsLayout->setSpacing(12);

    auto *toolsTitle = new QLabel("Media Engine Integration", toolsCard);
    toolsTitle->setStyleSheet("font-size: 14px; font-weight: 700;");
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

    // Support & Software Inquiries Card
    auto *supportCard = new QFrame(container);
    supportCard->setObjectName("cardFrame");
    auto *supportLayout = new QVBoxLayout(supportCard);
    supportLayout->setContentsMargins(16, 16, 16, 16);
    supportLayout->setSpacing(8);

    auto *supportTitle = new QLabel("Technical Help & Project Inquiries", supportCard);
    supportTitle->setStyleSheet("font-size: 14px; font-weight: 700; color: #7cc6fe;");
    supportLayout->addWidget(supportTitle);

    auto *supportDesc = new QLabel(
        "For bug reports, technical help, or consulting regarding software projects:<br>"
        "<a href=\"https://abdullahcs.pages.dev/\" style=\"color: #7cc6fe; text-decoration: none; font-weight: bold;\">"
        "https://abdullahcs.pages.dev/</a>", supportCard);
    supportDesc->setOpenExternalLinks(true);
    supportDesc->setStyleSheet("font-size: 12.5px; line-height: 1.4;");
    supportLayout->addWidget(supportDesc);

    rootLayout->addWidget(supportCard);

    rootLayout->addStretch();

    scrollArea->setWidget(container);
    outerLayout->addWidget(scrollArea);

    // Connections
    connect(m_browseFolderBtn, &QPushButton::clicked, this, &SettingsPage::browseDownloadFolder);
    connect(m_folderEdit, &QLineEdit::textChanged, this, &SettingsPage::onFolderChanged);
    connect(m_concurrentSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsPage::onConcurrencyChanged);
    connect(m_qualityCombo, &QComboBox::currentTextChanged, this, &SettingsPage::onQualityChanged);
    connect(m_autoPasteCheck, &QCheckBox::toggled, this, &SettingsPage::onAutoPasteToggled);
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsPage::onThemeChanged);
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

void SettingsPage::onThemeChanged(int index)
{
    QString mode = m_themeCombo->itemData(index).toString();
    AppSettings::instance().setThemeMode(mode);
}

void SettingsPage::refreshEngineStatus()
{
    QString ytDlp = AppSettings::findExecutable("yt-dlp");
    if (QFileInfo::exists(ytDlp)) {
        m_ytDlpStatus->setText("Ready (" + ytDlp + ")");
        m_ytDlpStatus->setStyleSheet("color: #16a34a; font-weight: 600;");
    } else {
        m_ytDlpStatus->setText("Not Found (Bundled with installer)");
        m_ytDlpStatus->setStyleSheet("color: #d97706;");
    }

    QString ffmpeg = AppSettings::findExecutable("ffmpeg");
    if (QFileInfo::exists(ffmpeg)) {
        m_ffmpegStatus->setText("Ready (" + ffmpeg + ")");
        m_ffmpegStatus->setStyleSheet("color: #16a34a; font-weight: 600;");
    } else {
        m_ffmpegStatus->setText("Not Found (Bundled with installer)");
        m_ffmpegStatus->setStyleSheet("color: #d97706;");
    }
}
