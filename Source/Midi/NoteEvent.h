#pragma once

namespace mididestruct
{

// A single MIDI note, with timing expressed in beats (quarter notes) rather
// than ticks, so the randomizer never has to think about a file's PPQ.
struct NoteEvent
{
    int pitch = 60;
    int velocity = 100;
    int channel = 1;

    double startBeats = 0.0;
    double durationBeats = 1.0;
};

} // namespace mididestruct
