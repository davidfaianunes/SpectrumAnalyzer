/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomButtonLookAndFeel.h"

//==============================================================================
/**
*/
class SpectrumAnalyzerAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Button::Listener
{
public:
    SpectrumAnalyzerAudioProcessorEditor (SpectrumAnalyzerAudioProcessor&);
    ~SpectrumAnalyzerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    juce::ToggleButton none_peak_button;
    juce::ToggleButton fast_peak_button;
    juce::ToggleButton medium_peak_button;
    juce::ToggleButton slow_peak_button;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SpectrumAnalyzerAudioProcessor& audioProcessor;

    juce::Slider knob;
    juce::Label fallbackspeed_label;
    juce::Label peakhold_label;
    CustomButtonLookAndFeel customButtonLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAnalyzerAudioProcessorEditor)
};
