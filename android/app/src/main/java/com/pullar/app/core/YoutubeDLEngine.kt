package com.pullar.app.core

import android.content.Context
import android.util.Log
import com.pullar.app.data.model.DownloadEntity
import com.pullar.app.data.model.FormatOption
import com.pullar.app.data.model.VideoMetadata
import com.yausername.youtubedl_android.YoutubeDL
import com.yausername.youtubedl_android.YoutubeDLException
import com.yausername.youtubedl_android.YoutubeDLRequest
import com.yausername.youtubedl_android.YoutubeDLResponse
import com.yausername.ffmpeg.FFmpeg
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File
import java.util.regex.Pattern

object YoutubeDLEngine {

    private const val TAG = "PullarYoutubeDL"
    private var isInitialized = false

    fun init(context: Context) {
        if (isInitialized) return
        try {
            YoutubeDL.getInstance().init(context.applicationContext)
            FFmpeg.getInstance().init(context.applicationContext)
            isInitialized = true
            Log.i(TAG, "YoutubeDL and FFmpeg initialized successfully.")
        } catch (e: Exception) {
            Log.e(TAG, "Failed to initialize YoutubeDL engine: ${e.message}", e)
        }
    }

    suspend fun updateEngine(context: Context): Boolean = withContext(Dispatchers.IO) {
        try {
            val status = YoutubeDL.getInstance().updateYoutubeDL(context.applicationContext, YoutubeDL.UpdateChannel._STABLE)
            Log.i(TAG, "YoutubeDL updated status: $status")
            true
        } catch (e: Exception) {
            Log.e(TAG, "Failed to update YoutubeDL: ${e.message}")
            false
        }
    }

    fun isPlaylistUrl(url: String): Boolean {
        return url.contains("list=") || url.contains("/playlist") || url.contains("/sets/")
    }

    suspend fun extractInfo(url: String): Result<VideoMetadata> = withContext(Dispatchers.IO) {
        try {
            val isPlaylist = isPlaylistUrl(url)
            val request = YoutubeDLRequest(url)
            request.addOption("--dump-single-json")
            request.addOption("--no-warnings")
            request.addOption("--flat-playlist")

            val response: YoutubeDLResponse = YoutubeDL.getInstance().execute(request)
            val output = response.out

            val titleMatch = Regex("\"title\"\\s*:\\s*\"([^\"]+)\"").find(output)
            val title = titleMatch?.groupValues?.get(1) ?: "Unknown Title"

            val uploaderMatch = Regex("\"uploader\"\\s*:\\s*\"([^\"]+)\"").find(output)
                ?: Regex("\"channel\"\\s*:\\s*\"([^\"]+)\"").find(output)
            val uploader = uploaderMatch?.groupValues?.get(1) ?: ""

            val durationMatch = Regex("\"duration\"\\s*:\\s*([0-9]+)").find(output)
            val durationSeconds = durationMatch?.groupValues?.get(1)?.toLongOrNull() ?: 0L
            val durationFormatted = formatDuration(durationSeconds)

            val thumbMatch = Regex("\"thumbnail\"\\s*:\\s*\"([^\"]+)\"").find(output)
            val thumbnailUrl = thumbMatch?.groupValues?.get(1) ?: ""

            val playlistCount = if (isPlaylist) {
                Regex("\"entries\"\\s*:\\s*\\[").findAll(output).count()
            } else 0

            val formats = listOf(
                FormatOption(
                    formatId = "bestvideo[ext=mp4]+bestaudio[ext=m4a]/best[ext=mp4]/best",
                    label = "Best Quality (MP4)",
                    resolution = "Highest Available",
                    extension = "mp4",
                    isAudioOnly = false,
                    note = "Full quality video with merged audio"
                ),
                FormatOption(
                    formatId = "bestvideo[height<=1080][ext=mp4]+bestaudio[ext=m4a]/best[height<=1080]",
                    label = "1080p FHD",
                    resolution = "1920x1080",
                    extension = "mp4",
                    isAudioOnly = false,
                    note = "High Definition"
                ),
                FormatOption(
                    formatId = "bestvideo[height<=720][ext=mp4]+bestaudio[ext=m4a]/best[height<=720]",
                    label = "720p HD",
                    resolution = "1280x720",
                    extension = "mp4",
                    isAudioOnly = false,
                    note = "Standard High Definition"
                ),
                FormatOption(
                    formatId = "bestvideo[height<=480][ext=mp4]+bestaudio[ext=m4a]/best[height<=480]",
                    label = "480p SD",
                    resolution = "854x480",
                    extension = "mp4",
                    isAudioOnly = false,
                    note = "Standard Definition"
                ),
                FormatOption(
                    formatId = "bestaudio/best",
                    label = "Audio Only (MP3)",
                    resolution = "Audio 320kbps",
                    extension = "mp3",
                    isAudioOnly = true,
                    note = "Extracted high quality MP3"
                )
            )

            Result.success(
                VideoMetadata(
                    url = url,
                    title = title,
                    uploader = uploader,
                    durationSeconds = durationSeconds,
                    durationFormatted = durationFormatted,
                    thumbnailUrl = thumbnailUrl,
                    isPlaylist = isPlaylist,
                    playlistCount = playlistCount,
                    availableFormats = formats
                )
            )
        } catch (e: Exception) {
            Log.e(TAG, "Extraction failed: ${e.message}", e)
            Result.failure(e)
        }
    }

    fun buildDownloadRequest(
        item: DownloadEntity,
        outputDirectory: File
    ): YoutubeDLRequest {
        val request = YoutubeDLRequest(item.url)

        val destinationTemplate = if (item.isPlaylist) {
            "${outputDirectory.absolutePath}/%(playlist_title)s/%(playlist_index)s - %(title)s.%(ext)s"
        } else {
            "${outputDirectory.absolutePath}/%(title)s.%(ext)s"
        }
        request.addOption("-o", destinationTemplate)

        // IDM-Style High-Speed Segmented Multi-Connection Scheme
        request.addOption("--concurrent-fragments", "16")
        request.addOption("--buffer-size", "1024K")
        request.addOption("--http-chunk-size", "10M")
        request.addOption("--retries", "10")
        request.addOption("--fragment-retries", "10")

        // Playlist enforcement
        if (item.isPlaylist) {
            request.addOption("--yes-playlist")
        } else {
            // Strictly enforce --no-playlist to prevent repeated loop downloading on single watch URLs
            request.addOption("--no-playlist")
        }

        // Format & Audio handling
        if (item.isAudioOnly) {
            request.addOption("-x")
            request.addOption("--audio-format", "mp3")
            request.addOption("--audio-quality", "0")
        } else {
            request.addOption("-f", item.formatId)
            request.addOption("--merge-output-format", "mp4")
        }

        request.addOption("--no-warnings")
        request.addOption("--no-check-certificates")
        return request
    }

    private fun formatDuration(seconds: Long): String {
        if (seconds <= 0) return ""
        val hrs = seconds / 3600
        val mins = (seconds % 3600) / 60
        val secs = seconds % 60
        return if (hrs > 0) {
            String.format("%d:%02d:%02d", hrs, mins, secs)
        } else {
            String.format("%d:%02d", mins, secs)
        }
    }
}
