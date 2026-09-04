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

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void switchPage(int pageIndex);
    void updateQueueBadge(int activeCount, int totalCount);
    void showAboutDialog();

private:
    void setupUi();

    QPushButton *m_navHomeBtn;
    QPushButton *m_navDownloadsBtn;
    QPushButton *m_navHistoryBtn;
    QPushButton *m_navSettingsBtn;
    QPushButton *m_navAboutBtn;

    QLabel *m_queueBadge;

    QStackedWidget *m_pagesStack;
    HomePage *m_homePage;
    DownloadsPage *m_downloadsPage;
    HistoryPage *m_historyPage;
    SettingsPage *m_settingsPage;
};

#endif // MAINWINDOW_H
