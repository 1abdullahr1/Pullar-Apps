package com.pullar.app.data.model

import androidx.room.Entity
import androidx.room.PrimaryKey

@Entity(tableName = "downloads")
data class DownloadEntity(
    @PrimaryKey
    val id: String,
    val url: String,
    val title: String,
    val uploader: String = "",
    val thumbnailUrl: String = "",
    val formatId: String = "best",
    val qualityLabel: String = "Best Quality",
    val filePath: String = "",
    val downloadedBytes: Long = 0L,
    val totalBytes: Long = 0L,
    val progress: Float = 0f,
    val speed: String = "",
    val eta: String = "",
    val etaFriendly: String = "",
    val status: DownloadStatus = DownloadStatus.QUEUED,
    val isPlaylist: Boolean = false,
    val playlistTitle: String = "",
    val playlistIndex: Int = 0,
    val playlistTotal: Int = 0,
    val isAudioOnly: Boolean = false,
    val errorMessage: String = "",
    val createdAt: Long = System.currentTimeMillis()
)
