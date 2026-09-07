package com.pullar.app.util

import java.util.Locale

object FormatUtils {

    fun formatDuration(seconds: Long): String {
        if (seconds <= 0) return ""
        val hrs = seconds / 3600
        val mins = (seconds % 3600) / 60
        val secs = seconds % 60
        return if (hrs > 0) {
            String.format(Locale.US, "%d:%02d:%02d", hrs, mins, secs)
        } else {
            String.format(Locale.US, "%d:%02d", mins, secs)
        }
    }

    fun formatEta(etaSeconds: Long): String {
        if (etaSeconds <= 0) return "Calculating..."
        return when {
            etaSeconds < 60 -> "Less than 1 min remaining"
            etaSeconds < 120 -> "About 1 min remaining"
            etaSeconds < 3600 -> {
                val mins = (etaSeconds + 30) / 60
                "About $mins min remaining"
            }
            else -> {
                val hrs = etaSeconds / 3600
                val mins = ((etaSeconds % 3600) + 30) / 60
                if (mins > 0) {
                    "About $hrs hr $mins min remaining"
                } else {
                    "About $hrs hr remaining"
                }
            }
        }
    }

    fun formatBytes(bytes: Long): String {
        if (bytes <= 0) return "0 B"
        val units = arrayOf("B", "KB", "MB", "GB", "TB")
        var value = bytes.toDouble()
        var index = 0
        while (value >= 1024 && index < units.size - 1) {
            value /= 1024
            index++
        }
        return String.format(Locale.US, "%.1f %s", value, units[index])
    }
}
