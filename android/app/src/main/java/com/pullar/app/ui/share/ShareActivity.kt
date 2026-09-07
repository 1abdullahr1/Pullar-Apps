package com.pullar.app.ui.share

import android.content.Intent
import android.os.Bundle
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.interaction.MutableInteractionSource
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.Download
import androidx.compose.material.icons.filled.Warning
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Checkbox
import androidx.compose.material3.CheckboxDefaults
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.RadioButton
import androidx.compose.material3.RadioButtonDefaults
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.lifecycleScope
import coil.compose.AsyncImage
import com.pullar.app.MainActivity
import com.pullar.app.core.YoutubeDLEngine
import com.pullar.app.data.database.PullarDatabase
import com.pullar.app.data.model.DownloadEntity
import com.pullar.app.data.model.DownloadStatus
import com.pullar.app.data.model.FormatOption
import com.pullar.app.data.model.VideoMetadata
import com.pullar.app.data.preferences.ThemePreferences
import com.pullar.app.data.repository.DownloadRepository
import com.pullar.app.service.DownloadForegroundService
import com.pullar.app.ui.theme.CoffeeBean
import com.pullar.app.ui.theme.ErrorRed
import com.pullar.app.ui.theme.LightApricot
import com.pullar.app.ui.theme.MayaBlue
import com.pullar.app.ui.theme.PullarTheme
import com.pullar.app.ui.theme.ToffeeBrown
import com.pullar.app.util.NetworkCheckResult
import com.pullar.app.util.NetworkUtils
import kotlinx.coroutines.flow.firstOrNull
import kotlinx.coroutines.launch
import java.util.UUID

class ShareActivity : ComponentActivity() {

    private lateinit var repository: DownloadRepository
    private lateinit var themePreferences: ThemePreferences

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val db = PullarDatabase.getDatabase(applicationContext)
        repository = DownloadRepository(db.downloadDao())
        themePreferences = ThemePreferences(applicationContext)

        val rawText = intent?.getStringExtra(Intent.EXTRA_TEXT)
        val extractedUrl = extractUrl(rawText)

        if (extractedUrl.isNullOrBlank()) {
            Toast.makeText(this, "No valid link found in shared content", Toast.LENGTH_SHORT).show()
            finish()
            return
        }

        setContent {
            PullarTheme {
                ShareBottomSheet(
                    url = extractedUrl,
                    onDismiss = { finish() },
                    onStartDownload = { meta, format, downloadEntirePlaylist ->
                        performDownloadAndClose(meta, format, downloadEntirePlaylist)
                    },
                    onOpenInPullar = {
                        val mainIntent = Intent(this, MainActivity::class.java).apply {
                            action = Intent.ACTION_SEND
                            putExtra(Intent.EXTRA_TEXT, extractedUrl)
                            addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TOP)
                        }
                        startActivity(mainIntent)
                        finish()
                    }
                )
            }
        }
    }

    private fun extractUrl(text: String?): String? {
        if (text.isNullOrBlank()) return null
        val urlRegex = Regex("""https?://[^\s]+""")
        return urlRegex.find(text)?.value
    }

    private fun performDownloadAndClose(
        meta: VideoMetadata,
        format: FormatOption,
        downloadEntirePlaylist: Boolean
    ) {
        lifecycleScope.launch {
            val wifiEnabled = themePreferences.wifiDownloadsFlow.firstOrNull() ?: true
            val mobileDataEnabled = themePreferences.mobileDataDownloadsFlow.firstOrNull() ?: true

            val netCheck = NetworkUtils.isDownloadAllowed(this@ShareActivity, wifiEnabled, mobileDataEnabled)
            if (netCheck is NetworkCheckResult.Blocked) {
                Toast.makeText(this@ShareActivity, netCheck.reason, Toast.LENGTH_LONG).show()
                return@launch
            }

            if (meta.isPlaylist && meta.playlistItems.isNotEmpty() && downloadEntirePlaylist) {
                val entities = meta.playlistItems.mapIndexed { index, item ->
                    DownloadEntity(
                        id = UUID.randomUUID().toString(),
                        url = item.url,
                        title = item.title,
                        uploader = meta.uploader,
                        thumbnailUrl = item.thumbnailUrl.ifBlank { meta.thumbnailUrl },
                        formatId = format.formatId,
                        qualityLabel = format.label,
                        isPlaylist = true,
                        playlistTitle = meta.title,
                        playlistIndex = index + 1,
                        playlistTotal = meta.playlistItems.size,
                        isAudioOnly = format.isAudioOnly,
                        status = DownloadStatus.QUEUED
                    )
                }
                repository.enqueueAll(entities)
                DownloadForegroundService.startDownload(this@ShareActivity)
                Toast.makeText(
                    this@ShareActivity,
                    "Queued ${entities.size} videos from playlist in background",
                    Toast.LENGTH_SHORT
                ).show()
            } else {
                val taskId = UUID.randomUUID().toString()
                val entity = DownloadEntity(
                    id = taskId,
                    url = meta.url,
                    title = meta.title,
                    uploader = meta.uploader,
                    thumbnailUrl = meta.thumbnailUrl,
                    formatId = format.formatId,
                    qualityLabel = format.label,
                    isPlaylist = false,
                    isAudioOnly = format.isAudioOnly,
                    status = DownloadStatus.QUEUED
                )
                repository.enqueue(entity)
                DownloadForegroundService.startDownload(this@ShareActivity, taskId)
                Toast.makeText(
                    this@ShareActivity,
                    "Download started in background",
                    Toast.LENGTH_SHORT
                ).show()
            }

            finish()
        }
    }
}

@Composable
private fun ShareBottomSheet(
    url: String,
    onDismiss: () -> Unit,
    onStartDownload: (VideoMetadata, FormatOption, Boolean) -> Unit,
    onOpenInPullar: () -> Unit
) {
    var isAnalyzing by remember { mutableStateOf(true) }
    var metadata by remember { mutableStateOf<VideoMetadata?>(null) }
    var errorMessage by remember { mutableStateOf<String?>(null) }
    var selectedFormat by remember { mutableStateOf<FormatOption?>(null) }
    var downloadEntirePlaylist by remember { mutableStateOf(true) }

    LaunchedEffect(url) {
        isAnalyzing = true
        errorMessage = null
        val result = YoutubeDLEngine.extractInfo(url)
        result.onSuccess { meta ->
            isAnalyzing = false
            metadata = meta
            selectedFormat = meta.availableFormats.firstOrNull()
        }.onFailure { err ->
            isAnalyzing = false
            errorMessage = err.message ?: "Failed to extract video details"
        }
    }

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black.copy(alpha = 0.55f))
            .clickable(
                interactionSource = remember { MutableInteractionSource() },
                indication = null
            ) { onDismiss() },
        contentAlignment = Alignment.BottomCenter
    ) {
        Card(
            modifier = Modifier
                .fillMaxWidth()
                .navigationBarsPadding()
                .clickable(
                    interactionSource = remember { MutableInteractionSource() },
                    indication = null
                ) { /* Consume clicks inside sheet */ },
            shape = RoundedCornerShape(topStart = 24.dp, topEnd = 24.dp),
            colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceVariant)
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 20.dp, vertical = 16.dp)
            ) {
                // Drag Handle Pill
                Box(
                    modifier = Modifier
                        .align(Alignment.CenterHorizontally)
                        .size(width = 40.dp, height = 4.dp)
                        .clip(RoundedCornerShape(2.dp))
                        .background(MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.4f))
                )

                Spacer(modifier = Modifier.height(14.dp))

                // Header
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Column {
                        Text(
                            text = "Pullar Quick Download",
                            fontSize = 18.sp,
                            fontWeight = FontWeight.Bold,
                            color = MaterialTheme.colorScheme.primary
                        )
                        Text(
                            text = "Download without leaving your app",
                            fontSize = 12.sp,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                    IconButton(onClick = onDismiss) {
                        Icon(
                            Icons.Default.Close,
                            contentDescription = "Close",
                            tint = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                }

                Spacer(modifier = Modifier.height(14.dp))

                // State Content
                when {
                    isAnalyzing -> {
                        Box(
                            modifier = Modifier
                                .fillMaxWidth()
                                .height(160.dp),
                            contentAlignment = Alignment.Center
                        ) {
                            Column(horizontalAlignment = Alignment.CenterHorizontally) {
                                CircularProgressIndicator(
                                    color = MaterialTheme.colorScheme.primary,
                                    strokeWidth = 3.dp,
                                    modifier = Modifier.size(36.dp)
                                )
                                Spacer(modifier = Modifier.height(14.dp))
                                Text(
                                    text = "Analyzing link and formats...",
                                    fontSize = 14.sp,
                                    color = MaterialTheme.colorScheme.onSurface
                                )
                            }
                        }
                    }

                    errorMessage != null -> {
                        Column(
                            modifier = Modifier
                                .fillMaxWidth()
                                .padding(vertical = 12.dp),
                            horizontalAlignment = Alignment.CenterHorizontally
                        ) {
                            Icon(
                                Icons.Default.Warning,
                                contentDescription = null,
                                tint = ErrorRed,
                                modifier = Modifier.size(32.dp)
                            )
                            Spacer(modifier = Modifier.height(8.dp))
                            Text(
                                text = errorMessage ?: "Failed to analyze link",
                                color = ErrorRed,
                                fontSize = 13.sp
                            )
                            Spacer(modifier = Modifier.height(16.dp))
                            Row(horizontalArrangement = Arrangement.spacedBy(10.dp)) {
                                OutlinedButton(
                                    onClick = onDismiss,
                                    shape = RoundedCornerShape(8.dp)
                                ) {
                                    Text("Dismiss", fontSize = 13.sp)
                                }
                                Button(
                                    onClick = onOpenInPullar,
                                    colors = ButtonDefaults.buttonColors(
                                        containerColor = MaterialTheme.colorScheme.primary,
                                        contentColor = MaterialTheme.colorScheme.onPrimary
                                    ),
                                    shape = RoundedCornerShape(8.dp)
                                ) {
                                    Text("Open in Full App", fontSize = 13.sp, fontWeight = FontWeight.Bold)
                                }
                            }
                        }
                    }

                    metadata != null -> {
                        val meta = metadata!!
                        Column(
                            modifier = Modifier
                                .fillMaxWidth()
                                .verticalScroll(rememberScrollState())
                        ) {
                            // Video preview card
                            Row(
                                modifier = Modifier
                                    .fillMaxWidth()
                                    .background(MaterialTheme.colorScheme.surface, RoundedCornerShape(12.dp))
                                    .padding(10.dp),
                                verticalAlignment = Alignment.CenterVertically
                            ) {
                                if (meta.thumbnailUrl.isNotBlank()) {
                                    AsyncImage(
                                        model = meta.thumbnailUrl,
                                        contentDescription = null,
                                        contentScale = ContentScale.Crop,
                                        modifier = Modifier
                                            .size(width = 72.dp, height = 48.dp)
                                            .clip(RoundedCornerShape(8.dp))
                                    )
                                    Spacer(modifier = Modifier.width(10.dp))
                                }
                                Column(modifier = Modifier.weight(1f)) {
                                    Text(
                                        text = meta.title,
                                        fontSize = 13.sp,
                                        fontWeight = FontWeight.SemiBold,
                                        color = MaterialTheme.colorScheme.onSurface,
                                        maxLines = 2,
                                        overflow = TextOverflow.Ellipsis
                                    )
                                    if (meta.uploader.isNotBlank()) {
                                        Text(
                                            text = meta.uploader,
                                            fontSize = 11.sp,
                                            color = MaterialTheme.colorScheme.onSurfaceVariant
                                        )
                                    }
                                }
                            }

                            // Playlist option
                            if (meta.isPlaylist) {
                                Spacer(modifier = Modifier.height(10.dp))
                                Row(
                                    modifier = Modifier
                                        .fillMaxWidth()
                                        .background(MaterialTheme.colorScheme.surface, RoundedCornerShape(10.dp))
                                        .clickable { downloadEntirePlaylist = !downloadEntirePlaylist }
                                        .padding(10.dp),
                                    verticalAlignment = Alignment.CenterVertically
                                ) {
                                    Checkbox(
                                        checked = downloadEntirePlaylist,
                                        onCheckedChange = { downloadEntirePlaylist = it },
                                        colors = CheckboxDefaults.colors(
                                            checkedColor = MaterialTheme.colorScheme.primary,
                                            uncheckedColor = MaterialTheme.colorScheme.onSurfaceVariant
                                        )
                                    )
                                    Spacer(modifier = Modifier.width(6.dp))
                                    Text(
                                        text = "Download entire playlist (${meta.playlistItems.size} videos)",
                                        fontSize = 13.sp,
                                        color = MaterialTheme.colorScheme.onSurface
                                    )
                                }
                            }

                            Spacer(modifier = Modifier.height(12.dp))

                            Text(
                                text = "Choose Format",
                                fontSize = 13.sp,
                                fontWeight = FontWeight.SemiBold,
                                color = MaterialTheme.colorScheme.onSurface
                            )

                            Spacer(modifier = Modifier.height(6.dp))

                            meta.availableFormats.forEach { format ->
                                Row(
                                    modifier = Modifier
                                        .fillMaxWidth()
                                        .clip(RoundedCornerShape(8.dp))
                                        .clickable { selectedFormat = format }
                                        .padding(vertical = 4.dp),
                                    verticalAlignment = Alignment.CenterVertically
                                ) {
                                    RadioButton(
                                        selected = selectedFormat?.formatId == format.formatId,
                                        onClick = { selectedFormat = format },
                                        colors = RadioButtonDefaults.colors(
                                            selectedColor = MaterialTheme.colorScheme.primary,
                                            unselectedColor = MaterialTheme.colorScheme.onSurfaceVariant
                                        )
                                    )
                                    Spacer(modifier = Modifier.width(6.dp))
                                    Column {
                                        Text(
                                            text = format.label,
                                            fontSize = 13.sp,
                                            fontWeight = FontWeight.Medium,
                                            color = MaterialTheme.colorScheme.onSurface
                                        )
                                        if (format.note.isNotBlank()) {
                                            Text(
                                                text = format.note,
                                                fontSize = 10.sp,
                                                color = MaterialTheme.colorScheme.onSurfaceVariant
                                            )
                                        }
                                    }
                                }
                            }

                            Spacer(modifier = Modifier.height(16.dp))

                            Row(
                                modifier = Modifier.fillMaxWidth(),
                                horizontalArrangement = Arrangement.spacedBy(10.dp)
                            ) {
                                OutlinedButton(
                                    onClick = onDismiss,
                                    modifier = Modifier
                                        .weight(1f)
                                        .height(48.dp),
                                    shape = RoundedCornerShape(12.dp)
                                ) {
                                    Text("Cancel", fontSize = 14.sp)
                                }
                                Button(
                                    onClick = {
                                        val fmt = selectedFormat ?: meta.availableFormats.firstOrNull() ?: return@Button
                                        onStartDownload(meta, fmt, downloadEntirePlaylist)
                                    },
                                    modifier = Modifier
                                        .weight(1.5f)
                                        .height(48.dp),
                                    colors = ButtonDefaults.buttonColors(
                                        containerColor = MaterialTheme.colorScheme.primary,
                                        contentColor = MaterialTheme.colorScheme.onPrimary
                                    ),
                                    shape = RoundedCornerShape(12.dp)
                                ) {
                                    Icon(Icons.Default.Download, contentDescription = null, modifier = Modifier.size(18.dp))
                                    Spacer(modifier = Modifier.width(6.dp))
                                    Text("Download", fontSize = 14.sp, fontWeight = FontWeight.Bold)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
