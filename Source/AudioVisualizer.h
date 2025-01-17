#pragma once

#include <JuceHeader.h>

class AudioVisualizer : public juce::Component
{
public:
    explicit AudioVisualizer(int _width, int _height)
    {
        width = _width;
        height = _height;
        setSize(width, height);
    }

    // Set the waveform Path to be drawn
    void setWaveformPath(const juce::Path& path)
    {
        waveformPath = path;
        repaint();  // Trigger a repaint whenever the waveform is updated
    }

    // Draw the waveform in the component
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);  // Set background color to black
        g.setColour(juce::Colours::green);  // Set waveform color to green

        g.strokePath(waveformPath, juce::PathStrokeType(2.0f));  // Draw the waveform
    }

private:
    int width;
    int height;
    juce::Path waveformPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioVisualizer)
};
