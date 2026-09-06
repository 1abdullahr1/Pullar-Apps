package com.pullar.app.ui.navigation

import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Download
import androidx.compose.material.icons.filled.History
import androidx.compose.material.icons.filled.Home
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material3.Icon
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.NavigationBarItemDefaults
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.navigation.NavGraph.Companion.findStartDestination
import androidx.navigation.NavHostController
import androidx.navigation.NavType
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController
import androidx.navigation.navArgument
import com.pullar.app.ui.screens.downloader.DownloaderScreen
import com.pullar.app.ui.screens.downloads.DownloadsScreen
import com.pullar.app.ui.screens.history.HistoryScreen
import com.pullar.app.ui.screens.player.MediaPlayerScreen
import com.pullar.app.ui.screens.settings.SettingsScreen
import com.pullar.app.ui.theme.CoffeeBean
import com.pullar.app.ui.theme.LightApricot
import com.pullar.app.ui.theme.MayaBlue
import com.pullar.app.ui.theme.SurfaceCard
import com.pullar.app.ui.theme.TextMuted
import com.pullar.app.ui.theme.ToffeeBrown
import java.net.URLDecoder

data class NavItem(
    val route: String,
    val title: String,
    val icon: ImageVector
)

@Composable
fun PullarApp(
    navController: NavHostController = rememberNavController(),
    sharedUrl: String? = null
) {
    val navBackStackEntry by navController.currentBackStackEntryAsState()
    val currentRoute = navBackStackEntry?.destination?.route

    val navItems = listOf(
        NavItem(Screen.Downloader.route, "Downloader", Icons.Default.Home),
        NavItem(Screen.Downloads.route, "Downloads", Icons.Default.Download),
        NavItem(Screen.History.route, "History", Icons.Default.History),
        NavItem(Screen.Settings.route, "Settings", Icons.Default.Settings)
    )

    val showBottomBar = currentRoute in navItems.map { it.route }

    Scaffold(
        bottomBar = {
            if (showBottomBar) {
                NavigationBar(
                    containerColor = SurfaceCard,
                    contentColor = LightApricot
                ) {
                    navItems.forEach { item ->
                        val selected = currentRoute == item.route
                        NavigationBarItem(
                            selected = selected,
                            onClick = {
                                navController.navigate(item.route) {
                                    popUpTo(navController.graph.findStartDestination().id) {
                                        saveState = true
                                    }
                                    launchSingleTop = true
                                    restoreState = true
                                }
                            },
                            icon = { Icon(item.icon, contentDescription = item.title) },
                            label = { Text(item.title) },
                            colors = NavigationBarItemDefaults.colors(
                                selectedIconColor = CoffeeBean,
                                selectedTextColor = MayaBlue,
                                indicatorColor = MayaBlue,
                                unselectedIconColor = TextMuted,
                                unselectedTextColor = TextMuted
                            )
                        )
                    }
                }
            }
        },
        containerColor = CoffeeBean
    ) { innerPadding ->
        NavHost(
            navController = navController,
            startDestination = Screen.Downloader.route,
            modifier = Modifier.padding(innerPadding)
        ) {
            composable(Screen.Downloader.route) {
                DownloaderScreen(
                    onNavigateToDownloads = {
                        navController.navigate(Screen.Downloads.route) {
                            popUpTo(navController.graph.findStartDestination().id) {
                                saveState = true
                            }
                            launchSingleTop = true
                            restoreState = true
                        }
                    }
                )
            }

            composable(Screen.Downloads.route) {
                DownloadsScreen(
                    onPlayMedia = { path ->
                        navController.navigate(Screen.Player.createRoute(path))
                    }
                )
            }

            composable(Screen.History.route) {
                HistoryScreen(
                    onPlayMedia = { path ->
                        navController.navigate(Screen.Player.createRoute(path))
                    }
                )
            }

            composable(Screen.Settings.route) {
                SettingsScreen()
            }

            composable(
                route = Screen.Player.route,
                arguments = listOf(navArgument("videoUri") { type = NavType.StringType })
            ) { backStackEntry ->
                val encodedUri = backStackEntry.arguments?.getString("videoUri") ?: ""
                val decodedUri = URLDecoder.decode(encodedUri, "UTF-8")
                MediaPlayerScreen(
                    mediaUriString = decodedUri,
                    onBack = { navController.popBackStack() }
                )
            }
        }
    }
}
