/* PluginProcessor.cpp: DSP component of scope */
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

ScopeAudioProcessor::ScopeAudioProcessor()
  : AudioProcessor(BusesProperties()
                   .withInput("Input", juce::AudioChannelSet::mono())
                   .withOutput("Output", juce::AudioChannelSet::mono()))
{}

// specify accepted bus layouts
bool ScopeAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
  return layouts.getMainOutputChannels() == 1 &&
         layouts.getMainInputChannels()  == 1; // mono input and output
}

// main processing happens here
void ScopeAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
  juce::ScopedNoDenormals noDenormals;
  juce::ignoreUnused(midiMessages);

  const float *buf_in = getBusBuffer(buffer, true, 0).getReadPointer(0);
  float *buf_out = getBusBuffer(buffer, false, 0).getWritePointer(0);

  auto nsamples = buffer.getNumSamples();
  for (auto i = 0; i < nsamples; ++i) {
    // write to plot buffer
    plotbuf[wp] = buf_in[i];
    wp = (wp + 1) % plotbuf_size;
    // bypass input to output
    buf_out[i] = buf_in[i];
  }
}

// connect to GUI
juce::AudioProcessorEditor *ScopeAudioProcessor::createEditor()
{
  return new ScopeAudioProcessorEditor(*this);
}

// save and load state (on message thread, so doesn't have to be real-time safe)
void ScopeAudioProcessor::getStateInformation(juce::MemoryBlock &dest)
{
  auto xml = std::make_unique<juce::XmlElement>("State");

  // gui layout
  auto guixml = xml->createNewChildElement("ImGui");
  {
    const juce::ScopedLock lk(guiLayoutLock);
    guixml->setAttribute("layout", this->guiLayout);
  }

  copyXmlToBinary(*xml, dest);
}

void ScopeAudioProcessor::setStateInformation(const void *data, int size)
{
  auto xml = getXmlFromBinary(data, size);
  if (!xml)
    return;

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
}
