cmake_minimum_required(VERSION 3.15)
project(mwe VERSION 0.0.1)

# export compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG        7.0.2
  GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(juce)

FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        edf522b58a8f74dee3225a0d5bc71565b1b8f6e0)
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

juce_add_gui_app(mwe
  COMPANY_NAME kst
  PRODUCT_NAME "mwe")

juce_generate_juce_header(mwe)
add_subdirectory(src)

target_compile_definitions(mwe
  PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:mwe,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:mwe,JUCE_VERSION>")

target_link_libraries(mwe
  PRIVATE
    juce::juce_opengl
    imgui
    imgui_impl_juce
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
