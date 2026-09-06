cmake_minimum_required(VERSION 3.16)

project(Pullar VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt 6 modules
find_package(Qt6 6.2 COMPONENTS Core Gui Widgets Network Multimedia MultimediaWidgets REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
)

# Core sources
set(CORE_SOURCES
    src/core/VideoMetadata.h
    src/core/DownloadTask.h
    src/core/AppSettings.h
    src/core/AppSettings.cpp
    src/core/ExtractorEngine.h
    src/core/ExtractorEngine.cpp
    src/core/DownloaderWorker.h
    src/core/DownloaderWorker.cpp
    src/core/DownloadManager.h
    src/core/DownloadManager.cpp
    src/core/HistoryStorage.h
    src/core/HistoryStorage.cpp
)

# UI sources
set(UI_SOURCES
    src/ui/DownloadCardWidget.h
    src/ui/DownloadCardWidget.cpp
    src/ui/HomePage.h
    src/ui/HomePage.cpp
    src/ui/MediaPlayerWidget.h
    src/ui/MediaPlayerWidget.cpp
    src/ui/DownloadsPage.h
    src/ui/DownloadsPage.cpp
    src/ui/HistoryPage.h
    src/ui/HistoryPage.cpp
    src/ui/SettingsPage.h
    src/ui/SettingsPage.cpp
    src/ui/AboutDialog.h
    src/ui/AboutDialog.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
)

# Resources
set(RESOURCES
    resources/resources.qrc
)

# Windows specific resources (Icon and metadata)
if(WIN32)
    set(WINDOWS_RESOURCES
        resources/app.rc
    )
endif()

add_executable(VideoDownloader WIN32
    src/main.cpp
    ${CORE_SOURCES}
    ${UI_SOURCES}
    ${RESOURCES}
    ${WINDOWS_RESOURCES}
)

target_link_libraries(VideoDownloader PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)

if(MSVC)
    target_compile_options(VideoDownloader PRIVATE /W4 /utf-8)
else()
    target_compile_options(VideoDownloader PRIVATE -Wall -Wextra)
endif()

# Installation rules for deployment
install(TARGETS VideoDownloader
    BUNDLE DESTINATION .
    RUNTIME DESTINATION .
)
