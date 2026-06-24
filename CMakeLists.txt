cmake_minimum_required(VERSION 3.16)
project(oreon-system-manager VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Concurrent)

option(BUILD_COVERAGE "Instrument with gcov for coverage reporting" OFF)
if(BUILD_COVERAGE)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "BUILD_COVERAGE requires GCC or Clang")
    endif()
    add_compile_options(--coverage -O0)
    add_link_options(--coverage)
endif()

# All non-main code lives in a static lib so tests can link against it
# without re-compiling every source file.
add_library(oreon_core STATIC
    src/mainwindow.cpp
    src/widgets/sidebar.cpp
    src/widgets/collapsibleoutput.cpp
    src/pages/packagepage.cpp
    src/pages/repopage.cpp
    src/pages/containerpage.cpp
    src/pages/driverspage.cpp
    src/theme/thememanager.cpp
    resources.qrc
)

target_link_libraries(oreon_core PUBLIC Qt6::Widgets Qt6::Core Qt6::Concurrent)
target_include_directories(oreon_core PUBLIC src)

qt_add_executable(oreon-system-manager src/main.cpp)
target_link_libraries(oreon-system-manager PRIVATE oreon_core)

set_target_properties(oreon-system-manager PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
)

include(GNUInstallDirs)

install(TARGETS oreon-system-manager
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES assets/logo.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps
    RENAME oreon-system-manager.png
)

install(FILES packaging/oreon-system-manager.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

option(BUILD_TESTING "Build the test suite" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
