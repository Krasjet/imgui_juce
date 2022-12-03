/* PluginEditor.h: GUI component of sine */
#pragma once

#include <JuceHeader.h>

// forward declarations
class SineAudioProcessor;

class SineAudioProcessorEditor :
  public juce::AudioProcessorEditor,
  public juce::OpenGLRenderer
{
public:
  SineAudioProcessorEditor(SineAudioProcessor &p);

  // regular ui not used
  void paint(juce::Graphics &) override {}
  void resized() override {}

  // imgui needs opengl rendering
  void newOpenGLContextCreated() override;
  void renderOpenGL() override;
  void openGLContextClosing() override;

private:
  SineAudioProcessor &ap;
  juce::OpenGLContext glctx;
  ImGuiContext *imgui_ctx;

  void saveLayout();
  void loadLayout();

  void drawImGui();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SineAudioProcessorEditor)
};
