#pragma once

#include <atomic>
#include <memory>

#include <juce_audio_processors/juce_audio_processors.h>

#include "Midi/RhythmData.h"

namespace mididestruct
{

class MidiDestructAudioProcessor : public juce::AudioProcessor
{
public:
    MidiDestructAudioProcessor();
    ~MidiDestructAudioProcessor() override = default;

    // --- AudioProcessor ------------------------------------------------
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override { return true; }
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "MidiDestruct"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // --- MidiDestruct-specific API, used by the editor ------------------

    // Loads a .mid file as the new reference rhythm and immediately
    // (re-)randomizes it with a fresh seed. Returns false on failure; call
    // getLastErrorMessage() for details.
    bool loadMidiFile (const juce::File& file);

    // Writes the current pattern out as a .mid file.
    bool exportMidiFile (const juce::File& file);

    // Draws a new seed and regenerates the pattern with the current mode.
    void randomize();

    // Reloads the untouched original rhythm (identity transform).
    void resetToOriginal();

    void setGridDivisionsPerBeat (int divisions);

    bool hasLoadedRhythm() const { return ! originalRhythm.isEmpty(); }
    const OriginalRhythm& getOriginalRhythm() const { return originalRhythm; }
    RandomizeParams getParams() const { return params; }
    juce::String getLastErrorMessage() const { return lastErrorMessage; }

    // Thread-safe snapshot of the pattern currently being played/exported.
    std::shared_ptr<const std::vector<NoteEvent>> getCurrentPatternSnapshot() const
    {
        return std::atomic_load (&currentPattern);
    }

private:
    void regenerateFromParams();
    void publishPattern (std::vector<NoteEvent> notes);

    OriginalRhythm originalRhythm;
    RandomizeParams params;
    juce::String lastErrorMessage;

    // Read from the audio thread (Phase 2) via getCurrentPatternSnapshot();
    // written from the message thread. Atomic shared_ptr swap keeps the
    // audio thread lock-free.
    std::shared_ptr<const std::vector<NoteEvent>> currentPattern;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiDestructAudioProcessor)
};

} // namespace mididestruct
