/* PluginEditor.cpp: GUI component of scope */
#include <JuceHeader.h>
#include <imgui.h>
#include <implot.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_juce/imgui_impl_juce.h>

#include "PluginEditor.h"
#include "PluginProcessor.h"

ScopeAudioProcessorEditor::ScopeAudioProcessorEditor(ScopeAudioProcessor &p)
  : AudioProcessorEditor(&p), ap(p)
{
  setOpaque(true);
  setWantsKeyboardFocus(true);
  setResizable(true, false);

  // set up opengl context
  glctx.setOpenGLVersionRequired(juce::OpenGLContext::openGL3_2);
  glctx.setRenderer(this);
  glctx.attachTo(*this);
  glctx.setContinuousRepainting(true);

  setSize(768, 512);
}

void ScopeAudioProcessorEditor::saveLayout()
{
  const char *layout = ImGui::SaveIniSettingsToMemory();
  {
    const juce::ScopedLock lk(ap.guiLayoutLock);
    ap.guiLayout = layout;
  }
}

void ScopeAudioProcessorEditor::loadLayout()
{
  juce::String layout;
  {
    const juce::ScopedLock lk(ap.guiLayoutLock);
    layout = ap.guiLayout;
  }
  if (layout.length() > 0)
    ImGui::LoadIniSettingsFromMemory(layout.toRawUTF8(), layout.getNumBytesAsUTF8());
}

void ScopeAudioProcessorEditor::newOpenGLContextCreated()
{
  IMGUI_CHECKVERSION();
  imgui_ctx = ImGui::CreateContext();
  ImGui::SetCurrentContext(imgui_ctx);
  implot_ctx = ImPlot::CreateContext();
  ImPlot::SetCurrentContext(implot_ctx);

  ImGuiIO &io = ImGui::GetIO();
  io.IniFilename = nullptr; // we will store gui layout manually
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  loadLayout();

  ImGui_ImplJuce_Init(*this, glctx);
  ImGui_ImplOpenGL3_Init();
}

void ScopeAudioProcessorEditor::renderOpenGL()
{
  using namespace juce::gl;

  ImGui::SetCurrentContext(imgui_ctx);
  ImPlot::SetCurrentContext(implot_ctx);

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

void ScopeAudioProcessorEditor::openGLContextClosing()
{
  ImGui::SetCurrentContext(imgui_ctx);
  ImPlot::SetCurrentContext(implot_ctx);
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplJuce_Shutdown();
  saveLayout();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();
}

void ScopeAudioProcessorEditor::drawImGui()
{
  ImGuiID dock = ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

  ImGui::SetNextWindowDockID(dock, ImGuiCond_FirstUseEver);
  ImGui::Begin("scope", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  {
    if (ImPlot::BeginPlot("##scope", ImVec2(ImGui::GetWindowContentRegionWidth(), 150))) {
      ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels, 0);
      ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, (double)(ap.plotbuf_size-1), ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y1, -1.0, 1.0, ImGuiCond_Once);
      // technically not thread-safe, since there is no guarantee plotbuf won't
      // change after wp is read, but the visual glitch should be very rare.
      // if you want a more robust solution, use a separate buffer on gui thread
      // for plotting and pass data from audio thread using a thread-safe
      // ringbuffer (see oso).
      ImPlot::PlotLine("input", ap.plotbuf, (int)ap.plotbuf_size, 1.0, 0.0, 0, (int)ap.wp);
      ImPlot::EndPlot();
    }
  }
  ImGui::End();
}
