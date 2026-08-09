#include "TransportControls.h"

namespace mididestruct
{

TransportControls::TransportControls()
{
    addAndMakeVisible (loadButton);
    addAndMakeVisible (randomizeButton);
    addAndMakeVisible (resetButton);
    addAndMakeVisible (exportButton);

    loadButton.onClick = [this] { if (onLoadClicked) onLoadClicked(); };
    randomizeButton.onClick = [this] { if (onRandomizeClicked) onRandomizeClicked(); };
    resetButton.onClick = [this] { if (onResetClicked) onResetClicked(); };
    exportButton.onClick = [this] { if (onExportClicked) onExportClicked(); };

    addAndMakeVisible (gridLabel);
    addAndMakeVisible (gridCombo);
    gridCombo.addItem ("1/4", 1);
    gridCombo.addItem ("1/8", 2);
    gridCombo.addItem ("1/16", 4);
    gridCombo.addItem ("1/32", 8);
    gridCombo.setSelectedId (4, juce::dontSendNotification);
    gridCombo.onChange = [this]
    {
        if (onGridChanged) onGridChanged (gridCombo.getSelectedId());
    };
}

void TransportControls::resized()
{
    auto area = getLocalBounds().reduced (4);
    const int rowHeight = 28;
    const int gap = 6;

    auto buttonRow = area.removeFromTop (rowHeight);
    const int buttonWidth = buttonRow.getWidth() / 4;
    loadButton.setBounds (buttonRow.removeFromLeft (buttonWidth).reduced (gap / 2, 0));
    randomizeButton.setBounds (buttonRow.removeFromLeft (buttonWidth).reduced (gap / 2, 0));
    resetButton.setBounds (buttonRow.removeFromLeft (buttonWidth).reduced (gap / 2, 0));
    exportButton.setBounds (buttonRow.removeFromLeft (buttonWidth).reduced (gap / 2, 0));

    area.removeFromTop (gap);

    auto gridRow = area.removeFromTop (rowHeight);
    gridLabel.setBounds (gridRow.removeFromLeft (gridRow.getWidth() / 4));
    gridCombo.setBounds (gridRow.removeFromLeft (gridRow.getWidth() / 3));
}

} // namespace mididestruct
