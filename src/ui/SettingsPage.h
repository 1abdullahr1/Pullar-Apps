#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);

private slots:
    void browseDownloadFolder();
    void onFolderChanged(const QString &text);
    void onConcurrencyChanged(int val);
    void onQualityChanged(const QString &val);
    void onAutoPasteToggled(bool checked);
    void onThemeChanged(int index);
    void refreshEngineStatus();

private:
    void setupUi();

    QLineEdit *m_folderEdit;
    QPushButton *m_browseFolderBtn;
    QSpinBox *m_concurrentSpin;
    QComboBox *m_qualityCombo;
    QCheckBox *m_autoPasteCheck;
    QComboBox *m_themeCombo;

    QLabel *m_ytDlpStatus;
    QLabel *m_ffmpegStatus;
};

#endif // SETTINGSPAGE_H
