#include "MainWindow.h"
#include "AboutDialog.h"
#include "DownloadManager.h"
#include "AppSettings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QStatusBar>
#include <QFrame>
#include <QFile>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Simplest Video Downloader");
    setWindowIcon(QIcon(":/app.png"));
    setMinimumSize(880, 560);
    resize(1140, 740);
    setAcceptDrops(true);

    setupUi();

    connect(&DownloadManager::instance(), &DownloadManager::queueStatusChanged,
            this, &MainWindow::updateQueueBadge);

    connect(m_homePage, &HomePage::downloadStarted, this, [this]() {
        switchPage(1); // Switch to Downloads queue tab
    });

    connect(&AppSettings::instance(), &AppSettings::themeChanged, this, &MainWindow::applyTheme);

    // Initialize with current theme
    applyTheme(AppSettings::instance().themeMode());
}

void MainWindow::setupUi()
{
    auto *centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);

    auto *rootLayout = new QHBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // =========================================================================
    // SIDEBAR
    // =========================================================================
    auto *sidebar = new QWidget(centralWidget);
    sidebar->setObjectName("sidebarContainer");
    sidebar->setFixedWidth(220);

    auto *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(12, 20, 12, 16);
    sideLayout->setSpacing(6);

    // Brand Header
    auto *brandLayout = new QHBoxLayout();
    brandLayout->setContentsMargins(10, 0, 10, 14);

    auto *brandIcon = new QLabel(sidebar);
    QPixmap iconPix(":/app.png");
    if (!iconPix.isNull()) {
        brandIcon->setPixmap(iconPix.scaled(28, 28, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    brandLayout->addWidget(brandIcon);

    auto *brandTextLayout = new QVBoxLayout();
    brandTextLayout->setSpacing(0);

    auto *brandTitle = new QLabel("Video Downloader", sidebar);
    brandTitle->setStyleSheet("font-weight: 700; font-size: 14px;");
    auto *brandSub = new QLabel("by Ophira Labs", sidebar);
    brandSub->setStyleSheet("font-size: 11px; color: #2563eb; font-weight: 600;");

    brandTextLayout->addWidget(brandTitle);
    brandTextLayout->addWidget(brandSub);
    brandLayout->addLayout(brandTextLayout);
    brandLayout->addStretch();
    sideLayout->addLayout(brandLayout);

    // Separator line
    auto *sep = new QFrame(sidebar);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #e2e8f0; margin-bottom: 6px;");
    sideLayout->addWidget(sep);

    // Navigation Buttons (Zero emojis)
    m_navHomeBtn = new QPushButton("Downloader", sidebar);
    m_navHomeBtn->setObjectName("navButton");
    m_navHomeBtn->setCheckable(true);
    m_navHomeBtn->setChecked(true);

    m_navDownloadsBtn = new QPushButton("Downloads", sidebar);
    m_navDownloadsBtn->setObjectName("navButton");
    m_navDownloadsBtn->setCheckable(true);

    m_navHistoryBtn = new QPushButton("History", sidebar);
    m_navHistoryBtn->setObjectName("navButton");
    m_navHistoryBtn->setCheckable(true);

    m_navSettingsBtn = new QPushButton("Settings", sidebar);
    m_navSettingsBtn->setObjectName("navButton");
    m_navSettingsBtn->setCheckable(true);

    sideLayout->addWidget(m_navHomeBtn);
    sideLayout->addWidget(m_navDownloadsBtn);
    sideLayout->addWidget(m_navHistoryBtn);
    sideLayout->addWidget(m_navSettingsBtn);

    sideLayout->addStretch();

    // Theme Switcher Quick Button
    m_themeToggleBtn = new QPushButton("Dark Theme", sidebar);
    m_themeToggleBtn->setObjectName("navButton");
    sideLayout->addWidget(m_themeToggleBtn);

    // About Button
    m_navAboutBtn = new QPushButton("About App", sidebar);
    m_navAboutBtn->setObjectName("navButton");
    sideLayout->addWidget(m_navAboutBtn);

    rootLayout->addWidget(sidebar);

    // =========================================================================
    // MAIN CONTENT STACK
    // =========================================================================
    m_pagesStack = new QStackedWidget(centralWidget);
    m_pagesStack->setObjectName("pageContainer");
    m_pagesStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_homePage = new HomePage(m_pagesStack);
    m_downloadsPage = new DownloadsPage(m_pagesStack);
    m_historyPage = new HistoryPage(m_pagesStack);
    m_settingsPage = new SettingsPage(m_pagesStack);

    m_pagesStack->addWidget(m_homePage);       // Index 0
    m_pagesStack->addWidget(m_downloadsPage);  // Index 1
    m_pagesStack->addWidget(m_historyPage);    // Index 2
    m_pagesStack->addWidget(m_settingsPage);   // Index 3

    rootLayout->addWidget(m_pagesStack, 1);

    // Connections
    connect(m_navHomeBtn, &QPushButton::clicked, this, [this]() { switchPage(0); });
    connect(m_navDownloadsBtn, &QPushButton::clicked, this, [this]() { switchPage(1); });
    connect(m_navHistoryBtn, &QPushButton::clicked, this, [this]() { switchPage(2); });
    connect(m_navSettingsBtn, &QPushButton::clicked, this, [this]() { switchPage(3); });
    connect(m_navAboutBtn, &QPushButton::clicked, this, &MainWindow::showAboutDialog);
    connect(m_themeToggleBtn, &QPushButton::clicked, this, &MainWindow::toggleTheme);

    // Status bar
    statusBar()->setSizeGripEnabled(true);
}

void MainWindow::switchPage(int pageIndex)
{
    m_pagesStack->setCurrentIndex(pageIndex);

    m_navHomeBtn->setChecked(pageIndex == 0);
    m_navDownloadsBtn->setChecked(pageIndex == 1);
    m_navHistoryBtn->setChecked(pageIndex == 2);
    m_navSettingsBtn->setChecked(pageIndex == 3);
}

void MainWindow::updateQueueBadge(int activeCount, int totalCount)
{
    if (activeCount > 0) {
        m_navDownloadsBtn->setText(QString("Downloads (%1)").arg(activeCount));
    } else if (totalCount > 0) {
        m_navDownloadsBtn->setText(QString("Downloads (%1)").arg(totalCount));
    } else {
        m_navDownloadsBtn->setText("Downloads");
    }
}

void MainWindow::showAboutDialog()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::toggleTheme()
{
    QString current = AppSettings::instance().themeMode();
    QString next = (current == "dark") ? "light" : "dark";
    AppSettings::instance().setThemeMode(next);
}

void MainWindow::applyTheme(const QString &theme)
{
    QString qssPath = (theme == "dark") ? ":/styles/dark.qss" : ":/styles/light.qss";
    QFile qssFile(qssPath);
    if (qssFile.open(QFile::ReadOnly | QFile::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(qssFile.readAll()));
    }

    if (m_themeToggleBtn) {
        if (theme == "dark") {
            m_themeToggleBtn->setText("Light Theme");
        } else {
            m_themeToggleBtn->setText("Dark Theme");
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText() || event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QString text;
    if (event->mimeData()->hasUrls() && !event->mimeData()->urls().isEmpty()) {
        text = event->mimeData()->urls().first().toString();
    } else if (event->mimeData()->hasText()) {
        text = event->mimeData()->text();
    }

    if (!text.isEmpty()) {
        switchPage(0);
        m_homePage->setUrl(text);
        event->acceptProposedAction();
    }
}
