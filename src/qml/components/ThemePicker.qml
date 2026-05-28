import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    spacing: 10

    Text {
        text: qsTr("Тема оформления")
        color: Theme.text
        font.pixelSize: 14
        font.bold: true
    }

    GridLayout {
        columns: 5
        columnSpacing: 12
        rowSpacing: 12
        Layout.fillWidth: true

        Repeater {
            model: Theme.availableThemes
            delegate: Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 84
                radius: 12
                color: previewBg()
                border.width: Theme.name === modelData ? 2 : 1
                border.color: Theme.name === modelData ? Theme.accent : Theme.border

                function previewBg() {
                    if (modelData === "snow")      return "#FFFFFF"
                    if (modelData === "blackout")  return "#000000"
                    if (modelData === "dark")      return "#1B1B1F"
                    if (modelData === "pink")      return "#FFF1F4"
                    if (modelData === "blackpink") return "#150009"
                    return Theme.surface
                }
                function previewText() {
                    if (modelData === "snow" || modelData === "pink") return "#1A1A1A"
                    return "#F2F2F2"
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 6

                    // Accent stripe (янтарь во всех темах)
                    Rectangle {
                        Layout.fillWidth: true
                        height: 6
                        radius: 3
                        color: Theme.accent
                    }
                    Text {
                        text: Theme.displayName(modelData)
                        color: parent.parent.previewText()
                        font.pixelSize: 12
                        font.bold: true
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    Text {
                        text: modelData
                        color: parent.parent.previewText()
                        opacity: 0.7
                        font.pixelSize: 10
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: Theme.setTheme(modelData)
                }
            }
        }
    }
}
