#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QFrame>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "ExtractorEngine.h"
#include "VideoMetadata.h"

class HomePage : public QWidget {
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);

    void setUrl(const QString &url);

signals:
    void downloadStarted();

private slots:
    void pasteFromClipboard();
    void startAnalyze();
    void onAnalysisStarted();
    void onMetadataReady(const VideoMetadata &meta);
    void onAnalysisFailed(const QString &errorMessage);
    void browseDownloadFolder();
    void startDownload();
    void onThumbnailDownloaded(QNetworkReply *reply);

private:
    void setupUi();

    QLineEdit *m_urlEdit;
    QPushButton *m_pasteBtn;
    QPushButton *m_analyzeBtn;
    QLabel *m_statusLabel;

    // Video Preview Card
    QFrame *m_previewCard;
    QLabel *m_thumbLabel;
    QLabel *m_titleLabel;
    QLabel *m_creatorLabel;
    QLabel *m_durationBadge;
    QLabel *m_platformBadge;
    QComboBox *m_formatCombo;
    QLineEdit *m_folderEdit;
    QPushButton *m_browseFolderBtn;
    QPushButton *m_downloadBtn;

    ExtractorEngine *m_extractor;
    VideoMetadata m_currentMeta;
    QNetworkAccessManager *m_netManager;
};

#endif // HOMEPAGE_H
