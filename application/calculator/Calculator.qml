import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Contemporary
import com.vicr123.Contemporary
import com.vicr123.Contemporary.impl as Impl

Item {
    id: root

    property bool typing: false

    CalculatorController {
        id: controller

        onEvaluationError: instantResultContainer.flash()
    }

    Keys.onPressed: event => {
                        event.accepted = true

                        if (event.key === Qt.Key_Backspace) {
                            backspace()
                            return;
                        }

                        switch (event.text) {
                            case "*":
                                type("×");
                                break;
                            case "/":
                                type("÷");
                                break;
                            default:
                                type(event.text)
                        }

                    }
    Keys.onEscapePressed: event => {
                              event.accepted = true;
                              root.clear()
                          }
    Keys.onReturnPressed: event => {
                              event.accepted = true;
                              root.performEvaluation()
                          }
    Keys.onLeftPressed: event => {
                            event.accepted = true;
                            controller.cursorLeft();
                        }
    Keys.onRightPressed: event => {
                            event.accepted = true;
                            controller.cursorRight();
                        }

    function type(key) {
        root.typing = true;
        controller.pressKey(key)
        root.typing = false;
    }

    function backspace() {
        root.typing = true;
        controller.backspace()
        root.typing = false;
    }

    function clear() {
        controller.clearExpression()
    }

    function performEvaluation() {
        controller.performEvaluation();
    }

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

                Layer {
                    Layout.fillWidth: true
                    Layout.preferredHeight: controller.intellisenseAvailable ? intellisenseLayout.childrenRect.height + 20 : 0

                    clip: true

                    GridLayout {
                        id: intellisenseLayout
                        anchors.fill: parent
                        anchors.margins: 10

                        columns: 4

                        Label {
                            Layout.fillWidth: true
                            text: controller.intellisenseFunction
                        }

                        Button {
                            id: intellisensePreviousButton
                            icon.name: "go-previous"
                            implicitWidth: intellisensePreviousButton.height
                            flat: true
                            Layout.rowSpan: 2
                        }

                        Label {
                            text: "1/3"
                            Layout.rowSpan: 2
                        }

                        Button {
                            id: intellisenseNextButton
                            icon.name: "go-next"
                            implicitWidth: intellisenseNextButton.height
                            flat: true
                            Layout.rowSpan: 2
                        }

                        Label {
                            text: controller.intellisenseDescription
                        }

                        Label {
                            text: controller.intellisenseArguments
                            Layout.columnSpan: 4
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    spacing: 1

                    TextField {
                        id: expressionField
                        Layout.fillWidth: true
                        horizontalAlignment: TextArea.AlignRight
                        placeholderText: qsTr("Expression...");
                        font.pointSize: 20
                        background: Item { }
                        text: controller.expressionString
                        cursorPosition: controller.cursorPosition
                        color: Contemporary.foreground

                        onCursorPositionChanged: !root.typing && (controller.cursorPosition = expressionField.cursorPosition)

                        Keys.forwardTo: [root]
                    }

                    Label {
                        text: controller.balancingBrackets
                        font.pointSize: 20
                        color: Contemporary.disabled(Contemporary.foreground)
                    }
                }

                ErrorFlasher {
                    id: instantResultContainer
                    Layout.fillWidth: true
                    Layout.preferredHeight: instantResultLabel.implicitHeight

                    Label {
                        id: instantResultLabel
                        anchors.fill: parent
                        text: controller.instantResult
                        font.pointSize: 15
                        horizontalAlignment: TextArea.AlignRight
                    }
                }
            }
        }

        ButtonPad {
            Layout.preferredWidth: parent.width

            onKeyPressed: key => root.type(key)
            onClearPressed: () => root.clear()
            onBackspacePressed: () => root.backspace()
            onEvaluationRequested: () => root.performEvaluation()
        }
    }
}
