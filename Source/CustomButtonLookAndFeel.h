#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    CustomButtonLookAndFeel:
    A class for customizing the appearance of ToggleButtons. This LookAndFeel
    implementation renders the button as a rounded rectangle with the text
    centered inside it.
*/
class CustomButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomButtonLookAndFeel() = default;
    ~CustomButtonLookAndFeel() override = default;

    /**
        Customizes the appearance of ToggleButtons.

        @param g The graphics context used for drawing.
        @param toggleButton The toggle button to draw.
        @param shouldDrawButtonAsHighlighted Whether the button is highlighted (hovered).
        @param shouldDrawButtonAsDown Whether the button is pressed.
    */
    void drawToggleButton(juce::Graphics& g,
        juce::ToggleButton& toggleButton,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override
    {
        // Get button bounds
        auto bounds = toggleButton.getLocalBounds().toFloat();

        // Determine the button's color based on its state
        auto baseColor = toggleButton.getToggleState()
            ? juce::Colours::lightblue // Color when toggled on
            : juce::Colours::grey;    // Color when toggled off

        if (shouldDrawButtonAsDown)
            baseColor = baseColor.darker(0.2f); // Darken on mouse down
        else if (shouldDrawButtonAsHighlighted)
            baseColor = baseColor.brighter(0.2f); // Brighten on hover

        // Fill the button rectangle
        g.setColour(baseColor);
        g.fillRoundedRectangle(bounds, 5.0f); // Rounded corners with radius 5.0

        // Draw the text
        g.setColour(juce::Colours::white);
        g.drawText(toggleButton.getButtonText(),
            bounds,
            juce::Justification::centred,
            true); // Use ellipsis if text is too long
    }
};
