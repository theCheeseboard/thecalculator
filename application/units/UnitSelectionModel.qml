import QtQuick 2.15

ListModel {
    ListElement {
        name: qsTr("Distance")

        units: [
            ListElement {
                name: qsTr("Millimeters")
            },
            ListElement {
                name: qsTr("Centimeters")
            },
            ListElement {
                name: qsTr("Meters")
            },
            ListElement {
                name: qsTr("Kilometers")
            },
            ListElement {
                name: qsTr("Inches")
            },
            ListElement {
                name: qsTr("Feet")
            },
            ListElement {
                name: qsTr("Yards")
            },
            ListElement {
                name: qsTr("Miles")
            },
            ListElement {
                name: qsTr("Lightyears")
            }
        ]
    }
    ListElement {
        name: qsTr("Weight")
        units: []
    }
    ListElement {
        name: qsTr("Temperature")

        units: [
            ListElement {
                name: qsTr("Celsius")
                conversion: [
                    ListElement {
                        item: "x"
                    }

                ]
            },
            ListElement {
                name: qsTr("Fahrenheit")
                // conversion: ["5", "x", "9", "*", "/", "32", "+"]
            },
            ListElement {
                name: qsTr("Kelvin")
            }
        ]
    }
}
