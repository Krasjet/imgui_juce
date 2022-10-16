/* PluginProcessor.cpp: DSP component of sine */
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

SineAudioProcessor::SineAudioProcessor()
  : AudioProcessor(BusesProperties()
                   .withOutput("Output", juce::AudioChannelSet::stereo()))
  , params{*this, nullptr, "Parameters", createParameterLayout()}
{
  freqParam = dynamic_cast<juce::AudioParameterFloat*>(params.getParameter("freq"));
}

// specify accepted bus layouts
bool SineAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
  return layouts.getMainOutputChannels() > 0; // 1+ output channel
}

// reset state
void SineAudioProcessor::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock)
{
  juce::ignoreUnused(maximumExpectedSamplesPerBlock);
  this->sr = static_cast<float>(sampleRate);
  this->phase = 0;
}

// main processing happens here
void SineAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
  juce::ScopedNoDenormals noDenormals;
  juce::ignoreUnused(midiMessages);

  auto freq = freqParam->get();
  auto nch_out = getTotalNumOutputChannels();

  auto g = gain.load(std::memory_order_relaxed);

  float phs = this->phase;
  for (int ch = 0; ch < nch_out; ++ch) {
    auto *buf = buffer.getWritePointer(ch);
    auto nsamples = buffer.getNumSamples();

    phs = this->phase; // reset to initial phase for new channel
    for (auto i = 0; i < nsamples; ++i) {
      buf[i] = g*std::sin(juce::MathConstants<float>::twoPi*phs);

      phs += freq/sr;
      while (phs >= 1) phs--;
    }
  }
  this->phase = phs; // update phase after done with all channels
}

// connect to GUI
juce::AudioProcessorEditor *SineAudioProcessor::createEditor()
{
  return new SineAudioProcessorEditor(*this);
}

// save and load state (on message thread, so doesn't have to be real-time safe)
void SineAudioProcessor::getStateInformation(juce::MemoryBlock &dest)
{
  auto xml = std::make_unique<juce::XmlElement>("State");

  // audio plugin parameters
  if (auto apxml = params.copyState().createXml())
    xml->prependChildElement(apxml.release());

  // gui layout
  auto guixml = xml->createNewChildElement("ImGui");
  {
    const juce::ScopedLock lk(guiLayoutLock);
    guixml->setAttribute("layout", this->guiLayout);
  }

  // custom parameters
  auto cpxml = xml->createNewChildElement("CustomParams");
  cpxml->setAttribute("gain", gain.load());

  copyXmlToBinary(*xml, dest);
}

void SineAudioProcessor::setStateInformation(const void *data, int size)
{
  auto xml = getXmlFromBinary(data, size);
  if (!xml)
    return;

  // audio plugin parameters
  if (auto e = xml->getChildByName(params.state.getType()))
    params.replaceState(juce::ValueTree::fromXml(*e));

  // gui layout
  if (auto e = xml->getChildByName("ImGui")) {
    if (e->hasAttribute("layout")) {
      juce::String layout = e->getStringAttribute("layout");
      {
        const juce::ScopedLock lk(guiLayoutLock);
        this->guiLayout = layout;
      }
    }
  }

  // custom parameters
  if (auto e = xml->getChildByName("CustomParams")) {
    if (e->hasAttribute("gain"))
      gain = (float)e->getDoubleAttribute("gain");
  }
}
