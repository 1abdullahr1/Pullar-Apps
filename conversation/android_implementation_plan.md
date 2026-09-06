# Implementation Plan: Pullar for Android (Native Kotlin & Jetpack Compose)

## Goal Description
Port the desktop **Pullar** ("Don't just watch—pull it") video and audio downloader to a native Android application using **Kotlin** and **Jetpack Compose (Material 3)**. The application will replicate all core features, architecture, user flows, and aesthetic identity of the desktop application:
1. **Downloader Screen**: URL input, metadata extraction, format & quality selector, YouTube playlist toggle, and instant UX reset upon starting download.
2. **Downloads Queue Screen**: Active tasks with real-time progress bar, speed indicator, segmented fragment downloading, pause, resume, and cancel.
3. **History Screen**: Completed tasks with thumbnail preview, search/filter bar, file management, and playback trigger.
4. **Built-in Media Player**: Native Jetpack Media3 (ExoPlayer) embedded directly into the app for audio and video playback with Picture-in-Picture and background playback.
5. **Settings Screen**: Custom download directory, max concurrent downloads, default resolution, and over-the-air yt-dlp binary updates.
6. **Background Download Service**: Android Foreground Service with persistent status notifications showing live progress.
7. **Brand Identity & Strict Aesthetics**:
   - App Name: **Pullar**
   - Tagline: **Don't just watch—pull it**
   - Color Palette:
     - Coffee Bean: `#1a110f` (Main background, dark containers)
     - Light Apricot: `#ffebc2` (Primary text, headers, icon accents)
     - Toffee Brown: `#945e38` (Cards, surfaces, secondary buttons)
     - Maya Blue: `#7cc6fe` (Primary action buttons, sliders, progress bars)
   - Zero emojis anywhere in code, UI, notifications, or documentation.
8. **Remote Build via GitHub Actions**:
   - The user will develop locally by editing project files in the repository.
   - Code is pushed to GitHub.
   - GitHub Actions automatically builds the native Android APK and publishes downloadable artifacts.

---

## Architecture Overview

```mermaid
graph TD
    subgraph UI ["Jetpack Compose UI (Material 3)"]
        Nav[Navigation Graph & Bottom Bar]
        Home[DownloaderScreen]
        Queue[DownloadsScreen]
        Hist[HistoryScreen]
        Player[MediaPlayerScreen]
        Settings[SettingsScreen]
    end

    subgraph State ["ViewModels & State Holders"]
        DownloaderVM[DownloaderViewModel]
        DownloadsVM[DownloadsViewModel]
        HistoryVM[HistoryViewModel]
        SettingsVM[SettingsViewModel]
    end

    subgraph Service ["Android Services & Background Tasks"]
        ForegroundService[DownloadForegroundService]
        NotificationMgr[DownloadNotificationManager]
    end

    subgraph Core ["Core Engine & Storage"]
        YTDLEngine[YoutubeDLEngine (JNI / Native yt-dlp + FFmpeg)]
        RoomDB[(Room Database: DownloadTasks & History)]
        ExoPlayerCore[Media3 ExoPlayer]
    end

    Home --> DownloaderVM
    Queue --> DownloadsVM
    Hist --> HistoryVM
    Player --> ExoPlayerCore
    Settings --> SettingsVM

    DownloaderVM --> ForegroundService
    DownloadsVM --> ForegroundService
    ForegroundService --> YTDLEngine
    ForegroundService --> NotificationMgr
    ForegroundService --> RoomDB
    HistoryVM --> RoomDB
```

---

## User Review Required

> [!IMPORTANT]
> **No Local Android Build Required**: You do not need to install the Android SDK or build locally on your machine. All building and APK compilation will occur remotely in GitHub Actions on an Ubuntu runner. Once pushed, you can download the ready-to-install `.apk` directly from GitHub Actions summary page.

> [!NOTE]
> **Embedded yt-dlp Engine**: Android requires native ELF binaries compiled for ARM64 and x86 architectures. We will use the standard `io.github.junkfood02.youtubedl-android` library (which powers top open-source Android downloaders like *Seal*). It packages embedded Python 3, `yt-dlp`, and `ffmpeg` with JNI bindings and supports automatic in-app updates.

> [!NOTE]
> **Zero Emojis Policy**: Maintained strictly across all Kotlin code, Compose UI, string resources, notifications, and documentation.

---

## Proposed Changes

The Android application will be placed in a dedicated `android/` directory within this repository, alongside the existing desktop codebase. A GitHub Actions workflow will be added to `.github/workflows/android-build.yml`.

### 1. Build System & Configuration Layer

#### [NEW] `android/settings.gradle.kts`
- Configures plugin management, dependency resolution with `google()`, `mavenCentral()`, and `jitpack.io`.
- Defines root project name `Pullar` and includes `:app`.

#### [NEW] `android/build.gradle.kts`
- Root build file specifying Android Gradle Plugin (`8.7.3`), Kotlin (`2.0.21`), and KSP (`2.0.21-1.0.28`).

#### [NEW] `android/gradle.properties`
- Configures JVM memory (`-Xmx4096m`), AndroidX flags (`android.useAndroidX=true`), and parallel build optimizations.

#### [NEW] `android/gradle/wrapper/gradle-wrapper.properties`
- Configures Gradle distribution `8.9` or `8.10`.

#### [NEW] `android/gradlew` & `android/gradlew.bat`
- Portable Gradle wrapper scripts for Linux (GitHub Actions) and Windows.

#### [NEW] `android/app/build.gradle.kts`
- Configures:
  - `applicationId = "com.pullar.app"`
  - `minSdk = 24`, `targetSdk = 35`, `compileSdk = 35`
  - Java 17 compile options
  - Jetpack Compose with Kotlin Compose compiler plugin
  - NDK ABI filters: `arm64-v8a`, `armeabi-v7a`, `x86_64`, `x86`
  - Dependencies:
    - Compose BOM, Material 3, Navigation Compose
    - `io.github.junkfood02.youtubedl-android:library:0.18.1`
    - `io.github.junkfood02.youtubedl-android:ffmpeg:0.18.1`
    - `io.github.junkfood02.youtubedl-android:aria2c:0.18.1`
    - Media3 ExoPlayer & Media3 UI (`1.5.0`)
    - Room Database (`2.6.1`)
    - Coil for Jetpack Compose (`2.7.0`)
    - Coroutines & DataStore Preferences

---

### 2. Android Manifest & Application

#### [NEW] `android/app/src/main/AndroidManifest.xml`
- Declares required permissions:
  - `android.permission.INTERNET`
  - `android.permission.ACCESS_NETWORK_STATE`
  - `android.permission.FOREGROUND_SERVICE`
  - `android.permission.FOREGROUND_SERVICE_DATA_SYNC`
  - `android.permission.POST_NOTIFICATIONS`
  - `android.permission.WAKE_LOCK`
  - `android.permission.WRITE_EXTERNAL_STORAGE` (maxSdkVersion=28)
- Sets `android:extractNativeLibs="true"` for ELF binary execution.
- Registers `PullarApplication`, `MainActivity`, and `DownloadForegroundService`.

#### [NEW] `android/app/src/main/java/com/pullar/app/PullarApplication.kt`
- Initializes `YoutubeDL.getInstance().init(this)` and `FFmpeg.getInstance().init(this)`.
- Creates notification channels for download progress and completion.

---

### 3. Data & Storage Layer

#### [NEW] `android/app/src/main/java/com/pullar/app/data/model/DownloadEntity.kt`
- Room entity for persistent download tracking:
  - `id`: Unique task ID
  - `url`: Video or playlist URL
  - `title`: Video title
  - `thumbnailUrl`: Thumbnail image link
  - `formatId`: Selected format identifier
  - `qualityLabel`: Resolution / audio label (e.g., "1080p", "MP3 Audio")
  - `destinationPath`: Output file path
  - `downloadedBytes`: Bytes downloaded
  - `totalBytes`: Total file size
  - `speed`: Instantaneous download rate
  - `eta`: Estimated remaining time
  - `status`: Enum (`QUEUED`, `ANALYZING`, `DOWNLOADING`, `PAUSED`, `COMPLETED`, `FAILED`, `CANCELLED`)
  - `isPlaylist`: Boolean flag
  - `isAudioOnly`: Boolean flag
  - `createdAt`: Timestamp

#### [NEW] `android/app/src/main/java/com/pullar/app/data/dao/DownloadDao.kt`
- Flow-based queries for active downloads (`WHERE status IN ('QUEUED', 'DOWNLOADING', 'PAUSED')`).
- Queries for completed history (`WHERE status = 'COMPLETED' ORDER BY createdAt DESC`).
- Search query by title filter.
- Insert, update, and delete queries.

#### [NEW] `android/app/src/main/java/com/pullar/app/data/database/PullarDatabase.kt`
- Room database implementation with singleton instance.

---

### 4. Engine & Service Layer

#### [NEW] `android/app/src/main/java/com/pullar/app/core/YoutubeDLEngine.kt`
- `extractMetadata(url: String)`: Extracts title, duration, author, thumbnail, and format options using `--flat-playlist` or standard dump-json.
- `download(item: DownloadEntity, onProgress: (Float, String, String) -> Unit)`:
  - Replicates desktop download parameters:
    - `--concurrent-fragments 16`
    - `--buffer-size 1024K`
    - `--http-chunk-size 10M`
    - `--retries 10`
    - `--fragment-retries 10`
    - `--no-playlist` (enforced when `!isPlaylist`)
  - Streams real-time progress callbacks to the foreground service.

#### [NEW] `android/app/src/main/java/com/pullar/app/service/DownloadForegroundService.kt`
- Manages ongoing downloads in the background.
- Runs as a Foreground Service with an ongoing notification displaying progress bar, speed, and cancel button.
- Updates Room database state on start, progress update, pause, failure, and completion.

---

### 5. UI & Jetpack Compose Layer

#### [NEW] `android/app/src/main/java/com/pullar/app/ui/theme/Color.kt`
- Strict palette definition:
  ```kotlin
  val CoffeeBean = Color(0xFF1A110F)
  val LightApricot = Color(0xFFFFEBC2)
  val ToffeeBrown = Color(0xFF945E38)
  val MayaBlue = Color(0xFF7CC6FE)
  val SurfaceDark = Color(0xFF241816)
  val CardBorder = Color(0xFF3D2722)
  ```

#### [NEW] `android/app/src/main/java/com/pullar/app/ui/theme/Theme.kt`
- Material 3 theme applying the palette to `darkColorScheme` and typography.

#### [NEW] `android/app/src/main/java/com/pullar/app/ui/navigation/Screen.kt` & `NavGraph.kt`
- Bottom Navigation Bar with 4 primary destinations:
  1. **Downloader** (`Screen.Downloader`)
  2. **Downloads** (`Screen.Downloads`)
  3. **History** (`Screen.History`)
  4. **Settings** (`Screen.Settings`)
- Secondary route for **Player** (`Screen.Player(uri)`).

#### [NEW] `android/app/src/main/java/com/pullar/app/ui/screens/downloader/DownloaderScreen.kt`
- URL input field with Clipboard Paste action.
- Analyze button with loading spinner.
- Video preview card showing thumbnail, duration, author, and title.
- Format selector (Best, 1080p, 720p, 480p, Audio Only MP3).
- YouTube Playlist toggle checkbox ("Download Entire Playlist").
- "Pull Video" / "Start Download" button.
- **UX Reset Behavior**: Upon clicking download, the input text is cleared, the preview card is dismissed, and a clean queue confirmation card appears with a direct button: "View Downloads Queue".

#### [NEW] `android/app/src/main/java/com/pullar/app/ui/screens/downloads/DownloadsScreen.kt`
- List of active downloads.
- Cards showing thumbnail, title, format badge, Maya Blue progress bar, speed (MB/s), downloaded/total size.
- Action buttons: Pause, Resume, Cancel.
- "Play Media" button when a download completes.

#### [NEW] `android/app/src/main/java/com/pullar/app/ui/screens/history/HistoryScreen.kt`
- Search bar to filter completed downloads.
- Cards displaying completed media items, file size, format tag, and timestamp.
- Action buttons: "Play Media" (opens built-in player), "Share", "Delete".

#### [NEW] `android/app/src/main/java/com/pullar/app/ui/screens/player/MediaPlayerScreen.kt`
- Native Android Media3 (ExoPlayer) Jetpack Compose view.
- Supports both video and audio playback.
- Custom styled playback controls (Maya Blue seekbar, Light Apricot buttons, Coffee Bean background).
- Supports Picture-in-Picture (PiP) and background audio playback.

#### [NEW] `android/app/src/main/java/com/pullar/app/ui/screens/settings/SettingsScreen.kt`
- Download directory picker (scoped storage / Download folder).
- Max concurrent downloads selector (1 to 5).
- Default resolution & audio format options.
- "Update yt-dlp" action button.
- Pullar About card: Name "Pullar", Slogan "Don't just watch—pull it", version, license.

---

### 6. Remote GitHub Actions CI/CD Layer

#### [NEW] `.github/workflows/android-build.yml`
- Workflow triggered on push to `main` for changes in `android/**` or via manual `workflow_dispatch`.
- Runner: `ubuntu-latest`.
- Steps:
  1. Checkout repository.
  2. Setup Java 17 via `actions/setup-java`.
  3. Setup Android SDK.
  4. Cache Gradle packages.
  5. Run `./gradlew assembleDebug` (generates installable debug APK for instant testing on any Android device without signing keys) and `./gradlew assembleRelease`.
  6. Upload `Pullar-Android-APK` as a downloadable GitHub Actions artifact.

---

## Verification Plan

### Remote CI Verification
1. Commit and push all Android project files to GitHub branch `main`.
2. Monitor the remote execution of `.github/workflows/android-build.yml` using GitHub CLI (`gh run list`, `gh run watch`).
3. Verify that Gradle compiles Kotlin, Jetpack Compose, and JNI libraries without errors.
4. Verify that `Pullar-Android-APK` artifact is generated and available for download.

### Mobile Device Manual Verification
1. Download the generated `Pullar-debug.apk` onto an Android phone or emulator.
2. Install and launch the application.
3. Verify theme colors: `#1a110f`, `#ffebc2`, `#945e38`, `#7cc6fe`.
4. Paste a video URL (e.g., YouTube), analyze video, and verify format options.
5. Click "Start Download": verify that the Downloader screen immediately clears input and shows queue confirmation.
6. Check the Downloads tab: verify real-time segmented progress and notification in Android status bar.
7. Once finished, click "Play Media" to verify playback in the built-in Media3 ExoPlayer.
