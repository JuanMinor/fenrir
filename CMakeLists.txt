cmake_minimum_required(VERSION 3.15)

# Read version from VERSION file
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" FENRIR_VERSION_RAW)
string(STRIP "${FENRIR_VERSION_RAW}" FENRIR_VERSION)

project(fenrir VERSION ${FENRIR_VERSION} LANGUAGES CXX)

# Default to Debug build if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
endif()

message(STATUS "Building in ${CMAKE_BUILD_TYPE} mode")

# Global C++20 standard configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Common Warning Flags
set(COMMON_WARNING_FLAGS
    -Wall
    -Wextra
    -Werror
    -pedantic
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Wunused
    -Woverloaded-virtual
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
)

# Output directories mirroring the original Make layout
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Mode-specific compilation flags
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

# 1. Main Shared Library target: fenrir
add_library(fenrir SHARED
    src/chess/attacks.cpp
    src/chess/board.cpp
    src/chrono/chrono.cpp
    src/engine/engine.cpp
    src/chess/fen.cpp
    src/logger/logger.cpp
    src/modifier/modifier.cpp
    src/chess/move.cpp
    src/chess/moves.cpp
    src/pgn/pgn.cpp
    src/utils/utils.cpp
)

target_compile_definitions(fenrir PRIVATE
    FENRIR_BUILD_DLL
    FENRIR_VERSION="${FENRIR_VERSION}"
)

target_compile_options(fenrir PRIVATE ${COMMON_WARNING_FLAGS})
target_include_directories(fenrir PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Versioning and SOVERSION to match Make behavior
set_target_properties(fenrir PROPERTIES
    VERSION ${FENRIR_VERSION}
    SOVERSION 0
)

# 2. Main Executable target: mainh
add_executable(mainh main.cpp)
target_link_libraries(mainh PRIVATE fenrir)

# 3. Tests and Coverage target (only built in Debug mode as testing in Release is disabled)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_package(GTest REQUIRED)
    find_package(Threads REQUIRED)

    add_executable(tests
        tests/unit/fen.test.cpp
        tests/unit/board.test.cpp
        tests/unit/engine.test.cpp
        tests/unit/utils.test.cpp
        tests/unit/move.test.cpp
        tests/unit/moves.test.cpp
        tests/unit/logger.test.cpp
        tests/unit/chrono.test.cpp
        tests/unit/modifier.test.cpp
        tests/unit/pgn.test.cpp
    )

    target_link_libraries(tests PRIVATE
        fenrir
        GTest::gtest_main
        GTest::gtest
        Threads::Threads
    )

    target_compile_options(tests PRIVATE ${COMMON_WARNING_FLAGS})

    # Set specific output directory for test executable to match Make
    set_target_properties(tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bint/unit
    )

    # Enable coverage flags
    target_compile_options(fenrir PRIVATE -fprofile-arcs -ftest-coverage)
    target_link_options(fenrir PRIVATE -fprofile-arcs -ftest-coverage)

    target_compile_options(tests PRIVATE -fprofile-arcs -ftest-coverage)
    target_link_options(tests PRIVATE -fprofile-arcs -ftest-coverage)

    # 4. Coverage Custom Target (always runs generate_coverage.sh)
    add_custom_target(coverage
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/build/generate_coverage.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running tests, generating coverage report and enforcing 100% policy..."
    )
endif()

# 5. Installation Rules
include(GNUInstallDirs)

install(TARGETS fenrir
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fenrir
)

# Generate and install pkg-config file
set(PREFIX ${CMAKE_INSTALL_PREFIX})
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/build/fenrir.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/fenrir.pc
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fenrir.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
