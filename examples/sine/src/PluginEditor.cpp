/* PluginEditor.cpp: GUI component of sine */
#include <JuceHeader.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_juce/imgui_impl_juce.h>

#include "PluginEditor.h"
#include "PluginProcessor.h"

// helper function to set audio parameter from opengl thread.
// note that parameter writes must happen on message thread,
// or otherwise the event won't be broadcasted to the host
static inline void setAudioParameter(juce::AudioParameterFloat *p, float val)
{
  // copy by value here is necessary since the async callback
  // almost always live longer than the current scope.
  juce::MessageManager::callAsync([=](){
    p->beginChangeGesture();
    *p = val;
    p->endChangeGesture();
  });
}

SineAudioProcessorEditor::SineAudioProcessorEditor(SineAudioProcessor &p)
  : AudioProcessorEditor(&p), ap(p)
{
  setOpaque(true);

  // set up opengl context
  glctx.setOpenGLVersionRequired(juce::OpenGLContext::openGL3_2);
  glctx.setRenderer(this);
  glctx.attachTo(*this);
  glctx.setContinuousRepainting(true);

  setSize(768, 512);
}

void SineAudioProcessorEditor::saveLayout()
{
  const char *layout = ImGui::SaveIniSettingsToMemory();
  {
    const juce::ScopedLock lk(ap.guiLayoutLock);
    ap.guiLayout = layout;
  }
}

void SineAudioProcessorEditor::loadLayout()
{
  juce::String layout;
  {
    const juce::ScopedLock lk(ap.guiLayoutLock);
    layout = ap.guiLayout;
  }
  if (layout.length() > 0)
    ImGui::LoadIniSettingsFromMemory(layout.toRawUTF8(), layout.getNumBytesAsUTF8());
}

void SineAudioProcessorEditor::newOpenGLContextCreated()
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  io.IniFilename = nullptr; // we will store gui layout manually
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  loadLayout();

  ImGui_ImplJuce_Init(*this, glctx);
  ImGui_ImplOpenGL3_Init();
}

void SineAudioProcessorEditor::renderOpenGL()
{
  using namespace juce::gl;

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplJuce_NewFrame();
  ImGui::NewFrame();

  drawImGui();
  ImGui::Render();

  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  ImGuiIO &io = ImGui::GetIO();
  if (io.WantSaveIniSettings) {
    saveLayout();
    io.WantSaveIniSettings = false;
  }
}

void SineAudioProcessorEditor::openGLContextClosing()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplJuce_Shutdown();
  saveLayout();
  ImGui::DestroyContext();
}

void SineAudioProcessorEditor::drawImGui()
{
  ImGuiID dock = ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

  ImGuiSliderFlags sflags = ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat;

  ImGui::SetNextWindowDockID(dock, ImGuiCond_FirstUseEver);
  ImGui::Begin("parameters", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  {
    // example: DAW parameter
    float freq = ap.freqParam->get();
    if (ImGui::SliderFloat("freq", &freq, 20, 20000, "%.2f", sflags | ImGuiSliderFlags_Logarithmic))
      setAudioParameter(ap.freqParam, freq);

    // example: custom debug parameter
    float gain = ap.gain.load(std::memory_order_relaxed);
    if (ImGui::SliderFloat("gain", &gain, 0, 1, "%.3f", sflags | ImGuiSliderFlags_Logarithmic))
      ap.gain.store(gain, std::memory_order_relaxed);
  }
  ImGui::End();
}
