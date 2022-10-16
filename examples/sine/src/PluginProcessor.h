/* PluginProcessor.h: DSP component of sine */
#pragma once

#include <JuceHeader.h>

class SineAudioProcessor : public juce::AudioProcessor
{
public:
  SineAudioProcessor();

  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

  void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
  void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;
  void releaseResources() override {}

  bool hasEditor() const override { return true; }
  juce::AudioProcessorEditor *createEditor() override;

  void getStateInformation(juce::MemoryBlock &dest) override;
  void setStateInformation(const void *data, int size) override;

  const juce::String getName() const override { return JucePlugin_Name; }

  // no midi
  bool acceptsMidi() const override { return false; }
  bool producesMidi() const override { return false; }

  // no trailing output
  double getTailLengthSeconds() const override { return 0; }

  // no programs
  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return false; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return {}; }
  void changeProgramName(int, const juce::String &) override {}

private:
  juce::AudioProcessorValueTreeState params;

  // add new DAW-aware (automatable) audio plugin parameters here
  juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
  {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("freq", "Frequency",
          juce::NormalisableRange<float>(0, 20000, 1, 0.4f), 440));

    return layout;
  }
  juce::AudioParameterFloat *freqParam;

  // add custom parameters here (need to be thread-safe)
  // these will not be automatable in DAW, but controllable in gui
  std::atomic<float> gain = 0.5;

  // saved layout for imgui. will be updated by gui thread periodically
  juce::String guiLayout;
  juce::CriticalSection guiLayoutLock;

  // allow editor to access private variables
  friend class SineAudioProcessorEditor;

  // state variable for sine oscillator
  float phase;
  float sr;

  using AudioProcessor::processBlock; // suppress -Woverloaded-virtual
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SineAudioProcessor)
};
