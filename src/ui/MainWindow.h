#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>

#include "HomePage.h"
#include "DownloadsPage.h"
#include "HistoryPage.h"
#include "SettingsPage.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

    void applyTheme(const QString &theme);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void switchPage(int pageIndex);
    void updateQueueBadge(int activeCount, int totalCount);
    void showAboutDialog();
    void toggleTheme();

private:
    void setupUi();

    QPushButton *m_navHomeBtn = nullptr;
    QPushButton *m_navDownloadsBtn = nullptr;
    QPushButton *m_navHistoryBtn = nullptr;
    QPushButton *m_navSettingsBtn = nullptr;
    QPushButton *m_navAboutBtn = nullptr;
    QPushButton *m_themeToggleBtn = nullptr;

    QStackedWidget *m_pagesStack = nullptr;
    HomePage *m_homePage = nullptr;
    DownloadsPage *m_downloadsPage = nullptr;
    HistoryPage *m_historyPage = nullptr;
    SettingsPage *m_settingsPage = nullptr;
};

#endif // MAINWINDOW_H
