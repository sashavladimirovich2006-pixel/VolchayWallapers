import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: Theme.surface
    property int currentIndex: 0
    signal indexRequested(int idx)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 14

        // Header / logo
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            Image {
                source: "qrc:/icons/logo.svg"
                sourceSize.width: 36; sourceSize.height: 36
                width: 36; height: 36
                fillMode: Image.PreserveAspectFit
                smooth: true
            }
            ColumnLayout {
                spacing: 0
                Text {
                    text: "Volchay"
                    color: Theme.text
                    font.pixelSize: 18
                    font.bold: true
                    font.letterSpacing: 0.5
                }
                Text {
                    text: "Wallpapers"
                    color: Theme.accent
                    font.pixelSize: 12
                    font.bold: true
                    font.letterSpacing: 2
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.border
            opacity: 0.5
        }

        // Navigation
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            Repeater {
                model: [
                    { icon: "qrc:/icons/home.svg",     label: qsTr("Главная") },
                    { icon: "qrc:/icons/library.svg",  label: qsTr("Библиотека") },
                    { icon: "qrc:/icons/settings.svg", label: qsTr("Настройки") },
                    { icon: "qrc:/icons/about.svg",    label: qsTr("О программе") }
                ]
                delegate: Rectangle {
                    Layout.fillWidth: true
                    height: 44
                    radius: 10
                    property bool active: root.currentIndex === index
                    color: active ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.14)
                                  : (mouse.containsMouse ? Theme.surfaceAlt : "transparent")

                    Behavior on color { ColorAnimation { duration: 140 } }

                    Rectangle {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.active ? 3 : 0
                        height: 24
                        radius: 2
                        color: Theme.accent
                        Behavior on width { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        spacing: 12
                        Image {
                            source: modelData.icon
                            sourceSize.width: 18; sourceSize.height: 18
                            width: 18; height: 18
                            smooth: true
                            // Tint via ColorOverlay-like effect: we ship monochrome SVGs
                            // and color them through layer.effect on QML side at runtime.
                            opacity: parent.parent.active ? 1.0 : 0.85
                        }
                        Text {
                            text: modelData.label
                            color: parent.parent.parent.active ? Theme.accent : Theme.text
                            font.pixelSize: 14
                            font.bold: parent.parent.parent.active
                            Layout.fillWidth: true
                        }
                    }

                    MouseArea {
                        id: mouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.indexRequested(index)
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }

        // Footer: current state
        Rectangle {
            Layout.fillWidth: true
            height: 56
            radius: 10
            color: Theme.surfaceAlt
            border.color: Theme.border
            border.width: 1
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 2
                Text {
                    text: Engine.active ? qsTr("Обои активны")
                                        : qsTr("Обои не запущены")
                    color: Engine.active ? Theme.accent : Theme.textMuted
                    font.pixelSize: 12
                    font.bold: true
                }
                Text {
                    text: Settings.currentWallpaper.length > 0
                          ? Settings.currentWallpaper.substring(
                                Settings.currentWallpaper.lastIndexOf("/") + 1)
                          : qsTr("не выбрано")
                    color: Theme.textMuted
                    font.pixelSize: 11
                    elide: Text.ElideMiddle
                    Layout.fillWidth: true
                }
            }
        }
    }
}
