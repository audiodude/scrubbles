cmake_minimum_required(VERSION 3.22)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
project(Scrubbles VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
include(FetchContent)
if(DEFINED ENV{JUCE_TARBALL} AND EXISTS "$ENV{JUCE_TARBALL}")
    set(SCRUBBLES_JUCE_URL "$ENV{JUCE_TARBALL}")
else()
    set(SCRUBBLES_JUCE_URL "https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.14.tar.gz")
endif()
FetchContent_Declare(JUCE URL "${SCRUBBLES_JUCE_URL}")
FetchContent_MakeAvailable(JUCE)
option(SCRUBBLES_COPY_PLUGIN "Install VST3 into the user plugin directory after building" OFF)
set(SCRUBBLES_SOURCES Source/Engine.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
juce_add_plugin(Scrubbles
    COMPANY_NAME "audiodude"
    BUNDLE_ID "xyz.audiodude.scrubbles"
    PLUGIN_MANUFACTURER_CODE Vibz
    PLUGIN_CODE Scrb
    FORMATS VST3 Standalone
    PRODUCT_NAME "Scrubbles"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD ${SCRUBBLES_COPY_PLUGIN}
    VST3_CATEGORIES Instrument Synth)
target_sources(Scrubbles PRIVATE ${SCRUBBLES_SOURCES})
target_compile_definitions(Scrubbles PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(Scrubbles PRIVATE juce::juce_audio_utils juce::juce_dsp PUBLIC juce::juce_recommended_config_flags)

juce_add_console_app(ScrubblesCheck PRODUCT_NAME "ScrubblesCheck")
target_sources(ScrubblesCheck PRIVATE Verification/Check.cpp ${SCRUBBLES_SOURCES})
target_compile_definitions(ScrubblesCheck PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_MODAL_LOOPS_PERMITTED=1 JUCE_PLUGINHOST_VST3=1)
target_link_libraries(ScrubblesCheck PRIVATE juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
enable_testing()
add_test(NAME ScrubblesBehavior COMMAND ScrubblesCheck)
