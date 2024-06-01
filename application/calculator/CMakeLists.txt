cntp_add_qml_module(calc thecalculator
    URI com.vicr123.thecalculator.calculator
    VERSION 1.0
    QML_FILES
        Calculator.qml
        ButtonPad.qml
        ColorButton.qml
        ButtonPadOperations.qml
    SOURCES
        calculatorcontroller.h calculatorcontroller.cpp
        historymodel.h historymodel.cpp
)

set_target_properties(thecalculator-qmlmodule-calc PROPERTIES
    CXX_STANDARD 20
)
