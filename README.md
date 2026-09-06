# Pullar — Don't just watch—pull it

[![Build and Package](https://github.com/1abdullahr1/simplest-video-downloader/actions/workflows/build-and-release.yml/badge.svg)](https://github.com/1abdullahr1/simplest-video-downloader/actions/workflows/build-and-release.yml)
[![Platform](https://img.shields.io/badge/platform-Windows%20x64-blue.svg)](https://github.com/1abdullahr1/simplest-video-downloader)
[![Language](https://img.shields.io/badge/C%2B%2B-17-00599C.svg?logo=c%2B%2B)](https://isocpp.org/)
[![Framework](https://img.shields.io/badge/Qt-6-41CD52.svg?logo=qt)](https://www.qt.io/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

**Pullar** is a high-speed, multi-connection native Windows desktop application for pulling videos, playlists, and audio from **YouTube, X (Twitter), TikTok, Instagram, Facebook, Vimeo, Reddit, and direct media streams**. Built with **C++17** and **Qt 6**, it features high-speed IDM-style multi-fragment downloading, integrated media playback, real-time metrics, and a warm, refined aesthetic.

---

## Key Features

- **High-Speed Multi-Connection Engine**: IDM-style parallel stream fragment downloading, custom buffer sizes, and full saturation of available network bandwidth.
- **YouTube Playlist Support**: Option to pull full YouTube playlists or individual videos with automated playlist numbering.
- **Built-in Media Player**: Play completed video and audio downloads directly inside the application with seek, volume, and playback controls.
- **Asynchronous Non-Blocking Engine**: Video extraction and downloads run on dedicated background workers without interface stalls.
- **Clean Responsive Workflow**: Downloader page automatically resets and empties immediately after queueing a download, ready for the next link.
- **Universal Link Support**: Supports YouTube, X/Twitter, TikTok, Instagram, Facebook, Vimeo, Reddit, and direct MP4/M3U8 streams.
- **Quality & Format Selection**: Choose between Best Available, 1080p Full HD, 720p HD, 480p SD, or Audio Only (MP3 320kbps).
- **Download History**: Searchable journal of previously downloaded media with direct 1-click built-in playback and folder navigation.
- **Zero User Configuration**: Qt 6 runtime libraries, yt-dlp, and ffmpeg are bundled directly inside the installer. No manual CLI setup required.

---

## Download & Installation

### Option 1: Official Windows Installer (Recommended)
Download `VideoDownloader_Setup_v1.0.0.exe` from the latest [GitHub Release](https://github.com/1abdullahr1/simplest-video-downloader/releases).
1. Run `VideoDownloader_Setup_v1.0.0.exe`.
2. Follow the setup wizard to install with Start Menu and Desktop shortcuts.
3. Clean uninstaller is registered in Windows Control Panel.

### Option 2: Portable Standalone ZIP
Download `VideoDownloader_v1.0.0_Portable.zip`.
1. Extract the ZIP anywhere on your system.
2. Launch `VideoDownloader.exe` directly.

---

## Project Structure

```
simplest-video-downloader/
├── CMakeLists.txt                    # CMake build configuration
├── LICENSE                            # MIT License
├── README.md                          # Project documentation
├── .gitignore                         # Exclusions for build output
├── .github/
│   └── workflows/
│       └── build-and-release.yml      # CI/CD: Automated build, bundling, installer
├── installer/
│   └── downloader_installer.iss       # Inno Setup 6 installer script
├── resources/
│   ├── app.ico                        # Multi-resolution application icon
│   ├── app.png                        # Branding logo
│   ├── app.rc                         # Windows PE metadata and icon resource
│   ├── resources.qrc                  # Qt Resource bundle
│   └── styles/
│       └── modern.qss                 # Modern Windows 11 stylesheet
└── src/
    ├── main.cpp                       # Application entry point & DPI setup
    ├── core/
    │   ├── VideoMetadata.h            # Title, duration, thumbnail, formats struct
    │   ├── DownloadTask.h             # Queue task model & status enums
    │   ├── AppSettings.h/.cpp         # Configuration & external tool detection
    │   ├── ExtractorEngine.h/.cpp     # Asynchronous URL analysis via yt-dlp
    │   ├── DownloaderWorker.h/.cpp    # Process runner streaming real-time stdout
    │   ├── DownloadManager.h/.cpp     # Concurrency limiter & queue coordinator
    │   └── HistoryStorage.h/.cpp      # Persistent completed downloads journal
    └── ui/
        ├── DownloadCardWidget.h/.cpp  # Individual active download card
        ├── HomePage.h/.cpp            # URL input, platform tags, video preview
        ├── DownloadsPage.h/.cpp       # Active queue view
        ├── HistoryPage.h/.cpp         # Past downloads table with search & actions
        ├── SettingsPage.h/.cpp        # Preferences & tool verification
        ├── AboutDialog.h/.cpp         # About & author credits dialog
        └── MainWindow.h/.cpp          # Sidebar navigation and stacked views
```

---

## Building from Source

### Prerequisites
- Windows 10/11 (x64)
- CMake 3.16+
- Ninja or Visual Studio 2022
- Qt 6.2+ (Core, Gui, Widgets, Network)
- Inno Setup 6 (optional, for installer creation)

### Build Commands
```powershell
# Clone repository
git clone https://github.com/1abdullahr1/simplest-video-downloader.git
cd simplest-video-downloader

# Configure with CMake
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:/Qt/6.7.2/msvc2022_64"

# Build executable
cmake --build build --config Release

# Deploy Qt dependencies
mkdir dist
copy build\VideoDownloader.exe dist\VideoDownloader.exe
windeployqt --release --compiler-runtime --no-translations dist\VideoDownloader.exe

# Build Installer (Requires Inno Setup)
& "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" /DSourceDir="$PWD\dist" installer\downloader_installer.iss
```

---

## License

Simplest Video Downloader is open-source software licensed under the [MIT License](LICENSE).
Copyright (c) 2026 Abdullah Bhatti.
