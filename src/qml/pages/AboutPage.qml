import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

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
                source: "qrc:/icons/about.svg"
                width: 26; height: 26
                sourceSize.width: 26; sourceSize.height: 26
            }
            Text {
                text: qsTr("О программе")
                color: Theme.text
                font.pixelSize: 22
                font.bold: true
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 14
            color: Theme.surface
            border.color: Theme.border
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 24
                spacing: 12

                Image {
                    source: "qrc:/icons/logo.svg"
                    width: 96; height: 96
                    sourceSize.width: 96; sourceSize.height: 96
                    Layout.alignment: Qt.AlignHCenter
                }
                Text {
                    text: "Volchay Wallpapers"
                    color: Theme.text
                    font.pixelSize: 24
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }
                Text {
                    text: qsTr("Версия 0.1.0")
                    color: Theme.accent
                    font.pixelSize: 13
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }
                Text {
                    text: qsTr("Живые обои из видео для Windows. C++17, Qt 6, libmpv.")
                    color: Theme.textMuted
                    font.pixelSize: 13
                    Layout.alignment: Qt.AlignHCenter
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                }
                Item { Layout.fillHeight: true }
            }
        }
    }
}
