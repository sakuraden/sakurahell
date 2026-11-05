cmake_minimum_required(VERSION 3.22)
project(SakuraHell VERSION 1.0.0)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

if(DEFINED ENV{CI})
    set(SKHL_COPY FALSE)
else()
    set(SKHL_COPY TRUE)
endif()

juce_add_plugin(SakuraHell
    COMPANY_NAME "SakuraHell"
    PLUGIN_MANUFACTURER_CODE Sakr
    PLUGIN_CODE Skhl
    FORMATS VST3 Standalone
    PRODUCT_NAME "SakuraHell"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    COPY_PLUGIN_AFTER_BUILD ${SKHL_COPY})

juce_generate_juce_header(SakuraHell)

target_sources(SakuraHell PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(SakuraHell PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(SakuraHell PRIVATE
    juce::juce_audio_utils
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
