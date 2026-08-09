#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "Midi/MidiFileIO.h"
#include "Randomize/RhythmRandomizer.h"
#include "Randomize/Rng.h"

namespace mididestruct
{

MidiDestructAudioProcessor::MidiDestructAudioProcessor()
    : AudioProcessor (BusesProperties())
{
    publishPattern ({});
}

void MidiDestructAudioProcessor::prepareToPlay (double, int) {}

void MidiDestructAudioProcessor::processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&)
{
    // Phase 1: no host-synced playback yet - the plugin is a pure MIDI file
    // transform tool for now (Load / Randomize / Reset / Export). Phase 2
    // will read getCurrentPatternSnapshot() here and emit note on/offs
    // synced to the host's PPQ position.
}

juce::AudioProcessorEditor* MidiDestructAudioProcessor::createEditor()
{
    return new MidiDestructAudioProcessorEditor (*this);
}

void MidiDestructAudioProcessor::publishPattern (std::vector<NoteEvent> notes)
{
    std::atomic_store (&currentPattern, std::make_shared<const std::vector<NoteEvent>> (std::move (notes)));
}

void MidiDestructAudioProcessor::regenerateFromParams()
{
    publishPattern (randomizeRhythm (originalRhythm, params));
}

bool MidiDestructAudioProcessor::loadMidiFile (const juce::File& file)
{
    OriginalRhythm loaded;
    juce::String error;

    if (! mididestruct::loadMidiFile (file, loaded, error))
    {
        lastErrorMessage = error;
        return false;
    }

    originalRhythm = std::move (loaded);
    params.seed = makeRandomSeed();
    regenerateFromParams();
    lastErrorMessage.clear();
    return true;
}

bool MidiDestructAudioProcessor::exportMidiFile (const juce::File& file)
{
    auto snapshot = getCurrentPatternSnapshot();

    if (snapshot == nullptr || snapshot->empty())
    {
        lastErrorMessage = "Nothing to export yet - load a MIDI file first.";
        return false;
    }

    juce::String error;

    if (! writeMidiFile (file, originalRhythm, *snapshot, error))
    {
        lastErrorMessage = error;
        return false;
    }

    lastErrorMessage.clear();
    return true;
}

void MidiDestructAudioProcessor::randomize()
{
    if (! hasLoadedRhythm())
        return;

    params.seed = makeRandomSeed();
    regenerateFromParams();
}

void MidiDestructAudioProcessor::resetToOriginal()
{
    publishPattern (originalRhythm.notes);
}

void MidiDestructAudioProcessor::setGridDivisionsPerBeat (int divisions)
{
    params.gridDivisionsPerBeat = juce::jmax (1, divisions);

    if (hasLoadedRhythm())
        regenerateFromParams();
}

void MidiDestructAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto xml = rhythmToXml (originalRhythm, params);
    copyXmlToBinary (*xml, destData);
}

void MidiDestructAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);

    if (xml == nullptr)
        return;

    OriginalRhythm loadedRhythm;
    RandomizeParams loadedParams;

    if (! rhythmFromXml (*xml, loadedRhythm, loadedParams))
        return;

    originalRhythm = std::move (loadedRhythm);
    params = loadedParams;
    regenerateFromParams();
}

} // namespace mididestruct

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new mididestruct::MidiDestructAudioProcessor();
}
