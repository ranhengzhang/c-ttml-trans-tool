file(GLOB_RECURSE LYRIC_SOURCES
        "lyric_line/*.cpp"
        "lyric_object/*.cpp"
        "lyric_syl/*.cpp"
        "lyric_time/*.cpp"
        "utils/*.cpp"
)

add_library(lyric STATIC ${LYRIC_SOURCES})

target_include_directories(lyric PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/lyric_line"
        "${CMAKE_CURRENT_SOURCE_DIR}/lyric_object"
        "${CMAKE_CURRENT_SOURCE_DIR}/lyric_syl"
        "${CMAKE_CURRENT_SOURCE_DIR}/lyric_time"
        "${CMAKE_CURRENT_SOURCE_DIR}/utils"
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Xml Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Xml Gui)
target_link_libraries(lyric PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Xml
        Qt${QT_VERSION_MAJOR}::Gui
        "${CMAKE_CURRENT_SOURCE_DIR}/ferrous_opencc/ferrous_opencc.lib"
        ntdll.lib)
