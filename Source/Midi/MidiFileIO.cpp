#include "MidiFileIO.h"

namespace mididestruct
{

bool loadMidiFile (const juce::File& file, OriginalRhythm& outRhythm, juce::String& errorMessage)
{
    juce::FileInputStream inputStream (file);

    if (! inputStream.openedOk())
    {
        errorMessage = "Could not open file: " + file.getFullPathName();
        return false;
    }

    juce::MidiFile midiFile;

    if (! midiFile.readFrom (inputStream))
    {
        errorMessage = "Not a readable MIDI file: " + file.getFullPathName();
        return false;
    }

    const short timeFormat = midiFile.getTimeFormat();

    if (timeFormat <= 0)
    {
        errorMessage = "SMPTE-timecode MIDI files aren't supported yet.";
        return false;
    }

    const double ticksPerQuarterNote = (double) timeFormat;

    double bpm = 120.0;
    juce::MidiMessageSequence tempoEvents;
    midiFile.findAllTempoEvents (tempoEvents);

    if (tempoEvents.getNumEvents() > 0)
    {
        const double secondsPerQuarterNote = tempoEvents.getEventPointer (0)->message.getTempoSecondsPerQuarterNote();

        if (secondsPerQuarterNote > 0.0)
            bpm = 60.0 / secondsPerQuarterNote;
    }

    int timeSigNumerator = 4;
    int timeSigDenominator = 4;
    juce::MidiMessageSequence timeSigEvents;
    midiFile.findAllTimeSigEvents (timeSigEvents);

    if (timeSigEvents.getNumEvents() > 0)
        timeSigEvents.getEventPointer (0)->message.getTimeSignatureInfo (timeSigNumerator, timeSigDenominator);

    juce::MidiMessageSequence merged;

    for (int t = 0; t < midiFile.getNumTracks(); ++t)
        merged.addSequence (*midiFile.getTrack (t), 0.0);

    merged.updateMatchedPairs();

    std::vector<NoteEvent> notes;

    for (int i = 0; i < merged.getNumEvents(); ++i)
    {
        auto* holder = merged.getEventPointer (i);
        const auto& message = holder->message;

        if (! message.isNoteOn())
            continue;

        if (holder->noteOffObject == nullptr)
            continue; // hanging note-on with no matching off; skip rather than guess a duration

        NoteEvent note;
        note.pitch = message.getNoteNumber();
        note.velocity = message.getVelocity();
        note.channel = message.getChannel();
        note.startBeats = message.getTimeStamp() / ticksPerQuarterNote;
        note.durationBeats = (holder->noteOffObject->message.getTimeStamp() - message.getTimeStamp()) / ticksPerQuarterNote;

        if (note.durationBeats > 0.0)
            notes.push_back (note);
    }

    if (notes.empty())
    {
        errorMessage = "No notes found in file: " + file.getFullPathName();
        return false;
    }

    outRhythm = buildOriginalRhythm (std::move (notes), ticksPerQuarterNote, bpm, timeSigNumerator, timeSigDenominator);
    return true;
}

bool writeMidiFile (const juce::File& file, const OriginalRhythm& context, const std::vector<NoteEvent>& notes,
                     juce::String& errorMessage)
{
    if (notes.empty())
    {
        errorMessage = "Nothing to export - the pattern is empty.";
        return false;
    }

    const double ticksPerQuarterNote = context.ticksPerQuarterNote;

    juce::MidiMessageSequence sequence;

    const int microsecondsPerQuarterNote = (int) juce::roundToInt (60'000'000.0 / context.bpm);
    auto tempoEvent = juce::MidiMessage::tempoMetaEvent (microsecondsPerQuarterNote);
    tempoEvent.setTimeStamp (0.0);
    sequence.addEvent (tempoEvent);

    auto timeSigEvent = juce::MidiMessage::timeSignatureMetaEvent (context.timeSigNumerator, context.timeSigDenominator);
    timeSigEvent.setTimeStamp (0.0);
    sequence.addEvent (timeSigEvent);

    for (const auto& note : notes)
    {
        const double startTicks = note.startBeats * ticksPerQuarterNote;
        const double endTicks = (note.startBeats + note.durationBeats) * ticksPerQuarterNote;

        auto noteOn = juce::MidiMessage::noteOn (note.channel, note.pitch, (juce::uint8) juce::jlimit (0, 127, note.velocity));
        noteOn.setTimeStamp (startTicks);
        sequence.addEvent (noteOn);

        auto noteOff = juce::MidiMessage::noteOff (note.channel, note.pitch);
        noteOff.setTimeStamp (endTicks);
        sequence.addEvent (noteOff);
    }

    sequence.updateMatchedPairs();

    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote ((int) juce::roundToInt (ticksPerQuarterNote));
    midiFile.addTrack (sequence);

    if (file.existsAsFile() && ! file.deleteFile())
    {
        errorMessage = "Could not overwrite existing file: " + file.getFullPathName();
        return false;
    }

    juce::FileOutputStream outputStream (file);

    if (! outputStream.openedOk())
    {
        errorMessage = "Could not create file: " + file.getFullPathName();
        return false;
    }

    midiFile.writeTo (outputStream, 0);
    outputStream.flush();
    return true;
}

} // namespace mididestruct
