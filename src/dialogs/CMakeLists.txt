add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../lyric" "${CMAKE_CURRENT_BINARY_DIR}/lyric_build")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../widgets" "${CMAKE_CURRENT_BINARY_DIR}/widgets_build")

set(DIALOGS_SOURCES
        MainWindow.ui
        MainWindow.hpp
        MainWindow.cpp

        TimeFormatDialog.ui
        TimeFormatDialog.hpp
        TimeFormatDialog.cpp

        LangSelectDialog.ui
        LangSelectDialog.hpp
        LangSelectDialog.cpp
)

add_library(dialogs STATIC ${DIALOGS_SOURCES})

target_include_directories(dialogs PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Xml Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Xml Network)
target_link_libraries(dialogs PUBLIC
        lyric
        widgets
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Xml
        Qt${QT_VERSION_MAJOR}::Network)
