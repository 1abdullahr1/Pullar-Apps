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
    secondaryContainer = SurfaceCard,
    onSecondaryContainer = LightApricot,
    tertiary = MayaBlue,
    onTertiary = CoffeeBean,
    background = CoffeeBean,
    onBackground = LightApricot,
    surface = SurfaceDark,
    onSurface = LightApricot,
    surfaceVariant = SurfaceCard,
    onSurfaceVariant = TextMuted,
    outline = SurfaceBorder,
    error = ErrorRed,
    onError = CoffeeBean
)

private val PullarLightColorScheme = lightColorScheme(
    primary = ToffeeBrown,
    onPrimary = LightApricot,
    primaryContainer = MayaBlue,
    onPrimaryContainer = CoffeeBean,
    secondary = MayaBlueDark,
    onSecondary = LightSurface,
    secondaryContainer = LightSurfaceVariant,
    onSecondaryContainer = LightTextPrimary,
    tertiary = ToffeeBrown,
    onTertiary = LightApricot,
    background = LightBackground,
    onBackground = LightTextPrimary,
    surface = LightSurface,
    onSurface = LightTextPrimary,
    surfaceVariant = LightSurfaceVariant,
    onSurfaceVariant = LightTextMuted,
    outline = LightBorder,
    error = ErrorRed,
    onError = LightSurface
)

@Composable
fun PullarTheme(
    themeMode: ThemeMode = ThemeMode.SYSTEM,
    dynamicColor: Boolean = true,
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
