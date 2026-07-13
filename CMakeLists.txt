#   Copyright (c) 2026 Juan Minor
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.

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

option(ENABLE_AUTOTUNE "Enable the AutoTuner feature and standalone test" ON)

# Global C++20 standard configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ONNX Runtime Integration
include(FetchContent)
set(ONNXRUNTIME_VERSION "1.20.1")
if(WIN32)
    set(ONNXRUNTIME_URL "https://www.nuget.org/api/v2/package/Microsoft.ML.OnnxRuntime.DirectML/${ONNXRUNTIME_VERSION}")
else()
    set(ONNXRUNTIME_URL "https://github.com/microsoft/onnxruntime/releases/download/v${ONNXRUNTIME_VERSION}/onnxruntime-linux-x64-gpu-${ONNXRUNTIME_VERSION}.tgz")
endif()

if(WIN32)
    set(ONNXRUNTIME_DOWNLOAD_NAME onnxruntime_dml.zip)
else()
    set(ONNXRUNTIME_DOWNLOAD_NAME onnxruntime.tgz)
endif()

FetchContent_Declare(
  onnxruntime
  URL ${ONNXRUNTIME_URL}
  DOWNLOAD_NAME ${ONNXRUNTIME_DOWNLOAD_NAME}
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
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
    )
endif()

# Output directories mirroring the original Make layout
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${config} config_upper)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper} ${CMAKE_CURRENT_SOURCE_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config_upper} ${CMAKE_CURRENT_SOURCE_DIR}/bin/lib)
endforeach()

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
    src/hardware/hardware.cpp
    src/tuner/tuning_parameters.cpp
)

if (ENABLE_AUTOTUNE)
    target_sources(fenrir_core PRIVATE src/tuner/auto_tuner.cpp)
    target_compile_definitions(fenrir_core PRIVATE ENABLE_AUTOTUNE_FLAG)
    target_compile_definitions(fenrir_core PUBLIC ENABLE_AUTOTUNE_FLAG)
endif()

target_compile_definitions(fenrir_core PRIVATE
    FENRIR_VERSION="${FENRIR_VERSION}"
)

target_compile_options(fenrir_core PRIVATE ${COMMON_WARNING_FLAGS})
target_include_directories(fenrir_core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<$<BOOL:${WIN32}>:${onnxruntime_SOURCE_DIR}/build/native/include>
    $<$<NOT:$<BOOL:${WIN32}>>:${onnxruntime_SOURCE_DIR}/include>
)

if(WIN32)
    target_link_directories(fenrir_core PUBLIC ${onnxruntime_SOURCE_DIR}/runtimes/win-x64/native)
else()
    target_link_directories(fenrir_core PUBLIC ${onnxruntime_SOURCE_DIR}/lib)
endif()

target_link_libraries(fenrir_core PUBLIC onnxruntime)

# 2. Main Executable target: fenrir (was mainh)
add_executable(fenrir main.cpp)
target_link_libraries(fenrir PRIVATE fenrir_core)

if(WIN32)
    add_custom_command(TARGET fenrir POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${onnxruntime_SOURCE_DIR}/runtimes/win-x64/native/onnxruntime.dll"
            $<TARGET_FILE_DIR:fenrir>
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/onnx"
            "$<TARGET_FILE_DIR:fenrir>/onnx"
        COMMENT "Copying onnxruntime.dll and onnx models to binary directory"
    )
endif()

# 3. Tests and Coverage target
option(ENABLE_TESTING "Enable unit tests and coverage targets" OFF)
if(ENABLE_TESTING)
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
