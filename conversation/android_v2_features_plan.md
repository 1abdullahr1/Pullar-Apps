# Implementation Plan: Android Enhancements (APK Splits, Material You, Inline Player & Splash Screen)

## Goal Description
Enhance the native Android version of **Pullar** ("Don't just watch—pull it") with:
1. **Option 1 APK Size Reduction**: Gradle ABI splits producing an optimized **~55 MB** `Pullar-arm64-v8a.apk` for modern devices plus a universal fallback APK.
2. **Material You & Theme Toggle**: Support for Android 12+ Dynamic Theming (Material You) with user selection between **System Default**, **Dark**, and **Light** modes, falling back to Pullar's signature 4-color palette (`#1a110f`, `#ffebc2`, `#945e38`, `#7cc6fe`).
3. **Material 3 Video Player Experience**:
   - Play media in a sleek **inline card/sheet** rather than taking over full screen immediately.
   - Dedicated **Fullscreen Button** to toggle immersive view on demand.
   - **"Open in External Player" Button** using `FileProvider` so users can launch VLC, MX Player, or any installed media app.
   - Fix playback file resolution and permissions for seamless local media playback.
4. **Background Audio Playback**: Uninterrupted audio playback with screen turned off or app backgrounded.
5. **Fast Startup & Animated Splash Screen**: Official `androidx.core:core-splashscreen` implementation with an animated logo transition and non-blocking asynchronous engine initialization.
6. **Strict Aesthetics**: Zero emojis anywhere in code, UI, or documentation.

---

## Architecture Overview

```mermaid
graph TD
    subgraph UI ["Jetpack Compose UI (Material 3)"]
        Splash[Animated Splash Screen]
        ThemeController[Material You & Theme Engine]
        Nav[PullarNavGraph]
        MiniPlayer[Material 3 Player Card / Inline Sheet]
        FullScreenPlayer[Immersive Player View]
    end

    subgraph AudioEngine ["Audio & Background Engine"]
        Exo[ExoPlayer Core + AudioAttributes]
        FileProv[FileProvider & External Intent]
    end

    subgraph Storage ["Preferences & Database"]
        DataStore[ThemePreferences (DataStore)]
        Room[(Pullar Database)]
    end

    Splash --> Nav
    ThemeController --> Nav
    DataStore --> ThemeController
    Nav --> MiniPlayer
    MiniPlayer --> FullScreenPlayer
    MiniPlayer --> Exo
    MiniPlayer --> FileProv
```

---

## User Review Required

> [!IMPORTANT]
> **APK Size Reduction (Option 1)**:
> - `Pullar-arm64-v8a.apk`: **~55 MB** (targets 95%+ of all modern Android phones).
> - `Pullar-universal.apk`: **~230 MB** (universal bundle containing all architectures).
> Both APKs will be built and published to GitHub Releases and artifacts.

> [!NOTE]
> **Material You & Signature Palette**:
> On Android 12+ devices, users can enable dynamic Material You theming that harmonizes with their system wallpaper. When disabled or on Android 7–11, the app uses Pullar's custom signature palette (`Coffee Bean`, `Light Apricot`, `Toffee Brown`, `Maya Blue`).

> [!NOTE]
> **Zero Emojis Policy**: Maintained strictly across all components, notifications, and menus.

---

## Proposed Changes

### 1. Build System & Gradle Optimization

#### [MODIFY] `android/app/build.gradle.kts`
- Add `androidx.core:core-splashscreen:1.0.1`.
- Add `androidx.datastore:datastore-preferences:1.1.1`.
- Configure Gradle ABI splits:
  ```kotlin
  splits {
      abi {
          isEnable = true
          reset()
          include("arm64-v8a", "armeabi-v7a", "x86_64")
          isUniversalApk = true
      }
  }
  ```
- Configure compile options and packaging exclusions.

#### [MODIFY] `.github/workflows/android-build.yml`
- Update artifact and release publishing steps to collect and upload both:
  - `Pullar-v1.2.0-arm64-v8a.apk` (~55 MB)
  - `Pullar-v1.2.0-universal.apk` (~230 MB)

---

### 2. File Provider & External Player Support

#### [NEW] `android/app/src/main/res/xml/file_paths.xml`
- Declares accessible paths for `FileProvider`:
  - Public `Download/Pullar` directory
  - Internal application storage

#### [MODIFY] `android/app/src/main/AndroidManifest.xml`
- Register `androidx.core.content.FileProvider`.
- Declare `android.permission.FOREGROUND_SERVICE_MEDIA_PLAYBACK`.
- Set splash screen theme on `MainActivity`.

---

### 3. Theme Engine & Material You

#### [NEW] `android/app/src/main/java/com/pullar/app/data/preferences/ThemePreferences.kt`
- DataStore implementation storing:
  - `ThemeMode`: `SYSTEM`, `DARK`, `LIGHT`
  - `useDynamicColor`: Boolean (Material You dynamic theming)

#### [MODIFY] `android/app/src/main/java/com/pullar/app/ui/theme/Color.kt`
- Define full light and dark color schemes adhering strictly to:
  - Coffee Bean (`#1a110f`)
  - Light Apricot (`#ffebc2`)
  - Toffee Brown (`#945e38`)
  - Maya Blue (`#7cc6fe`)

#### [MODIFY] `android/app/src/main/java/com/pullar/app/ui/theme/Theme.kt`
- Implement dynamic color support (`dynamicLightColorScheme` / `dynamicDarkColorScheme`) for Android 12+.
- Support dynamic switching between System, Dark, and Light themes based on user settings.

---

### 4. Player Component: Material 3 Inline Card & Background Audio

#### [NEW] `android/app/src/main/java/com/pullar/app/ui/components/MediaPlayerCard.kt`
- Inline Material 3 Card / Sheet player:
  - Plays video directly inside an elegant container instead of taking over the entire screen.
  - Video preview with aspect ratio fit/fill.
  - Custom styled timeline scrubber in Maya Blue (`#7cc6fe`).
  - Media controls: Play/Pause, Skip 10s back/forward, Elapsed/Total time.
  - **Fullscreen Button**: Expands to full screen on demand.
  - **"External Player" Button**: Resolves `FileProvider` URI and launches external video players (VLC, MX Player, system gallery).
  - Audio configuration: `AUDIO_CONTENT_TYPE_MUSIC` enabling background audio when app is minimized or screen is locked.

#### [MODIFY] `android/app/src/main/java/com/pullar/app/ui/screens/downloads/DownloadsScreen.kt`
- Integrate `MediaPlayerCard` when a user selects a completed item.

#### [MODIFY] `android/app/src/main/java/com/pullar/app/ui/screens/history/HistoryScreen.kt`
- Integrate `MediaPlayerCard` when playing any historical media item.

---

### 5. Settings Screen & Theming Controls

#### [MODIFY] `android/app/src/main/java/com/pullar/app/ui/screens/settings/SettingsScreen.kt` & `SettingsViewModel.kt`
- Add **Appearance Card**:
  - Theme mode selector: **System Default**, **Dark**, **Light**.
  - Dynamic Color toggle: **Material You Colors** (enabled on Android 12+).

---

### 6. Fast Startup & Animated Splash Screen

#### [NEW] `android/app/src/main/res/drawable/splash_logo.xml`
- Vector drawable of the Pullar logo configured for Android 12 splash screen.

#### [MODIFY] `android/app/src/main/res/values/themes.xml`
- Define `Theme.Pullar.Splash` using `Theme.SplashScreen` from AndroidX.

#### [MODIFY] `android/app/src/main/java/com/pullar/app/MainActivity.kt`
- Integrate `installSplashScreen()`.
- Add an animated scale/alpha intro for the app logo on initial launch.
- Initialize `YoutubeDLEngine` asynchronously on `Dispatchers.IO` so app opens instantly.

---

## Verification Plan

### Automated CI Build Verification
1. Push changes to GitHub.
2. Verify remote GitHub Actions run for `Build and Package Pullar (Android)`.
3. Confirm that both `Pullar-v1.2.0-arm64-v8a.apk` (~55 MB) and `Pullar-v1.2.0-universal.apk` (~230 MB) are generated.

### Mobile Feature Verification
1. **Splash Screen**: Confirm clean animated logo transition on launch.
2. **Theme Switching**: Toggle between System, Dark, and Light in Settings. On Android 12+, verify Material You dynamic colors.
3. **Inline Player**: Double-click or click "Play Media" on a downloaded video; confirm it plays in the Material 3 card player with seek controls.
4. **Fullscreen Toggle**: Click Fullscreen and verify immersive playback.
5. **External Player**: Click "External Player" and verify that Android's app chooser launches (VLC, Photos, MX Player).
6. **Background Audio**: Play an audio or video file, background the app or lock the screen; confirm audio playback continues.
