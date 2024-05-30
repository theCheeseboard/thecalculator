cntp_add_qml_module(calc thecalculator
    URI com.vicr123.thecalculator.calculator
    VERSION 1.0
    QML_FILES
        Calculator.qml
        ButtonPad.qml
        ColorButton.qml
    SOURCES
        calculatorcontroller.h calculatorcontroller.cpp
)
