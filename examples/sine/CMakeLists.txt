cmake_minimum_required(VERSION 3.15)
project(sine VERSION 0.0.1)

# export compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG        8.0.6
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(juce)

FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  GIT_TAG        v1.91.9b-docking
  PATCH_COMMAND git restore . && git apply --ignore-space-change --ignore-whitespace ${CMAKE_CURRENT_SOURCE_DIR}/patches/imgui/multi_instance.patch)
FetchContent_MakeAvailable(imgui)

# replace with following for local testing
# add_subdirectory(../../modules imgui_juce)
FetchContent_Declare(imgui_juce
  GIT_REPOSITORY https://github.com/Krasjet/imgui_juce
  GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(imgui_juce)

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui
  PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends)
set_target_properties(imgui
  PROPERTIES POSITION_INDEPENDENT_CODE ON)

juce_add_plugin(sine
  FORMATS VST3 Standalone
  COMPANY_NAME kst
  PLUGIN_MANUFACTURER_CODE Krjt
  PLUGIN_CODE Ksin
  PRODUCT_NAME "Sine"
  IS_SYNTH TRUE
  # we don't use midi but ableton won't load vst3 synth
  # without midi input
  NEEDS_MIDI_INPUT TRUE)

juce_generate_juce_header(sine)
add_subdirectory(src)

target_compile_definitions(sine
  PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(sine
  PRIVATE
    juce::juce_audio_utils
    juce::juce_opengl
    imgui
    imgui_impl_juce
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

# uncomment to automatically copy plugin after build
# juce_enable_copy_plugin_step(sine)
