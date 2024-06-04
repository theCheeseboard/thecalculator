import QtQuick 2.15
import QtQuick.Layouts
import QtQuick.Controls
import Contemporary
import com.vicr123.Contemporary

Item {
    id: root

    UnitsController {
        id: controller
    }

    LayerCalculator {
        id: layer2
        layer: 2
    }

    Grandstand {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        id: grandstand

        innerTopMargin: SafeZone.top

        z: 10
        text: "Units";
        color: layer2.color
    }

    property string baseValue: ""

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: grandstand.bottom
        anchors.topMargin: 6
        anchors.bottomMargin: SafeZone.bottom + 10
        anchors.leftMargin: 10
        anchors.rightMargin: 10

        Rectangle {
            Layout.preferredHeight: topLayout.implicitHeight + 10
            Layout.fillWidth: true

            color: Contemporary.background
            z: 10

            ColumnLayout {
                id: topLayout

                anchors.left: parent.left
                anchors.right: parent.right

                RowLayout {
                    spacing: 10

                    Label {
                        text: qsTr("Unit Type")
                        verticalAlignment: Qt.AlignVCenter
                    }

                    ComboBox {
                        id: unitSelectionBox

                        flat: true
                        textRole: "name"
                        valueRole: "units"

                        model: UnitSelectionModel {

                        }
                    }
                }

                Layer {
                    Layout.preferredHeight: mainUnitLayout.implicitHeight + 20
                    Layout.fillWidth: true

                    RowLayout {
                        id: mainUnitLayout
                        anchors.fill: parent
                        anchors.margins: 10

                        function updateBaseValue() {
                            root.baseValue = currentUnitValue.text === "" ? "" : controller.evaluate(currentUnitSelection.currentValue, currentUnitValue.text, false);
                        }

                        ComboBox {
                            id: currentUnitSelection
                            model: unitSelectionBox.currentValue
                            textRole: "name"
                            valueRole: "toBase"

                            onCurrentValueChanged: mainUnitLayout.updateBaseValue()
                        }

                        TextField {
                            id: currentUnitValue
                            Layout.fillWidth: true

                            placeholderText: qsTr("Quantity...")
                            background: Item { }

                            onTextChanged: mainUnitLayout.updateBaseValue()
                        }
                    }
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                height: 1
                color: Contemporary.line
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: unitsList
                anchors.fill: parent

                model: unitSelectionBox.currentValue
                spacing: 10

                delegate: Layer {
                    id: unitItem
                    required property string name
                    required property string toBase
                    required property string fromBase
                    required property string suffix

                    implicitWidth: unitsList.width
                    implicitHeight: unitLayout.implicitHeight + 20

                    RowLayout {
                        id: unitLayout
                        anchors.fill: parent
                        anchors.margins: 10

                        SubtitleLabel {
                            Layout.preferredWidth: 200
                            text: unitItem.name
                        }

                        Label {
                            Layout.fillWidth: true

                            text: root.baseValue === "" ? "" : controller.evaluate(unitItem.fromBase ? unitItem.fromBase : unitItem.toBase, root.baseValue, true) + unitItem.suffix
                        }
                    }
                }
            }
        }
    }
}
