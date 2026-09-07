package com.pullar.app.ui.navigation

import androidx.compose.animation.animateColorAsState
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.interaction.MutableInteractionSource
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Download
import androidx.compose.material.icons.filled.History
import androidx.compose.material.icons.filled.Home
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material3.Badge
import androidx.compose.material3.BadgedBox
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.navigation.NavGraph.Companion.findStartDestination
import androidx.navigation.NavHostController
import androidx.navigation.NavType
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController
import androidx.navigation.navArgument
import com.pullar.app.data.database.PullarDatabase
import com.pullar.app.ui.screens.downloader.DownloaderScreen
import com.pullar.app.ui.screens.downloads.DownloadsScreen
import com.pullar.app.ui.screens.history.HistoryScreen
import com.pullar.app.ui.screens.player.MediaPlayerScreen
import com.pullar.app.ui.screens.settings.SettingsScreen
import com.pullar.app.ui.theme.MayaBlue
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

    val context = LocalContext.current
    val database = remember { PullarDatabase.getDatabase(context) }
    val activeDownloads by database.downloadDao().getActiveDownloads().collectAsState(initial = emptyList())
    val activeCount = activeDownloads.size

    Scaffold(
        bottomBar = {
            if (showBottomBar) {
                FloatingNavigationDock(
                    navItems = navItems,
                    currentRoute = currentRoute,
                    activeCount = activeCount,
                    onNavigate = { route ->
                        navController.navigate(route) {
                            popUpTo(navController.graph.findStartDestination().id) {
                                saveState = true
                            }
                            launchSingleTop = true
                            restoreState = true
                        }
                    }
                )
            }
        },
        containerColor = MaterialTheme.colorScheme.background
    ) { innerPadding ->
        NavHost(
            navController = navController,
            startDestination = Screen.Downloader.route,
            modifier = Modifier.padding(innerPadding)
        ) {
            composable(Screen.Downloader.route) {
                DownloaderScreen(
                    sharedUrl = sharedUrl,
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

@Composable
private fun FloatingNavigationDock(
    navItems: List<NavItem>,
    currentRoute: String?,
    activeCount: Int,
    onNavigate: (String) -> Unit
) {
    Box(
        modifier = Modifier
            .navigationBarsPadding()
            .padding(horizontal = 16.dp, vertical = 10.dp)
            .fillMaxWidth(),
        contentAlignment = Alignment.Center
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .height(64.dp)
                .clip(RoundedCornerShape(24.dp))
                .background(MaterialTheme.colorScheme.surfaceVariant)
                .border(1.dp, ToffeeBrown.copy(alpha = 0.35f), RoundedCornerShape(24.dp))
                .padding(horizontal = 8.dp),
            horizontalArrangement = Arrangement.SpaceAround,
            verticalAlignment = Alignment.CenterVertically
        ) {
            navItems.forEach { item ->
                val isSelected = currentRoute == item.route
                val contentColor by animateColorAsState(
                    targetValue = if (isSelected) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.onSurfaceVariant,
                    label = "dock_color"
                )

                Box(
                    modifier = Modifier
                        .weight(1f)
                        .clip(RoundedCornerShape(16.dp))
                        .clickable(
                            interactionSource = remember { MutableInteractionSource() },
                            indication = null
                        ) { onNavigate(item.route) }
                        .padding(vertical = 6.dp),
                    contentAlignment = Alignment.Center
                ) {
                    Column(
                        horizontalAlignment = Alignment.CenterHorizontally,
                        verticalArrangement = Arrangement.Center
                    ) {
                        if (item.route == Screen.Downloads.route && activeCount > 0) {
                            BadgedBox(
                                badge = {
                                    Badge(
                                        containerColor = MayaBlue,
                                        contentColor = MaterialTheme.colorScheme.onPrimary
                                    ) {
                                        Text(
                                            text = "$activeCount",
                                            fontSize = 10.sp,
                                            fontWeight = FontWeight.Bold
                                        )
                                    }
                                }
                            ) {
                                Icon(
                                    imageVector = item.icon,
                                    contentDescription = item.title,
                                    tint = contentColor,
                                    modifier = Modifier.size(22.dp)
                                )
                            }
                        } else {
                            Icon(
                                imageVector = item.icon,
                                contentDescription = item.title,
                                tint = contentColor,
                                modifier = Modifier.size(22.dp)
                            )
                        }

                        Spacer(modifier = Modifier.height(3.dp))

                        Text(
                            text = item.title,
                            color = contentColor,
                            fontSize = 11.sp,
                            fontWeight = if (isSelected) FontWeight.Bold else FontWeight.Normal
                        )
                    }
                }
            }
        }
    }
}
