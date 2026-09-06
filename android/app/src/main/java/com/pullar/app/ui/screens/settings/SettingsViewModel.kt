package com.pullar.app.ui.screens.settings

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.pullar.app.core.YoutubeDLEngine
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch

data class SettingsUiState(
    val maxConcurrentDownloads: Int = 3,
    val defaultQuality: String = "Best Quality",
    val isUpdatingEngine: Boolean = false,
    val engineUpdateResult: String? = null
)

class SettingsViewModel(application: Application) : AndroidViewModel(application) {

    private val _uiState = MutableStateFlow(SettingsUiState())
    val uiState: StateFlow<SettingsUiState> = _uiState.asStateFlow()

    fun onMaxConcurrentChanged(count: Int) {
        _uiState.value = _uiState.value.copy(maxConcurrentDownloads = count)
    }

    fun onDefaultQualityChanged(quality: String) {
        _uiState.value = _uiState.value.copy(defaultQuality = quality)
    }

    fun updateEngine() {
        viewModelScope.launch {
            _uiState.value = _uiState.value.copy(isUpdatingEngine = true, engineUpdateResult = null)
            val success = YoutubeDLEngine.updateEngine(getApplication())
            _uiState.value = _uiState.value.copy(
                isUpdatingEngine = false,
                engineUpdateResult = if (success) "yt-dlp engine updated to latest version" else "Failed to update engine. Please try again."
            )
        }
    }
}
