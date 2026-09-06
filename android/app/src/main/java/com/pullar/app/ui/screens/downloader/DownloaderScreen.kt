package com.pullar.app.ui.screens.downloader

import android.content.ClipboardManager
import android.content.Context
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
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
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Clear
import androidx.compose.material.icons.filled.ContentPaste
import androidx.compose.material.icons.filled.Download
import androidx.compose.material.icons.filled.Queue
import androidx.compose.material.icons.filled.Search
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Checkbox
import androidx.compose.material3.CheckboxDefaults
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.OutlinedTextFieldDefaults
import androidx.compose.material3.RadioButton
import androidx.compose.material3.RadioButtonDefaults
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.viewmodel.compose.viewModel
import coil.compose.AsyncImage
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
fun DownloaderScreen(
    viewModel: DownloaderViewModel = viewModel(),
    onNavigateToDownloads: () -> Unit
) {
    val state by viewModel.uiState.collectAsState()
    val context = LocalContext.current
    val scrollState = rememberScrollState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(CoffeeBean)
            .padding(horizontal = 20.dp, vertical = 16.dp)
            .verticalScroll(scrollState)
    ) {
        // App Header & Branding
        Text(
            text = "Pullar",
            fontSize = 28.sp,
            fontWeight = FontWeight.Bold,
            color = MayaBlue
        )
        Text(
            text = "Don't just watch—pull it",
            fontSize = 14.sp,
            color = LightApricot.copy(alpha = 0.8f),
            modifier = Modifier.padding(bottom = 20.dp)
        )

        // URL Input Field
        OutlinedTextField(
            value = state.urlInput,
            onValueChange = { viewModel.onUrlChanged(it) },
            label = { Text("Paste Video or Playlist URL", color = TextMuted) },
            placeholder = { Text("https://www.youtube.com/watch?v=...", color = TextMuted.copy(alpha = 0.5f)) },
            trailingIcon = {
                Row {
                    if (state.urlInput.isNotEmpty()) {
                        IconButton(onClick = { viewModel.onUrlChanged("") }) {
                            Icon(Icons.Default.Clear, contentDescription = "Clear", tint = TextMuted)
                        }
                    }
                    IconButton(onClick = {
                        val clipboard = context.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
                        val clip = clipboard.primaryClip
                        if (clip != null && clip.itemCount > 0) {
                            val text = clip.getItemAt(0).text?.toString() ?: ""
                            viewModel.onUrlChanged(text)
                        }
                    }) {
                        Icon(Icons.Default.ContentPaste, contentDescription = "Paste", tint = MayaBlue)
                    }
                }
            },
            modifier = Modifier.fillMaxWidth(),
            singleLine = true,
            colors = OutlinedTextFieldDefaults.colors(
                focusedTextColor = LightApricot,
                unfocusedTextColor = LightApricot,
                focusedBorderColor = MayaBlue,
                unfocusedBorderColor = SurfaceBorder,
                focusedContainerColor = SurfaceDark,
                unfocusedContainerColor = SurfaceDark
            ),
            shape = RoundedCornerShape(12.dp)
        )

        Spacer(modifier = Modifier.height(12.dp))

        // Analyze Button
        Button(
            onClick = { viewModel.analyzeVideo() },
            enabled = !state.isAnalyzing && state.urlInput.isNotBlank(),
            modifier = Modifier
                .fillMaxWidth()
                .height(48.dp),
            colors = ButtonDefaults.buttonColors(
                containerColor = MayaBlue,
                contentColor = CoffeeBean,
                disabledContainerColor = SurfaceBorder,
                disabledContentColor = TextMuted
            ),
            shape = RoundedCornerShape(12.dp)
        ) {
            if (state.isAnalyzing) {
                CircularProgressIndicator(
                    modifier = Modifier.size(20.dp),
                    color = CoffeeBean,
                    strokeWidth = 2.dp
                )
                Spacer(modifier = Modifier.width(8.dp))
                Text("Analyzing Video...", fontWeight = FontWeight.SemiBold)
            } else {
                Icon(Icons.Default.Search, contentDescription = null, modifier = Modifier.size(18.dp))
                Spacer(modifier = Modifier.width(8.dp))
                Text("Analyze Video", fontWeight = FontWeight.Bold)
            }
        }

        // Error Message
        if (state.errorMessage != null) {
            Spacer(modifier = Modifier.height(14.dp))
            Card(
                modifier = Modifier.fillMaxWidth(),
                colors = CardDefaults.cardColors(containerColor = SurfaceCard),
                shape = RoundedCornerShape(10.dp)
            ) {
                Text(
                    text = state.errorMessage ?: "",
                    color = ErrorRed,
                    fontSize = 13.sp,
                    modifier = Modifier.padding(14.dp)
                )
            }
        }

        // Queue Feedback Banner (UX auto-reset confirmation)
        if (state.showQueueConfirmation) {
            Spacer(modifier = Modifier.height(16.dp))
            Card(
                modifier = Modifier
                    .fillMaxWidth()
                    .border(1.dp, MayaBlue.copy(alpha = 0.5f), RoundedCornerShape(12.dp)),
                colors = CardDefaults.cardColors(containerColor = SurfaceCard),
                shape = RoundedCornerShape(12.dp)
            ) {
                Column(modifier = Modifier.padding(16.dp)) {
                    Text(
                        text = "Download Queued Successfully",
                        color = MayaBlue,
                        fontWeight = FontWeight.Bold,
                        fontSize = 15.sp
                    )
                    if (state.queuedTaskTitle.isNotBlank()) {
                        Text(
                            text = state.queuedTaskTitle,
                            color = LightApricot,
                            fontSize = 13.sp,
                            maxLines = 2,
                            overflow = TextOverflow.Ellipsis,
                            modifier = Modifier.padding(vertical = 4.dp)
                        )
                    }
                    Text(
                        text = "The download has begun in background. The downloader is ready for a new video.",
                        color = TextMuted,
                        fontSize = 12.sp,
                        modifier = Modifier.padding(bottom = 12.dp)
                    )
                    Button(
                        onClick = onNavigateToDownloads,
                        colors = ButtonDefaults.buttonColors(
                            containerColor = MayaBlue,
                            contentColor = CoffeeBean
                        ),
                        shape = RoundedCornerShape(8.dp)
                    ) {
                        Icon(Icons.Default.Queue, contentDescription = null, modifier = Modifier.size(16.dp))
                        Spacer(modifier = Modifier.width(6.dp))
                        Text("View Downloads Queue", fontWeight = FontWeight.Bold)
                    }
                }
            }
        }

        // Analyzed Metadata & Download Options Card
        val meta = state.metadata
        if (meta != null) {
            Spacer(modifier = Modifier.height(20.dp))
            Card(
                modifier = Modifier
                    .fillMaxWidth()
                    .border(1.dp, ToffeeBrown, RoundedCornerShape(16.dp)),
                colors = CardDefaults.cardColors(containerColor = SurfaceCard),
                shape = RoundedCornerShape(16.dp)
            ) {
                Column(modifier = Modifier.padding(16.dp)) {
                    // Thumbnail
                    if (meta.thumbnailUrl.isNotBlank()) {
                        AsyncImage(
                            model = meta.thumbnailUrl,
                            contentDescription = meta.title,
                            contentScale = ContentScale.Crop,
                            modifier = Modifier
                                .fillMaxWidth()
                                .height(180.dp)
                                .clip(RoundedCornerShape(10.dp))
                        )
                        Spacer(modifier = Modifier.height(12.dp))
                    }

                    // Title
                    Text(
                        text = meta.title,
                        fontSize = 16.sp,
                        fontWeight = FontWeight.Bold,
                        color = LightApricot,
                        maxLines = 2,
                        overflow = TextOverflow.Ellipsis
                    )

                    // Uploader & Duration
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(top = 4.dp, bottom = 12.dp),
                        horizontalArrangement = Arrangement.SpaceBetween
                    ) {
                        if (meta.uploader.isNotBlank()) {
                            Text(text = meta.uploader, fontSize = 13.sp, color = TextMuted)
                        }
                        if (meta.durationFormatted.isNotBlank()) {
                            Text(text = meta.durationFormatted, fontSize = 13.sp, color = MayaBlue)
                        }
                    }

                    // Playlist Toggle Option
                    if (meta.isPlaylist) {
                        Card(
                            modifier = Modifier
                                .fillMaxWidth()
                                .padding(vertical = 8.dp),
                            colors = CardDefaults.cardColors(containerColor = SurfaceDark),
                            shape = RoundedCornerShape(10.dp)
                        ) {
                            Row(
                                modifier = Modifier
                                    .fillMaxWidth()
                                    .clickable { viewModel.onPlaylistToggled(!state.isPlaylistChecked) }
                                    .padding(12.dp),
                                verticalAlignment = Alignment.CenterVertically
                            ) {
                                Checkbox(
                                    checked = state.isPlaylistChecked,
                                    onCheckedChange = { viewModel.onPlaylistToggled(it) },
                                    colors = CheckboxDefaults.colors(
                                        checkedColor = MayaBlue,
                                        uncheckedColor = TextMuted,
                                        checkmarkColor = CoffeeBean
                                    )
                                )
                                Spacer(modifier = Modifier.width(8.dp))
                                Column {
                                    Text(
                                        text = "Download Entire Playlist",
                                        fontWeight = FontWeight.SemiBold,
                                        color = LightApricot,
                                        fontSize = 14.sp
                                    )
                                    Text(
                                        text = "Downloads all playlist videos into a dedicated folder",
                                        color = TextMuted,
                                        fontSize = 12.sp
                                    )
                                }
                            }
                        }
                    }

                    // Format Selection
                    Text(
                        text = "Select Format & Quality",
                        fontSize = 14.sp,
                        fontWeight = FontWeight.SemiBold,
                        color = LightApricot,
                        modifier = Modifier.padding(top = 6.dp, bottom = 8.dp)
                    )

                    meta.availableFormats.forEach { format ->
                        Row(
                            modifier = Modifier
                                .fillMaxWidth()
                                .clip(RoundedCornerShape(8.dp))
                                .clickable { viewModel.onFormatSelected(format) }
                                .padding(vertical = 4.dp, horizontal = 6.dp),
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            RadioButton(
                                selected = state.selectedFormat?.formatId == format.formatId,
                                onClick = { viewModel.onFormatSelected(format) },
                                colors = RadioButtonDefaults.colors(
                                    selectedColor = MayaBlue,
                                    unselectedColor = TextMuted
                                )
                            )
                            Spacer(modifier = Modifier.width(6.dp))
                            Column {
                                Text(
                                    text = format.label,
                                    fontSize = 14.sp,
                                    fontWeight = FontWeight.Medium,
                                    color = LightApricot
                                )
                                if (format.note.isNotBlank()) {
                                    Text(
                                        text = format.note,
                                        fontSize = 11.sp,
                                        color = TextMuted
                                    )
                                }
                            }
                        }
                    }

                    Spacer(modifier = Modifier.height(16.dp))

                    // Start Download / Pull Video Button
                    Button(
                        onClick = { viewModel.startDownload() },
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(50.dp),
                        colors = ButtonDefaults.buttonColors(
                            containerColor = MayaBlue,
                            contentColor = CoffeeBean
                        ),
                        shape = RoundedCornerShape(12.dp)
                    ) {
                        Icon(Icons.Default.Download, contentDescription = null, modifier = Modifier.size(20.dp))
                        Spacer(modifier = Modifier.width(8.dp))
                        Text("Start Download", fontSize = 16.sp, fontWeight = FontWeight.Bold)
                    }
                }
            }
        }
    }
}
