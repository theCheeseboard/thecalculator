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
    ListElement {
        name: qsTr("Area")

        units: [
            ListElement {
                name: qsTr("Square Millimeters")
                toBase: "0.000001"
                suffix: " mm²"
            },
            ListElement {
                name: qsTr("Square Centimeters")
                toBase: "0.0001"
                suffix: " cm²"
            },
            ListElement {
                name: qsTr("Square Meters")
                toBase: "x"
                suffix: " m²"
            },
            ListElement {
                name: qsTr("Hectares")
                toBase: "10000"
                suffix: " ha"
            },
            ListElement {
                name: qsTr("Square Kilometers")
                toBase: "1000000"
                suffix: " km²"
            },
            ListElement {
                name: qsTr("Square Feet")
                toBase: "0.092903"
                suffix: " ft²"
            },
            ListElement {
                name: qsTr("Square Yards")
                toBase: "0.836127"
                suffix: " yd²"
            },
            ListElement {
                name: qsTr("Acres")
                toBase: "4046.86"
                suffix: " ac"
            },
            ListElement {
                name: qsTr("Square Miles")
                toBase: "2589988.11"
                suffix: " mi²"
            }
        ]
    }
    ListElement {
        name: qsTr("Power")

        units: [
            ListElement {
                name: qsTr("Watts")
                toBase: "x"
                suffix: " W"
            },
            ListElement {
                name: qsTr("Kilowatts")
                toBase: "1000"
                suffix: " kW"
            },
            ListElement {
                name: qsTr("Megawatts")
                toBase: "1000000"
                suffix: " MW"
            },
            ListElement {
                name: qsTr("Horsepower")
                toBase: "745.7"
                suffix: " hp"
            },
            ListElement {
                name: qsTr("Foot-pounds per minute")
                toBase: "0.022597"
                suffix: " ft·lb/min"
            },
            ListElement {
                name: qsTr("British Thermal Units per hour")
                toBase: "0.293071"
                suffix: " BTU/h"
            },
            ListElement {
                name: qsTr("British Thermal Units per minute")
                toBase: "17.584264"
                suffix: " BTU/min"
            },
            ListElement {
                name: qsTr("Calories per second")
                toBase: "4.184"
                suffix: " cal/s"
            }
        ]
    }
    ListElement {
        name: qsTr("Data")

        units: [
            ListElement {
                name: qsTr("Bytes")
                toBase: "x"
                suffix: " B"
            },
            ListElement {
                name: qsTr("Kilobytes")
                toBase: "1000"
                suffix: " kB"
            },
            ListElement {
                name: qsTr("Megabytes")
                toBase: "1000000"
                suffix: " MB"
            },
            ListElement {
                name: qsTr("Gigabytes")
                toBase: "1000000000"
                suffix: " GB"
            },
            ListElement {
                name: qsTr("Terabytes")
                toBase: "1000000000000"
                suffix: " TB"
            },
            ListElement {
                name: qsTr("Petabytes")
                toBase: "1000000000000000"
                suffix: " PB"
            },
            ListElement {
                name: qsTr("Exabytes")
                toBase: "1000000000000000000"
                suffix: " EB"
            },
            ListElement {
                name: qsTr("Zettabytes")
                toBase: "1000000000000000000000"
                suffix: " ZB"
            },
            ListElement {
                name: qsTr("Yottabytes")
                toBase: "1000000000000000000000000"
                suffix: " YB"
            },
            ListElement {
                name: qsTr("Kibibytes")
                toBase: "1024"
                suffix: " KiB"
            },
            ListElement {
                name: qsTr("Mebibytes")
                toBase: "1048576"
                suffix: " MiB"
            },
            ListElement {
                name: qsTr("Gibibytes")
                toBase: "1073741824"
                suffix: " GiB"
            },
            ListElement {
                name: qsTr("Tebibytes")
                toBase: "1099511627776"
                suffix: " TiB"
            },
            ListElement {
                name: qsTr("Pebibytes")
                toBase: "1125899906842624"
                suffix: " PiB"
            },
            ListElement {
                name: qsTr("Exbibytes")
                toBase: "1152921504606846976"
                suffix: " EiB"
            },
            ListElement {
                name: qsTr("Zebibytes")
                toBase: "1180591620717411303424"
                suffix: " ZiB"
            },
            ListElement {
                name: qsTr("Yobibytes")
                toBase: "1208925819614629174706176"
                suffix: " YiB"
            },
            ListElement {
                name: qsTr("Bits")
                toBase: "0.125"
                suffix: " b"
            },
            ListElement {
                name: qsTr("Kilobits")
                toBase: "125"
                suffix: " kb"
            },
            ListElement {
                name: qsTr("Megabits")
                toBase: "125000"
                suffix: " Mb"
            },
            ListElement {
                name: qsTr("Gigabits")
                toBase: "125000000"
                suffix: " Gb"
            },
            ListElement {
                name: qsTr("Terabits")
                toBase: "125000000000"
                suffix: " Tb"
            },
            ListElement {
                name: qsTr("Petabits")
                toBase: "125000000000000"
                suffix: " Pb"
            },
            ListElement {
                name: qsTr("Exabits")
                toBase: "125000000000000000"
                suffix: " Eb"
            },
            ListElement {
                name: qsTr("Zettabits")
                toBase: "125000000000000000000"
                suffix: " Zb"
            },
            ListElement {
                name: qsTr("Yottabits")
                toBase: "125000000000000000000000"
                suffix: " Yb"
            }
        ]
    }
}
