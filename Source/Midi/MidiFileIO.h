#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "RhythmData.h"

namespace mididestruct
{

// Parses a .mid file into an OriginalRhythm (beats-based, groupId assigned).
// Returns false and fills errorMessage on failure (e.g. SMPTE-based files,
// which aren't supported in v1).
bool loadMidiFile (const juce::File& file, OriginalRhythm& outRhythm, juce::String& errorMessage);

// Writes `notes` out as a single-track .mid file, using the tempo/time-sig/
// PPQ context captured in `context` (normally the OriginalRhythm that the
// notes were derived from) so the exported file matches the source's feel.
bool writeMidiFile (const juce::File& file, const OriginalRhythm& context, const std::vector<NoteEvent>& notes,
                     juce::String& errorMessage);

} // namespace mididestruct
