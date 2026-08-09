#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "PluginProcessor.h"
#include "UI/PianoRollView.h"
#include "UI/TransportControls.h"

namespace mididestruct
{

class MidiDestructAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::FileDragAndDropTarget
{
public:
    explicit MidiDestructAudioProcessorEditor (MidiDestructAudioProcessor& processorToEdit);

    void paint (juce::Graphics& g) override;
    void resized() override;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

private:
    void refreshFromProcessor();
    void loadFile (const juce::File& file);
    void showError (const juce::String& message);

    MidiDestructAudioProcessor& midiDestructProcessor;

    TransportControls transportControls;
    PianoRollView pianoRollView;
    juce::Label statusLabel;

    std::unique_ptr<juce::FileChooser> activeFileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiDestructAudioProcessorEditor)
};

} // namespace mididestruct
