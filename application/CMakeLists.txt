project(thecalculator VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(libcontemporary REQUIRED)

set(SOURCES
        calcbutton.cpp
        expressionlineedit.cpp
        historydelegate.cpp
        main.cpp
        mainwindow.cpp
        mainwindow.ui
        calculator/calculatorwidget.cpp
        calculator/calculatorwidget.ui
        calculator/logbasepopover.cpp
        calculator/logbasepopover.ui
        calculator/nthrootpopover.cpp
        calculator/nthrootpopover.ui
        functions/customs/branchbox.cpp
        functions/customs/branchbox.ui
        functions/customs/conditionbox.cpp
        functions/customs/conditionbox.ui
        functions/customs/overloadbox.cpp
        functions/customs/overloadbox.ui
        functions/functionwidget.cpp
        functions/functionwidget.ui
        graph/addfunctiondialog.cpp
        graph/addfunctiondialog.ui
        graph/graphfunction.cpp
        graph/graphview.cpp
        graph/graphwidget.cpp
        graph/graphwidget.ui
        graph/renderdialog.cpp
        graph/renderdialog.ui
        stats/statwidget.cpp
        stats/statwidget.ui)

set(HEADERS
        calcbutton.h
        expressionlineedit.h
        historydelegate.h
        mainwindow.h
        calculator/calculatorwidget.h
        calculator/logbasepopover.h
        calculator/nthrootpopover.h
        functions/customs/branchbox.h
        functions/customs/conditionbox.h
        functions/customs/overloadbox.h
        functions/functionwidget.h
        graph/addfunctiondialog.h
        graph/graphfunction.h
        graph/graphview.h
        graph/graphwidget.h
        graph/renderdialog.h
        stats/statwidget.h)

# Find Apple lproj files
set(LPROJ_SOURCES)
file(GLOB LPROJ_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/translations/apple-lproj/*.lproj)
FOREACH(DIR ${LPROJ_DIRS})
    cmake_path(GET DIR FILENAME DIR_NAME)
    file(GLOB LPROJ_FILES ${DIR}/*.strings)
    FOREACH(FILE ${LPROJ_FILES})
        list(APPEND LPROJ_SOURCES ${FILE})
        set_source_files_properties(${FILE} PROPERTIES
                MACOSX_PACKAGE_LOCATION Resources/${DIR_NAME})
    ENDFOREACH()
ENDFOREACH()
set(SOURCES ${SOURCES} ${LPROJ_SOURCES})

add_executable(thecalculator ${SOURCES} ${HEADERS})

set_target_properties(thecalculator PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
        MACOSX_BUNDLE_GUI_IDENTIFIER com.vicr123.thecalculator
        WIN32_EXECUTABLE TRUE
        CNTP_DATA_SUBDIR thecalculator)

cntp_init(thecalculator 20)
cntp_translate(thecalculator)
cntp_target_name(thecalculator "theCalculator")
#cntp_app_icon(thecalculator
#    BASEICON icons/baseicon.svg
#    BASEICON_MAC icons/baseicon-mac.svg)
cntp_target_desktop_file(thecalculator
    DESKTOP_FILE com.vicr123.thecalculator.desktop
    BLUEPRINT_DESKTOP_FILE com.vicr123.thecalculator_blueprint.desktop)
cntp_defaults_file(thecalculator
    DEFAULTS_FILE thecalculator.conf)

target_link_libraries(thecalculator Qt6::Widgets libcontemporary libthecalculator)
target_include_directories(thecalculator PUBLIC ../libthecalculator/)

install(TARGETS thecalculator
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION /Applications)
