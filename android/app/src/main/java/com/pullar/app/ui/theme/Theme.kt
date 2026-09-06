package com.pullar.app.ui.theme

import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable

private val DarkColorScheme = darkColorScheme(
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

@Composable
fun PullarTheme(
    content: @Composable () -> Unit
) {
    MaterialTheme(
        colorScheme = DarkColorScheme,
        typography = Typography,
        content = content
    )
}
