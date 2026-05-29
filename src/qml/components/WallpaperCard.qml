import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Volchay.Mpv 1.0

Rectangle {
    id: root
    radius: 12
    color: Theme.surface
    clip: true

    property string filePath: ""
    property string title: ""
    property string subtitle: ""
    property bool   selected: false
    signal apply()
    signal remove()
    signal preview()

    border.color: selected ? Theme.accent : Theme.border
    border.width: selected ? 2 : 1
    Behavior on border.color { ColorAnimation { duration: 160 } }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Thumb area: статичная иконка + live-превью при наведении.
        Rectangle {
            id: thumb
            Layout.fillWidth: true
            Layout.preferredHeight: 140
            color: Theme.surfaceAlt
            clip: true
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.18) }
                GradientStop { position: 1.0; color: Theme.surfaceAlt }
            }

            HoverHandler {
                id: hover
                acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            }

            // Небольшая задержка перед запуском mpv: если курсор просто
            // пролетел над карточкой, мы не успеваем поднять плеер.
            Timer {
                id: warmupTimer
                interval: 220
                repeat: false
                onTriggered: previewLoader.active = true
            }

            Connections {
                target: hover
                function onHoveredChanged() {
                    if (hover.hovered && root.filePath.length > 0 && !root.selected) {
                        warmupTimer.restart()
                    } else {
                        warmupTimer.stop()
                        previewLoader.active = false
                    }
                }
            }

            // Live-превью — мини-инстанс mpv. Создаётся только когда нужен,
            // уничтожается при уходе курсора. См. README, журнал за 2026-05-29.
            Loader {
                id: previewLoader
                anchors.fill: parent
                active: false
                opacity: active && item ? 1.0 : 0.0
                Behavior on opacity { NumberAnimation { duration: 180 } }
                sourceComponent: Component {
                    MpvObject {
                        source: root.filePath
                        volume: 0
                        mute: true
                        scaleMode: "fill"
                        // 24 FPS достаточно для превью; снижает нагрузку,
                        // если пользователь долго держит курсор на карточке
                        // или быстро перелетает между несколькими.
                        fpsLimit: 24
                    }
                }
            }

            // Статичная иконка «play». Прячется, когда превью прогрелось,
            // чтобы не просвечивала через видео тёмным пятном.
            Image {
                anchors.centerIn: parent
                source: "qrc:/icons/play.svg"
                width: 40; height: 40
                sourceSize.width: 40; sourceSize.height: 40
                opacity: previewLoader.opacity < 0.5 ? 0.85 : 0.0
                Behavior on opacity { NumberAnimation { duration: 180 } }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.preview()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.margins: 12
            spacing: 4
            Text {
                text: root.title
                color: Theme.text
                font.pixelSize: 14
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            Text {
                text: root.subtitle
                color: Theme.textMuted
                font.pixelSize: 11
                elide: Text.ElideMiddle
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 10
            spacing: 8

            AccentButton {
                text: qsTr("Применить")
                iconSource: "qrc:/icons/apply.svg"
                Layout.fillWidth: true
                onClicked: root.apply()
            }
            IconButton {
                iconSource: "qrc:/icons/delete.svg"
                onClicked: root.remove()
            }
        }
    }
}
