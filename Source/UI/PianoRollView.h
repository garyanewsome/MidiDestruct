#pragma once

#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../Midi/NoteEvent.h"

namespace mididestruct
{

// Draws the original rhythm (dim outlines) behind the current randomized
// pattern (filled bars), so a Randomize/Reset click is visible at a glance.
class PianoRollView : public juce::Component
{
public:
    PianoRollView() = default;

    void setData (std::vector<NoteEvent> original, std::vector<NoteEvent> current, double totalLengthBeats);
    void paint (juce::Graphics& g) override;

private:
    juce::Rectangle<float> noteBounds (const NoteEvent& note, juce::Rectangle<float> area) const;

    std::vector<NoteEvent> originalNotes;
    std::vector<NoteEvent> currentNotes;
    double totalBeats = 1.0;
    int lowestPitch = 48;
    int highestPitch = 72;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PianoRollView)
};

} // namespace mididestruct
