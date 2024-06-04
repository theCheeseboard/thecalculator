import QtQuick
import QtQuick.Controls
import Contemporary

Button {
    id: root
    padding: 0
    property color color: Contemporary.window

    background: Rectangle {
        radius: 4
        color: Contemporary.calculateColor(root.color, root.hovered, root.down, !root.enabled)
    }
}
