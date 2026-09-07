package com.pullar.app.data.preferences

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.booleanPreferencesKey
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.stringPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map

enum class ThemeMode {
    SYSTEM,
    DARK,
    LIGHT
}

private val Context.dataStore: DataStore<Preferences> by preferencesDataStore(name = "pullar_preferences")

class ThemePreferences(private val context: Context) {

    companion object {
        val KEY_THEME_MODE = stringPreferencesKey("theme_mode")
        val KEY_DYNAMIC_COLOR = booleanPreferencesKey("dynamic_color")
        val KEY_WIFI_DOWNLOADS = booleanPreferencesKey("wifi_downloads")
        val KEY_MOBILE_DOWNLOADS = booleanPreferencesKey("mobile_downloads")
    }

    val themeModeFlow: Flow<ThemeMode> = context.dataStore.data.map { preferences ->
        when (preferences[KEY_THEME_MODE]) {
            ThemeMode.DARK.name -> ThemeMode.DARK
            ThemeMode.LIGHT.name -> ThemeMode.LIGHT
            else -> ThemeMode.SYSTEM
        }
    }

    val dynamicColorFlow: Flow<Boolean> = context.dataStore.data.map { preferences ->
        preferences[KEY_DYNAMIC_COLOR] ?: false
    }

    val wifiDownloadsFlow: Flow<Boolean> = context.dataStore.data.map { preferences ->
        preferences[KEY_WIFI_DOWNLOADS] ?: true
    }

    val mobileDataDownloadsFlow: Flow<Boolean> = context.dataStore.data.map { preferences ->
        preferences[KEY_MOBILE_DOWNLOADS] ?: true
    }

    suspend fun setThemeMode(mode: ThemeMode) {
        context.dataStore.edit { preferences ->
            preferences[KEY_THEME_MODE] = mode.name
        }
    }

    suspend fun setDynamicColor(enabled: Boolean) {
        context.dataStore.edit { preferences ->
            preferences[KEY_DYNAMIC_COLOR] = enabled
        }
    }

    suspend fun setWifiDownloads(enabled: Boolean) {
        context.dataStore.edit { preferences ->
            preferences[KEY_WIFI_DOWNLOADS] = enabled
        }
    }

    suspend fun setMobileDataDownloads(enabled: Boolean) {
        context.dataStore.edit { preferences ->
            preferences[KEY_MOBILE_DOWNLOADS] = enabled
        }
    }
}
