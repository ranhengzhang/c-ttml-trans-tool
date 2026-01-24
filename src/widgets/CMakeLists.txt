file(GLOB_RECURSE WIDGET_SOURCES
        "*.cpp"
)

add_library(widgets STATIC ${WIDGET_SOURCES})

target_include_directories(widgets PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
target_link_libraries(widgets PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets)
