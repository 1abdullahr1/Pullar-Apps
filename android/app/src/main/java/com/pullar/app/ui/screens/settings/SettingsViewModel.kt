package com.pullar.app.ui.screens.settings

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.pullar.app.core.YoutubeDLEngine
import com.pullar.app.data.preferences.ThemeMode
import com.pullar.app.data.preferences.ThemePreferences
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.launch

data class SettingsUiState(
    val maxConcurrentDownloads: Int = 3,
    val defaultQuality: String = "Best Quality",
    val isUpdatingEngine: Boolean = false,
    val engineUpdateResult: String? = null
)

class SettingsViewModel(application: Application) : AndroidViewModel(application) {

    private val themePreferences = ThemePreferences(application)
    private val _uiState = MutableStateFlow(SettingsUiState())
    val uiState: StateFlow<SettingsUiState> = _uiState.asStateFlow()

    val themeMode: StateFlow<ThemeMode> = themePreferences.themeModeFlow.stateIn(
        viewModelScope,
        SharingStarted.WhileSubscribed(5000),
        ThemeMode.SYSTEM
    )

    val dynamicColor: StateFlow<Boolean> = themePreferences.dynamicColorFlow.stateIn(
        viewModelScope,
        SharingStarted.WhileSubscribed(5000),
        false
    )

    val wifiDownloads: StateFlow<Boolean> = themePreferences.wifiDownloadsFlow.stateIn(
        viewModelScope,
        SharingStarted.WhileSubscribed(5000),
        true
    )

    val mobileDataDownloads: StateFlow<Boolean> = themePreferences.mobileDataDownloadsFlow.stateIn(
        viewModelScope,
        SharingStarted.WhileSubscribed(5000),
        true
    )

    fun onThemeModeChanged(mode: ThemeMode) {
        viewModelScope.launch {
            themePreferences.setThemeMode(mode)
        }
    }

    fun onDynamicColorToggled(enabled: Boolean) {
        viewModelScope.launch {
            themePreferences.setDynamicColor(enabled)
        }
    }

    fun onWifiDownloadsToggled(enabled: Boolean) {
        viewModelScope.launch {
            themePreferences.setWifiDownloads(enabled)
        }
    }

    fun onMobileDataDownloadsToggled(enabled: Boolean) {
        viewModelScope.launch {
            themePreferences.setMobileDataDownloads(enabled)
        }
    }

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
