# Pullar

[![Build and Package Windows](https://github.com/1abdullahr1/Pullar-Apps/actions/workflows/build-and-release.yml/badge.svg)](https://github.com/1abdullahr1/Pullar-Apps/actions/workflows/build-and-release.yml)
[![Build and Package Android](https://github.com/1abdullahr1/Pullar-Apps/actions/workflows/android-build.yml/badge.svg)](https://github.com/1abdullahr1/Pullar-Apps/actions/workflows/android-build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Pullar ("Don't just watch—pull it") is a high-speed media downloader for Windows and Android. It pulls videos, playlists, and audio from YouTube, X (Twitter), TikTok, Instagram, Facebook, Vimeo, Reddit, and direct media streams with multi-connection acceleration.

---

## Downloads

Official releases are available on the [Releases Page](https://github.com/1abdullahr1/Pullar-Apps/releases/latest).

### Windows (64-bit)

- **Installer**: `Pullar_Setup_v1.2.0.exe` (Recommended - installs with Start Menu and Desktop shortcuts)
- **Portable**: `Pullar_v1.2.0_Portable.zip` (Extract and run without installation)

---

### Android APK Selection Guide

Choose the APK matching your device:

| APK File | Size | Target Devices | Phone Model Examples |
| :--- | :--- | :--- | :--- |
| **Pullar-v1.2.0-arm64-v8a.apk**<br>*(Recommended for 95%+ of users)* | ~79 MB | Modern 64-bit Android phones (2016 to present) | **Samsung**: Galaxy S20, S21, S22, S23, S24, Note 20, A52, A53, A54, Z Flip/Fold<br>**Google**: Pixel 4, 5, 6, 7, 8, 9<br>**Xiaomi / Redmi / POCO**: Redmi Note 10/11/12/13, POCO X/F series, Xiaomi 12/13/14<br>**OnePlus**: 7, 8, 9, 10, 11, 12, Nord series<br>**Motorola**: Edge series, Moto G Power/Stylus (2020+)<br>**Others**: Oppo, Vivo, Realme, Nothing Phone (1/2/2a) |
| **Pullar-v1.2.0-armeabi-v7a.apk** | ~72 MB | Older 32-bit Android phones (pre-2016) and budget Android Go devices | **Samsung**: Galaxy J2, J3, J5, J7, Grand Prime<br>**Motorola**: Moto E, Moto G (1st–4th Gen)<br>**Other**: Older budget Android Go devices, older TV boxes |
| **Pullar-v1.2.0-x86_64.apk** | ~82 MB | PC emulators and Intel/AMD Chromebooks | **PC Emulators**: BlueStacks, LDPlayer, NoxPlayer, Android Studio Emulator<br>**Laptops**: Chromebooks with Intel or AMD processors |
| **Pullar-v1.2.0-universal.apk** | ~233 MB | Universal fallback containing all architectures | Any device (use only if you do not know your processor architecture) |

---

## Key Features

### Windows (C++17 & Qt 6)
- 16x multi-connection segmented stream downloading.
- Complete YouTube playlist extraction with automatic numbering.
- Built-in media player for instant preview and playback.
- Searchable download history journal.
- Bundled yt-dlp and FFmpeg binaries.

### Android (Kotlin & Jetpack Compose)
- Signature brand palette: Toffee Brown (#945e38), Light Apricot (#ffebc2), Coffee Bean (#1a110f), and Maya Blue (#7cc6fe).
- Theme mode support: System Default, Dark Mode, and Light Mode with high contrast.
- Seamless Android Share Sheet download flow: Tap Share in YouTube or any browser to select quality and download in background without switching apps.
- YouTube playlist downloading: Batch download entire playlists or select individual videos with item checklist.
- Dynamic download-time estimate: Live updates ("About 2 min remaining", "Less than 1 min remaining").
- Network usage controls: Configurable Wi-Fi and Mobile Data download permissions with real-time network detection.
- Direct "Open Downloads Folder" action for instant file manager access.
- Inline Material 3 media player with 16:9 card, fullscreen toggle, and external player launcher.
- Background audio playback with screen off or app minimized.
- Fast cold startup with zero artificial delay.

---

## Technical Inquiries & Help

For technical assistance, bug reports, or custom software projects:
https://abdullahcs.pages.dev/

---

## License

Pullar is open-source software licensed under the [MIT License](LICENSE).
Copyright (c) 2026 Abdullah Bhatti.
