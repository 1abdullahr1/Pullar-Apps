package com.pullar.app.ui.theme

import android.os.Build
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.dynamicDarkColorScheme
import androidx.compose.material3.dynamicLightColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.platform.LocalContext
import com.pullar.app.data.preferences.ThemeMode

private val PullarDarkColorScheme = darkColorScheme(
    primary = MayaBlue,
    onPrimary = CoffeeBean,
    primaryContainer = ToffeeBrown,
    onPrimaryContainer = LightApricot,
    secondary = ToffeeBrown,
    onSecondary = LightApricot,
    secondaryContainer = DarkSurfaceElevated,
    onSecondaryContainer = LightApricot,
    tertiary = MayaBlue,
    onTertiary = CoffeeBean,
    background = DarkSurfaceBase,
    onBackground = DarkTextPrimary,
    surface = DarkSurfaceCard,
    onSurface = DarkTextPrimary,
    surfaceVariant = DarkSurfaceElevated,
    onSurfaceVariant = DarkTextSecondary,
    outline = DarkSurfaceBorder,
    error = ErrorRed,
    onError = CoffeeBean
)

private val PullarLightColorScheme = lightColorScheme(
    primary = CoffeeBean,
    onPrimary = LightApricot,
    primaryContainer = MayaBlueDeep,
    onPrimaryContainer = LightSurfaceCard,
    secondary = ToffeeBrown,
    onSecondary = LightApricot,
    secondaryContainer = LightSurfaceElevated,
    onSecondaryContainer = LightTextPrimary,
    tertiary = MayaBlueDeep,
    onTertiary = LightSurfaceCard,
    background = LightSurfaceBase,
    onBackground = LightTextPrimary,
    surface = LightSurfaceCard,
    onSurface = LightTextPrimary,
    surfaceVariant = LightSurfaceElevated,
    onSurfaceVariant = LightTextSecondary,
    outline = LightSurfaceBorder,
    error = ErrorRed,
    onError = LightSurfaceCard
)

@Composable
fun PullarTheme(
    themeMode: ThemeMode = ThemeMode.SYSTEM,
    dynamicColor: Boolean = false,
    content: @Composable () -> Unit
) {
    val isDark = when (themeMode) {
        ThemeMode.DARK -> true
        ThemeMode.LIGHT -> false
        ThemeMode.SYSTEM -> isSystemInDarkTheme()
    }

    val context = LocalContext.current
    val colorScheme = when {
        dynamicColor && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S -> {
            if (isDark) dynamicDarkColorScheme(context) else dynamicLightColorScheme(context)
        }
        isDark -> PullarDarkColorScheme
        else -> PullarLightColorScheme
    }

    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography,
        content = content
    )
}
