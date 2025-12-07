cmake_minimum_required(VERSION 3.5)

project(ttml-tool VERSION 0.8 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Xml Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Xml Network)

set(PROJECT_SOURCES
        main.cpp

        res.qrc
        app_win32.rc

        mainwindow.cpp
        mainwindow.h
        mainwindow.ui

        lyric.cpp
        lyric.h

        lyricline.cpp
        lyricline.h

        lyricsyl.cpp
        lyricsyl.h

        lyrictime.cpp
        lyrictime.h

        lyricplus.h
        lyricplus.cpp

        lyriclineplus.h
        lyriclineplus.cpp

        lyricsylplus.h
        lyricsylplus.cpp

        lyrictimeplus.h
        lyrictimeplus.cpp

        mplaintextedit.cpp
        mplaintextedit.h

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
        F:/Qt/ttml-tool/ferrous_opencc.dll.lib
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
