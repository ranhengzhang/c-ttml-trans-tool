cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_PROJECT_NAME OR PROJECT_IS_TOP_LEVEL)
    project(LyricParser LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

file(GLOB_RECURSE LYRIC_SOURCES
        "lyric_line/*.cpp"
        "lyric_object/*.cpp"
        "lyric_syl/*.cpp"
        "lyric_time/*.cpp"
        "utils/*.cpp"
)

add_library(lyric STATIC ${LYRIC_SOURCES})

target_compile_features(lyric PUBLIC cxx_std_23)

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
        Qt${QT_VERSION_MAJOR}::Gui)

if(MSVC)
    target_compile_options(lyric PUBLIC /Zc:__cplusplus)
endif()
