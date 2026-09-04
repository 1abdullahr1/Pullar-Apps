#ifndef EXTRACTORENGINE_H
#define EXTRACTORENGINE_H

#include <QObject>
#include <QProcess>
#include "VideoMetadata.h"

class ExtractorEngine : public QObject {
    Q_OBJECT

public:
    explicit ExtractorEngine(QObject *parent = nullptr);
    ~ExtractorEngine() override;

    void analyzeUrl(const QString &url);
    void cancel();
    bool isRunning() const;

    static QString detectPlatform(const QString &url);

signals:
    void analysisStarted();
    void metadataReady(const VideoMetadata &metadata);
    void analysisFailed(const QString &errorMessage);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError err);

private:
    QProcess *m_process;
    QString m_currentUrl;
    QByteArray m_outputBuffer;
};

#endif // EXTRACTORENGINE_H
