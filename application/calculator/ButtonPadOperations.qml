import QtQuick

ListModel {
    ListElement {
        buttonText: "x²"
        buttonAction: "²"
        buttonTooltip: qsTr("Square")
    }
    ListElement {
        buttonText: "x³"
        buttonAction: "³"
        buttonTooltip: qsTr("Cube")
    }
    ListElement {
        buttonText: "xⁿ"
        buttonAction: "^"
        buttonTooltip: qsTr("Exponent")
    }
    ListElement {
        buttonText: "√"
        buttonAction: "√"
        buttonTooltip: qsTr("Square Root")
    }
    ListElement {
        buttonText: "³√"
        buttonAction: "³√"
        buttonTooltip: qsTr("Cube Root")
    }
    ListElement {
        buttonText: "ⁿ√"
        buttonAction: "nth-root"
        buttonTooltip: qsTr("nth Root")
    }
    ListElement {
        buttonText: "sin"
        buttonAction: "sin("
        buttonTooltip: qsTr("Sine")
    }
    ListElement {
        buttonText: "cos"
        buttonAction: "cos("
        buttonTooltip: qsTr("Cosine")
    }
    ListElement {
        buttonText: "tan"
        buttonAction: "tan("
        buttonTooltip: qsTr("Tangent")
    }
    ListElement {
        buttonText: "asin"
        buttonAction: "asin("
        buttonTooltip: qsTr("Inverse Sine")
    }
    ListElement {
        buttonText: "acos"
        buttonAction: "acos("
        buttonTooltip: qsTr("Inverse Cosine")
    }
    ListElement {
        buttonText: "atan"
        buttonAction: "atan("
        buttonTooltip: qsTr("Inverse Tangent")
    }
    ListElement {
        buttonText: "log"
        buttonAction: "log("
        buttonTooltip: qsTr("Logarithm (base 10)")
    }
    ListElement {
        buttonText: "ln"
        buttonAction: "ln("
        buttonTooltip: qsTr("Natural Logarithm")
    }
    ListElement {
        buttonText: "logₓy"
        buttonAction: "log-generic"
        buttonTooltip: qsTr("Other Logarithm")
    }
}
