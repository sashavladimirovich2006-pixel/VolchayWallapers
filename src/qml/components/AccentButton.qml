import QtQuick
import QtQuick.Controls

Button {
    id: control
    property color baseColor: Theme.accent
    property color hoverColor: Theme.accentSoft
    property color textColor: "#1A1A1A"
    property string iconSource: ""

    height: 40
    leftPadding: 16
    rightPadding: 16

    background: Rectangle {
        radius: 8
        color: control.down ? Qt.darker(control.baseColor, 1.15)
              : (control.hovered ? control.hoverColor : control.baseColor)
        Behavior on color { ColorAnimation { duration: 120 } }
    }

    contentItem: Row {
        spacing: 8
        anchors.verticalCenter: parent.verticalCenter
        Image {
            visible: control.iconSource !== ""
            source: control.iconSource
            sourceSize.width: 18; sourceSize.height: 18
            width: 18; height: 18
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            text: control.text
            color: control.textColor
            font.pixelSize: 14
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
