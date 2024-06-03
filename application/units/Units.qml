import QtQuick 2.15
import QtQuick.Layouts
import QtQuick.Controls
import Contemporary
import com.vicr123.Contemporary

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: SafeZone.top + 10
        anchors.bottomMargin: SafeZone.bottom + 10
        anchors.leftMargin: 10
        anchors.rightMargin: 10

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

                    implicitWidth: unitsList.width
                    implicitHeight: unitLayout.implicitHeight + 20

                    RowLayout {
                        id: unitLayout
                        anchors.fill: parent
                        anchors.margins: 10

                        SubtitleLabel {
                            text: unitItem.name
                        }

                        TextField {
                            Layout.fillWidth: true

                            background: Item { }
                        }
                    }
                }
            }
        }
    }
}
