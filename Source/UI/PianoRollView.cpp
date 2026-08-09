#include "PianoRollView.h"

#include <algorithm>

namespace mididestruct
{

void PianoRollView::setData (std::vector<NoteEvent> original, std::vector<NoteEvent> current, double totalLengthBeats)
{
    originalNotes = std::move (original);
    currentNotes = std::move (current);
    totalBeats = std::max (totalLengthBeats, 1.0);

    lowestPitch = 127;
    highestPitch = 0;

    for (const auto* list : { &originalNotes, &currentNotes })
    {
        for (const auto& note : *list)
        {
            lowestPitch = std::min (lowestPitch, note.pitch);
            highestPitch = std::max (highestPitch, note.pitch);
        }
    }

    if (lowestPitch > highestPitch)
    {
        lowestPitch = 48;
        highestPitch = 72;
    }

    // A little headroom above/below the note range.
    lowestPitch = std::max (0, lowestPitch - 2);
    highestPitch = std::min (127, highestPitch + 2);

    repaint();
}

juce::Rectangle<float> PianoRollView::noteBounds (const NoteEvent& note, juce::Rectangle<float> area) const
{
    const int pitchRange = std::max (1, highestPitch - lowestPitch);
    const float rowHeight = area.getHeight() / (float) pitchRange;

    const float x = area.getX() + (float) (note.startBeats / totalBeats) * area.getWidth();
    const float w = std::max (2.0f, (float) (note.durationBeats / totalBeats) * area.getWidth());
    const float y = area.getY() + (float) (highestPitch - note.pitch) * rowHeight;

    return { x, y, w, std::max (2.0f, rowHeight * 0.8f) };
}

void PianoRollView::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat().reduced (4.0f);

    g.fillAll (juce::Colours::black.withAlpha (0.85f));

    // Beat grid.
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    const int numBeats = (int) std::ceil (totalBeats);

    for (int b = 0; b <= numBeats; ++b)
    {
        const float x = area.getX() + (float) (b / totalBeats) * area.getWidth();
        g.drawVerticalLine ((int) x, area.getY(), area.getBottom());
    }

    // Original rhythm, dim outlines.
    g.setColour (juce::Colours::white.withAlpha (0.35f));

    for (const auto& note : originalNotes)
        g.drawRect (noteBounds (note, area), 1.0f);

    // Current pattern, filled.
    g.setColour (juce::Colours::limegreen.withAlpha (0.85f));

    for (const auto& note : currentNotes)
        g.fillRoundedRectangle (noteBounds (note, area), 2.0f);

    if (originalNotes.empty() && currentNotes.empty())
    {
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.drawText ("Drop a .mid file here", area, juce::Justification::centred);
    }
}

} // namespace mididestruct
