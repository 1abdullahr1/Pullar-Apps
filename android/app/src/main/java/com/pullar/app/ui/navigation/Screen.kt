package com.pullar.app.ui.navigation

sealed class Screen(val route: String, val title: String) {
    object Downloader : Screen("downloader", "Downloader")
    object Downloads : Screen("downloads", "Downloads")
    object History : Screen("history", "History")
    object Settings : Screen("settings", "Settings")
    object Player : Screen("player/{videoUri}", "Player") {
        fun createRoute(videoUri: String): String = "player/${java.net.URLEncoder.encode(videoUri, "UTF-8")}"
    }
}
