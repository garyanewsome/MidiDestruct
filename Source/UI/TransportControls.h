#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

namespace mididestruct
{

// Load / Randomize / Reset / Export buttons plus the chop grid-resolution
// selector. Pure UI - it owns no rhythm data and just reports user intent
// via callbacks.
class TransportControls : public juce::Component
{
public:
    TransportControls();

    void resized() override;

    std::function<void()> onLoadClicked;
    std::function<void()> onRandomizeClicked;
    std::function<void()> onResetClicked;
    std::function<void()> onExportClicked;
    std::function<void (int)> onGridChanged;

private:
    juce::TextButton loadButton { "Load..." };
    juce::TextButton randomizeButton { "Randomize" };
    juce::TextButton resetButton { "Reset" };
    juce::TextButton exportButton { "Export..." };

    juce::Label gridLabel { {}, "Grid" };
    juce::ComboBox gridCombo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransportControls)
};

} // namespace mididestruct
