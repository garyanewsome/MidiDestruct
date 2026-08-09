#pragma once

#include <vector>

#include "../Midi/RhythmData.h"

namespace mididestruct
{

// Pure function: (original, params) -> each note independently re-chopped
// in place into 1+ shorter re-triggered hits (on params.gridDivisionsPerBeat)
// with some segments dropped as rests, so note count can decrease. A note is
// never relocated or extended past its own original span. Deterministic for
// a given params.seed.
std::vector<NoteEvent> randomizeRhythm (const OriginalRhythm& original, const RandomizeParams& params);

} // namespace mididestruct
