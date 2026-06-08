cmake_minimum_required(VERSION 3.16)
project(oreon-system-manager VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Concurrent)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/widgets/sidebar.cpp
    src/pages/packagepage.cpp
    src/pages/repopage.cpp
    src/pages/containerpage.cpp
    src/pages/driverspage.cpp
)

set(HEADERS
    src/mainwindow.h
    src/widgets/sidebar.h
    src/pages/packagepage.h
    src/pages/repopage.h
    src/pages/containerpage.h
    src/pages/driverspage.h
)

qt_add_executable(oreon-system-manager
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(oreon-system-manager PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Concurrent
)

target_include_directories(oreon-system-manager PRIVATE src)

install(TARGETS oreon-system-manager
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
