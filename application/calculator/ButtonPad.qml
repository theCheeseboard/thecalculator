import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Contemporary
import com.vicr123.Contemporary.impl as Impl

Rectangle {
    id: root
    color: Contemporary.accent
    // color: Contemporary.background
    radius: 4

    Layout.preferredHeight: buttonsPane.height

    signal keyPressed(string key)
    signal clearPressed
    signal backspacePressed
    signal evaluationRequested

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

            onClicked: root.clearPressed()
        }

        Button {
            Layout.columnSpan: 2
            id: backspaceButton
            text: "<" // TODO: Use an icon?
            Layout.fillHeight: true
            Layout.fillWidth: true

            onClicked: root.backspacePressed()
        }

        Repeater {
            model: [7, 8, 9, 4, 5, 6, 1, 2, 3]

            Button {
                required property int modelData
                text: Qt.locale().toString(modelData)
                Layout.fillHeight: true
                Layout.fillWidth: true

                onClicked: root.keyPressed(Qt.locale().toString(modelData))
            }
        }

        Button {
            id: decimalButton
            text: Qt.locale().decimalPoint
            Layout.fillHeight: true
            Layout.fillWidth: true

            onClicked: root.keyPressed(Qt.locale().decimalPoint)
        }

        Button {
            id: zeroButton
            text: Qt.locale().zeroDigit
            Layout.fillHeight: true
            Layout.fillWidth: true

            onClicked: root.keyPressed(Qt.locale().zeroDigit)
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

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        implicitWidth: parent.width / 3 * 2

        GridLayout {
            id: operationsPane
            rows: 5
            columns: 3

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: advancedOperationsColor.left

            Repeater {
                model: ["(", ")", "%", "π", "e", "i", "×", "÷", "<<", "+", "-", ">>"]

                ColorButton {
                    required property var modelData
                    color: operationsColor.color
                    text: modelData

                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    onClicked: root.keyPressed(modelData)
                }
            }

            ColorButton {
                color: operationsColor.color
                text: "="

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.columnSpan: 3

                onClicked: root.evaluationRequested()
            }
        }

        Rectangle {
            id: advancedOperationsColor
            color: Contemporary.accent.lighter(1.4)
            radius: 4

            clip: true

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right

            implicitWidth: parent.width / 2

            Flickable {
                id: advancedOperationsPane
                anchors.fill: parent

                contentHeight: operationsGrid.childrenRect.height

                GridLayout {
                    id: operationsGrid
                    columns: 3

                    width: advancedOperationsPane.width

                    ButtonPadOperations {
                        id: buttonPadOperations
                    }

                    Repeater {
                        model: buttonPadOperations

                        ColorButton {
                            id: opButton
                            required property string buttonAction
                            required property string buttonText
                            required property string buttonTooltip
                            color: advancedOperationsColor.color
                            text: opButton.buttonText
                            hoverEnabled: true

                            Layout.fillHeight: true
                            Layout.fillWidth: true

                            ToolTip.delay: 1000
                            ToolTip.timeout: 5000
                            ToolTip.visible: hovered
                            ToolTip.text: opButton.buttonTooltip

                            onClicked: () => {
                                switch (opButton.buttonAction) {
                                    case "nth-root":
                                    case "log-generic":
                                       // TODO
                                       break;
                                    default:
                                        root.keyPressed(opButton.buttonAction)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
