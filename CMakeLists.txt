cmake_minimum_required(VERSION 3.21)
project(VolchayWallpapers
    VERSION 0.1.0
    DESCRIPTION "Live video wallpapers for Windows"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(MSVC)
    add_compile_options(/utf-8 /W4 /permissive- /Zc:__cplusplus)
    add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN UNICODE _UNICODE)
endif()

# ---------- Qt ----------
find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
    Svg
    Widgets
)

qt_standard_project_setup(REQUIRES 6.5)

# ---------- libmpv ----------
# MPV_ROOT is expected to contain include/ and lib/ (and the runtime dll)
set(MPV_ROOT "" CACHE PATH "Root of libmpv SDK (contains include/ and lib/)")

find_path(MPV_INCLUDE_DIR
    NAMES mpv/client.h
    HINTS "${MPV_ROOT}/include" "${MPV_ROOT}"
)
find_library(MPV_LIBRARY
    NAMES mpv libmpv mpv-2 libmpv-2
    HINTS "${MPV_ROOT}/lib" "${MPV_ROOT}"
)

if(NOT MPV_INCLUDE_DIR OR NOT MPV_LIBRARY)
    if(MPV_ROOT)
        # User explicitly pointed at an SDK but we still couldn't resolve it —
        # fail loudly so we don't ship a stub-mode build to users.
        message(FATAL_ERROR
            "MPV_ROOT='${MPV_ROOT}' was provided but libmpv could not be resolved. "
            "Looked for 'mpv/client.h' under '${MPV_ROOT}/include' and a library "
            "named mpv/libmpv/mpv-2/libmpv-2 under '${MPV_ROOT}/lib'. "
            "On Windows, the import library may need to be generated from mpv.def "
            "via 'lib.exe /def:mpv.def /out:mpv.lib'.")
    endif()
    message(WARNING
        "libmpv not found. Set -DMPV_ROOT=<path>. "
        "Build will continue but the mpv backend will be stubbed.")
    set(VOLCHAY_HAVE_MPV OFF)
else()
    set(VOLCHAY_HAVE_MPV ON)
    message(STATUS "libmpv: ${MPV_LIBRARY}")
    message(STATUS "libmpv include: ${MPV_INCLUDE_DIR}")
endif()

# ---------- Sources ----------
set(VOLCHAY_SOURCES
    src/main.cpp
    src/core/Logger.h
    src/core/Logger.cpp
    src/core/Settings.h
    src/core/Settings.cpp
    src/core/ThemeManager.h
    src/core/ThemeManager.cpp
    src/core/WallpaperLibrary.h
    src/core/WallpaperLibrary.cpp
    src/core/WallpaperEngine.h
    src/core/WallpaperEngine.cpp
    src/core/SystemTray.h
    src/core/SystemTray.cpp
    src/core/MpvObject.h
    src/core/MpvObject.cpp
    src/core/PowerWatcher.h
    src/core/PowerWatcher.cpp
)

qt_add_executable(VolchayWallpapers WIN32 ${VOLCHAY_SOURCES})

qt_add_resources(VolchayWallpapers "qml_resources"
    PREFIX "/"
    FILES
        src/qml/Main.qml
        src/qml/pages/HomePage.qml
        src/qml/pages/LibraryPage.qml
        src/qml/pages/SettingsPage.qml
        src/qml/pages/AboutPage.qml
        src/qml/components/SideBar.qml
        src/qml/components/WallpaperCard.qml
        src/qml/components/AccentButton.qml
        src/qml/components/IconButton.qml
        src/qml/components/ThemePicker.qml
        src/qml/components/PreviewArea.qml
        src/qml/dialogs/ConfirmDialog.qml
)

qt_add_resources(VolchayWallpapers "icon_resources"
    PREFIX "/icons"
    BASE   "src/resources/icons"
    FILES
        src/resources/icons/home.svg
        src/resources/icons/library.svg
        src/resources/icons/settings.svg
        src/resources/icons/about.svg
        src/resources/icons/play.svg
        src/resources/icons/pause.svg
        src/resources/icons/apply.svg
        src/resources/icons/delete.svg
        src/resources/icons/folder.svg
        src/resources/icons/theme.svg
        src/resources/icons/monitor.svg
        src/resources/icons/logo.svg
)

target_include_directories(VolchayWallpapers PRIVATE
    src
    src/core
)

target_link_libraries(VolchayWallpapers PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Svg
    Qt6::Widgets
)

if(WIN32)
    # shell32 — SHQueryUserNotificationState в PowerWatcher.
    target_link_libraries(VolchayWallpapers PRIVATE user32 dwmapi gdi32 shell32)
endif()

if(VOLCHAY_HAVE_MPV)
    target_include_directories(VolchayWallpapers PRIVATE ${MPV_INCLUDE_DIR})
    target_link_libraries(VolchayWallpapers PRIVATE ${MPV_LIBRARY})
    target_compile_definitions(VolchayWallpapers PRIVATE VOLCHAY_HAVE_MPV=1)
else()
    target_compile_definitions(VolchayWallpapers PRIVATE VOLCHAY_HAVE_MPV=0)
endif()

set_target_properties(VolchayWallpapers PROPERTIES
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME "VolchayWallpapers"
)

# ---------- Install ----------
install(TARGETS VolchayWallpapers
    RUNTIME DESTINATION .
    BUNDLE  DESTINATION .
)
