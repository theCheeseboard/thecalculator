cntp_add_qml_module(calc thecalculator
    URI com.vicr123.thecalculator.calculator
    VERSION 1.0
    QML_FILES
        Calculator.qml
        ButtonPad.qml
        KeypadButton.qml
        ButtonPadOperations.qml
        ControlButton.qml
    SOURCES
        calculatorcontroller.h calculatorcontroller.cpp
        historymodel.h historymodel.cpp
)

target_link_libraries(thecalculator-qmlmodule-calc PUBLIC libtcalc)
