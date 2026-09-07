package com.pullar.app.core

import android.content.Context
import android.util.Log
import com.pullar.app.data.model.DownloadEntity
import com.pullar.app.data.model.PlaylistItem
import com.pullar.app.data.model.PresetFormats
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

data class QuickMetadata(
    val title: String,
    val uploader: String,
    val durationSeconds: Long,
    val thumbnailUrl: String,
    val isPlaylist: Boolean,
    val playlistItems: List<PlaylistItem> = emptyList()
)

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

    suspend fun quickExtractMetadata(url: String): QuickMetadata? = withContext(Dispatchers.IO) {
        try {
            val request = YoutubeDLRequest(url)
            request.addOption("--dump-single-json")
            request.addOption("--no-warnings")
            request.addOption("--flat-playlist")
            request.addOption("--extractor-args", "youtube:player_client=android,web")
            request.addOption("--compat-options", "no-youtube-unavailable-videos")
            request.addOption("--socket-timeout", "20")

            val response: YoutubeDLResponse = YoutubeDL.getInstance().execute(request)
            val json = JSONObject(response.out)

            val title = json.optString("title", "Unknown Title")
            val uploader = json.optString("uploader", json.optString("channel", ""))
            val duration = json.optLong("duration", 0L)

            var thumb = json.optString("thumbnail", "")
            if (thumb.isBlank() && json.has("thumbnails")) {
                val thumbs = json.optJSONArray("thumbnails")
                if (thumbs != null && thumbs.length() > 0) {
                    val last = thumbs.optJSONObject(thumbs.length() - 1)
                    thumb = last?.optString("url", "") ?: ""
                }
            }

            val items = mutableListOf<PlaylistItem>()
            var isPlaylist = isPlaylistUrl(url)
            if (json.has("entries")) {
                val entries = json.optJSONArray("entries")
                if (entries != null) {
                    isPlaylist = true
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

                        items.add(
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

            QuickMetadata(
                title = title,
                uploader = uploader,
                durationSeconds = duration,
                thumbnailUrl = thumb,
                isPlaylist = isPlaylist,
                playlistItems = items
            )
        } catch (e: Exception) {
            Log.w(TAG, "Quick metadata extraction failed: ${e.message}")
            null
        }
    }

    suspend fun extractInfo(url: String): Result<VideoMetadata> = withContext(Dispatchers.IO) {
        try {
            val quick = quickExtractMetadata(url)
            if (quick != null) {
                Result.success(
                    VideoMetadata(
                        url = url,
                        title = quick.title,
                        uploader = quick.uploader,
                        durationSeconds = quick.durationSeconds,
                        durationFormatted = FormatUtils.formatDuration(quick.durationSeconds),
                        thumbnailUrl = quick.thumbnailUrl,
                        isPlaylist = quick.isPlaylist,
                        playlistCount = quick.playlistItems.size,
                        playlistItems = quick.playlistItems,
                        availableFormats = PresetFormats.allPresets
                    )
                )
            } else {
                // Fallback basic metadata with instant presets
                Result.success(
                    VideoMetadata(
                        url = url,
                        title = "Video Link",
                        uploader = "",
                        durationSeconds = 0L,
                        durationFormatted = "",
                        thumbnailUrl = "",
                        isPlaylist = isPlaylistUrl(url),
                        playlistCount = 0,
                        playlistItems = emptyList(),
                        availableFormats = PresetFormats.allPresets
                    )
                )
            }
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

        val destinationTemplate = "${outputDirectory.absolutePath}/%(title).180B.%(ext)s"
        request.addOption("-o", destinationTemplate)
        request.addOption("--windows-filenames")

        // YouTube mobile API client routing to bypass bot detection and SABR throttling
        request.addOption("--extractor-args", "youtube:player_client=android,web")
        request.addOption("--compat-options", "no-youtube-unavailable-videos")

        // Safe mobile networking parameters
        request.addOption("--retries", "5")
        request.addOption("--fragment-retries", "5")
        request.addOption("--socket-timeout", "30")
        request.addOption("--no-mtime")
        request.addOption("--no-warnings")
        request.addOption("--no-check-certificates")

        // Single video enforcement per task
        request.addOption("--no-playlist")

        // Format and Audio stream handling
        if (item.isAudioOnly) {
            request.addOption("-x")
            when (item.formatId) {
                "mp3" -> {
                    request.addOption("--audio-format", "mp3")
                    request.addOption("--audio-quality", "0")
                }
                "m4a" -> {
                    request.addOption("--audio-format", "m4a")
                    request.addOption("--audio-quality", "0")
                }
                else -> {
                    request.addOption("-f", "ba/b")
                }
            }
        } else {
            val formatSelector = if (item.formatId.isNotBlank()) item.formatId else "bv*+ba/b"
            request.addOption("-f", formatSelector)
            request.addOption("--merge-output-format", "mp4")
        }

        return request
    }
}
