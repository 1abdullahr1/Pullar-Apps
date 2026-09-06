#include "HistoryPage.h"
#include "HistoryStorage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QClipboard>
#include <QGuiApplication>
#include <QMessageBox>
#include <QFrame>
#include <QLabel>

HistoryPage::HistoryPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    connect(&HistoryStorage::instance(), &HistoryStorage::historyUpdated, this, &HistoryPage::refreshHistory);
    refreshHistory();
}

void HistoryPage::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(14);

    // Top Header & Controls Card
    auto *headerCard = new QFrame(this);
    headerCard->setObjectName("cardFrame");
    auto *headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(16, 12, 16, 12);
    headerLayout->setSpacing(10);

    auto *titleLayout = new QVBoxLayout();
    auto *titleLabel = new QLabel("Download History", headerCard);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: 700; color: #0f172a;");
    auto *subLabel = new QLabel("Browse and open previously downloaded media files", headerCard);
    subLabel->setStyleSheet("font-size: 12px; color: #64748b;");
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subLabel);
    headerLayout->addLayout(titleLayout);

    headerLayout->addStretch();

    m_searchEdit = new QLineEdit(headerCard);
    m_searchEdit->setPlaceholderText("Search history by title...");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedWidth(240);
    headerLayout->addWidget(m_searchEdit);

    m_clearBtn = new QPushButton("Clear History", headerCard);
    m_clearBtn->setObjectName("dangerButton");
    headerLayout->addWidget(m_clearBtn);
    rootLayout->addWidget(headerCard);

    // History Table Card
    auto *tableCard = new QFrame(this);
    tableCard->setObjectName("cardFrame");
    auto *tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(12, 12, 12, 12);
    tableLayout->setSpacing(10);

    m_table = new QTableWidget(0, 5, tableCard);
    m_table->setHorizontalHeaderLabels({"Title", "Format", "Size", "Date Downloaded", "File Path"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setDefaultSectionSize(34);
    m_table->verticalHeader()->setVisible(false);
    tableLayout->addWidget(m_table);

    // Action Row below table
    auto *actionRow = new QHBoxLayout();
    m_playBtn = new QPushButton("Play Media", tableCard);
    m_playBtn->setObjectName("primaryButton");
    m_playBtn->setEnabled(false);

    m_folderBtn = new QPushButton("Show in Folder", tableCard);
    m_folderBtn->setEnabled(false);

    m_copyUrlBtn = new QPushButton("Copy Link", tableCard);
    m_copyUrlBtn->setEnabled(false);

    actionRow->addWidget(m_playBtn);
    actionRow->addWidget(m_folderBtn);
    actionRow->addWidget(m_copyUrlBtn);
    actionRow->addStretch();
    tableLayout->addLayout(actionRow);

    rootLayout->addWidget(tableCard, 1);

    // Embedded Media Player Widget
    m_playerWidget = new MediaPlayerWidget(this);
    rootLayout->addWidget(m_playerWidget);

    // Connections
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &HistoryPage::onSelectionChanged);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &HistoryPage::onCellDoubleClicked);
    connect(m_playBtn, &QPushButton::clicked, this, &HistoryPage::playSelectedMedia);
    connect(m_folderBtn, &QPushButton::clicked, this, &HistoryPage::openSelectedFolder);
    connect(m_copyUrlBtn, &QPushButton::clicked, this, &HistoryPage::copySelectedUrl);
    connect(m_clearBtn, &QPushButton::clicked, this, &HistoryPage::clearAllHistory);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &HistoryPage::filterHistory);
}

void HistoryPage::refreshHistory()
{
    m_table->setRowCount(0);
    const auto &items = HistoryStorage::instance().items();
    m_table->setRowCount(items.size());

    for (int r = 0; r < items.size(); ++r) {
        const HistoryItem &it = items[r];
        m_table->setItem(r, 0, new QTableWidgetItem(it.title));
        m_table->setItem(r, 1, new QTableWidgetItem(it.format));
        m_table->setItem(r, 2, new QTableWidgetItem(it.formattedSize()));
        m_table->setItem(r, 3, new QTableWidgetItem(it.formattedDate()));
        m_table->setItem(r, 4, new QTableWidgetItem(it.filePath));
    }

    onSelectionChanged();
}

void HistoryPage::onSelectionChanged()
{
    int row = m_table->currentRow();
    bool hasSelection = (row >= 0 && row < m_table->rowCount());
    m_playBtn->setEnabled(hasSelection);
    m_folderBtn->setEnabled(hasSelection);
    m_copyUrlBtn->setEnabled(hasSelection);
}

void HistoryPage::playSelectedMedia()
{
    int row = m_table->currentRow();
    const auto &items = HistoryStorage::instance().items();
    if (row >= 0 && row < items.size()) {
        QString path = items[row].filePath;
        if (QFileInfo::exists(path)) {
            QString ext = QFileInfo(path).suffix().toLower();
            bool isAudio = (ext == "mp3" || ext == "m4a" || ext == "wav" || ext == "flac" || ext == "aac" || ext == "ogg");
            m_playerWidget->playMedia(path, items[row].title, isAudio);
        } else {
            QMessageBox::warning(this, "File Not Found", "The downloaded file could not be found at its original location.");
        }
    }
}

void HistoryPage::onCellDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    m_table->selectRow(row);
    playSelectedMedia();
}

void HistoryPage::openSelectedFolder()
{
    int row = m_table->currentRow();
    const auto &items = HistoryStorage::instance().items();
    if (row >= 0 && row < items.size()) {
        QString path = items[row].filePath;
        QString folder = QFileInfo(path).absolutePath();
        QDesktopServices::openUrl(QUrl::fromLocalFile(folder));
    }
}

void HistoryPage::copySelectedUrl()
{
    int row = m_table->currentRow();
    const auto &items = HistoryStorage::instance().items();
    if (row >= 0 && row < items.size()) {
        QGuiApplication::clipboard()->setText(items[row].url);
        QMessageBox::information(this, "Copied", "Video URL copied to clipboard.");
    }
}

void HistoryPage::clearAllHistory()
{
    if (HistoryStorage::instance().items().isEmpty()) return;

    auto res = QMessageBox::question(this, "Clear History", "Are you sure you want to clear your download history?",
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (res == QMessageBox::Yes) {
        HistoryStorage::instance().clear();
    }
}

void HistoryPage::filterHistory(const QString &query)
{
    for (int r = 0; r < m_table->rowCount(); ++r) {
        QString title = m_table->item(r, 0)->text();
        bool match = title.contains(query, Qt::CaseInsensitive);
        m_table->setRowHidden(r, !match);
    }
}
