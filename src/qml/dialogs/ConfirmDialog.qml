import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dlg
    modal: true
    property string body: ""

    background: Rectangle {
        radius: 14
        color: Theme.surface
        border.color: Theme.border
        border.width: 1
    }

    header: Rectangle {
        color: "transparent"
        height: 44
        Text {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 16
            text: dlg.title
            color: Theme.text
            font.pixelSize: 16
            font.bold: true
        }
    }

    contentItem: ColumnLayout {
        spacing: 16
        Text {
            text: dlg.body
            color: Theme.text
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            Layout.preferredWidth: 340
        }
    }

    standardButtons: Dialog.Ok | Dialog.Cancel
}
