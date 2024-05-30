import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Contemporary
import com.vicr123.Contemporary
import com.vicr123.Contemporary.impl as Impl

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: SafeZone.top + 10
        anchors.bottomMargin: SafeZone.bottom + 10
        anchors.leftMargin: 10
        anchors.rightMargin: 10

        Layer {
            Layout.fillHeight: true
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }

                Rectangle {
                    Layout.preferredHeight: 1
                    Layout.fillWidth: true
                    color: Contemporary.line
                }

                TextField {
                    Layout.fillWidth: true
                    horizontalAlignment: TextArea.AlignRight
                    placeholderText: qsTr("Expression...");
                    font.pointSize: 20
                    background: Item { }
                }

                Label {
                    Layout.fillWidth: true
                    horizontalAlignment: TextArea.AlignRight
                    text: "Intermediate answer goes here"
                    font.pointSize: 15
                }
            }
        }

        Rectangle {
            color: Contemporary.accent
            radius: 4

            Layout.preferredHeight: buttonsPane.height
            Layout.preferredWidth: parent.width

            GridLayout {
                id: buttonsPane
                rows: 4
                columns: 3
                anchors.left: parent.left
                anchors.right: operationsColor.left

                Button {
                    id: clearButton
                    text: "C"
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }

                Button {
                    Layout.columnSpan: 2
                    id: backspaceButton
                    text: "<" // TODO: Use an icon?
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }

                Repeater {
                    model: [7, 8, 9, 4, 5, 6, 1, 2, 3]

                    Button {
                        required property int modelData
                        text: Qt.locale().toString(modelData)
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }
                }

                Button {
                    id: decimalButton
                    text: Qt.locale().decimalPoint
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }

                Button {
                    id: zeroButton
                    text: Qt.locale().zeroDigit
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }

                Button {
                    id: answerButton
                    text: qsTr("Ans")
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }
            }


            Rectangle {
                id: operationsColor
                color: Contemporary.accent.lighter(1.2)
                radius: 4

                clip: true

                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right

                implicitWidth: parent.width / 2

                Flickable {
                    id: operationsPane
                    anchors.fill: parent

                    contentHeight: operationsGrid.childrenRect.height

                    GridLayout {
                        id: operationsGrid
                        columns: 3

                        width: operationsPane.width

                        Repeater {
                            model: 30

                            Button {
                                required property int index
                                id: button
                                text: `OpBtn ${Qt.locale().toString(index)}`

                                background: Impl.ButtonBackground {
                                    buttonColor: operationsColor.color
                                    flat: button.flat
                                    highlighted: button.highlighted
                                    checked: button.checked
                                    pressed: button.pressed
                                    enabled: button.enabled
                                    hovered: button.hovered

                                    Impl.FocusDecoration {
                                        anchors.fill: parent
                                        focused: button.visualFocus
                                        radius: 4
                                    }
                                }

                                Layout.fillHeight: true
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }
        }
    }
}
