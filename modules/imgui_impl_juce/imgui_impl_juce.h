/*
  BEGIN_JUCE_MODULE_DECLARATION
  ID:                 imgui_impl_juce
  vendor:             kst
  version:            1.0.0
  name:               JUCE backend for ImGui
  description:        JUCE backend for ImGui
  license:            LGPLv3
  minimumCppStandard: 14
  dependencies:       juce_opengl
  END_JUCE_MODULE_DECLARATION
*/
#pragma once

#include <imgui.h>

// forward declarations
namespace juce
{
  class Component;
  class OpenGLContext;
}

IMGUI_IMPL_API void ImGui_ImplJuce_Init(juce::Component &component, juce::OpenGLContext &context);
IMGUI_IMPL_API void ImGui_ImplJuce_Shutdown();
IMGUI_IMPL_API void ImGui_ImplJuce_NewFrame();
