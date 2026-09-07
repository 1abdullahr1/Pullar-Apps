package com.pullar.app.ui.screens.downloader

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.pullar.app.core.YoutubeDLEngine
import com.pullar.app.data.database.PullarDatabase
import com.pullar.app.data.model.DownloadEntity
import com.pullar.app.data.model.DownloadStatus
import com.pullar.app.data.model.FormatOption
import com.pullar.app.data.model.VideoMetadata
import com.pullar.app.data.preferences.ThemePreferences
import com.pullar.app.data.repository.DownloadRepository
import com.pullar.app.service.DownloadForegroundService
import com.pullar.app.util.NetworkCheckResult
import com.pullar.app.util.NetworkUtils
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.firstOrNull
import kotlinx.coroutines.launch
import java.util.UUID

data class DownloaderUiState(
    val urlInput: String = "",
    val isAnalyzing: Boolean = false,
    val metadata: VideoMetadata? = null,
    val selectedFormat: FormatOption? = null,
    val isPlaylistMode: Boolean = false,
    val downloadEntirePlaylist: Boolean = true,
    val selectedItemIds: Set<String> = emptySet(),
    val errorMessage: String? = null,
    val showQueueConfirmation: Boolean = false,
    val queuedTaskTitle: String = ""
)

class DownloaderViewModel(application: Application) : AndroidViewModel(application) {

    private val repository: DownloadRepository
    private val themePreferences: ThemePreferences
    private val _uiState = MutableStateFlow(DownloaderUiState())
    val uiState: StateFlow<DownloaderUiState> = _uiState.asStateFlow()

    init {
        val db = PullarDatabase.getDatabase(application)
        repository = DownloadRepository(db.downloadDao())
        themePreferences = ThemePreferences(application)
    }

    fun onUrlChanged(newUrl: String) {
        _uiState.value = _uiState.value.copy(
            urlInput = newUrl,
            errorMessage = null,
            showQueueConfirmation = false
        )
    }

    fun onFormatSelected(format: FormatOption) {
        _uiState.value = _uiState.value.copy(selectedFormat = format)
    }

    fun onDownloadEntirePlaylistToggled(entire: Boolean) {
        _uiState.value = _uiState.value.copy(downloadEntirePlaylist = entire)
    }

    fun onTogglePlaylistItem(itemId: String) {
        val current = _uiState.value.selectedItemIds.toMutableSet()
        if (current.contains(itemId)) {
            current.remove(itemId)
        } else {
            current.add(itemId)
        }
        _uiState.value = _uiState.value.copy(selectedItemIds = current)
    }

    fun onSelectAllPlaylistItems() {
        val allIds = _uiState.value.metadata?.playlistItems?.map { it.id }?.toSet() ?: emptySet()
        _uiState.value = _uiState.value.copy(selectedItemIds = allIds)
    }

    fun onClearPlaylistItems() {
        _uiState.value = _uiState.value.copy(selectedItemIds = emptySet())
    }

    fun analyzeVideo() {
        val url = _uiState.value.urlInput.trim()
        if (url.isBlank()) {
            _uiState.value = _uiState.value.copy(errorMessage = "Please enter a valid video or playlist link")
            return
        }

        viewModelScope.launch {
            _uiState.value = _uiState.value.copy(
                isAnalyzing = true,
                errorMessage = null,
                showQueueConfirmation = false
            )

            val result = YoutubeDLEngine.extractInfo(url)
            result.onSuccess { meta ->
                val defaultFormat = meta.availableFormats.firstOrNull()
                val allIds = meta.playlistItems.map { it.id }.toSet()

                _uiState.value = _uiState.value.copy(
                    isAnalyzing = false,
                    metadata = meta,
                    selectedFormat = defaultFormat,
                    isPlaylistMode = meta.isPlaylist,
                    downloadEntirePlaylist = true,
                    selectedItemIds = allIds
                )
            }.onFailure { err ->
                _uiState.value = _uiState.value.copy(
                    isAnalyzing = false,
                    errorMessage = err.message ?: "Failed to extract video information"
                )
            }
        }
    }

    fun startDownload() {
        val state = _uiState.value
        val meta = state.metadata ?: return
        val format = state.selectedFormat ?: meta.availableFormats.firstOrNull() ?: return

        viewModelScope.launch {
            // Check network rules from user preferences
            val wifiEnabled = themePreferences.wifiDownloadsFlow.firstOrNull() ?: true
            val mobileDataEnabled = themePreferences.mobileDataDownloadsFlow.firstOrNull() ?: true

            val netCheck = NetworkUtils.isDownloadAllowed(getApplication(), wifiEnabled, mobileDataEnabled)
            if (netCheck is NetworkCheckResult.Blocked) {
                _uiState.value = _uiState.value.copy(errorMessage = netCheck.reason)
                return@launch
            }

            if (state.isPlaylistMode && meta.playlistItems.isNotEmpty()) {
                // Batch Playlist Enqueue
                val targetItems = if (state.downloadEntirePlaylist) {
                    meta.playlistItems
                } else {
                    meta.playlistItems.filter { state.selectedItemIds.contains(it.id) }
                }

                if (targetItems.isEmpty()) {
                    _uiState.value = _uiState.value.copy(errorMessage = "Please select at least one video to download")
                    return@launch
                }

                val playlistEntities = targetItems.mapIndexed { index, item ->
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
                        playlistTotal = targetItems.size,
                        isAudioOnly = format.isAudioOnly,
                        status = DownloadStatus.QUEUED
                    )
                }

                repository.enqueueAll(playlistEntities)
                DownloadForegroundService.startDownload(getApplication())

                _uiState.value = DownloaderUiState(
                    urlInput = "",
                    isAnalyzing = false,
                    metadata = null,
                    selectedFormat = null,
                    errorMessage = null,
                    showQueueConfirmation = true,
                    queuedTaskTitle = "Queued ${targetItems.size} videos from \"${meta.title}\""
                )

            } else {
                // Single Video Enqueue
                val taskId = UUID.randomUUID().toString()
                val downloadEntity = DownloadEntity(
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

                repository.enqueue(downloadEntity)
                DownloadForegroundService.startDownload(getApplication(), taskId)

                _uiState.value = DownloaderUiState(
                    urlInput = "",
                    isAnalyzing = false,
                    metadata = null,
                    selectedFormat = null,
                    errorMessage = null,
                    showQueueConfirmation = true,
                    queuedTaskTitle = meta.title
                )
            }
        }
    }

    fun dismissQueueConfirmation() {
        _uiState.value = _uiState.value.copy(showQueueConfirmation = false)
    }
}
