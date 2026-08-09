#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <juce_core/juce_core.h>

#include "NoteEvent.h"

namespace mididestruct
{

// The note list exactly as parsed from a dropped file, plus enough context
// to write a faithful MIDI file back out. Never mutated after loading -
// this is the "reference" that Reset always reproduces.
struct OriginalRhythm
{
    std::vector<NoteEvent> notes;

    double ticksPerQuarterNote = 960.0;
    double bpm = 120.0;
    int timeSigNumerator = 4;
    int timeSigDenominator = 4;

    // Anchor/span of the pattern, in beats. patternStartBeats is the first
    // note's onset; totalLengthBeats runs from there to the last note-off.
    double patternStartBeats = 0.0;
    double totalLengthBeats = 0.0;

    bool isEmpty() const { return notes.empty(); }
};

struct RandomizeParams
{
    // Chop's subdivision grid. 4 == 1/16 notes, 2 == 1/8, etc.
    int gridDivisionsPerBeat = 4;

    uint64_t seed = 1;
};

// Sorts notes by onset and computes patternStartBeats/totalLengthBeats. Call
// this once right after parsing a file, before storing the result as an
// OriginalRhythm.
OriginalRhythm buildOriginalRhythm (std::vector<NoteEvent> notes,
                                    double ticksPerQuarterNote,
                                    double bpm,
                                    int timeSigNumerator,
                                    int timeSigDenominator);

// Serialization for plugin state (getStateInformation/setStateInformation).
std::unique_ptr<juce::XmlElement> rhythmToXml (const OriginalRhythm& rhythm,
                                                const RandomizeParams& params);

bool rhythmFromXml (const juce::XmlElement& xml,
                     OriginalRhythm& outRhythm,
                     RandomizeParams& outParams);

} // namespace mididestruct
