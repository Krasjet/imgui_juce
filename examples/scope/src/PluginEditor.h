/* PluginEditor.h: GUI component of scope */
#pragma once

#include <JuceHeader.h>
#include <implot.h>

// forward declarations
class ScopeAudioProcessor;

class ScopeAudioProcessorEditor :
  public juce::AudioProcessorEditor,
  public juce::OpenGLRenderer
{
public:
  ScopeAudioProcessorEditor(ScopeAudioProcessor &p);

  // regular ui not used
  void paint(juce::Graphics &) override {}
  void resized() override {}

  // imgui needs opengl rendering
  void newOpenGLContextCreated() override;
  void renderOpenGL() override;
  void openGLContextClosing() override;

private:
  ScopeAudioProcessor &ap;
  juce::OpenGLContext glctx;
  ImGuiContext *imgui_ctx;
  ImPlotContext *implot_ctx;

  void saveLayout();
  void loadLayout();

  void drawImGui();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScopeAudioProcessorEditor)
};
