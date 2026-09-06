package com.pullar.app.data.model

data class VideoMetadata(
    val url: String,
    val title: String,
    val uploader: String = "",
    val durationSeconds: Long = 0,
    val durationFormatted: String = "",
    val thumbnailUrl: String = "",
    val isPlaylist: Boolean = false,
    val playlistCount: Int = 0,
    val availableFormats: List<FormatOption> = emptyList()
)
