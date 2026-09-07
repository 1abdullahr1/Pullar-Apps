package com.pullar.app.data.model

enum class MediaType {
    VIDEO,
    AUDIO
}

data class FormatOption(
    val formatId: String,
    val label: String,
    val resolution: String = "",
    val extension: String = "mp4",
    val isAudioOnly: Boolean = false,
    val note: String = ""
)

object PresetFormats {
    val VideoBest = FormatOption(
        formatId = "bv*+ba/b",
        label = "Best Available Quality",
        resolution = "Highest (4K / 1080p)",
        extension = "mp4",
        isAudioOnly = false,
        note = "Highest video and audio quality merged into MP4"
    )

    val Video1080p = FormatOption(
        formatId = "bv*[height<=?1080]+ba/b[height<=?1080]/bv*+ba/b",
        label = "1080p Full HD",
        resolution = "1920x1080",
        extension = "mp4",
        isAudioOnly = false,
        note = "High Definition MP4"
    )

    val Video720p = FormatOption(
        formatId = "bv*[height<=?720]+ba/b[height<=?720]/bv*+ba/b",
        label = "720p HD",
        resolution = "1280x720",
        extension = "mp4",
        isAudioOnly = false,
        note = "Standard High Definition"
    )

    val Video480p = FormatOption(
        formatId = "bv*[height<=?480]+ba/b[height<=?480]/bv*+ba/b",
        label = "480p SD",
        resolution = "854x480",
        extension = "mp4",
        isAudioOnly = false,
        note = "Standard Definition for smaller file size"
    )

    val Video360p = FormatOption(
        formatId = "bv*[height<=?360]+ba/b[height<=?360]/bv*+ba/b",
        label = "360p Data Saver",
        resolution = "640x360",
        extension = "mp4",
        isAudioOnly = false,
        note = "Fastest download and compact storage"
    )

    val AudioBest = FormatOption(
        formatId = "ba/b",
        label = "Best Audio (Original)",
        resolution = "Original Stream",
        extension = "m4a",
        isAudioOnly = true,
        note = "Direct best quality audio stream"
    )

    val AudioMp3 = FormatOption(
        formatId = "mp3",
        label = "MP3 (Universal Audio)",
        resolution = "320 kbps",
        extension = "mp3",
        isAudioOnly = true,
        note = "Compatible with all players and devices"
    )

    val AudioM4a = FormatOption(
        formatId = "m4a",
        label = "M4A (AAC Audio)",
        resolution = "High Quality AAC",
        extension = "m4a",
        isAudioOnly = true,
        note = "Native AAC audio container"
    )

    val videoPresets = listOf(VideoBest, Video1080p, Video720p, Video480p, Video360p)
    val audioPresets = listOf(AudioBest, AudioMp3, AudioM4a)
    val allPresets = videoPresets + audioPresets
}
