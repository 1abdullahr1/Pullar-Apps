package com.pullar.app.data.dao

import androidx.room.Dao
import androidx.room.Delete
import androidx.room.Insert
import androidx.room.OnConflictStrategy
import androidx.room.Query
import androidx.room.Update
import com.pullar.app.data.model.DownloadEntity
import com.pullar.app.data.model.DownloadStatus
import kotlinx.coroutines.flow.Flow

@Dao
interface DownloadDao {

    @Query("SELECT * FROM downloads WHERE status IN ('QUEUED', 'ANALYZING', 'DOWNLOADING', 'PAUSED') ORDER BY createdAt ASC")
    fun getActiveDownloads(): Flow<List<DownloadEntity>>

    @Query("SELECT * FROM downloads WHERE status = 'COMPLETED' ORDER BY createdAt DESC")
    fun getHistory(): Flow<List<DownloadEntity>>

    @Query("SELECT * FROM downloads WHERE status = 'COMPLETED' AND title LIKE '%' || :query || '%' ORDER BY createdAt DESC")
    fun searchHistory(query: String): Flow<List<DownloadEntity>>

    @Query("SELECT * FROM downloads WHERE id = :id LIMIT 1")
    suspend fun getById(id: String): DownloadEntity?

    @Query("SELECT * FROM downloads WHERE status = 'QUEUED' ORDER BY createdAt ASC LIMIT 1")
    suspend fun getNextQueued(): DownloadEntity?

    @Query("SELECT * FROM downloads WHERE url = :url AND status IN ('QUEUED', 'DOWNLOADING', 'PAUSED') LIMIT 1")
    suspend fun getActiveByUrl(url: String): DownloadEntity?

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insert(download: DownloadEntity)

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insertAll(downloads: List<DownloadEntity>)

    @Update
    suspend fun update(download: DownloadEntity)

    @Query("UPDATE downloads SET progress = :progress, speed = :speed, eta = :eta, etaFriendly = :etaFriendly, downloadedBytes = :downloaded, totalBytes = :total, status = :status WHERE id = :id")
    suspend fun updateProgress(
        id: String,
        progress: Float,
        speed: String,
        eta: String,
        etaFriendly: String,
        downloaded: Long,
        total: Long,
        status: DownloadStatus
    )

    @Query("UPDATE downloads SET filePath = :filePath, progress = 100.0, speed = 'Completed', eta = '', etaFriendly = '', downloadedBytes = :downloaded, totalBytes = :total, status = 'COMPLETED' WHERE id = :id")
    suspend fun updateCompleted(
        id: String,
        filePath: String,
        downloaded: Long,
        total: Long
    )

    @Query("UPDATE downloads SET status = :status WHERE id = :id")
    suspend fun updateStatus(id: String, status: DownloadStatus)

    @Query("UPDATE downloads SET status = :status, errorMessage = :errorMessage WHERE id = :id")
    suspend fun updateFailed(id: String, status: DownloadStatus = DownloadStatus.FAILED, errorMessage: String)

    @Delete
    suspend fun delete(download: DownloadEntity)

    @Query("DELETE FROM downloads WHERE id = :id")
    suspend fun deleteById(id: String)

    @Query("DELETE FROM downloads WHERE status = 'COMPLETED'")
    suspend fun clearHistory()
}
