package com.pullar.app.data.repository

import com.pullar.app.data.dao.DownloadDao
import com.pullar.app.data.model.DownloadEntity
import com.pullar.app.data.model.DownloadStatus
import kotlinx.coroutines.flow.Flow

class DownloadRepository(private val downloadDao: DownloadDao) {

    val activeDownloads: Flow<List<DownloadEntity>> = downloadDao.getActiveDownloads()
    val history: Flow<List<DownloadEntity>> = downloadDao.getHistory()

    fun searchHistory(query: String): Flow<List<DownloadEntity>> {
        return if (query.isBlank()) {
            downloadDao.getHistory()
        } else {
            downloadDao.searchHistory(query)
        }
    }

    suspend fun getById(id: String): DownloadEntity? = downloadDao.getById(id)

    suspend fun getActiveByUrl(url: String): DownloadEntity? = downloadDao.getActiveByUrl(url)

    suspend fun enqueue(download: DownloadEntity) = downloadDao.insert(download)

    suspend fun update(download: DownloadEntity) = downloadDao.update(download)

    suspend fun updateProgress(
        id: String,
        progress: Float,
        speed: String,
        eta: String,
        downloaded: Long,
        total: Long,
        status: DownloadStatus
    ) = downloadDao.updateProgress(id, progress, speed, eta, downloaded, total, status)

    suspend fun updateStatus(id: String, status: DownloadStatus) = downloadDao.updateStatus(id, status)

    suspend fun delete(download: DownloadEntity) = downloadDao.delete(download)

    suspend fun deleteById(id: String) = downloadDao.deleteById(id)

    suspend fun clearHistory() = downloadDao.clearHistory()
}
