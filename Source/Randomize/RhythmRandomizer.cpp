#include "RhythmRandomizer.h"
#include "Rng.h"

#include <algorithm>
#include <cmath>

namespace mididestruct
{

namespace
{
    // Chopping is a per-note, in-place transformation: it never relocates a
    // note, so it's automatically confined to wherever the note already was.
    constexpr double kChopRestProbability = 0.3;
    constexpr int kMaxChopSegmentsPerNote = 4;
    constexpr double kChopArticulationRatio = 0.85;

    std::vector<NoteEvent> chopRhythm (const OriginalRhythm& original, int gridDivisionsPerBeat, Rng& rng)
    {
        const double gridStep = 1.0 / std::max (1, gridDivisionsPerBeat);
        std::vector<NoteEvent> result;

        for (const auto& note : original.notes)
        {
            const int numCells = std::max (1, (int) std::lround (note.durationBeats / gridStep));
            const int maxSegments = std::min (numCells, kMaxChopSegmentsPerNote);

            if (maxSegments <= 1)
            {
                result.push_back (note); // too short to chop meaningfully - leave it exactly as-is
                continue;
            }

            const int numSegments = 1 + rng.nextInt (maxSegments);

            if (numSegments == 1)
            {
                result.push_back (note); // this cycle's roll came up "don't chop this one"
                continue;
            }

            // Stars-and-bars: pick (numSegments - 1) distinct internal grid
            // boundaries out of (numCells - 1) available ones, so the chosen
            // segments exactly tile the note's original span with no gaps
            // introduced by the cut points themselves.
            const auto cuts = rng.pickDistinctSorted (numCells - 1, numSegments - 1);

            std::vector<int> boundaries;
            boundaries.push_back (0);
            for (int cut : cuts)
                boundaries.push_back (cut + 1);
            boundaries.push_back (numCells);

            for (size_t seg = 0; seg + 1 < boundaries.size(); ++seg)
            {
                if (rng.nextDouble() < kChopRestProbability)
                    continue; // this segment becomes a rest - just emit nothing

                // Shaved slightly short of its full slot: two adjacent hits
                // of the same pitch with zero gap between them render as one
                // unbroken block in most DAW piano rolls, which would look
                // (and can sound, on some synths) like nothing was chopped
                // at all. This keeps every re-trigger visibly/audibly distinct.
                NoteEvent hit = note;
                hit.startBeats = note.startBeats + boundaries[seg] * gridStep;
                const double slotLength = (boundaries[seg + 1] - boundaries[seg]) * gridStep;
                hit.durationBeats = std::max (slotLength * kChopArticulationRatio, 1.0e-6);
                result.push_back (hit);
            }
        }

        std::sort (result.begin(), result.end(), [] (const NoteEvent& a, const NoteEvent& b)
        {
            return a.startBeats < b.startBeats;
        });

        return result;
    }
}

std::vector<NoteEvent> randomizeRhythm (const OriginalRhythm& original, const RandomizeParams& params)
{
    if (original.isEmpty())
        return {};

    Rng rng (params.seed);
    return chopRhythm (original, params.gridDivisionsPerBeat, rng);
}

} // namespace mididestruct
