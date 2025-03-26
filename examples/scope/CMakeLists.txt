cmake_minimum_required(VERSION 3.15)
project(scope VERSION 0.0.1)

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
  GIT_TAG        v1.91.9b-docking
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  PATCH_COMMAND git restore . && git apply --ignore-space-change --ignore-whitespace ${CMAKE_CURRENT_SOURCE_DIR}/patches/imgui/multi_instance.patch)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(implot
  GIT_REPOSITORY https://github.com/epezent/implot.git
  GIT_TAG        3da8bd34299965d3b0ab124df743fe3e076fa222
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  PATCH_COMMAND git restore . && git apply --ignore-space-change --ignore-whitespace ${CMAKE_CURRENT_SOURCE_DIR}/patches/implot/multi_instance.patch)
FetchContent_MakeAvailable(implot)

# replace with following for local testing
# add_subdirectory(../../modules imgui_juce)
FetchContent_Declare(imgui_juce
  GIT_REPOSITORY https://github.com/Krasjet/imgui_juce
  GIT_PROGRESS   TRUE
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

add_library(implot STATIC
  ${implot_SOURCE_DIR}/implot.cpp
  ${implot_SOURCE_DIR}/implot_items.cpp
  ${implot_SOURCE_DIR}/implot_demo.cpp)
target_include_directories(implot
  PUBLIC
    ${implot_SOURCE_DIR})
target_link_libraries(implot
  PUBLIC
    imgui)
set_target_properties(implot
  PROPERTIES POSITION_INDEPENDENT_CODE ON)

juce_add_plugin(scope
  FORMATS VST3 Standalone
  COMPANY_NAME kst
  PLUGIN_MANUFACTURER_CODE Krjt
  PLUGIN_CODE Ksco
  PRODUCT_NAME "Scope")

juce_generate_juce_header(scope)
add_subdirectory(src)

target_compile_definitions(scope
  PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(scope
  PRIVATE
    juce::juce_audio_utils
    juce::juce_opengl
    imgui
    implot
    imgui_impl_juce
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

# uncomment to automatically copy plugin after build
# juce_enable_copy_plugin_step(scope)
