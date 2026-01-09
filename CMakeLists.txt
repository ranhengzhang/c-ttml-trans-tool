cmake_minimum_required(VERSION 3.5)

project(ttml-tool VERSION 1.4 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Xml Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Xml Network)

set(PROJECT_SOURCES
        # main
        main.cpp

        # resources
        res.qrc
        app_win32.rc

        # frames
        MainWindow.ui
        MainWindow.h
        MainWindow.cpp

        TimeFormatDialog.ui
        TimeFormatDialog.h
        TimeFormatDialog.cpp

        LangSelectDialog.h
        LangSelectDialog.cpp
        LangSelectDialog.ui

        # widgets
        MPlainTextEdit.cpp
        MPlainTextEdit.h

        # class
        LyricObject.h
        LyricObjectCore.cpp
        LyricObjectTTML.cpp
        LyricObjectASS.cpp
        LyricObjectLRC.cpp
        LyricObjectSPL.cpp
        LyricObjectQRC.cpp
        LyricObjectLYS.cpp
        LyricObjectYRC.cpp
        LyricObjectKRC.cpp

        LyricLine.h
        LyricLineCore.cpp
        LyricLineTTML.cpp
        LyricLineASS.cpp
        LyricLineLRC.cpp
        LyricLineSPL.cpp
        LyricLineQRC.cpp
        LyricLineLYS.cpp
        LyricLineYRC.cpp
        LyricLineKRC.cpp

        LyricSyl.h
        LyricSylCore.cpp
        LyricSylTTML.cpp
        LyricSylASS.cpp
        LyricSylSPL.cpp
        LyricSylQRC.cpp
        LyricSylYRC.cpp
        LyricSylKRC.cpp

        LyricTime.h
        LyricTime.cpp

        # others
        utils.h
        utils.cpp

        opencc.h
)

if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ttml-tool
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
    )
    # Define target properties for Android with Qt 6 as:
    #    set_property(TARGET ttml-tool APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else ()
    if (ANDROID)
        add_library(ttml-tool SHARED
                ${PROJECT_SOURCES}
        )
        # Define properties for Android with Qt 5 after find_package() calls as:
        #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else ()
        add_executable(ttml-tool
                ${PROJECT_SOURCES}
        )
    endif ()
endif ()

target_link_libraries(ttml-tool PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Xml
        Qt${QT_VERSION_MAJOR}::Network
        F:/Qt/ttml-tool/ferrous_opencc.lib
        ntdll.lib
)

set_target_properties(ttml-tool PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "C_TTML_TRANS_TOOL"
)

install(TARGETS ttml-tool
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if (QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ttml-tool)
endif ()
