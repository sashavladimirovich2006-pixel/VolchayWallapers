import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Effects
import Volchay.Mpv 1.0
import "components"
import "pages"

ApplicationWindow {
    id: root
    width: 1180
    height: 740
    minimumWidth: 960
    minimumHeight: 600
    visible: true
    title: qsTr("Volchay Wallpapers")
    color: Theme.background

    property int currentPage: 0  // 0 home, 1 library, 2 settings, 3 about

    // Smooth color transitions when the theme changes.
    Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.OutCubic } }

    // ----- Background accent glow (subtle, fills mood) -----
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: Theme.background }
            GradientStop { position: 1.0; color: Theme.surface }
        }
    }

    Item {
        anchors.fill: parent
        clip: true
        Rectangle {
            width: 480; height: 480; radius: 240
            x: -120; y: -160
            color: Theme.accent
            opacity: 0.10
            layer.enabled: true
            layer.effect: MultiEffect {
                blurEnabled: true
                blurMax: 64
                blur: 1.0
            }
        }
        Rectangle {
            width: 360; height: 360; radius: 180
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: -80
            anchors.bottomMargin: -100
            color: Theme.accentSoft
            opacity: 0.08
            layer.enabled: true
            layer.effect: MultiEffect {
                blurEnabled: true
                blurMax: 64
                blur: 1.0
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        SideBar {
            id: sidebar
            Layout.fillHeight: true
            Layout.preferredWidth: 224
            currentIndex: root.currentPage
            onIndexRequested: function(idx) { root.currentPage = idx }
        }

        // Vertical separator
        Rectangle {
            width: 1
            Layout.fillHeight: true
            color: Theme.border
            opacity: 0.6
        }

        // Stack of pages
        StackLayout {
            id: stack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.currentPage

            HomePage    {}
            LibraryPage {}
            SettingsPage{}
            AboutPage   {}
        }
    }

    // ----- Global toast / error feedback -----
    Connections {
        target: Library
        function onErrorOccurred(message) { toast.show(message, true) }
    }
    Connections {
        target: Engine
        function onEngineError(message) { toast.show(message, true) }
        function onAttached() {
            // After wallpaper window is attached to WorkerW, raise the
            // main window so it stays accessible above the wallpaper layer.
            root.raise()
            root.requestActivate()
        }
    }

    Rectangle {
        id: toast
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 28
        radius: 10
        color: Theme.surfaceAlt
        border.color: Theme.border
        border.width: 1
        opacity: 0
        height: 44
        width: Math.min(560, parent.width - 80)
        z: 100

        property bool error: false

        function show(text, isError) {
            toastText.text = text
            toast.error = isError === true
            opacity = 1
            hideTimer.restart()
        }

        Behavior on opacity { NumberAnimation { duration: 220 } }
        Timer { id: hideTimer; interval: 2600; onTriggered: toast.opacity = 0 }

        Rectangle {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 12
            width: 8; height: 8; radius: 4
            color: toast.error ? Theme.danger : Theme.accent
        }
        Text {
            id: toastText
            anchors.fill: parent
            anchors.leftMargin: 28
            anchors.rightMargin: 14
            verticalAlignment: Text.AlignVCenter
            color: Theme.text
            font.pixelSize: 13
            elide: Text.ElideRight
        }
    }

    // ----- Wallpaper host window (separate, parented to WorkerW) -----
    Loader {
        id: wallpaperHostLoader
        active: Settings.wallpaperEnabled && Settings.currentWallpaper.length > 0
        sourceComponent: Component {
            Window {
                id: hostWindow
                visible: true
                flags: Qt.FramelessWindowHint
                color: "black"
                title: "VolchayWallpaperHost"

                MpvObject {
                    id: wallpaperMpv
                    anchors.fill: parent
                    source: Settings.currentWallpaper
                    volume: Settings.volume
                    mute: Settings.muteOnBattery && Power.onBattery
                    scaleMode: Settings.scaleMode
                    fpsLimit: Settings.fpsLimit

                    onPlayingChanged: {
                        if (playing) {
                            safetyTimer.stop()
                            // Switch render pump from bootstrap (QExposeEvent)
                            // to steady mode to avoid GL deadlock
                            Engine.notifyFirstRender()
                        }
                    }

                    onMpvError: function(message) {
                        toast.show(message, true)
                        Settings.wallpaperEnabled = false
                    }
                }

                // Safety: если mpv завис и не начал играть за 30 сек — отключаем
                Timer {
                    id: safetyTimer
                    interval: 30000
                    running: true
                    onTriggered: {
                        if (!wallpaperMpv.playing) {
                            Settings.wallpaperEnabled = false
                        }
                    }
                }

                Connections {
                    target: Power
                    function onFullscreenActiveChanged(active) {
                        if (!Settings.pauseOnFullscreen) return
                        if (active) wallpaperMpv.pause()
                        else        wallpaperMpv.play()
                    }
                }
                Connections {
                    target: Settings
                    function onPauseOnFullscreenChanged() {
                        if (Settings.pauseOnFullscreen && Power.fullscreenActive)
                            wallpaperMpv.pause()
                        else if (!Settings.pauseOnFullscreen)
                            wallpaperMpv.play()
                    }
                }

                Component.onCompleted: {
                    if (!Engine.attach(hostWindow)) {
                        Settings.wallpaperEnabled = false
                        return
                    }
                    if (Settings.pauseOnFullscreen && Power.fullscreenActive)
                        wallpaperMpv.pause()
                }
                Component.onDestruction: Engine.detach()
            }
        }
    }
}
