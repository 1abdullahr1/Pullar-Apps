package com.pullar.app.data.model

data class FormatOption(
    val formatId: String,
    val label: String,
    val resolution: String = "",
    val extension: String = "mp4",
    val isAudioOnly: Boolean = false,
    val note: String = ""
)
