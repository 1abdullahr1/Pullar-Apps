#include "DownloadsPage.h"
#include "DownloadManager.h"
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>

DownloadsPage::DownloadsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();

    connect(&DownloadManager::instance(), &DownloadManager::taskAdded, this, &DownloadsPage::addTask);
    connect(&DownloadManager::instance(), &DownloadManager::taskProgress, this, &DownloadsPage::updateTaskProgress);
    connect(&DownloadManager::instance(), &DownloadManager::taskCompleted, this, [this](const DownloadTask &t) {
        updateTaskStatus(t.id, TaskStatus::Completed, t.targetFilePath);
    });
    connect(&DownloadManager::instance(), &DownloadManager::taskFailed, this, [this](const QString &id, const QString &err) {
        updateTaskStatus(id, TaskStatus::Failed, QString(), err);
    });
    connect(&DownloadManager::instance(), &DownloadManager::taskCanceled, this, [this](const QString &id) {
        updateTaskStatus(id, TaskStatus::Canceled);
    });
}

void DownloadsPage::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(14);

    // Header Bar
    auto *headerCard = new QFrame(this);
    headerCard->setObjectName("cardFrame");
    auto *headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(16, 12, 16, 12);

    auto *titleLayout = new QVBoxLayout();
    auto *title = new QLabel("Active Downloads Queue", headerCard);
    title->setStyleSheet("font-size: 16px; font-weight: 700;");
    m_countLabel = new QLabel("0 downloads in progress", headerCard);
    m_countLabel->setStyleSheet("font-size: 12px; color: #64748b;");
    titleLayout->addWidget(title);
    titleLayout->addWidget(m_countLabel);

    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    m_clearCompletedBtn = new QPushButton("Clear Finished", headerCard);
    m_clearCompletedBtn->setFixedWidth(130);
    headerLayout->addWidget(m_clearCompletedBtn);

    rootLayout->addWidget(headerCard);

    // Scroll Area for Download Cards
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *container = new QWidget();
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_cardsLayout = new QVBoxLayout(container);
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setSpacing(10);

    // Empty state placeholder (zero emojis)
    m_emptyLabelWidget = new QWidget(container);
    auto *emptyLayout = new QVBoxLayout(m_emptyLabelWidget);
    emptyLayout->setAlignment(Qt::AlignCenter);
    emptyLayout->setContentsMargins(40, 60, 40, 60);

    auto *emptyTitle = new QLabel("No Active Downloads", m_emptyLabelWidget);
    emptyTitle->setStyleSheet("font-size: 17px; font-weight: 700; color: #64748b; margin-bottom: 4px;");
    emptyTitle->setAlignment(Qt::AlignCenter);

    auto *emptyDesc = new QLabel("Enter a video or audio link on the Downloader page to start.", m_emptyLabelWidget);
    emptyDesc->setStyleSheet("font-size: 13px; color: #94a3b8;");
    emptyDesc->setAlignment(Qt::AlignCenter);

    emptyLayout->addWidget(emptyTitle);
    emptyLayout->addWidget(emptyDesc);
    m_cardsLayout->addWidget(m_emptyLabelWidget);

    m_cardsLayout->addStretch();
    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea, 1);

    // Embedded Media Player Widget (docked at the bottom of the page)
    m_playerWidget = new MediaPlayerWidget(this);
    rootLayout->addWidget(m_playerWidget);

    connect(m_clearCompletedBtn, &QPushButton::clicked, this, &DownloadsPage::clearFinished);
}

void DownloadsPage::addTask(const DownloadTask &task)
{
    m_emptyLabelWidget->setVisible(false);

    auto *card = new DownloadCardWidget(task, this);
    m_cards[task.id] = card;

    // Insert before the trailing stretch
    m_cardsLayout->insertWidget(m_cardsLayout->count() - 1, card);

    connect(card, &DownloadCardWidget::pauseRequested, this, [](const QString &id) {
        DownloadManager::instance().pauseTask(id);
    });
    connect(card, &DownloadCardWidget::resumeRequested, this, [](const QString &id) {
        DownloadManager::instance().resumeTask(id);
    });
    connect(card, &DownloadCardWidget::cancelRequested, this, [](const QString &id) {
        DownloadManager::instance().cancelTask(id);
    });
    connect(card, &DownloadCardWidget::removeRequested, this, [this](const QString &id) {
        DownloadManager::instance().removeTask(id);
        if (m_cards.contains(id)) {
            DownloadCardWidget *c = m_cards.take(id);
            c->deleteLater();
        }
        updateHeader();
        if (m_cards.isEmpty()) {
            m_emptyLabelWidget->setVisible(true);
        }
    });
    connect(card, &DownloadCardWidget::playRequested, this, [this](const QString &filePath, const QString &title, bool isAudioOnly) {
        m_playerWidget->playMedia(filePath, title, isAudioOnly);
    });

    updateHeader();
}

void DownloadsPage::updateTaskProgress(const QString &taskId, double percent, const QString &speed, const QString &eta)
{
    if (m_cards.contains(taskId)) {
        m_cards[taskId]->updateProgress(percent, speed, eta);
    }
}

void DownloadsPage::updateTaskStatus(const QString &taskId, TaskStatus status, const QString &path, const QString &err)
{
    if (m_cards.contains(taskId)) {
        m_cards[taskId]->setStatus(status, path, err);
        updateHeader();
    }
}

void DownloadsPage::clearFinished()
{
    DownloadManager::instance().clearCompleted();
    for (auto it = m_cards.begin(); it != m_cards.end();) {
        TaskStatus s = it.value()->status();
        if (s == TaskStatus::Completed || s == TaskStatus::Failed || s == TaskStatus::Canceled) {
            DownloadCardWidget *card = it.value();
            it = m_cards.erase(it);
            card->deleteLater();
        } else {
            ++it;
        }
    }

    if (m_cards.isEmpty()) {
        m_emptyLabelWidget->setVisible(true);
    }
    updateHeader();
}

void DownloadsPage::updateHeader()
{
    int active = DownloadManager::instance().activeCount();
    int total = DownloadManager::instance().totalCount();

    if (active > 0) {
        m_countLabel->setText(QString("%1 active • %2 total in queue").arg(active).arg(total));
    } else {
        m_countLabel->setText(QString("%1 tasks in queue").arg(total));
    }
}
