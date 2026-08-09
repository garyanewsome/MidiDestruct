#include "RhythmData.h"

#include <algorithm>

namespace mididestruct
{

OriginalRhythm buildOriginalRhythm (std::vector<NoteEvent> notes,
                                     double ticksPerQuarterNote,
                                     double bpm,
                                     int timeSigNumerator,
                                     int timeSigDenominator)
{
    OriginalRhythm rhythm;
    rhythm.ticksPerQuarterNote = ticksPerQuarterNote;
    rhythm.bpm = bpm;
    rhythm.timeSigNumerator = timeSigNumerator;
    rhythm.timeSigDenominator = timeSigDenominator;

    if (notes.empty())
    {
        rhythm.notes = std::move (notes);
        return rhythm;
    }

    std::sort (notes.begin(), notes.end(), [] (const NoteEvent& a, const NoteEvent& b)
    {
        return a.startBeats < b.startBeats;
    });

    double patternEnd = notes[0].startBeats + notes[0].durationBeats;

    for (size_t i = 1; i < notes.size(); ++i)
        patternEnd = std::max (patternEnd, notes[i].startBeats + notes[i].durationBeats);

    rhythm.patternStartBeats = notes[0].startBeats;
    rhythm.totalLengthBeats = patternEnd - rhythm.patternStartBeats;
    rhythm.notes = std::move (notes);

    return rhythm;
}

std::unique_ptr<juce::XmlElement> rhythmToXml (const OriginalRhythm& rhythm, const RandomizeParams& params)
{
    auto root = std::make_unique<juce::XmlElement> ("MidiDestructState");

    auto* paramsXml = root->createNewChildElement ("Params");
    paramsXml->setAttribute ("gridDivisionsPerBeat", params.gridDivisionsPerBeat);
    paramsXml->setAttribute ("seed", juce::String ((juce::int64) params.seed));

    auto* rhythmXml = root->createNewChildElement ("OriginalRhythm");
    rhythmXml->setAttribute ("ticksPerQuarterNote", rhythm.ticksPerQuarterNote);
    rhythmXml->setAttribute ("bpm", rhythm.bpm);
    rhythmXml->setAttribute ("timeSigNumerator", rhythm.timeSigNumerator);
    rhythmXml->setAttribute ("timeSigDenominator", rhythm.timeSigDenominator);
    rhythmXml->setAttribute ("patternStartBeats", rhythm.patternStartBeats);
    rhythmXml->setAttribute ("totalLengthBeats", rhythm.totalLengthBeats);

    for (const auto& note : rhythm.notes)
    {
        auto* noteXml = rhythmXml->createNewChildElement ("Note");
        noteXml->setAttribute ("pitch", note.pitch);
        noteXml->setAttribute ("velocity", note.velocity);
        noteXml->setAttribute ("channel", note.channel);
        noteXml->setAttribute ("startBeats", note.startBeats);
        noteXml->setAttribute ("durationBeats", note.durationBeats);
    }

    return root;
}

bool rhythmFromXml (const juce::XmlElement& xml, OriginalRhythm& outRhythm, RandomizeParams& outParams)
{
    if (! xml.hasTagName ("MidiDestructState"))
        return false;

    auto* paramsXml = xml.getChildByName ("Params");
    auto* rhythmXml = xml.getChildByName ("OriginalRhythm");

    if (paramsXml == nullptr || rhythmXml == nullptr)
        return false;

    outParams.gridDivisionsPerBeat = paramsXml->getIntAttribute ("gridDivisionsPerBeat", 4);
    outParams.seed = (uint64_t) paramsXml->getStringAttribute ("seed").getLargeIntValue();

    OriginalRhythm rhythm;
    rhythm.ticksPerQuarterNote = rhythmXml->getDoubleAttribute ("ticksPerQuarterNote", 960.0);
    rhythm.bpm = rhythmXml->getDoubleAttribute ("bpm", 120.0);
    rhythm.timeSigNumerator = rhythmXml->getIntAttribute ("timeSigNumerator", 4);
    rhythm.timeSigDenominator = rhythmXml->getIntAttribute ("timeSigDenominator", 4);
    rhythm.patternStartBeats = rhythmXml->getDoubleAttribute ("patternStartBeats", 0.0);
    rhythm.totalLengthBeats = rhythmXml->getDoubleAttribute ("totalLengthBeats", 0.0);

    for (auto* noteXml : rhythmXml->getChildWithTagNameIterator ("Note"))
    {
        NoteEvent note;
        note.pitch = noteXml->getIntAttribute ("pitch", 60);
        note.velocity = noteXml->getIntAttribute ("velocity", 100);
        note.channel = noteXml->getIntAttribute ("channel", 1);
        note.startBeats = noteXml->getDoubleAttribute ("startBeats", 0.0);
        note.durationBeats = noteXml->getDoubleAttribute ("durationBeats", 1.0);
        rhythm.notes.push_back (note);
    }

    outRhythm = std::move (rhythm);
    return true;
}

} // namespace mididestruct
