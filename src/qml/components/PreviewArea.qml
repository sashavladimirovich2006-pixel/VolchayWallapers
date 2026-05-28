import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Volchay.Mpv 1.0

Rectangle {
    id: root
    radius: 14
    color: Theme.surface
    border.color: Theme.border
    border.width: 1
    clip: true

    property alias source: mpv.source
    property alias volume: mpv.volume
    property alias mute: mpv.mute
    property alias scaleMode: mpv.scaleMode

    MpvObject {
        id: mpv
        anchors.fill: parent
        anchors.margins: 1
        volume: Settings.volume
        mute: true
        scaleMode: Settings.scaleMode
    }

    // Empty state
    Item {
        anchors.fill: parent
        visible: mpv.source === ""
        Column {
            anchors.centerIn: parent
            spacing: 10
            Image {
                source: "qrc:/icons/library.svg"
                width: 56; height: 56
                sourceSize.width: 56; sourceSize.height: 56
                opacity: 0.4
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: qsTr("Превью появится после выбора видео")
                color: Theme.textMuted
                font.pixelSize: 13
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }

    // Controls overlay
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 12
        height: 44
        radius: 10
        color: Qt.rgba(0, 0, 0, 0.55)
        visible: mpv.source !== ""

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 10

            IconButton {
                iconSource: mpv.playing ? "qrc:/icons/pause.svg" : "qrc:/icons/play.svg"
                onClicked: mpv.playing ? mpv.pause() : mpv.play()
            }
            Slider {
                id: vol
                from: 0; to: 100
                value: Settings.volume
                Layout.fillWidth: true
                onMoved: { Settings.volume = value; mpv.volume = value; mpv.mute = (value === 0) }
            }
            Text {
                text: Math.round(vol.value) + "%"
                color: "white"
                font.pixelSize: 12
                Layout.preferredWidth: 40
                horizontalAlignment: Text.AlignRight
            }
        }
    }
}
