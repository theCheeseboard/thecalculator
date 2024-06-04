import QtQuick 2.15

ListModel {
    ListElement {
        name: qsTr("Distance")

        units: [
            ListElement {
                name: qsTr("Millimeters")
                toBase: "0.001"
                suffix: " mm"
            },
            ListElement {
                name: qsTr("Centimeters")
                toBase: "0.01"
                suffix: " cm"
            },
            ListElement {
                name: qsTr("Meters")
                toBase: "x"
                suffix: " m"
            },
            ListElement {
                name: qsTr("Kilometers")
                toBase: "1000"
                suffix: " km"
            },
            ListElement {
                name: qsTr("Inches")
                toBase: "0.0254"
                suffix: " in"
            },
            ListElement {
                name: qsTr("Feet")
                toBase: "0.3048"
                suffix: " ft"
            },
            ListElement {
                name: qsTr("Yards")
                toBase: "0.9144"
                suffix: " yd"
            },
            ListElement {
                name: qsTr("Miles")
                toBase: "1609.34"
                suffix: " mi"
            },
            ListElement {
                name: qsTr("Lightyears")
                toBase: "9.4607379375591e+15"
            }
        ]
    }
    ListElement {
        name: qsTr("Mass")
        units: [
            ListElement {
                name: qsTr("Gram")
                toBase: "0.001"
                suffix: " g"
            },
            ListElement {
                name: qsTr("Kilogram")
                toBase: "x"
                suffix: " kg"
            },
            ListElement {
                name: qsTr("Ounce")
                toBase: "0.0283495"
                suffix: " oz"
            },
            ListElement {
                name: qsTr("Pound")
                toBase: "0.453592"
                suffix: " lb"
            }
        ]
    }
    ListElement {
        name: qsTr("Temperature")

        units: [
            ListElement {
                name: qsTr("Celsius")
                toBase: "x"
                suffix: "°C"
            },
            ListElement {
                name: qsTr("Fahrenheit")
                toBase: "(x-32)*5/9"
                fromBase: "x*9/5+32"
                suffix: "°F"
            },
            ListElement {
                name: qsTr("Kelvin")
                toBase: "-273.15+"
                suffix: "°K"
            }
        ]
    }
}
