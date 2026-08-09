#include <juce_core/juce_core.h>

#include "../Source/Midi/RhythmData.h"

using namespace mididestruct;

class RhythmDataTests : public juce::UnitTest
{
public:
    RhythmDataTests() : juce::UnitTest ("RhythmData", "MidiDestruct") {}

    void runTest() override
    {
        beginTest ("Notes are sorted by onset regardless of input order");
        {
            std::vector<NoteEvent> notes;
            NoteEvent a; a.pitch = 67; a.startBeats = 2.0; a.durationBeats = 1.0; notes.push_back (a);
            NoteEvent b; b.pitch = 60; b.startBeats = 0.0; b.durationBeats = 1.0; notes.push_back (b);
            NoteEvent c; c.pitch = 64; c.startBeats = 1.0; c.durationBeats = 1.0; notes.push_back (c);

            const auto rhythm = buildOriginalRhythm (std::move (notes), 960.0, 120.0, 4, 4);

            expectEquals ((int) rhythm.notes.size(), 3);
            expectEquals (rhythm.notes[0].pitch, 60);
            expectEquals (rhythm.notes[1].pitch, 64);
            expectEquals (rhythm.notes[2].pitch, 67);
        }

        beginTest ("Pattern span runs from the first onset to the last note-off");
        {
            std::vector<NoteEvent> notes;
            NoteEvent a; a.pitch = 60; a.startBeats = 2.0; a.durationBeats = 1.0; notes.push_back (a); // ends at 3.0
            NoteEvent b; b.pitch = 64; b.startBeats = 4.0; b.durationBeats = 3.0; notes.push_back (b); // ends at 7.0, the latest

            const auto rhythm = buildOriginalRhythm (std::move (notes), 960.0, 120.0, 4, 4);

            expectWithinAbsoluteError (rhythm.patternStartBeats, 2.0, 1.0e-9);
            expectWithinAbsoluteError (rhythm.totalLengthBeats, 5.0, 1.0e-9); // 7.0 - 2.0
        }

        beginTest ("An empty note list produces an empty rhythm");
        {
            const auto rhythm = buildOriginalRhythm ({}, 960.0, 120.0, 4, 4);
            expect (rhythm.isEmpty());
        }
    }
};

static RhythmDataTests rhythmDataTestsInstance;
