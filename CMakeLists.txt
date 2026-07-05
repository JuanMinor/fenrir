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

# ONNX Runtime Integration
include(FetchContent)
set(ONNXRUNTIME_VERSION "1.16.3")
if(WIN32)
    set(ONNXRUNTIME_URL "https://github.com/microsoft/onnxruntime/releases/download/v${ONNXRUNTIME_VERSION}/onnxruntime-win-x64-${ONNXRUNTIME_VERSION}.zip")
else()
    set(ONNXRUNTIME_URL "https://github.com/microsoft/onnxruntime/releases/download/v${ONNXRUNTIME_VERSION}/onnxruntime-linux-x64-${ONNXRUNTIME_VERSION}.tgz")
endif()

FetchContent_Declare(
  onnxruntime
  URL ${ONNXRUNTIME_URL}
  DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_MakeAvailable(onnxruntime)

# Common Warning Flags
if(MSVC)
    set(COMMON_WARNING_FLAGS
        /W4
        /WX
    )
else()
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
endif()

# Output directories mirroring the original Make layout
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Mode-specific compilation flags (only for GCC/Clang)
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -flto -march=native")
endif()

# 1. Main Static Library target: fenrir_core
add_library(fenrir_core STATIC
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
    src/abi/c_api.cpp
    src/search/mcts.cpp
    src/eval/nn_eval.cpp
    src/uci/uci.cpp
    src/selfplay/selfplay.cpp
)

target_compile_definitions(fenrir_core PRIVATE
    FENRIR_VERSION="${FENRIR_VERSION}"
)

target_compile_options(fenrir_core PRIVATE ${COMMON_WARNING_FLAGS})
target_include_directories(fenrir_core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${onnxruntime_SOURCE_DIR}/include
)
target_link_directories(fenrir_core PUBLIC ${onnxruntime_SOURCE_DIR}/lib)
target_link_libraries(fenrir_core PUBLIC onnxruntime)

# 2. Main Executable target: fenrir (was mainh)
add_executable(fenrir main.cpp)
target_link_libraries(fenrir PRIVATE fenrir_core)

if(WIN32)
    add_custom_command(TARGET fenrir POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${onnxruntime_SOURCE_DIR}/lib/onnxruntime.dll"
            $<TARGET_FILE_DIR:fenrir>
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/onnx"
            "$<TARGET_FILE_DIR:fenrir>/onnx"
        COMMENT "Copying onnxruntime.dll and onnx models to binary directory"
    )
endif()

# 2.5 Perft Executable target: perft
add_executable(perft src/perft/perft.cpp)
target_link_libraries(perft PRIVATE fenrir_core)
target_compile_options(perft PRIVATE ${COMMON_WARNING_FLAGS})

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
        tests/unit/c_api.test.cpp
        tests/unit/mcts.test.cpp
        tests/unit/nn_eval.test.cpp
        tests/unit/uci.test.cpp
        tests/unit/selfplay.test.cpp
    )

    target_link_libraries(tests PRIVATE
        fenrir_core
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
    target_compile_options(fenrir_core PRIVATE -fprofile-arcs -ftest-coverage)
    target_link_options(fenrir_core PRIVATE -fprofile-arcs -ftest-coverage)

    target_compile_options(fenrir PRIVATE -fprofile-arcs -ftest-coverage)
    target_link_options(fenrir PRIVATE -fprofile-arcs -ftest-coverage)

    target_compile_options(tests PRIVATE -fprofile-arcs -ftest-coverage)
    target_link_options(tests PRIVATE -fprofile-arcs -ftest-coverage)
    target_compile_options(perft PRIVATE -fprofile-arcs -ftest-coverage)
    target_link_options(perft PRIVATE -fprofile-arcs -ftest-coverage)

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
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
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
