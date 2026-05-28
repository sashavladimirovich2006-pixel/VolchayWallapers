import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Item {
    id: page

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: page.width
            spacing: 22

            // Header
            RowLayout {
                Layout.fillWidth: true
                Layout.margins: 28
                Layout.bottomMargin: 0
                spacing: 14
                Image {
                    source: "qrc:/icons/settings.svg"
                    width: 26; height: 26
                    sourceSize.width: 26; sourceSize.height: 26
                }
                ColumnLayout {
                    spacing: 2
                    Text {
                        text: qsTr("Настройки")
                        color: Theme.text
                        font.pixelSize: 22
                        font.bold: true
                    }
                    Text {
                        text: qsTr("Темы, воспроизведение и поведение программы")
                        color: Theme.textMuted
                        font.pixelSize: 13
                    }
                }
            }

            // Theme section
            Rectangle {
                Layout.fillWidth: true
                Layout.margins: 28
                Layout.topMargin: 0
                radius: 14
                color: Theme.surface
                border.color: Theme.border
                border.width: 1
                Layout.preferredHeight: themeCol.implicitHeight + 28

                ColumnLayout {
                    id: themeCol
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 14
                    ThemePicker {}
                }
            }

            // Playback section
            Rectangle {
                Layout.fillWidth: true
                Layout.margins: 28
                Layout.topMargin: 0
                radius: 14
                color: Theme.surface
                border.color: Theme.border
                border.width: 1
                Layout.preferredHeight: playCol.implicitHeight + 28

                ColumnLayout {
                    id: playCol
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 12

                    Text {
                        text: qsTr("Воспроизведение")
                        color: Theme.text
                        font.pixelSize: 14
                        font.bold: true
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: qsTr("Громкость"); color: Theme.text; Layout.preferredWidth: 180 }
                        Slider {
                            from: 0; to: 100
                            value: Settings.volume
                            onMoved: Settings.volume = value
                            Layout.fillWidth: true
                        }
                        Text { text: Settings.volume + "%"; color: Theme.textMuted; Layout.preferredWidth: 50 }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: qsTr("Лимит FPS"); color: Theme.text; Layout.preferredWidth: 180 }
                        ComboBox {
                            model: [24, 30, 60, 120]
                            currentIndex: Math.max(0, [24,30,60,120].indexOf(Settings.fpsLimit))
                            onActivated: Settings.fpsLimit = model[currentIndex]
                        }
                        Item { Layout.fillWidth: true }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: qsTr("Масштабирование"); color: Theme.text; Layout.preferredWidth: 180 }
                        ComboBox {
                            property var modes: ["fill", "fit", "stretch", "center"]
                            model: [qsTr("Заполнить"), qsTr("Вписать"), qsTr("Растянуть"), qsTr("По центру")]
                            currentIndex: Math.max(0, modes.indexOf(Settings.scaleMode))
                            onActivated: Settings.scaleMode = modes[currentIndex]
                        }
                        Item { Layout.fillWidth: true }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: qsTr("Монитор"); color: Theme.text; Layout.preferredWidth: 180 }
                        ComboBox {
                            Layout.fillWidth: true
                            model: Engine.monitors
                            // index 0 = "all monitors" => Settings.targetMonitor = -1
                            currentIndex: Settings.targetMonitor < 0
                                          ? 0
                                          : Math.min(Settings.targetMonitor + 1, model.length - 1)
                            onActivated: Settings.targetMonitor = (currentIndex === 0 ? -1 : currentIndex - 1)
                        }
                    }

                    Switch {
                        text: qsTr("Пауза при полноэкранных приложениях")
                        checked: Settings.pauseOnFullscreen
                        onToggled: Settings.pauseOnFullscreen = checked
                        contentItem: Text {
                            text: parent.text
                            color: Theme.text
                            leftPadding: parent.indicator.width + 12
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                    Switch {
                        text: qsTr("Без звука при работе от батареи")
                        checked: Settings.muteOnBattery
                        onToggled: Settings.muteOnBattery = checked
                        contentItem: Text {
                            text: parent.text
                            color: Theme.text
                            leftPadding: parent.indicator.width + 12
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            // System section
            Rectangle {
                Layout.fillWidth: true
                Layout.margins: 28
                Layout.topMargin: 0
                radius: 14
                color: Theme.surface
                border.color: Theme.border
                border.width: 1
                Layout.preferredHeight: sysCol.implicitHeight + 28

                ColumnLayout {
                    id: sysCol
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 12

                    Text {
                        text: qsTr("Система")
                        color: Theme.text
                        font.pixelSize: 14
                        font.bold: true
                    }

                    Switch {
                        text: qsTr("Запускать вместе с Windows")
                        checked: Settings.autoStart
                        onToggled: Settings.autoStart = checked
                        contentItem: Text {
                            text: parent.text
                            color: Theme.text
                            leftPadding: parent.indicator.width + 12
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                    Text {
                        text: qsTr("Лог-файлы сохраняются рядом с настройками программы и автоматически очищаются через 14 дней.")
                        color: Theme.textMuted
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }

            Item { Layout.preferredHeight: 28 }
        }
    }
}
