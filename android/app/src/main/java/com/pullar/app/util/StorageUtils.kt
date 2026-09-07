package com.pullar.app.util

import android.app.DownloadManager
import android.content.Context
import android.content.Intent
import android.media.MediaScannerConnection
import android.net.Uri
import android.os.Build
import android.os.Environment
import android.provider.DocumentsContract
import android.util.Log
import android.widget.Toast
import androidx.core.content.FileProvider
import java.io.File

object StorageUtils {

    private const val TAG = "PullarStorage"

    fun getDownloadDirectory(context: Context, playlistSubfolder: String? = null): File {
        val publicDownloads = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
        val pullarRoot = File(publicDownloads, "Pullar")

        val targetDir = if (pullarRoot.exists() || pullarRoot.mkdirs() || pullarRoot.canWrite()) {
            pullarRoot
        } else {
            // App-specific external files fallback (guaranteed write access across all Android versions)
            val fallback = File(context.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS), "Pullar")
            if (!fallback.exists()) fallback.mkdirs()
            fallback
        }

        return if (!playlistSubfolder.isNullOrBlank()) {
            val sanitized = sanitizeFilename(playlistSubfolder)
            val subDir = File(targetDir, sanitized)
            if (!subDir.exists()) subDir.mkdirs()
            subDir
        } else {
            targetDir
        }
    }

    fun sanitizeFilename(name: String): String {
        return name.replace(Regex("[\\\\/:*?\"<>|]"), "_").trim()
    }

    fun scanMediaFile(context: Context, file: File, mimeType: String? = null) {
        if (!file.exists()) return
        try {
            MediaScannerConnection.scanFile(
                context.applicationContext,
                arrayOf(file.absolutePath),
                if (mimeType != null) arrayOf(mimeType) else null
            ) { path, uri ->
                Log.i(TAG, "Scanned media into Android index: $path -> $uri")
            }
        } catch (e: Exception) {
            Log.w(TAG, "Media scanner registration failed: ${e.message}")
        }
    }

    fun openDownloadsFolder(context: Context) {
        val pullarDir = getDownloadDirectory(context)

        val launched = tryOpeningWithDocumentsProvider(context, pullarDir)
            || tryOpeningWithDownloadManager(context)
            || tryOpeningWithFileProvider(context, pullarDir)
            || tryOpeningGenericDirectory(context, pullarDir)

        if (!launched) {
            Toast.makeText(
                context,
                "Pullar folder: ${pullarDir.absolutePath}",
                Toast.LENGTH_LONG
            ).show()
        }
    }

    private fun tryOpeningWithDocumentsProvider(context: Context, dir: File): Boolean {
        return try {
            val uri = Uri.parse("content://com.android.externalstorage.documents/document/primary:Download%2FPullar")
            val intent = Intent(Intent.ACTION_VIEW).apply {
                setDataAndType(uri, DocumentsContract.Document.MIME_TYPE_DIR)
                addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_ACTIVITY_NEW_TASK)
            }
            context.startActivity(intent)
            true
        } catch (e: Exception) {
            false
        }
    }

    private fun tryOpeningWithDownloadManager(context: Context): Boolean {
        return try {
            val intent = Intent(DownloadManager.ACTION_VIEW_DOWNLOADS).apply {
                addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            }
            context.startActivity(intent)
            true
        } catch (e: Exception) {
            false
        }
    }

    private fun tryOpeningWithFileProvider(context: Context, dir: File): Boolean {
        return try {
            val uri = FileProvider.getUriForFile(context, "${context.packageName}.fileprovider", dir)
            val intent = Intent(Intent.ACTION_VIEW).apply {
                setDataAndType(uri, "*/*")
                addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_ACTIVITY_NEW_TASK)
            }
            context.startActivity(Intent.createChooser(intent, "Open Pullar Folder").apply {
                addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            })
            true
        } catch (e: Exception) {
            false
        }
    }

    private fun tryOpeningGenericDirectory(context: Context, dir: File): Boolean {
        return try {
            val intent = Intent(Intent.ACTION_VIEW).apply {
                setDataAndType(Uri.fromFile(dir), "resource/folder")
                addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            }
            context.startActivity(intent)
            true
        } catch (e: Exception) {
            false
        }
    }
}
