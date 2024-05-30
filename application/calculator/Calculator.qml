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

        ButtonPad {
            Layout.preferredWidth: parent.width
        }
    }
}
