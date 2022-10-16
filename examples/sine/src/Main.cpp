/* Main.cpp: audio plugin interface needed by JUCE */
#include "PluginProcessor.h"

juce::AudioProcessor * JUCE_CALLTYPE createPluginFilter()
{
  return new SineAudioProcessor();
}
