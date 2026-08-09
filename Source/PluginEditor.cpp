#include "PluginEditor.h"

namespace mididestruct
{

namespace
{
    constexpr int kTransportHeight = 70;
    constexpr int kStatusHeight = 20;
}

MidiDestructAudioProcessorEditor::MidiDestructAudioProcessorEditor (MidiDestructAudioProcessor& processorToEdit)
    : AudioProcessorEditor (&processorToEdit), midiDestructProcessor (processorToEdit)
{
    addAndMakeVisible (transportControls);
    addAndMakeVisible (pianoRollView);
    addAndMakeVisible (statusLabel);

    statusLabel.setJustificationType (juce::Justification::centredLeft);
    statusLabel.setFont (juce::Font (juce::FontOptions (13.0f)));

    transportControls.onLoadClicked = [this]
    {
        activeFileChooser = std::make_unique<juce::FileChooser> ("Load a MIDI file", juce::File(), "*.mid;*.midi");

        activeFileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& chooser)
            {
                const auto file = chooser.getResult();
                if (file.existsAsFile())
                    loadFile (file);
            });
    };

    transportControls.onRandomizeClicked = [this]
    {
        midiDestructProcessor.randomize();
        refreshFromProcessor();
    };

    transportControls.onResetClicked = [this]
    {
        midiDestructProcessor.resetToOriginal();
        refreshFromProcessor();
    };

    transportControls.onExportClicked = [this]
    {
        activeFileChooser = std::make_unique<juce::FileChooser> ("Export randomized MIDI", juce::File(), "*.mid");

        activeFileChooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
            [this] (const juce::FileChooser& chooser)
            {
                const auto file = chooser.getResult();
                if (file == juce::File())
                    return;

                if (midiDestructProcessor.exportMidiFile (file))
                    statusLabel.setText ("Exported " + file.getFileName(), juce::dontSendNotification);
                else
                    showError (midiDestructProcessor.getLastErrorMessage());
            });
    };

    transportControls.onGridChanged = [this] (int divisions)
    {
        midiDestructProcessor.setGridDivisionsPerBeat (divisions);
        refreshFromProcessor();
    };

    setSize (640, 420);
    refreshFromProcessor();
}

void MidiDestructAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MidiDestructAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    transportControls.setBounds (area.removeFromTop (kTransportHeight));
    statusLabel.setBounds (area.removeFromBottom (kStatusHeight).reduced (4, 0));
    pianoRollView.setBounds (area.reduced (4));
}

bool MidiDestructAudioProcessorEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (const auto& path : files)
        if (path.endsWithIgnoreCase (".mid") || path.endsWithIgnoreCase (".midi"))
            return true;

    return false;
}

void MidiDestructAudioProcessorEditor::filesDropped (const juce::StringArray& files, int, int)
{
    for (const auto& path : files)
    {
        const juce::File file (path);

        if (file.hasFileExtension ("mid;midi"))
        {
            loadFile (file);
            break;
        }
    }
}

void MidiDestructAudioProcessorEditor::loadFile (const juce::File& file)
{
    if (midiDestructProcessor.loadMidiFile (file))
    {
        statusLabel.setText ("Loaded " + file.getFileName(), juce::dontSendNotification);
        refreshFromProcessor();
    }
    else
    {
        showError (midiDestructProcessor.getLastErrorMessage());
    }
}

void MidiDestructAudioProcessorEditor::showError (const juce::String& message)
{
    statusLabel.setText (message, juce::dontSendNotification);
}

void MidiDestructAudioProcessorEditor::refreshFromProcessor()
{
    const auto& original = midiDestructProcessor.getOriginalRhythm();
    auto currentSnapshot = midiDestructProcessor.getCurrentPatternSnapshot();

    std::vector<NoteEvent> current = currentSnapshot != nullptr ? *currentSnapshot : std::vector<NoteEvent>();
    const double totalLength = original.isEmpty() ? 16.0 : original.totalLengthBeats;

    pianoRollView.setData (original.notes, std::move (current), totalLength);
}

} // namespace mididestruct
