package com.pullar.app.ui.screens.settings

import android.os.Environment
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Folder
import androidx.compose.material.icons.filled.Info
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material.icons.filled.Speed
import androidx.compose.material.icons.filled.VideoSettings
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Icon
import androidx.compose.material3.Slider
import androidx.compose.material3.SliderDefaults
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.viewmodel.compose.viewModel
import com.pullar.app.ui.theme.CoffeeBean
import com.pullar.app.ui.theme.LightApricot
import com.pullar.app.ui.theme.MayaBlue
import com.pullar.app.ui.theme.SurfaceBorder
import com.pullar.app.ui.theme.SurfaceCard
import com.pullar.app.ui.theme.SurfaceDark
import com.pullar.app.ui.theme.TextMuted
import com.pullar.app.ui.theme.ToffeeBrown
import java.io.File

@Composable
fun SettingsScreen(
    viewModel: SettingsViewModel = viewModel()
) {
    val state by viewModel.uiState.collectAsState()
    val scrollState = rememberScrollState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(CoffeeBean)
            .padding(horizontal = 20.dp, vertical = 16.dp)
            .verticalScroll(scrollState)
    ) {
        // Header
        Text(
            text = "Settings",
            fontSize = 24.sp,
            fontWeight = FontWeight.Bold,
            color = MayaBlue
        )
        Text(
            text = "Preferences and engine configuration",
            fontSize = 13.sp,
            color = TextMuted,
            modifier = Modifier.padding(bottom = 20.dp)
        )

        // Storage Location Card
        Card(
            modifier = Modifier
                .fillMaxWidth()
                .border(1.dp, SurfaceBorder, RoundedCornerShape(14.dp)),
            colors = CardDefaults.cardColors(containerColor = SurfaceCard),
            shape = RoundedCornerShape(14.dp)
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Icon(Icons.Default.Folder, contentDescription = null, tint = MayaBlue)
                    Spacer(modifier = Modifier.width(10.dp))
                    Text(
                        text = "Storage Location",
                        fontSize = 15.sp,
                        fontWeight = FontWeight.SemiBold,
                        color = LightApricot
                    )
                }
                Spacer(modifier = Modifier.height(8.dp))
                val defaultPath = File(
                    Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS),
                    "Pullar"
                ).absolutePath
                Text(
                    text = defaultPath,
                    fontSize = 12.sp,
                    color = TextMuted,
                    modifier = Modifier
                        .background(SurfaceDark, RoundedCornerShape(6.dp))
                        .padding(horizontal = 10.dp, vertical = 8.dp)
                        .fillMaxWidth()
                )
            }
        }

        Spacer(modifier = Modifier.height(14.dp))

        // Max Concurrent Downloads Card
        Card(
            modifier = Modifier
                .fillMaxWidth()
                .border(1.dp, SurfaceBorder, RoundedCornerShape(14.dp)),
            colors = CardDefaults.cardColors(containerColor = SurfaceCard),
            shape = RoundedCornerShape(14.dp)
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Icon(Icons.Default.Speed, contentDescription = null, tint = MayaBlue)
                        Spacer(modifier = Modifier.width(10.dp))
                        Text(
                            text = "Concurrent Downloads",
                            fontSize = 15.sp,
                            fontWeight = FontWeight.SemiBold,
                            color = LightApricot
                        )
                    }
                    Text(
                        text = "${state.maxConcurrentDownloads}",
                        fontSize = 15.sp,
                        fontWeight = FontWeight.Bold,
                        color = MayaBlue
                    )
                }
                Spacer(modifier = Modifier.height(10.dp))
                Slider(
                    value = state.maxConcurrentDownloads.toFloat(),
                    onValueChange = { viewModel.onMaxConcurrentChanged(it.toInt()) },
                    valueRange = 1f..5f,
                    steps = 3,
                    colors = SliderDefaults.colors(
                        thumbColor = MayaBlue,
                        activeTrackColor = MayaBlue,
                        inactiveTrackColor = SurfaceBorder
                    )
                )
            }
        }

        Spacer(modifier = Modifier.height(14.dp))

        // Engine Updater Card
        Card(
            modifier = Modifier
                .fillMaxWidth()
                .border(1.dp, SurfaceBorder, RoundedCornerShape(14.dp)),
            colors = CardDefaults.cardColors(containerColor = SurfaceCard),
            shape = RoundedCornerShape(14.dp)
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Icon(Icons.Default.Refresh, contentDescription = null, tint = MayaBlue)
                    Spacer(modifier = Modifier.width(10.dp))
                    Text(
                        text = "yt-dlp Engine Update",
                        fontSize = 15.sp,
                        fontWeight = FontWeight.SemiBold,
                        color = LightApricot
                    )
                }
                Spacer(modifier = Modifier.height(6.dp))
                Text(
                    text = "Update the internal yt-dlp binary over the air to ensure compatibility with YouTube format changes.",
                    fontSize = 12.sp,
                    color = TextMuted
                )
                Spacer(modifier = Modifier.height(12.dp))
                Button(
                    onClick = { viewModel.updateEngine() },
                    enabled = !state.isUpdatingEngine,
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MayaBlue,
                        contentColor = CoffeeBean,
                        disabledContainerColor = SurfaceBorder,
                        disabledContentColor = TextMuted
                    ),
                    shape = RoundedCornerShape(8.dp)
                ) {
                    if (state.isUpdatingEngine) {
                        CircularProgressIndicator(
                            modifier = Modifier.size(16.dp),
                            color = CoffeeBean,
                            strokeWidth = 2.dp
                        )
                        Spacer(modifier = Modifier.width(8.dp))
                        Text("Updating...", fontSize = 12.sp)
                    } else {
                        Text("Check for yt-dlp Update", fontSize = 12.sp, fontWeight = FontWeight.Bold)
                    }
                }
                if (state.engineUpdateResult != null) {
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = state.engineUpdateResult ?: "",
                        fontSize = 12.sp,
                        color = MayaBlue
                    )
                }
            }
        }

        Spacer(modifier = Modifier.height(14.dp))

        // About Pullar Card
        Card(
            modifier = Modifier
                .fillMaxWidth()
                .border(1.dp, ToffeeBrown, RoundedCornerShape(14.dp)),
            colors = CardDefaults.cardColors(containerColor = SurfaceCard),
            shape = RoundedCornerShape(14.dp)
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Icon(Icons.Default.Info, contentDescription = null, tint = MayaBlue)
                    Spacer(modifier = Modifier.width(10.dp))
                    Text(
                        text = "About Pullar",
                        fontSize = 15.sp,
                        fontWeight = FontWeight.SemiBold,
                        color = LightApricot
                    )
                }
                Spacer(modifier = Modifier.height(8.dp))
                Text(
                    text = "Pullar",
                    fontSize = 18.sp,
                    fontWeight = FontWeight.Bold,
                    color = LightApricot
                )
                Text(
                    text = "Don't just watch—pull it",
                    fontSize = 13.sp,
                    color = MayaBlue
                )
                Spacer(modifier = Modifier.height(6.dp))
                Text(
                    text = "Version 1.0.0 (Android Native Kotlin & Jetpack Compose)",
                    fontSize = 11.sp,
                    color = TextMuted
                )
                Text(
                    text = "High-speed multi-connection segmented video and audio downloader powered by yt-dlp and FFmpeg.",
                    fontSize = 11.sp,
                    color = TextMuted,
                    modifier = Modifier.padding(top = 4.dp)
                )
                Spacer(modifier = Modifier.height(10.dp))
                Text(
                    text = "Technical Help & Software Projects:",
                    fontSize = 11.sp,
                    fontWeight = FontWeight.SemiBold,
                    color = LightApricot
                )
                Text(
                    text = "https://abdullahcs.pages.dev/",
                    fontSize = 12.sp,
                    fontWeight = FontWeight.Bold,
                    color = MayaBlue
                )
            }
        }
    }
}
