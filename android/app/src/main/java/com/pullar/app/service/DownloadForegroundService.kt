package com.pullar.app.service

import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.app.Service
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.Environment
import android.os.IBinder
import android.util.Log
import androidx.core.app.NotificationCompat
import com.pullar.app.MainActivity
import com.pullar.app.R
import com.pullar.app.core.YoutubeDLEngine
import com.pullar.app.data.database.PullarDatabase
import com.pullar.app.data.model.DownloadEntity
import com.pullar.app.data.model.DownloadStatus
import com.yausername.youtubedl_android.YoutubeDL
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch
import java.io.File

class DownloadForegroundService : Service() {

    private val serviceScope = CoroutineScope(SupervisorJob() + Dispatchers.IO)
    private var currentDownloadJob: Job? = null
    private var activeTaskId: String? = null
    private lateinit var notificationManager: NotificationManager

    companion object {
        const val CHANNEL_ID = "pullar_downloads"
        const val NOTIFICATION_ID = 1001

        const val ACTION_START_DOWNLOAD = "com.pullar.app.START_DOWNLOAD"
        const val ACTION_CANCEL_DOWNLOAD = "com.pullar.app.CANCEL_DOWNLOAD"
        const val EXTRA_TASK_ID = "extra_task_id"

        fun startDownload(context: Context, taskId: String) {
            val intent = Intent(context, DownloadForegroundService::class.java).apply {
                action = ACTION_START_DOWNLOAD
                putExtra(EXTRA_TASK_ID, taskId)
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
                val taskId = intent.getStringExtra(EXTRA_TASK_ID)
                if (!taskId.isNullOrBlank()) {
                    startForeground(NOTIFICATION_ID, buildNotification("Starting download...", 0, ""))
                    processDownload(taskId)
                }
            }
            ACTION_CANCEL_DOWNLOAD -> {
                val taskId = intent.getStringExtra(EXTRA_TASK_ID)
                if (taskId != null && taskId == activeTaskId) {
                    cancelCurrentDownload()
                }
            }
        }
        return START_NOT_STICKY
    }

    private fun processDownload(taskId: String) {
        currentDownloadJob?.cancel()
        currentDownloadJob = serviceScope.launch {
            activeTaskId = taskId
            val db = PullarDatabase.getDatabase(applicationContext)
            val dao = db.downloadDao()
            val task = dao.getById(taskId)

            if (task == null) {
                stopSelf()
                return@launch
            }

            try {
                dao.updateStatus(taskId, DownloadStatus.DOWNLOADING)

                val downloadDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
                val pullarDir = File(downloadDir, "Pullar")
                if (!pullarDir.exists()) {
                    pullarDir.mkdirs()
                }

                val request = YoutubeDLEngine.buildDownloadRequest(task, pullarDir)

                YoutubeDL.getInstance().execute(request, taskId) { progress, etaInSeconds, line ->
                    val speed = extractSpeed(line)
                    val etaFormatted = if (etaInSeconds > 0) "${etaInSeconds}s" else ""

                    notificationManager.notify(
                        NOTIFICATION_ID,
                        buildNotification(task.title, progress.toInt(), speed)
                    )

                    serviceScope.launch {
                        dao.updateProgress(
                            id = taskId,
                            progress = progress,
                            speed = speed,
                            eta = etaFormatted,
                            downloaded = 0L,
                            total = 0L,
                            status = DownloadStatus.DOWNLOADING
                        )
                    }
                }

                // Download completed
                dao.updateProgress(
                    id = taskId,
                    progress = 100f,
                    speed = "Completed",
                    eta = "",
                    downloaded = 0L,
                    total = 0L,
                    status = DownloadStatus.COMPLETED
                )

                showCompletionNotification(task.title)

            } catch (e: Exception) {
                Log.e("DownloadService", "Download error: ${e.message}", e)
                dao.updateStatus(taskId, DownloadStatus.FAILED)
            } finally {
                activeTaskId = null
                stopForeground(STOP_FOREGROUND_DETACH)
                stopSelf()
            }
        }
    }

    private fun cancelCurrentDownload() {
        val taskId = activeTaskId
        currentDownloadJob?.cancel()
        if (taskId != null) {
            YoutubeDL.getInstance().destroyProcessById(taskId)
            serviceScope.launch {
                val db = PullarDatabase.getDatabase(applicationContext)
                db.downloadDao().updateStatus(taskId, DownloadStatus.CANCELLED)
            }
        }
        stopForeground(STOP_FOREGROUND_REMOVE)
        stopSelf()
    }

    private fun extractSpeed(line: String): String {
        val speedMatch = Regex("([0-9.]+\\s*(?:KiB|MiB|GiB|KB|MB|GB)/s)").find(line)
        return speedMatch?.groupValues?.get(1) ?: ""
    }

    private fun buildNotification(title: String, progress: Int, speed: String): android.app.Notification {
        val launchIntent = Intent(this, MainActivity::class.java)
        val pendingIntent = PendingIntent.getActivity(
            this,
            0,
            launchIntent,
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        val contentText = if (speed.isNotBlank()) "$progress% • $speed" else "$progress%"

        return NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle(title)
            .setContentText(contentText)
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
