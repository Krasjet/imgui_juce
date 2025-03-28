cmake_minimum_required(VERSION 3.15)
project(hidpi VERSION 0.0.1)

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
  GIT_TAG        v1.91.9b-docking)
FetchContent_MakeAvailable(imgui)

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

juce_add_gui_app(hidpi
  COMPANY_NAME kst
  PRODUCT_NAME "hidpi")

juce_generate_juce_header(hidpi)
add_subdirectory(src)

target_compile_definitions(hidpi
  PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:hidpi,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:hidpi,JUCE_VERSION>")

target_link_libraries(hidpi
  PRIVATE
    juce::juce_opengl
    imgui
    imgui_impl_juce
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
