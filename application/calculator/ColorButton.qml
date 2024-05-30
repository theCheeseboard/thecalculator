import QtQuick
import QtQuick.Controls
import Contemporary
import com.vicr123.Contemporary.impl as Impl

Button {
    property var color: Contemporary.accent
    id: button

    background: Impl.ButtonBackground {
        buttonColor: button.color
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
}
