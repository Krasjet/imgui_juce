/* PluginProcessor.h: DSP component of scope */
#pragma once

#include <JuceHeader.h>

class ScopeAudioProcessor : public juce::AudioProcessor
{
public:
  ScopeAudioProcessor();

  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

  void prepareToPlay(double, int) override {};
  void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;
  void releaseResources() override {}

  bool hasEditor() const override { return true; }
  juce::AudioProcessorEditor *createEditor() override;

  void getStateInformation(juce::MemoryBlock &dest) override;
  void setStateInformation(const void *data, int size) override;

  const juce::String getName() const override { return JucePlugin_Name; }

  bool acceptsMidi() const override { return false; }
  bool producesMidi() const override { return false; }

  // no trailing output
  double getTailLengthSeconds() const override { return 0; }

  // no programs
  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return {}; }
  void changeProgramName(int, const juce::String &) override {}

private:
  // plot buffer for implot. you can do something fancier with a dynamic size, etc.
  static constexpr size_t plotbuf_size = 48000*2;
  float plotbuf[plotbuf_size] = {0};
  std::atomic<size_t> wp = 0;

  // saved layout for imgui. will be updated by gui thread periodically
  juce::String guiLayout;
  juce::CriticalSection guiLayoutLock;

  // allow editor to access private variables
  friend class ScopeAudioProcessorEditor;

  using AudioProcessor::processBlock; // suppress -Woverloaded-virtual
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScopeAudioProcessor)
};
