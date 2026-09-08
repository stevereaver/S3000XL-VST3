# ------------------------------------------------------------------------------
# S3000XL-VST plugin target
# ------------------------------------------------------------------------------
set(PLUGIN_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")
set(MAME_SRC_DIR "${MAME_ROOT}/src")

# ------------------------------------------------------------------------------
# JUCE plugin target
# ------------------------------------------------------------------------------
juce_add_plugin(S3000XL-VST
    VERSION "1.0.1"
    PLUGIN_NAME "S3000XL-VST"
    DESCRIPTION "S3000XL-VST MAME Emu VST"
    PLUGIN_MANUFACTURER_CODE Esma
    PLUGIN_CODE S3kx
    COMPANY_NAME "MPC3000MAME"
    COMPANY_COPYRIGHT "Open Source GPLv2/3"
    COMPANY_EMAIL "sjs@sojusrecords.com"
    COMPANY_WEBSITE "www.sojusrecords.com"
    BUNDLE_ID com.sojusrecords.S3000XLVST
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    FORMATS VST3 Standalone
    PRODUCT_NAME "S3000XL-VST"
    VST3_CATEGORIES "Sampler Instrument Synth"
    COPY_PLUGIN_AFTER_BUILD FALSE)

# ------------------------------------------------------------------------------
# Binary resources (mirrors the Projucer resource="1" files)
# ------------------------------------------------------------------------------
juce_add_binary_data(S3000XL_VST_BinaryData
    NAMESPACE BinaryData
    SOURCES
        "${PLUGIN_SOURCE_DIR}/q.svg"
        "${PLUGIN_SOURCE_DIR}/cart.svg"
        "${PLUGIN_SOURCE_DIR}/floppy.svg"
        "${PLUGIN_SOURCE_DIR}/labels_rack.png"
        "${PLUGIN_SOURCE_DIR}/labels_compact.png"
        "${PLUGIN_SOURCE_DIR}/labels_tablet.png"
        "${PLUGIN_SOURCE_DIR}/labels_full.png"
        "${PLUGIN_SOURCE_DIR}/mame_plugins.zip")

# ------------------------------------------------------------------------------
# Plugin sources
# ------------------------------------------------------------------------------
target_sources(S3000XL-VST
    PRIVATE
        "${PLUGIN_SOURCE_DIR}/PluginProcessor.cpp"
        "${PLUGIN_SOURCE_DIR}/PluginEditor.cpp"
        "${PLUGIN_SOURCE_DIR}/PresetBrowserComponent.cpp"
    PUBLIC
        "${PLUGIN_SOURCE_DIR}/PluginProcessor.h"
        "${PLUGIN_SOURCE_DIR}/PluginEditor.h"
        "${PLUGIN_SOURCE_DIR}/PresetBrowserComponent.h"
        "${PLUGIN_SOURCE_DIR}/mpc_clickmap.h")

# ------------------------------------------------------------------------------
# Generated JuceHeader.h (the source code uses #include <JuceHeader.h>)
# ------------------------------------------------------------------------------
juce_generate_juce_header(S3000XL-VST)

target_compile_features(S3000XL-VST PUBLIC cxx_std_20)

# ------------------------------------------------------------------------------
# Compile definitions
# ------------------------------------------------------------------------------
target_compile_definitions(S3000XL-VST
    PUBLIC
        # MAME mandatory macros (must be visible before any MAME include)
        PTR64=1
        LSB_FIRST=1
        __STDC_LIMIT_MACROS=1
        __STDC_FORMAT_MACROS=1
        __STDC_CONSTANT_MACROS=1
        # MAME's bundled boost::asio needs standalone mode
        ASIO_STANDALONE=1
        # JUCE options carried over from the original .jucer
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_USE_FLAC=0
        JUCE_WEB_BROWSER=0
    PRIVATE
        # Tell JUCE not to bring in JACK on Windows
        JUCE_JACK=0)

# ------------------------------------------------------------------------------
# Include directories
# ------------------------------------------------------------------------------
target_include_directories(S3000XL-VST
    PUBLIC
        "${PLUGIN_SOURCE_DIR}"
        "${MAME_SRC_DIR}"
        "${MAME_SRC_DIR}/frontend/mame"
        "${MAME_SRC_DIR}/emu"
        "${MAME_SRC_DIR}/osd"
        "${MAME_SRC_DIR}/lib"
        "${MAME_SRC_DIR}/lib/util"
        "${MAME_SRC_DIR}/osd/modules/render"
        "${MAME_ROOT}/3rdparty"
        "${MAME_ROOT}/3rdparty/asio/include"
        "${MAME_ROOT}/3rdparty/zlib"
        "${MAME_SRC_DIR}/devices"
        "${MAME_SRC_DIR}/devices/machine"
        "${MAME_SRC_DIR}/mame/akai")

# ------------------------------------------------------------------------------
# Library search paths for the prebuilt MAME static libraries
# ------------------------------------------------------------------------------
target_link_directories(S3000XL-VST
    PUBLIC
        "${MAME_ROOT}/build/vs2022/bin/x64/Release"
        "${MAME_ROOT}/build/vs2022/bin/x64/Release/mame_s3000xl")

# ------------------------------------------------------------------------------
# JUCE modules and recommended flags
# ------------------------------------------------------------------------------
target_link_libraries(S3000XL-VST
    PUBLIC
        S3000XL_VST_BinaryData
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_processors_headless
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_midi_ci
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
    PRIVATE
        # MAME static libraries (order matters)
        frontend
        mame_s3000xl
        emu
        ocore_windows
        utils
        optional
        expat
        zlib
        portmidi
        flac
        7z
        softfloat3
        utf8proc
        jpeg
        sqlite3
        zstd
        formats
        dasm
        portaudio
        lua
        lualibs
        asmjit
        bgfx
        bimg
        bx
        ymfm
        wdlfft
        linenoise
        qtdbg_windows
        osd_windows
        # Windows system libraries required by MAME
        ws2_32
        winmm
        dinput8
        dxguid
        opengl32
        dsound
        xinput
        comctl32
        shlwapi
        ntdll
        userenv
        bcrypt
        advapi32)

# ------------------------------------------------------------------------------
# Compiler / linker settings
# ------------------------------------------------------------------------------
if(MSVC)
    target_compile_options(S3000XL-VST
        PRIVATE
            /W2
            /Ot
            /Oi
            /arch:AVX2
            /fp:fast
            /Ob2
            /MP
            /bigobj)

    target_link_options(S3000XL-VST
        PRIVATE
            /INCREMENTAL:NO)
endif()

# ------------------------------------------------------------------------------
# LTO for release builds
# ------------------------------------------------------------------------------
set_property(TARGET S3000XL-VST PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
