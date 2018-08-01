#-------------------------------------------------
#
# Project created by QtCreator 2017-11-07T12:52:29
#
#-------------------------------------------------

QT       += core gui thelib
CONFIG   += c++14

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = thecalculator
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    calcbutton.cpp \
    expression.cpp

HEADERS += \
        mainwindow.h \
    calcbutton.h \
    expression.h

FORMS += \
        mainwindow.ui

DISTFILES += \
    parser/calc.y \
    parser/calc.l \
    thecalculator.desktop

FLEX = parser/calc.l
BISON = parser/calc.y

flexsource.input = FLEX
flexsource.output = ${QMAKE_FILE_BASE}.cpp
flexsource.commands = flex --header-file=${QMAKE_FILE_BASE}.h -o ${QMAKE_FILE_BASE}.cpp ${QMAKE_FILE_IN}
flexsource.variable_out = SOURCES
flexsource.name = Flex Sources ${QMAKE_FILE_IN}
flexsource.CONFIG += target_predeps

bisonsource.input = BISON
bisonsource.output = ${QMAKE_FILE_BASE}.bison.cpp
bisonsource.commands = bison -d -o ${QMAKE_FILE_BASE}.bison.cpp ${QMAKE_FILE_IN} -v
bisonsource.variable_out = SOURCES
bisonsource.name = Bison Sources ${QMAKE_FILE_IN}
bisonsource.CONFIG += target_predeps

bisonheader.input = BISON
bisonheader.output = ${QMAKE_FILE_BASE}.bison.hpp
bisonheader.commands = @true
bisonheader.variable_out = HEADERS
bisonheader.name = Bison Headers ${QMAKE_FILE_IN}
bisonheader.CONFIG += target_predeps no_link

flexheader.input = FLEX
flexheader.output = ${QMAKE_FILE_BASE}.h
flexheader.commands = @true
flexheader.variable_out = HEADERS
flexheader.name = Flex Headers ${QMAKE_FILE_IN}
flexheader.CONFIG += target_predeps no_link

QMAKE_EXTRA_COMPILERS += bisonsource bisonheader flexheader flexsource

unix:!macx {
    target.path = /usr/bin

    #translations.path = /usr/share/theslate/translations
    #translations.files = translations/*

    desktop.path = /usr/share/applications
    desktop.files = theslate.desktop

    INSTALLS += target desktop #translations
}
