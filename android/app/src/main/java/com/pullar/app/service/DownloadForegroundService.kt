package com.pullar.app.service

import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.app.Service
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.IBinder
import android.util.Log
import androidx.core.app.NotificationCompat
import com.pullar.app.MainActivity
import com.pullar.app.R
import com.pullar.app.core.YoutubeDLEngine
import com.pullar.app.data.database.PullarDatabase
import com.pullar.app.data.model.DownloadEntity
import com.pullar.app.data.model.DownloadStatus
import com.pullar.app.data.preferences.ThemePreferences
import com.pullar.app.util.FormatUtils
import com.pullar.app.util.NetworkCheckResult
import com.pullar.app.util.NetworkUtils
import com.pullar.app.util.StorageUtils
import com.yausername.youtubedl_android.YoutubeDL
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.flow.firstOrNull
import kotlinx.coroutines.launch
import java.io.File
import java.util.UUID

class DownloadForegroundService : Service() {

    private val serviceScope = CoroutineScope(SupervisorJob() + Dispatchers.IO)
    private var queueJob: Job? = null
    @Volatile
    private var activeTaskId: String? = null
    private lateinit var notificationManager: NotificationManager

    companion object {
        const val CHANNEL_ID = "pullar_downloads"
        const val NOTIFICATION_ID = 1001

        const val ACTION_START_DOWNLOAD = "com.pullar.app.START_DOWNLOAD"
        const val ACTION_CANCEL_DOWNLOAD = "com.pullar.app.CANCEL_DOWNLOAD"
        const val EXTRA_TASK_ID = "extra_task_id"

        fun startDownload(context: Context, taskId: String? = null) {
            val intent = Intent(context, DownloadForegroundService::class.java).apply {
                action = ACTION_START_DOWNLOAD
                if (taskId != null) putExtra(EXTRA_TASK_ID, taskId)
            }
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                context.startForegroundService(intent)
            } else {
                context.startService(intent)
            }
        }

        fun cancelDownload(context: Context, taskId: String) {
            val intent = Intent(context, DownloadForegroundService::class.java).apply {
                action = ACTION_CANCEL_DOWNLOAD
                putExtra(EXTRA_TASK_ID, taskId)
            }
            context.startService(intent)
        }
    }

    override fun onCreate() {
        super.onCreate()
        notificationManager = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        createNotificationChannel()
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        when (intent?.action) {
            ACTION_START_DOWNLOAD -> {
                startForeground(NOTIFICATION_ID, buildNotification("Pullar Downloader", "Starting queue...", 0))
                startQueueProcessor()
            }
            ACTION_CANCEL_DOWNLOAD -> {
                val taskId = intent.getStringExtra(EXTRA_TASK_ID)
                if (taskId != null) {
                    cancelTask(taskId)
                }
            }
        }
        return START_NOT_STICKY
    }

    private fun startQueueProcessor() {
        if (queueJob?.isActive == true) return

        queueJob = serviceScope.launch {
            val db = PullarDatabase.getDatabase(applicationContext)
            val dao = db.downloadDao()
            val themePreferences = ThemePreferences(applicationContext)

            while (true) {
                val nextTask = dao.getNextQueued() ?: break
                activeTaskId = nextTask.id

                // Network policy verification
                val wifiEnabled = themePreferences.wifiDownloadsFlow.firstOrNull() ?: true
                val mobileDataEnabled = themePreferences.mobileDataDownloadsFlow.firstOrNull() ?: true

                val netCheck = NetworkUtils.isDownloadAllowed(applicationContext, wifiEnabled, mobileDataEnabled)
                if (netCheck is NetworkCheckResult.Blocked) {
                    dao.updateFailed(nextTask.id, errorMessage = netCheck.reason)
                    showBlockedNotification(nextTask.title, netCheck.reason)
                    continue
                }

                // Execute task download
                processSingleTask(nextTask, dao)
            }

            activeTaskId = null
            stopForeground(STOP_FOREGROUND_DETACH)
            stopSelf()
        }
    }

    private suspend fun processSingleTask(task: DownloadEntity, dao: com.pullar.app.data.dao.DownloadDao) {
        val taskId = task.id
        var effectiveTitle = task.title
        val outputDir = StorageUtils.getDownloadDirectory(applicationContext, task.playlistTitle)

        try {
            // Background metadata resolution if task has placeholder title
            if (task.uploader.isBlank() || task.title.startsWith("Shared Video") || task.title.startsWith("Video Download")) {
                dao.updateStatus(taskId, DownloadStatus.ANALYZING)
                notificationManager.notify(
                    NOTIFICATION_ID,
                    buildNotification("Pullar Downloader", "Resolving stream details...", 0)
                )

                val quick = YoutubeDLEngine.quickExtractMetadata(task.url)
                if (quick != null) {
                    effectiveTitle = quick.title
                    dao.updateMetadata(taskId, quick.title, quick.uploader, quick.thumbnailUrl)

                    // If it was a playlist link with downloadEntirePlaylist, enqueue remaining items
                    if (task.isPlaylist && quick.playlistItems.size > 1) {
                        val remainingEntities = quick.playlistItems.drop(1).mapIndexed { index, item ->
                            DownloadEntity(
                                id = UUID.randomUUID().toString(),
                                url = item.url,
                                title = item.title,
                                uploader = quick.uploader,
                                thumbnailUrl = item.thumbnailUrl.ifBlank { quick.thumbnailUrl },
                                formatId = task.formatId,
                                qualityLabel = task.qualityLabel,
                                isPlaylist = true,
                                playlistTitle = quick.title,
                                playlistIndex = index + 2,
                                playlistTotal = quick.playlistItems.size,
                                isAudioOnly = task.isAudioOnly,
                                status = DownloadStatus.QUEUED
                            )
                        }
                        dao.insertAll(remainingEntities)
                    }
                }
            }

            dao.updateStatus(taskId, DownloadStatus.DOWNLOADING)
            val request = YoutubeDLEngine.buildDownloadRequest(task, outputDir)

            YoutubeDL.getInstance().execute(request, taskId) { progress, etaInSeconds, line ->
                val speed = extractSpeed(line)
                val friendlyEta = FormatUtils.formatEta(etaInSeconds.toLong())

                val subtext = buildString {
                    if (task.playlistTotal > 0) append("[${task.playlistIndex}/${task.playlistTotal}] ")
                    if (speed.isNotBlank()) append("$speed • ")
                    append(friendlyEta)
                }

                notificationManager.notify(
                    NOTIFICATION_ID,
                    buildNotification(effectiveTitle, subtext, progress.toInt())
                )

                serviceScope.launch {
                    dao.updateProgress(
                        id = taskId,
                        progress = progress,
                        speed = speed,
                        eta = if (etaInSeconds > 0) "${etaInSeconds}s" else "",
                        etaFriendly = friendlyEta,
                        downloaded = 0L,
                        total = 0L,
                        status = DownloadStatus.DOWNLOADING
                    )
                }
            }

            // Task completed - discover downloaded file on disk
            val downloadedFile = findDownloadedFile(outputDir, effectiveTitle)
            val finalPath = downloadedFile?.absolutePath ?: File(outputDir, "$effectiveTitle.mp4").absolutePath
            val finalSize = downloadedFile?.length() ?: 0L

            dao.updateCompleted(
                id = taskId,
                filePath = finalPath,
                downloaded = finalSize,
                total = finalSize
            )

            if (downloadedFile != null) {
                StorageUtils.scanMediaFile(applicationContext, downloadedFile)
            }

            showCompletionNotification(effectiveTitle)

        } catch (e: Exception) {
            val errorMsg = e.message ?: "Download failed"
            Log.e("DownloadService", "Error downloading task $taskId: $errorMsg", e)
            dao.updateFailed(taskId, errorMessage = errorMsg)

            // Auto-sync yt-dlp binary if failure indicates cipher/extractor discrepancy
            if (errorMsg.contains("bot", ignoreCase = true) ||
                errorMsg.contains("403", ignoreCase = true) ||
                errorMsg.contains("cipher", ignoreCase = true) ||
                errorMsg.contains("challenge", ignoreCase = true)
            ) {
                serviceScope.launch {
                    YoutubeDLEngine.updateEngine(applicationContext)
                }
            }
        }
    }

    private fun findDownloadedFile(dir: File, title: String): File? {
        if (!dir.exists()) return null
        val files = dir.listFiles() ?: return null
        val sanitized = StorageUtils.sanitizeFilename(title).take(20).lowercase()
        return files.filter { it.isFile && (it.extension == "mp4" || it.extension == "mp3" || it.extension == "m4a" || it.extension == "mkv") }
            .maxByOrNull { file ->
                var score = 0
                if (file.name.lowercase().contains(sanitized)) score += 10
                if (System.currentTimeMillis() - file.lastModified() < 300_000) score += 5
                score
            }
    }

    private fun cancelTask(taskId: String) {
        if (taskId == activeTaskId) {
            YoutubeDL.getInstance().destroyProcessById(taskId)
        }
        serviceScope.launch {
            val db = PullarDatabase.getDatabase(applicationContext)
            db.downloadDao().updateStatus(taskId, DownloadStatus.CANCELLED)
        }
    }

    private fun extractSpeed(line: String): String {
        val speedMatch = Regex("([0-9.]+\\s*(?:KiB|MiB|GiB|KB|MB|GB)/s)").find(line)
        return speedMatch?.groupValues?.get(1) ?: ""
    }

    private fun buildNotification(title: String, subtitle: String, progress: Int): android.app.Notification {
        val launchIntent = Intent(this, MainActivity::class.java)
        val pendingIntent = PendingIntent.getActivity(
            this,
            0,
            launchIntent,
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        return NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle(title)
            .setContentText(subtitle)
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .setProgress(100, progress, progress == 0)
            .setOngoing(true)
            .setContentIntent(pendingIntent)
            .build()
    }

    private fun showCompletionNotification(title: String) {
        val launchIntent = Intent(this, MainActivity::class.java)
        val pendingIntent = PendingIntent.getActivity(
            this,
            1,
            launchIntent,
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        val completionNotification = NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("Download Finished")
            .setContentText(title)
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .setAutoCancel(true)
            .setContentIntent(pendingIntent)
            .build()

        notificationManager.notify(System.currentTimeMillis().toInt(), completionNotification)
    }

    private fun showBlockedNotification(title: String, reason: String) {
        val launchIntent = Intent(this, MainActivity::class.java)
        val pendingIntent = PendingIntent.getActivity(
            this,
            2,
            launchIntent,
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        val blockedNotification = NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("Download Blocked")
            .setContentText("$title: $reason")
            .setStyle(NotificationCompat.BigTextStyle().bigText("$title\n$reason"))
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .setAutoCancel(true)
            .setContentIntent(pendingIntent)
            .build()

        notificationManager.notify(System.currentTimeMillis().toInt(), blockedNotification)
    }

    private fun createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val channel = NotificationChannel(
                CHANNEL_ID,
                getString(R.string.channel_name),
                NotificationManager.IMPORTANCE_LOW
            ).apply {
                description = getString(R.string.channel_description)
                setShowBadge(false)
            }
            notificationManager.createNotificationChannel(channel)
        }
    }

    override fun onBind(intent: Intent?): IBinder? = null

    override fun onDestroy() {
        super.onDestroy()
        serviceScope.cancel()
    }
}
