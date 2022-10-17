/* Main.cpp: main file for application */
#include <JuceHeader.h>
#include "ImGuiComponent.h"

class MainWindow : public juce::DocumentWindow
{
public:
  explicit MainWindow(const juce::String &name)
    : juce::DocumentWindow(name, juce::Colours::black, allButtons)
  {
    setContentOwned(new ImGuiComponent(), true);

    centreWithSize(getWidth(), getHeight());
    setVisible(true);
  }

  void closeButtonPressed() override
  {
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
  }

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

class HiDpiApp : public juce::JUCEApplication
{
public:
  const juce::String getApplicationName() override { return JUCE_APPLICATION_NAME_STRING; }
  const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }

  void initialise(const juce::String &commandLine) override
  {
    juce::ignoreUnused (commandLine);
    win.reset(new MainWindow(getApplicationName()));
  }
  void shutdown() override { win = nullptr; }

private:
  std::unique_ptr<MainWindow> win;
};

START_JUCE_APPLICATION(HiDpiApp)
