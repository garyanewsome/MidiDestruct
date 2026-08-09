#include <juce_core/juce_core.h>

#include "../Source/Randomize/RhythmRandomizer.h"

using namespace mididestruct;

namespace
{
    constexpr double kEpsilon = 1.0e-6;

    // A whole-note chord (3 voices) plus two shorter single notes - enough
    // variety to exercise chop's per-note independence and its "too short
    // to chop" fallback.
    OriginalRhythm makeTestRhythm()
    {
        std::vector<NoteEvent> notes;

        NoteEvent n;
        n.pitch = 60; n.startBeats = 0.0; n.durationBeats = 4.0; notes.push_back (n);
        n.pitch = 64; n.startBeats = 0.0; n.durationBeats = 4.0; notes.push_back (n);
        n.pitch = 67; n.startBeats = 0.0; n.durationBeats = 4.0; notes.push_back (n);
        n.pitch = 62; n.startBeats = 4.0; n.durationBeats = 2.0; notes.push_back (n);
        n.pitch = 65; n.startBeats = 6.0; n.durationBeats = 0.1; notes.push_back (n); // too short to chop on a 1/16 grid

        return buildOriginalRhythm (std::move (notes), 960.0, 120.0, 4, 4);
    }

    std::vector<const NoteEvent*> findAllByPitch (const std::vector<NoteEvent>& notes, int pitch)
    {
        std::vector<const NoteEvent*> matches;

        for (const auto& note : notes)
            if (note.pitch == pitch)
                matches.push_back (&note);

        return matches;
    }
}

class RhythmRandomizerTests : public juce::UnitTest
{
public:
    RhythmRandomizerTests() : juce::UnitTest ("RhythmRandomizer", "MidiDestruct") {}

    void runTest() override
    {
        const auto original = makeTestRhythm();

        beginTest ("Determinism: same seed produces identical output");
        {
            RandomizeParams params;
            params.gridDivisionsPerBeat = 4;
            params.seed = 42;

            const auto resultA = randomizeRhythm (original, params);
            const auto resultB = randomizeRhythm (original, params);

            expectEquals ((int) resultA.size(), (int) resultB.size());

            for (size_t i = 0; i < resultA.size(); ++i)
            {
                expectEquals (resultA[i].pitch, resultB[i].pitch);
                expectWithinAbsoluteError (resultA[i].startBeats, resultB[i].startBeats, kEpsilon);
                expectWithinAbsoluteError (resultA[i].durationBeats, resultB[i].durationBeats, kEpsilon);
            }
        }

        beginTest ("Every hit stays within its source note's own span, never overlapping a sibling hit");
        {
            for (uint64_t seed = 0; seed < 50; ++seed)
            {
                RandomizeParams params;
                params.gridDivisionsPerBeat = 4;
                params.seed = seed;

                const auto result = randomizeRhythm (original, params);

                for (const auto& note : original.notes)
                {
                    const auto hits = findAllByPitch (result, note.pitch);
                    double previousEnd = note.startBeats;

                    for (const auto* hit : hits)
                    {
                        expect (hit->startBeats >= previousEnd - kEpsilon);
                        expect (hit->startBeats >= note.startBeats - kEpsilon);
                        expect (hit->startBeats + hit->durationBeats <= note.startBeats + note.durationBeats + kEpsilon);
                        previousEnd = hit->startBeats + hit->durationBeats;
                    }
                }
            }
        }

        beginTest ("Only pitches from the original note list ever appear in the output");
        {
            RandomizeParams params;
            params.gridDivisionsPerBeat = 4;
            params.seed = 5;

            const auto result = randomizeRhythm (original, params);

            for (const auto& hit : result)
            {
                const bool pitchExistsInOriginal = ! findAllByPitch (original.notes, hit.pitch).empty();
                expect (pitchExistsInOriginal);
            }
        }

        beginTest ("A note too short for the grid is left untouched");
        {
            RandomizeParams params;
            params.gridDivisionsPerBeat = 4; // 1/16 = 0.25 beats; the test note is only 0.1 beats long

            for (uint64_t seed = 0; seed < 20; ++seed)
            {
                params.seed = seed;
                const auto result = randomizeRhythm (original, params);
                const auto hits = findAllByPitch (result, 65);

                expectEquals ((int) hits.size(), 1);

                if (hits.size() == 1)
                {
                    expectWithinAbsoluteError (hits[0]->startBeats, 6.0, kEpsilon);
                    expectWithinAbsoluteError (hits[0]->durationBeats, 0.1, kEpsilon);
                }
            }
        }

        beginTest ("Across enough seeds, a chopped note both subdivides and fully rests at least once");
        {
            // Tracked per-note (not as a total note count) because other
            // notes subdividing in the same trial can offset one note's
            // rests, masking the effect if only the overall total is checked.
            bool sawMultipleHitsForPitch60 = false;
            bool sawZeroHitsForPitch60 = false;

            for (uint64_t seed = 0; seed < 200; ++seed)
            {
                RandomizeParams params;
                params.gridDivisionsPerBeat = 4;
                params.seed = seed;

                const auto result = randomizeRhythm (original, params);
                const auto hitCount = findAllByPitch (result, 60).size();

                if (hitCount > 1)
                    sawMultipleHitsForPitch60 = true;

                if (hitCount == 0)
                    sawZeroHitsForPitch60 = true;
            }

            expect (sawMultipleHitsForPitch60);
            expect (sawZeroHitsForPitch60);
        }
    }
};

static RhythmRandomizerTests rhythmRandomizerTestsInstance;
