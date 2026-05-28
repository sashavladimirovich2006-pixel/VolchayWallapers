import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    radius: 12
    color: Theme.surface
    border.color: Theme.border
    border.width: 1
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

        // Thumb placeholder
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 140
            color: Theme.surfaceAlt
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.18) }
                GradientStop { position: 1.0; color: Theme.surfaceAlt }
            }
            Image {
                anchors.centerIn: parent
                source: "qrc:/icons/play.svg"
                width: 40; height: 40
                sourceSize.width: 40; sourceSize.height: 40
                opacity: 0.85
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
