import QtQuick
import QtQuick.Controls

Button {
    id: control
    property string iconSource: ""
    property color iconTint: Theme.text

    flat: true
    implicitWidth: 36
    implicitHeight: 36
    padding: 0

    background: Rectangle {
        radius: 8
        color: control.hovered ? Theme.surfaceAlt : "transparent"
        border.color: Theme.border
        border.width: control.hovered ? 1 : 0
        Behavior on color { ColorAnimation { duration: 120 } }
    }

    contentItem: Item {
        Image {
            anchors.centerIn: parent
            source: control.iconSource
            sourceSize.width: 18; sourceSize.height: 18
            width: 18; height: 18
        }
    }
}
