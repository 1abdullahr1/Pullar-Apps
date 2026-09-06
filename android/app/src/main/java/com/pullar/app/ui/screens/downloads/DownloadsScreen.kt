package com.pullar.app.ui.screens.downloads

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Cancel
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.viewmodel.compose.viewModel
import coil.compose.AsyncImage
import com.pullar.app.data.model.DownloadEntity
import com.pullar.app.data.model.DownloadStatus
import com.pullar.app.ui.theme.CoffeeBean
import com.pullar.app.ui.theme.ErrorRed
import com.pullar.app.ui.theme.LightApricot
import com.pullar.app.ui.theme.MayaBlue
import com.pullar.app.ui.theme.SurfaceBorder
import com.pullar.app.ui.theme.SurfaceCard
import com.pullar.app.ui.theme.SurfaceDark
import com.pullar.app.ui.theme.TextMuted
import com.pullar.app.ui.theme.ToffeeBrown

@Composable
fun DownloadsScreen(
    viewModel: DownloadsViewModel = viewModel(),
    onPlayMedia: (String) -> Unit
) {
    val downloads by viewModel.activeDownloads.collectAsState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(CoffeeBean)
            .padding(horizontal = 20.dp, vertical = 16.dp)
    ) {
        // Header
        Text(
            text = "Active Downloads",
            fontSize = 24.sp,
            fontWeight = FontWeight.Bold,
            color = MayaBlue
        )
        Text(
            text = "${downloads.size} task(s) currently processing",
            fontSize = 13.sp,
            color = TextMuted,
            modifier = Modifier.padding(bottom = 16.dp)
        )

        if (downloads.isEmpty()) {
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(bottom = 40.dp),
                contentAlignment = Alignment.Center
            ) {
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Text(
                        text = "No Active Downloads",
                        fontSize = 18.sp,
                        fontWeight = FontWeight.SemiBold,
                        color = LightApricot
                    )
                    Spacer(modifier = Modifier.height(6.dp))
                    Text(
                        text = "Paste a link in the Downloader tab to start a download.",
                        fontSize = 13.sp,
                        color = TextMuted
                    )
                }
            }
        } else {
            LazyColumn(
                verticalArrangement = Arrangement.spacedBy(14.dp)
            ) {
                items(downloads, key = { it.id }) { task ->
                    DownloadCard(
                        task = task,
                        onCancel = { viewModel.cancelDownload(task) },
                        onRetry = { viewModel.retryDownload(task) },
                        onPlay = { onPlayMedia(task.filePath) }
                    )
                }
            }
        }
    }
}

@Composable
fun DownloadCard(
    task: DownloadEntity,
    onCancel: () -> Unit,
    onRetry: () -> Unit,
    onPlay: () -> Unit
) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .border(1.dp, SurfaceBorder, RoundedCornerShape(14.dp)),
        colors = CardDefaults.cardColors(containerColor = SurfaceCard),
        shape = RoundedCornerShape(14.dp)
    ) {
        Column(modifier = Modifier.padding(14.dp)) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically
            ) {
                // Thumbnail
                if (task.thumbnailUrl.isNotBlank()) {
                    AsyncImage(
                        model = task.thumbnailUrl,
                        contentDescription = null,
                        contentScale = ContentScale.Crop,
                        modifier = Modifier
                            .size(width = 80.dp, height = 55.dp)
                            .clip(RoundedCornerShape(8.dp))
                    )
                    Spacer(modifier = Modifier.width(12.dp))
                }

                // Title & Badges
                Column(modifier = Modifier.weight(1f)) {
                    Text(
                        text = task.title,
                        fontWeight = FontWeight.SemiBold,
                        fontSize = 14.sp,
                        color = LightApricot,
                        maxLines = 2,
                        overflow = TextOverflow.Ellipsis
                    )
                    Row(
                        modifier = Modifier.padding(top = 4.dp),
                        horizontalArrangement = Arrangement.spacedBy(6.dp)
                    ) {
                        Text(
                            text = task.qualityLabel,
                            fontSize = 11.sp,
                            fontWeight = FontWeight.Medium,
                            color = MayaBlue,
                            modifier = Modifier
                                .background(CoffeeBean, RoundedCornerShape(4.dp))
                                .padding(horizontal = 6.dp, vertical = 2.dp)
                        )
                        if (task.isPlaylist) {
                            Text(
                                text = "Playlist",
                                fontSize = 11.sp,
                                fontWeight = FontWeight.Medium,
                                color = ToffeeBrown,
                                modifier = Modifier
                                    .background(CoffeeBean, RoundedCornerShape(4.dp))
                                    .padding(horizontal = 6.dp, vertical = 2.dp)
                            )
                        }
                    }
                }

                // Cancel Button
                IconButton(onClick = onCancel) {
                    Icon(
                        Icons.Default.Cancel,
                        contentDescription = "Cancel",
                        tint = TextMuted
                    )
                }
            }

            Spacer(modifier = Modifier.height(10.dp))

            // Progress Bar
            LinearProgressIndicator(
                progress = { (task.progress / 100f).coerceIn(0f, 1f) },
                modifier = Modifier
                    .fillMaxWidth()
                    .height(6.dp)
                    .clip(RoundedCornerShape(3.dp)),
                color = MayaBlue,
                trackColor = SurfaceDark
            )

            Spacer(modifier = Modifier.height(6.dp))

            // Status, Speed, ETA
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                val statusText = when (task.status) {
                    DownloadStatus.QUEUED -> "Queued"
                    DownloadStatus.ANALYZING -> "Analyzing..."
                    DownloadStatus.DOWNLOADING -> "${task.progress.toInt()}%"
                    DownloadStatus.PAUSED -> "Paused"
                    DownloadStatus.COMPLETED -> "Completed"
                    DownloadStatus.FAILED -> "Failed"
                    DownloadStatus.CANCELLED -> "Cancelled"
                }

                Text(
                    text = statusText,
                    fontSize = 12.sp,
                    fontWeight = FontWeight.Medium,
                    color = if (task.status == DownloadStatus.FAILED) ErrorRed else MayaBlue
                )

                if (task.speed.isNotBlank()) {
                    Text(
                        text = "${task.speed}  ${task.eta}",
                        fontSize = 12.sp,
                        color = TextMuted
                    )
                }
            }

            // Retry or Play Media button
            if (task.status == DownloadStatus.FAILED) {
                Spacer(modifier = Modifier.height(8.dp))
                OutlinedButton(
                    onClick = onRetry,
                    colors = ButtonDefaults.outlinedButtonColors(contentColor = MayaBlue),
                    shape = RoundedCornerShape(8.dp)
                ) {
                    Icon(Icons.Default.Refresh, contentDescription = null, modifier = Modifier.size(16.dp))
                    Spacer(modifier = Modifier.width(6.dp))
                    Text("Retry Download", fontSize = 12.sp)
                }
            } else if (task.status == DownloadStatus.COMPLETED) {
                Spacer(modifier = Modifier.height(8.dp))
                Button(
                    onClick = onPlay,
                    colors = ButtonDefaults.buttonColors(containerColor = MayaBlue, contentColor = CoffeeBean),
                    shape = RoundedCornerShape(8.dp)
                ) {
                    Icon(Icons.Default.PlayArrow, contentDescription = null, modifier = Modifier.size(16.dp))
                    Spacer(modifier = Modifier.width(6.dp))
                    Text("Play Media", fontSize = 12.sp, fontWeight = FontWeight.Bold)
                }
            }
        }
    }
}
