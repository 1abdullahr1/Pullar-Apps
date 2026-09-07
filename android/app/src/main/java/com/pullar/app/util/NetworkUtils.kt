package com.pullar.app.util

import android.content.Context
import android.net.ConnectivityManager
import android.net.NetworkCapabilities
import android.os.Build

enum class NetworkType {
    WIFI,
    CELLULAR,
    ETHERNET,
    OTHER,
    NONE
}

sealed class NetworkCheckResult {
    object Allowed : NetworkCheckResult()
    data class Blocked(val reason: String) : NetworkCheckResult()
}

object NetworkUtils {

    fun getCurrentNetworkType(context: Context): NetworkType {
        val cm = context.getSystemService(Context.CONNECTIVITY_SERVICE) as? ConnectivityManager
            ?: return NetworkType.NONE

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            val activeNetwork = cm.activeNetwork ?: return NetworkType.NONE
            val capabilities = cm.getNetworkCapabilities(activeNetwork) ?: return NetworkType.NONE

            return when {
                capabilities.hasTransport(NetworkCapabilities.TRANSPORT_WIFI) -> NetworkType.WIFI
                capabilities.hasTransport(NetworkCapabilities.TRANSPORT_CELLULAR) -> NetworkType.CELLULAR
                capabilities.hasTransport(NetworkCapabilities.TRANSPORT_ETHERNET) -> NetworkType.ETHERNET
                else -> NetworkType.OTHER
            }
        } else {
            @Suppress("DEPRECATION")
            val networkInfo = cm.activeNetworkInfo ?: return NetworkType.NONE
            @Suppress("DEPRECATION")
            return when (networkInfo.type) {
                ConnectivityManager.TYPE_WIFI -> NetworkType.WIFI
                ConnectivityManager.TYPE_MOBILE -> NetworkType.CELLULAR
                ConnectivityManager.TYPE_ETHERNET -> NetworkType.ETHERNET
                else -> NetworkType.OTHER
            }
        }
    }

    fun isDownloadAllowed(
        context: Context,
        wifiEnabled: Boolean,
        mobileDataEnabled: Boolean
    ): NetworkCheckResult {
        val currentType = getCurrentNetworkType(context)
        return when (currentType) {
            NetworkType.NONE -> NetworkCheckResult.Blocked("No internet connection detected. Please check your network.")
            NetworkType.WIFI -> {
                if (wifiEnabled) {
                    NetworkCheckResult.Allowed
                } else {
                    NetworkCheckResult.Blocked("Wi-Fi downloads are disabled in Settings. Enable Wi-Fi downloads to continue.")
                }
            }
            NetworkType.CELLULAR -> {
                if (mobileDataEnabled) {
                    NetworkCheckResult.Allowed
                } else {
                    NetworkCheckResult.Blocked("Mobile data downloads are disabled in Settings. Connect to Wi-Fi or enable mobile data downloads.")
                }
            }
            NetworkType.ETHERNET, NetworkType.OTHER -> NetworkCheckResult.Allowed
        }
    }
}
