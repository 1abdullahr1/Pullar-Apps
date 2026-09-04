#include "HistoryStorage.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

HistoryStorage& HistoryStorage::instance()
{
    static HistoryStorage inst;
    return inst;
}

HistoryStorage::HistoryStorage(QObject *parent)
    : QObject(parent)
{
    load();
}

QString HistoryStorage::filePath() const
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    return QDir(dataDir).filePath("download_history.json");
}

void HistoryStorage::recordCompleted(const DownloadTask &task)
{
    HistoryItem item;
    item.title = task.title;
    item.url = task.url;
    item.filePath = task.targetFilePath;
    item.format = task.formatName;
    item.completedAt = QDateTime::currentDateTime();

    QFileInfo fi(task.targetFilePath);
    if (fi.exists()) {
        item.fileSizeBytes = fi.size();
    }

    m_items.prepend(item);
    if (m_items.size() > 200) {
        m_items.resize(200);
    }

    save();
    emit historyUpdated();
}

const QVector<HistoryItem>& HistoryStorage::items() const
{
    return m_items;
}

void HistoryStorage::clear()
{
    m_items.clear();
    save();
    emit historyUpdated();
}

void HistoryStorage::removeAt(int index)
{
    if (index >= 0 && index < m_items.size()) {
        m_items.removeAt(index);
        save();
        emit historyUpdated();
    }
}

void HistoryStorage::load()
{
    QFile f(filePath());
    if (!f.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    if (!doc.isArray()) return;

    m_items.clear();
    QJsonArray arr = doc.array();
    for (const QJsonValue &val : arr) {
        QJsonObject obj = val.toObject();
        HistoryItem it;
        it.title = obj["title"].toString();
        it.url = obj["url"].toString();
        it.filePath = obj["filePath"].toString();
        it.format = obj["format"].toString();
        it.fileSizeBytes = obj["size"].toVariant().toLongLong();
        it.completedAt = QDateTime::fromString(obj["completedAt"].toString(), Qt::ISODate);
        m_items.append(it);
    }
}

void HistoryStorage::save()
{
    QJsonArray arr;
    for (const HistoryItem &it : m_items) {
        QJsonObject obj;
        obj["title"] = it.title;
        obj["url"] = it.url;
        obj["filePath"] = it.filePath;
        obj["format"] = it.format;
        obj["size"] = it.fileSizeBytes;
        obj["completedAt"] = it.completedAt.toString(Qt::ISODate);
        arr.append(obj);
    }

    QFile f(filePath());
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(arr).toJson(QJsonDocument::Compact));
        f.close();
    }
}
