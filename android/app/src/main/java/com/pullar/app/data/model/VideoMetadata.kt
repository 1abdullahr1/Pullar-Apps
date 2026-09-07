package com.pullar.app.data.model

data class PlaylistItem(
    val id: String,
    val title: String,
    val url: String,
    val durationSeconds: Long = 0,
    val durationFormatted: String = "",
    val thumbnailUrl: String = "",
    val index: Int = 0
)

data class VideoMetadata(
    val url: String,
    val title: String,
    val uploader: String = "",
    val durationSeconds: Long = 0,
    val durationFormatted: String = "",
    val thumbnailUrl: String = "",
    val isPlaylist: Boolean = false,
    val playlistCount: Int = 0,
    val playlistItems: List<PlaylistItem> = emptyList(),
    val availableFormats: List<FormatOption> = emptyList()
)
