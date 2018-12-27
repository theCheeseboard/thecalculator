#-------------------------------------------------
#
# Project created by QtCreator 2017-11-07T12:52:29
#
#-------------------------------------------------

QT       += core gui thelib
CONFIG   += c++14

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = theCalculator
TEMPLATE = app

unix:!macx {
    TARGET = thecalculator
}

macx {
    INCLUDEPATH += "/usr/local/include/the-libs"
    LIBS += -L/usr/local/lib -lthe-libs
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
    aboutwindow.cpp \
    historydelegate.cpp \
    customs/overloadbox.cpp \
    customs/branchbox.cpp \
    customs/conditionbox.cpp \
    expressionlineedit.cpp

HEADERS += \
        mainwindow.h \
    calcbutton.h \
    expression.h \
    aboutwindow.h \
    historydelegate.h \
    customs/overloadbox.h \
    customs/branchbox.h \
    customs/conditionbox.h \
    expressionlineedit.h

FORMS += \
        mainwindow.ui \
    aboutwindow.ui \
    customs/overloadbox.ui \
    customs/branchbox.ui \
    customs/conditionbox.ui

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

TRANSLATIONS += translations/vi_VN.ts \
    translations/da_DK.ts \
    translations/es_ES.ts \
    translations/lt_LT.ts \
    translations/nl_NL.ts \
    translations/pl_PL.ts \
    translations/pt_BR.ts \
    translations/ru_RU.ts \
    translations/sv_SE.ts \
    translations/en_AU.ts \
    translations/en_US.ts \
    translations/en_GB.ts \
    translations/en_NZ.ts \
    translations/de_DE.ts \
    translations/id_ID.ts \
    translations/au_AU.ts \
    translations/it_IT.ts \
    translations/nb_NO.ts \
    translations/no_NO.ts \
    translations/ro_RO.ts \
    translations/cy_GB.ts \
    translations/fr_FR.ts

qtPrepareTool(LUPDATE, lupdate)
genlang.commands = "$$LUPDATE -no-obsolete -source-language en_US $$_PRO_FILE_"

qtPrepareTool(LRELEASE, lrelease)
rellang.commands = "$$LRELEASE -removeidentical $$_PRO_FILE_"
QMAKE_EXTRA_TARGETS = genlang rellang
PRE_TARGETDEPS = genlang rellang

# Turn off stripping as this causes the install to fail :(
QMAKE_STRIP = echo

unix:!macx {
    target.path = /usr/bin

    translations.path = /usr/share/thecalculator/translations
    translations.files = translations/*.qm

    desktop.path = /usr/share/applications
    desktop.files = thecalculator.desktop

    INSTALLS += target desktop translations
}

RESOURCES += \
    icons.qrc
