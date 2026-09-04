#include "MainWindow.h"
#include "AboutDialog.h"
#include "DownloadManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QStatusBar>
#include <QFrame>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Simplest Video Downloader");
    setWindowIcon(QIcon(":/app.png"));
    resize(1140, 740);
    setAcceptDrops(true);

    setupUi();

    connect(&DownloadManager::instance(), &DownloadManager::queueStatusChanged,
            this, &MainWindow::updateQueueBadge);

    connect(m_homePage, &HomePage::downloadStarted, this, [this]() {
        switchPage(1); // Switch to Downloads queue tab
    });
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
    // 1. LEFT SIDEBAR NAVIGATION
    // =========================================================================
    auto *sidebar = new QWidget(centralWidget);
    sidebar->setObjectName("sidebarContainer");
    sidebar->setFixedWidth(230);
    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(12, 20, 12, 16);
    sidebarLayout->setSpacing(6);

    // App Branding in Sidebar
    auto *brandLayout = new QHBoxLayout();
    brandLayout->setContentsMargins(8, 0, 8, 16);
    brandLayout->setSpacing(10);

    auto *logoLabel = new QLabel(sidebar);
    QPixmap icon(":/app.png");
    if (!icon.isNull()) {
        logoLabel->setPixmap(icon.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    brandLayout->addWidget(logoLabel);

    auto *brandTitleLayout = new QVBoxLayout();
    brandTitleLayout->setSpacing(0);
    auto *titleText = new QLabel("Downloader", sidebar);
    titleText->setStyleSheet("font-size: 15px; font-weight: 700; color: #0f172a;");
    auto *subText = new QLabel("Simplest Media Tool", sidebar);
    subText->setStyleSheet("font-size: 11px; color: #64748b;");
    brandTitleLayout->addWidget(titleText);
    brandTitleLayout->addWidget(subText);
    brandLayout->addLayout(brandTitleLayout);
    brandLayout->addStretch();
    sidebarLayout->addLayout(brandLayout);

    // Navigation Buttons
    m_navHomeBtn = new QPushButton("📥   Downloader", sidebar);
    m_navHomeBtn->setObjectName("navButton");
    m_navHomeBtn->setCheckable(true);
    m_navHomeBtn->setChecked(true);

    auto *downloadsBtnRow = new QHBoxLayout();
    m_navDownloadsBtn = new QPushButton("📋   Active Queue", sidebar);
    m_navDownloadsBtn->setObjectName("navButton");
    m_navDownloadsBtn->setCheckable(true);

    m_queueBadge = new QLabel("0", sidebar);
    m_queueBadge->setObjectName("qualityBadge");
    m_queueBadge->setVisible(false);

    m_navHistoryBtn = new QPushButton("📜   History", sidebar);
    m_navHistoryBtn->setObjectName("navButton");
    m_navHistoryBtn->setCheckable(true);

    m_navSettingsBtn = new QPushButton("⚙️   Settings", sidebar);
    m_navSettingsBtn->setObjectName("navButton");
    m_navSettingsBtn->setCheckable(true);

    sidebarLayout->addWidget(m_navHomeBtn);
    sidebarLayout->addWidget(m_navDownloadsBtn);
    sidebarLayout->addWidget(m_navHistoryBtn);
    sidebarLayout->addWidget(m_navSettingsBtn);

    sidebarLayout->addStretch();

    m_navAboutBtn = new QPushButton("ℹ️   About Sorta", sidebar);
    m_navAboutBtn->setObjectName("navButton");
    sidebarLayout->addWidget(m_navAboutBtn);

    rootLayout->addWidget(sidebar);

    // =========================================================================
    // 2. RIGHT STACKED PAGES
    // =========================================================================
    m_pagesStack = new QStackedWidget(centralWidget);
    m_pagesStack->setObjectName("pageContainer");

    m_homePage = new HomePage(m_pagesStack);
    m_downloadsPage = new DownloadsPage(m_pagesStack);
    m_historyPage = new HistoryPage(m_pagesStack);
    m_settingsPage = new SettingsPage(m_pagesStack);

    m_pagesStack->addWidget(m_homePage);       // Index 0
    m_pagesStack->addWidget(m_downloadsPage);  // Index 1
    m_pagesStack->addWidget(m_historyPage);    // Index 2
    m_pagesStack->addWidget(m_settingsPage);   // Index 3

    rootLayout->addWidget(m_pagesStack, 1);

    // Status Bar
    statusBar()->showMessage("Ready. Paste a link to get started.");

    // Navigation Connections
    connect(m_navHomeBtn, &QPushButton::clicked, this, [this]() { switchPage(0); });
    connect(m_navDownloadsBtn, &QPushButton::clicked, this, [this]() { switchPage(1); });
    connect(m_navHistoryBtn, &QPushButton::clicked, this, [this]() { switchPage(2); });
    connect(m_navSettingsBtn, &QPushButton::clicked, this, [this]() { switchPage(3); });
    connect(m_navAboutBtn, &QPushButton::clicked, this, &MainWindow::showAboutDialog);
}

void MainWindow::switchPage(int pageIndex)
{
    m_pagesStack->setCurrentIndex(pageIndex);
    m_navHomeBtn->setChecked(pageIndex == 0);
    m_navDownloadsBtn->setChecked(pageIndex == 1);
    m_navHistoryBtn->setChecked(pageIndex == 2);
    m_navSettingsBtn->setChecked(pageIndex == 3);

    if (pageIndex == 2) {
        m_historyPage->refreshHistory();
    }
}

void MainWindow::updateQueueBadge(int activeCount, int totalCount)
{
    if (activeCount > 0) {
        m_navDownloadsBtn->setText(QString("📋   Active Queue (%1)").arg(activeCount));
    } else if (totalCount > 0) {
        m_navDownloadsBtn->setText(QString("📋   Active Queue (%1)").arg(totalCount));
    } else {
        m_navDownloadsBtn->setText("📋   Active Queue");
    }
}

void MainWindow::showAboutDialog()
{
    AboutDialog dlg(this);
    dlg.exec();
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
    }

    event->acceptProposedAction();
}
