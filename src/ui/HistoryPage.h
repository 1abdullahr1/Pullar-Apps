#ifndef HISTORYPAGE_H
#define HISTORYPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>

class HistoryPage : public QWidget {
    Q_OBJECT

public:
    explicit HistoryPage(QWidget *parent = nullptr);

public slots:
    void refreshHistory();

private slots:
    void onSelectionChanged();
    void openSelectedFile();
    void openSelectedFolder();
    void copySelectedUrl();
    void clearAllHistory();
    void filterHistory(const QString &query);

private:
    void setupUi();

    QLineEdit *m_searchEdit;
    QTableWidget *m_table;
    QPushButton *m_playBtn;
    QPushButton *m_folderBtn;
    QPushButton *m_copyUrlBtn;
    QPushButton *m_clearBtn;
};

#endif // HISTORYPAGE_H
