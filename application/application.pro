#-------------------------------------------------
#
# Project created by QtCreator 2017-11-07T12:52:29
#
#-------------------------------------------------

QT       += core gui thelib
CONFIG   += c++14
SHARE_APP_NAME = thecalculator

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = theCalculator
TEMPLATE = app

unix:!macx {
    DESKTOP_FILE = com.vicr123.thecalculator.desktop
    DESKTOP_FILE_BLUEPRINT = com.vicr123.thecalculator_blueprint.desktop

    # Include the-libs build tools
    equals(THELIBS_BUILDTOOLS_PATH, "") {
        THELIBS_BUILDTOOLS_PATH = $$[QT_INSTALL_PREFIX]/share/the-libs/pri
    }
    include($$THELIBS_BUILDTOOLS_PATH/buildmaster.pri)

    TARGET = thecalculator

    target.path = $$THELIBS_INSTALL_BIN

    blueprint {
        metainfo.files = com.vicr123.thecalculator_blueprint.metainfo.xml
        icon.files = icons/com.vicr123.thecalculator_blueprint.svg
    } else {
        metainfo.files = com.vicr123.thecalculator.metainfo.xml
        icon.files = icons/com.vicr123.thecalculator.svg
    }

    icon.path = $$THELIBS_INSTALL_PREFIX/share/icons/hicolor/scalable/apps/
    metainfo.path = $$THELIBS_INSTALL_PREFIX/share/metainfo

    INSTALLS += target icon metainfo
}

macx {
    # Include the-libs build tools
    include(/usr/local/share/the-libs/pri/buildmaster.pri)

    INCLUDEPATH += "/usr/local/include/the-libs"
    LIBS += -L/usr/local/lib -lthe-libs -framework CoreFoundation -framework AppKit
    ICON = icon.icns
}

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
    expression.cpp \
    historydelegate.cpp \
    customs/overloadbox.cpp \
    customs/branchbox.cpp \
    customs/conditionbox.cpp \
    expressionlineedit.cpp \
    evaluationengine.cpp \
    parts/calculator/calculatorwidget.cpp \
    parts/calculator/logbasepopover.cpp \
    parts/calculator/nthrootpopover.cpp \
    parts/functions/functionwidget.cpp \
    parts/graph/graphwidget.cpp \
    parts/graph/graphfunction.cpp \
    parts/graph/graphview.cpp \
    parts/graph/addfunctiondialog.cpp \
    parts/stats/statwidget.cpp \
    parts/graph/renderdialog.cpp

HEADERS += \
        mainwindow.h \
    calcbutton.h \
    expression.h \
    historydelegate.h \
    customs/overloadbox.h \
    customs/branchbox.h \
    customs/conditionbox.h \
    expressionlineedit.h \
    evaluationengine.h \
    evaluationengineheaders.h \
    parts/calculator/calculatorwidget.h \
    parts/calculator/logbasepopover.h \
    parts/calculator/nthrootpopover.h \
    parts/functions/functionwidget.h \
    parts/graph/graphwidget.h \
    parts/graph/graphfunction.h \
    parts/graph/graphview.h \
    parts/graph/addfunctiondialog.h \
    parts/stats/statwidget.h \
    parts/graph/renderdialog.h

FORMS += \
        mainwindow.ui \
    customs/overloadbox.ui \
    customs/branchbox.ui \
    customs/conditionbox.ui \
    parts/calculator/calculatorwidget.ui \
    parts/calculator/logbasepopover.ui \
    parts/calculator/nthrootpopover.ui \
    parts/functions/functionwidget.ui \
    parts/graph/graphwidget.ui \
    parts/graph/addfunctiondialog.ui \
    parts/stats/statwidget.ui \
    parts/graph/renderdialog.ui

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

# Turn off stripping as this causes the install to fail :(
QMAKE_STRIP = echo

RESOURCES += \
    icons.qrc
