import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "../components"

Item {
    id: page

    property string previewPath: ""

    FileDialog {
        id: filePicker
        title: qsTr("Выберите видео для обоев")
        nameFilters: [
            "Видео (*.mp4 *.webm *.mkv *.mov *.avi *.m4v *.wmv)",
            "Все файлы (*)"
        ]
        onAccepted: {
            const url = selectedFile.toString()
            // strip file:/// prefix on Windows
            let path = url
            if (path.startsWith("file:///")) path = path.substring(8)
            else if (path.startsWith("file://")) path = path.substring(7)
            page.previewPath = path
            if (Library.addFile(path)) {
                // ok
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 28
        spacing: 20

        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 14
            Image {
                source: "qrc:/icons/home.svg"
                width: 26; height: 26
                sourceSize.width: 26; sourceSize.height: 26
            }
            ColumnLayout {
                spacing: 2
                Text {
                    text: qsTr("Главная")
                    color: Theme.text
                    font.pixelSize: 22
                    font.bold: true
                }
                Text {
                    text: qsTr("Загрузите видео и примените его как живые обои")
                    color: Theme.textMuted
                    font.pixelSize: 13
                }
            }
            Item { Layout.fillWidth: true }
        }

        // Action panel
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 96
            radius: 14
            color: Theme.surface
            border.color: Theme.border
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 14

                Image {
                    source: "qrc:/icons/folder.svg"
                    width: 36; height: 36
                    sourceSize.width: 36; sourceSize.height: 36
                }
                ColumnLayout {
                    spacing: 2
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("Загрузить видео")
                        color: Theme.text
                        font.pixelSize: 15
                        font.bold: true
                    }
                    Text {
                        text: qsTr("Поддерживаются mp4, webm, mkv, mov, avi и др.")
                        color: Theme.textMuted
                        font.pixelSize: 12
                    }
                }
                AccentButton {
                    text: qsTr("Выбрать файл")
                    iconSource: "qrc:/icons/folder.svg"
                    onClicked: filePicker.open()
                }
            }
        }

        // Preview + Apply
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 18

            PreviewArea {
                id: preview
                Layout.fillWidth: true
                Layout.fillHeight: true
                source: page.previewPath
            }

            ColumnLayout {
                Layout.preferredWidth: 280
                Layout.fillHeight: true
                spacing: 14

                Rectangle {
                    Layout.fillWidth: true
                    radius: 12
                    color: Theme.surface
                    border.color: Theme.border
                    border.width: 1
                    Layout.preferredHeight: applyCol.implicitHeight + 28

                    ColumnLayout {
                        id: applyCol
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 10

                        Text {
                            text: qsTr("Применение")
                            color: Theme.text
                            font.pixelSize: 14
                            font.bold: true
                        }
                        Text {
                            text: page.previewPath.length > 0
                                  ? page.previewPath.substring(page.previewPath.lastIndexOf("/") + 1)
                                  : qsTr("Файл не выбран")
                            color: Theme.textMuted
                            font.pixelSize: 12
                            elide: Text.ElideMiddle
                            Layout.fillWidth: true
                        }
                        AccentButton {
                            Layout.fillWidth: true
                            text: Engine.active ? qsTr("Перезапустить обои") : qsTr("Применить как обои")
                            iconSource: "qrc:/icons/apply.svg"
                            enabled: page.previewPath.length > 0
                            onClicked: {
                                Settings.currentWallpaper = page.previewPath
                                Settings.wallpaperEnabled = true
                            }
                        }
                        Button {
                            Layout.fillWidth: true
                            visible: Engine.active
                            text: qsTr("Остановить")
                            onClicked: Settings.wallpaperEnabled = false
                            background: Rectangle {
                                radius: 8
                                color: parent.hovered ? Theme.surfaceAlt : "transparent"
                                border.color: Theme.border
                                border.width: 1
                            }
                            contentItem: Text {
                                text: parent.text
                                color: Theme.text
                                font.pixelSize: 13
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 12
                    color: Theme.surface
                    border.color: Theme.border
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 8
                        Text {
                            text: qsTr("Подсказка")
                            color: Theme.text
                            font.pixelSize: 14
                            font.bold: true
                        }
                        Text {
                            text: qsTr("Файл добавится в библиотеку автоматически. Управление громкостью и режимом масштабирования — в Настройках.")
                            color: Theme.textMuted
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }
}
