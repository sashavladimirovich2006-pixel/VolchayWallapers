import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Item {
    id: page

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 28
        spacing: 18

        RowLayout {
            Layout.fillWidth: true
            spacing: 14
            Image {
                source: "qrc:/icons/library.svg"
                width: 26; height: 26
                sourceSize.width: 26; sourceSize.height: 26
            }
            ColumnLayout {
                spacing: 2
                Text {
                    text: qsTr("Библиотека")
                    color: Theme.text
                    font.pixelSize: 22
                    font.bold: true
                }
                Text {
                    text: Library.count + " " + qsTr("обоев")
                    color: Theme.textMuted
                    font.pixelSize: 13
                }
            }
            Item { Layout.fillWidth: true }
        }

        Item {
            visible: Library.count === 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Column {
                anchors.centerIn: parent
                spacing: 8
                Image {
                    source: "qrc:/icons/library.svg"
                    width: 60; height: 60
                    sourceSize.width: 60; sourceSize.height: 60
                    opacity: 0.35
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    text: qsTr("Библиотека пуста")
                    color: Theme.textMuted
                    font.pixelSize: 14
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    text: qsTr("Добавьте видео на главной странице")
                    color: Theme.textMuted
                    font.pixelSize: 12
                    opacity: 0.8
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }

        GridView {
            id: grid
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: Library.count > 0
            clip: true
            cellWidth: 260
            cellHeight: 250
            model: Library
            delegate: WallpaperCard {
                width: grid.cellWidth - 12
                height: grid.cellHeight - 12
                title: name
                subtitle: filePath
                filePath: model.filePath
                selected: Settings.currentWallpaper === model.filePath
                onApply: {
                    Settings.currentWallpaper = model.filePath
                    Settings.wallpaperEnabled = true
                }
                onRemove: Library.removeAt(index)
                onPreview: Settings.currentWallpaper = model.filePath
            }
        }
    }
}
