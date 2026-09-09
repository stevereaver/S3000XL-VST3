cmake_minimum_required(VERSION 3.22)

project(S3000XL-VST VERSION 1.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# Locate JUCE and MAME
#
# You can override these with environment variables:
#   JUCE_ROOT = path to a JUCE 8.0.15 (or compatible) source tree
#   MAME_ROOT = path to a MAME 0.288 source tree
#
# By default the project expects them as sibling clones inside the repo root.
# ------------------------------------------------------------------------------
set(JUCE_ROOT "$ENV{JUCE_ROOT}" CACHE PATH "Path to the JUCE source tree")
if(NOT JUCE_ROOT OR NOT EXISTS "${JUCE_ROOT}/CMakeLists.txt")
    set(JUCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/JUCE" CACHE PATH "Path to the JUCE source tree" FORCE)
endif()

set(MAME_ROOT "$ENV{MAME_ROOT}" CACHE PATH "Path to the MAME source tree")
if(NOT MAME_ROOT OR NOT EXISTS "${MAME_ROOT}/makefile")
    set(MAME_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/mame" CACHE PATH "Path to the MAME source tree" FORCE)
endif()

if(NOT EXISTS "${JUCE_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR
        "JUCE not found at ${JUCE_ROOT}.\n"
        "Clone it with:\n"
        "  git clone --depth 1 --branch 8.0.15 https://github.com/juce-framework/JUCE.git ${CMAKE_CURRENT_SOURCE_DIR}/JUCE")
endif()

if(NOT EXISTS "${MAME_ROOT}/makefile")
    message(FATAL_ERROR
        "MAME not found at ${MAME_ROOT}.\n"
        "Clone it with:\n"
        "  git clone --depth 1 --branch mame0288 https://github.com/mamedev/mame.git ${CMAKE_CURRENT_SOURCE_DIR}/mame")
endif()

# ------------------------------------------------------------------------------
# Static runtime on Windows to match the MAME libs (useRuntimeLibDLL="0")
# ------------------------------------------------------------------------------
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_subdirectory("${JUCE_ROOT}" "${CMAKE_BINARY_DIR}/JUCE")
add_subdirectory(plugin)
