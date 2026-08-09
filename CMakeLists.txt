cmake_minimum_required(VERSION 3.22)

project(MidiDestruct VERSION 0.3.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# ---------------------------------------------------------------------------
# Core: pure MIDI/randomization logic, no audio-processor / GUI dependencies.
# Kept separate so it can be unit-tested without building the plugin shell.
# ---------------------------------------------------------------------------
add_library(MidiDestructCore STATIC
    Source/Midi/RhythmData.cpp
    Source/Midi/MidiFileIO.cpp
    Source/Randomize/RhythmRandomizer.cpp
)

target_include_directories(MidiDestructCore PUBLIC Source)

target_link_libraries(MidiDestructCore
    PUBLIC
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_data_structures
)

target_compile_definitions(MidiDestructCore PUBLIC
    JUCE_STRICT_REFCOUNTEDPOINTER=1
)

# ---------------------------------------------------------------------------
# Plugin: VST3 + AU + Standalone
# ---------------------------------------------------------------------------
juce_add_plugin(MidiDestructPlugin
    PRODUCT_NAME "MidiDestruct"
    COMPANY_NAME "Gary A. Newsome"
    BUNDLE_ID "com.mididestruct.plugin"
    PLUGIN_MANUFACTURER_CODE Mdst
    PLUGIN_CODE Mdrx
    FORMATS VST3 AU Standalone
    IS_SYNTH FALSE
    IS_MIDI_EFFECT TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    VST3_CATEGORIES "Fx" "Tools"
    AU_MAIN_TYPE "kAudioUnitType_MIDIProcessor"
)

target_sources(MidiDestructPlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/UI/PianoRollView.cpp
    Source/UI/TransportControls.cpp
)

target_include_directories(MidiDestructPlugin PRIVATE Source)

target_compile_definitions(MidiDestructPlugin PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(MidiDestructPlugin
    PRIVATE
        MidiDestructCore
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ---------------------------------------------------------------------------
# Tests: pure-logic unit tests, does not link the plugin shell.
# ---------------------------------------------------------------------------
enable_testing()

add_executable(MidiDestructTests
    Tests/TestRunner.cpp
    Tests/RhythmDataTests.cpp
    Tests/RhythmRandomizerTests.cpp
)

target_link_libraries(MidiDestructTests
    PRIVATE
        MidiDestructCore
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_formats
)

add_test(NAME MidiDestructTests COMMAND MidiDestructTests)
