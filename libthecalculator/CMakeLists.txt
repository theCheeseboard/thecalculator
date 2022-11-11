project(libthecalculator VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED)
find_package(libcontemporary REQUIRED)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED) 

set(SOURCES
        evaluationengine.cpp
        expression.cpp
        idouble.cpp
)

set(HEADERS
    libthecalculator_global.h
    evaluationengine.h
    evaluationengineheaders.h
    expression.h
    idouble.h
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_compile_definitions(YY_NO_UNISTD_H _CRT_NONSTDC_NO_DEPRECATE)
endif()

# Do not generate flex output in the source directory
cmake_policy(SET CMP0098 NEW)

BISON_TARGET(theCalculatorParser parser/calc.y ${CMAKE_CURRENT_BINARY_DIR}/calc.bison.cpp 
             DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/calc.bison.h)
FLEX_TARGET(theCalculatorLexer parser/calc.l  ${CMAKE_CURRENT_BINARY_DIR}/calc.lexer.cpp)

list(APPEND SOURCES ${BISON_theCalculatorParser_OUTPUTS} ${FLEX_theCalculatorLexer_OUTPUTS})

add_library(libthecalculator SHARED ${SOURCES} ${HEADERS})
set_target_properties(libthecalculator PROPERTIES
        CNTP_DATA_SUBDIR thecalculator/libthecalculator)
cntp_init(libthecalculator 20)
cntp_translate(libthecalculator)
set_target_properties(libthecalculator PROPERTIES
        OUTPUT_NAME thecalculator
        FRAMEWORK TRUE
        MACOSX_FRAMEWORK_IDENTIFIER com.vicr123.libthecalculator
        VERSION 1.0.0
        PUBLIC_HEADER "${HEADERS}")

target_link_libraries(libthecalculator Qt6::Widgets libcontemporary)
target_compile_definitions(libthecalculator PRIVATE LIBTHECALCULATOR_LIBRARY)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKECONFIG_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/libthecalculator.framework/Resources/CMake)
    set(HEADER_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
    set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/libthecalculator.framework/Headers)
    set(LIBRARY_INSTALL_DIR ../)
ELSE()
    set(CMAKECONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/libthecalculator)
    set(HEADER_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/libthecalculator)
    set(INCLUDE_INSTALL_DIR ${HEADER_INSTALL_DIR})
    set(LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
ENDIF()

configure_package_config_file(libthecalculatorConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/libthecalculatorConfig.cmake
        INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
        PATH_VARS HEADER_INSTALL_DIR LIBRARY_INSTALL_DIR)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libthecalculatorConfig.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})
install(EXPORT libthecalculatorTargets
        FILE libthecalculatorTargets.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(TARGETS libthecalculator
        EXPORT libthecalculatorTargets
        PUBLIC_HEADER DESTINATION ${HEADER_INSTALL_DIR}
        INCLUDES DESTINATION ${INCLUDE_INSTALL_DIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR})

cntp_mark_support_library_directory()
