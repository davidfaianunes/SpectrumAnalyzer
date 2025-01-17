/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomButtonLookAndFeel.h"
#include "AudioVisualizer.h"

//==============================================================================
/**
*/
class SpectrumAnalyzerAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Button::Listener, public juce::Timer, public juce::Slider::Listener
{
public:
    SpectrumAnalyzerAudioProcessorEditor (SpectrumAnalyzerAudioProcessor&);
    ~SpectrumAnalyzerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    void SpectrumAnalyzerAudioProcessorEditor::sliderValueChanged(juce::Slider* slider) override;
    void SpectrumAnalyzerAudioProcessorEditor::timerCallback() override;

private:

    int SpectrumAnalyzerAudioProcessorEditor::getPeakHoldMode();

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SpectrumAnalyzerAudioProcessor& audioProcessor;

    juce::ToggleButton none_peak_button;
    juce::ToggleButton fast_peak_button;
    juce::ToggleButton medium_peak_button;
    juce::ToggleButton slow_peak_button;
    juce::ToggleButton* buttons[4];
    juce::Slider knob;
    juce::Slider lowPassKnob;
    juce::Label fallbackspeed_label;
    juce::Label lowPass_label;
    juce::Label peakhold_label;
    CustomButtonLookAndFeel customButtonLookAndFeel;
    AudioVisualizer* audioVisualizer;
    AudioVisualizer* spectrumVisualizer;
    juce::Path waveformPath;
    juce::Path spectrumPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAnalyzerAudioProcessorEditor)
};
