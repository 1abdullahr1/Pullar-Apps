package com.pullar.app.core

import android.content.Context
import android.util.Log
import com.pullar.app.data.model.DownloadEntity
import com.pullar.app.data.model.FormatOption
import com.pullar.app.data.model.PlaylistItem
import com.pullar.app.data.model.VideoMetadata
import com.pullar.app.util.FormatUtils
import com.yausername.ffmpeg.FFmpeg
import com.yausername.youtubedl_android.YoutubeDL
import com.yausername.youtubedl_android.YoutubeDLRequest
import com.yausername.youtubedl_android.YoutubeDLResponse
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONObject
import java.io.File

object YoutubeDLEngine {

    private const val TAG = "PullarYoutubeDL"
    @Volatile
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
            request.addOption("--socket-timeout", "25")

            val response: YoutubeDLResponse = YoutubeDL.getInstance().execute(request)
            val output = response.out

            val json = JSONObject(output)
            val title = json.optString("title", "Unknown Title")
            val uploader = json.optString("uploader", json.optString("channel", ""))
            val durationSeconds = json.optLong("duration", 0L)
            val durationFormatted = FormatUtils.formatDuration(durationSeconds)

            // Extract thumbnail
            var thumbnailUrl = json.optString("thumbnail", "")
            if (thumbnailUrl.isBlank() && json.has("thumbnails")) {
                val thumbs = json.optJSONArray("thumbnails")
                if (thumbs != null && thumbs.length() > 0) {
                    val lastThumb = thumbs.optJSONObject(thumbs.length() - 1)
                    thumbnailUrl = lastThumb?.optString("url", "") ?: ""
                }
            }

            // Playlist items extraction
            val playlistItems = mutableListOf<PlaylistItem>()
            var playlistCount = 0

            if (json.has("entries")) {
                val entries = json.optJSONArray("entries")
                if (entries != null) {
                    playlistCount = entries.length()
                    for (i in 0 until entries.length()) {
                        val entry = entries.optJSONObject(i) ?: continue
                        val itemTitle = entry.optString("title", "Video #${i + 1}")
                        val itemId = entry.optString("id", "")
                        var itemUrl = entry.optString("url", "")
                        if (itemUrl.isBlank() || !itemUrl.startsWith("http")) {
                            itemUrl = if (itemId.isNotBlank()) "https://www.youtube.com/watch?v=$itemId" else url
                        }
                        val itemDuration = entry.optLong("duration", 0L)
                        val itemThumb = entry.optString("thumbnail", "")

                        playlistItems.add(
                            PlaylistItem(
                                id = itemId.ifBlank { "item_$i" },
                                title = itemTitle,
                                url = itemUrl,
                                durationSeconds = itemDuration,
                                durationFormatted = FormatUtils.formatDuration(itemDuration),
                                thumbnailUrl = itemThumb,
                                index = i + 1
                            )
                        )
                    }
                }
            }

            val formats = listOf(
                FormatOption(
                    formatId = "bestvideo+bestaudio/best",
                    label = "Best Quality (MP4)",
                    resolution = "Highest Available",
                    extension = "mp4",
                    isAudioOnly = false,
                    note = "Full quality merged with FFmpeg"
                ),
                FormatOption(
                    formatId = "bestvideo[height<=1080]+bestaudio/best[height<=1080]/best",
                    label = "1080p Full HD",
                    resolution = "1920x1080",
                    extension = "mp4",
                    isAudioOnly = false,
                    note = "High Definition MP4"
                ),
                FormatOption(
                    formatId = "bestvideo[height<=720]+bestaudio/best[height<=720]/best",
                    label = "720p HD",
                    resolution = "1280x720",
                    extension = "mp4",
                    isAudioOnly = false,
                    note = "Standard High Definition"
                ),
                FormatOption(
                    formatId = "bestvideo[height<=480]+bestaudio/best[height<=480]/best",
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
                    isPlaylist = isPlaylist || playlistCount > 0,
                    playlistCount = playlistCount,
                    playlistItems = playlistItems,
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

        val destinationTemplate = "${outputDirectory.absolutePath}/%(title)s.%(ext)s"
        request.addOption("-o", destinationTemplate)

        // Safe mobile networking scheme
        request.addOption("--retries", "5")
        request.addOption("--fragment-retries", "5")
        request.addOption("--socket-timeout", "30")
        request.addOption("--no-mtime")
        request.addOption("--no-warnings")
        request.addOption("--no-check-certificates")

        // Single video enforcement per discrete task
        request.addOption("--no-playlist")

        // Format & Audio handling
        if (item.isAudioOnly) {
            request.addOption("-x")
            request.addOption("--audio-format", "mp3")
            request.addOption("--audio-quality", "0")
        } else {
            request.addOption("-f", item.formatId)
            request.addOption("--merge-output-format", "mp4")
        }

        return request
    }
}
