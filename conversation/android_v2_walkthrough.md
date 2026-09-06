# Pullar Android v1.2.0 Release Walkthrough

## Overview

Pullar for Android has been upgraded to **v1.2.0** with five major feature additions:
1. **Lightweight ABI Split Builds**: Reduced modern smartphone download sizes from ~244 MB down to **~79 MB** for ARM64 devices (`Pullar-v1.2.0-arm64-v8a.apk`), while retaining a universal fallback APK.
2. **Material You Dynamic Theming**: Full Android 12+ wallpaper-adaptive color integration, with manual user control between **System Default**, **Dark**, and **Light** modes, seamlessly backed by Pullar's custom 4-color palette.
3. **Inline Material 3 Card Media Player**: Rather than taking over full screen immediately upon tapping play, media renders inside an inline 16:9 card player with:
   - Play/pause, 10s skip back/forward, and timeline scrubber in Maya Blue (`#7cc6fe`).
   - One-tap **Fullscreen Expansion** button.
   - One-tap **External Player Launcher** (`FileProvider`) to play in VLC, MX Player, or gallery apps.
4. **Background Audio Playback**: Configured `AudioAttributes` (`CONTENT_TYPE_MUSIC` + `USAGE_MEDIA`) ensuring uninterrupted listening when the app is backgrounded or the screen is turned off.
5. **Animated Splash Screen & Fast Startup**: Implemented `androidx.core:core-splashscreen` with smooth vector logo scaling and non-blocking asynchronous engine initialization.

---

## 1. APK Size Optimization (Option 1)

### Architecture Split Configuration

The fat APK bundled precompiled C/C++ libraries (Python 3, yt-dlp, FFmpeg, and aria2c) across four architectures (`arm64-v8a`, `armeabi-v7a`, `x86`, `x86_64`). By configuring Gradle ABI splits in [build.gradle.kts](file:///D:/My%20Projects%2014%20aug/antigravity%20cli/Rust/simplest%20Video%20downloader/android/app/build.gradle.kts), individual APKs were generated for modern devices:

| Asset Name | Target Architecture | File Size | Reduction |
| :--- | :--- | :--- | :--- |
| `Pullar-v1.2.0-arm64-v8a.apk` | Modern Android phones (ARM64) | **~79.1 MB** | **67.6% smaller** |
| `Pullar-v1.2.0-armeabi-v7a.apk` | Older 32-bit devices (ARMv7) | **~72.2 MB** | **70.4% smaller** |
| `Pullar-v1.2.0-x86_64.apk` | Android emulators & Chromebooks | **~82.0 MB** | **66.4% smaller** |
| `Pullar-v1.2.0-universal.apk` | Universal (all architectures) | **~232.6 MB** | Fallback |

Both the ARM64 APK and the universal APK are uploaded to the GitHub Release:
[GitHub Release v1.2.0](https://github.com/1abdullahr1/simplest-video-downloader/releases/tag/v1.2.0)

---

## 2. Material You & Appearance Settings

### Dynamic Color & Custom Palette

- **Material You (Android 12+)**: Uses `dynamicDarkColorScheme(context)` and `dynamicLightColorScheme(context)` to harmonize UI tones with the device's system wallpaper.
- **Pullar Signature Palette Fallback**: When Material You is disabled or on Android 7 to 11, the app renders with Pullar's custom palette:
  - Coffee Bean: `#1a110f`
  - Light Apricot: `#ffebc2`
  - Toffee Brown: `#945e38`
  - Maya Blue: `#7cc6fe`

### Preference Storage

Stored persistently via Jetpack DataStore in [ThemePreferences.kt](file:///D:/My%20Projects%2014%20aug/antigravity%20cli/Rust/simplest%20Video%20downloader/android/app/src/main/java/com/pullar/app/data/preferences/ThemePreferences.kt):
- `ThemeMode`: `SYSTEM`, `DARK`, `LIGHT`
- `dynamicColor`: `true` / `false`

Settings are exposed in the new **Appearance** card on [SettingsScreen.kt](file:///D:/My%20Projects%2014%20aug/antigravity%20cli/Rust/simplest%20Video%20downloader/android/app/src/main/java/com/pullar/app/ui/screens/settings/SettingsScreen.kt).

---

## 3. Media Player Enhancements

### Inline Material 3 Card Component

Located in [MediaPlayerCard.kt](file:///D:/My%20Projects%2014%20aug/antigravity%20cli/Rust/simplest%20Video%20downloader/android/app/src/main/java/com/pullar/app/ui/components/MediaPlayerCard.kt):
- **Compact Card Playback**: When tapping "Play" on any completed download in [DownloadsScreen.kt](file:///D:/My%20Projects%2014%20aug/antigravity%20cli/Rust/simplest%20Video%20downloader/android/app/src/main/java/com/pullar/app/ui/screens/downloads/DownloadsScreen.kt) or [HistoryScreen.kt](file:///D:/My%20Projects%2014%20aug/antigravity%20cli/Rust/simplest%20Video%20downloader/android/app/src/main/java/com/pullar/app/ui/screens/history/HistoryScreen.kt), the media opens in an inline card above the list rather than replacing the screen.
- **Controls**:
  - Play/Pause toggle button.
  - Skip forward 10 seconds / Skip backward 10 seconds.
  - Interactive slider tracking current playback time against total duration.
- **Fullscreen Expansion**: An overlay button in the bottom right corner opens an edge-to-edge dialog player.
- **External Player Chooser**: A top action button uses [FileProvider](file:///D:/My%20Projects%2014%20aug/antigravity%20cli/Rust/simplest%20Video%20downloader/android/app/src/main/res/xml/file_paths.xml) to grant temporary read permissions, opening Android's native application chooser (VLC, MX Player, system gallery).
- **Background Audio**: Initialized with `AudioAttributes.Builder().setContentType(C.AUDIO_CONTENT_TYPE_MUSIC).setUsage(C.USAGE_MEDIA).build()`. Audio continues playing when navigating away or locking the device.

---

## 4. Splash Screen & Fast App Startup

### Implementation Details

1. **System Splash Theme**: Registered `Theme.Pullar.Splash` in [themes.xml](file:///D:/My%20Projects%2014%20aug/antigravity%20cli/Rust/simplest%20Video%20downloader/android/app/src/main/res/values/themes.xml) pointing to the vector asset [splash_logo.xml](file:///D:/My%20Projects%2014%20aug/antigravity%20cli/Rust/simplest%20Video%20downloader/android/app/src/main/res/drawable/splash_logo.xml).
2. **Exit Transition**: Configured in [MainActivity.kt](file:///D:/My%20Projects%2014%20aug/antigravity%20cli/Rust/simplest%20Video%20downloader/android/app/src/main/java/com/pullar/app/MainActivity.kt) using `installSplashScreen().setOnExitAnimationListener` with an anticipate alpha fade-out.
3. **Animated Startup Logo**: Jetpack Compose plays a gentle scale/fade transition (0.85x to 1.05x to 1.0x) over 450 ms while the UI hydrates underneath, dissolving seamlessly into the home screen.

---

## 5. Website Updates

In [DownloadSection.astro](file:///D:/My%20Projects%2014%20aug/antigravity%20cli/Rust/pullar-website/src/components/DownloadSection.astro), the Android card now features:
- **Primary button**: `Download APK (ARM64 ~79 MB)` linking directly to `Pullar-v1.2.0-arm64-v8a.apk`.
- **Secondary button**: `Universal APK (All Devices)` linking directly to `Pullar-v1.2.0-universal.apk`.
- Committed and pushed to [pullar-website repository](https://github.com/1abdullahr1/pullar-website).

---

## Build Verification

- **GitHub Actions Run ID**: `34054415419` (`Build & Package Android APK`)
- **Status**: Completed successfully in 4 minutes 28 seconds.
- **Published Assets**:
  - `Pullar-v1.2.0-arm64-v8a.apk` (82,973,867 bytes)
  - `Pullar-v1.2.0-universal.apk` (243,922,301 bytes)
  - `Pullar-v1.2.0-armeabi-v7a.apk` (75,735,387 bytes)
  - `Pullar-v1.2.0-x86_64.apk` (86,001,653 bytes)
