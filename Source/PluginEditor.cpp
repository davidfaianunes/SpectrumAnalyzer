/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#define PADDING 8 //absolute no pixels
#define ITEM_SIZE 100 //absolute no pixels
#define BUTTON_HEIGHT 20 //absolute no pixels
#define BUTTON_WIDTH 60 //absolute no pixels
#define VISUALIZER_WIDTH 800 //absolute no pixels
#define VISUALIZER_HEIGHT 150 //absolute no pixels
#define VISUAL_FRAMERATE 30 //in hertz
#define SPECTRUM_HEIGHT 250 //absolute no pixels
#define SPECTRUM_WIDTH 200 //absolute no pixels

//==============================================================================
SpectrumAnalyzerAudioProcessorEditor::SpectrumAnalyzerAudioProcessorEditor (SpectrumAnalyzerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    // Enable resizing
    setResizable(true, true);

    // Set minimum and maximum size (minWidth, minHeight, maxWidth, maxHeight)
    setResizeLimits(600, 400, 1000, 800);

    // Configure the knob's properties
    knob.setSliderStyle(juce::Slider::Rotary);
    knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    knob.setRange(0.1, 10, 0.01); // Min, Max, Step size
    knob.setValue(0.5); // Default value


    lowPassKnob.setSliderStyle(juce::Slider::Rotary);
    lowPassKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    lowPassKnob.setRange(20, 20000, 0.01); // Min, Max, Step size
    lowPassKnob.setValue(20000); // Default value

    fallbackspeed_label.setText("Fall-Back Speed", juce::NotificationType::dontSendNotification);
    peakhold_label.setText("Peak Hold", juce::NotificationType::dontSendNotification);
    lowPass_label.setText("Low Pass Filter", juce::NotificationType::dontSendNotification);


    none_peak_button.setButtonText("None");
    fast_peak_button.setButtonText("Fast");
    medium_peak_button.setButtonText("Medium");
    slow_peak_button.setButtonText("Slow");
    buttons[0] = &none_peak_button;
    buttons[1] = &fast_peak_button;
    buttons[2] = &medium_peak_button;
    buttons[3] = &slow_peak_button;

    for (int i = 0; i < 4; ++i)
    {
        buttons[i]->addListener(this);  // Add this editor as a listener to each button
    }

    lowPassKnob.addListener(this);

    none_peak_button.setLookAndFeel(&customButtonLookAndFeel);
    fast_peak_button.setLookAndFeel(&customButtonLookAndFeel);
    medium_peak_button.setLookAndFeel(&customButtonLookAndFeel);
    slow_peak_button.setLookAndFeel(&customButtonLookAndFeel);

    audioVisualizer = new AudioVisualizer(VISUALIZER_WIDTH, VISUALIZER_HEIGHT);
    spectrumVisualizer = new AudioVisualizer(SPECTRUM_WIDTH, SPECTRUM_HEIGHT);
    startTimerHz(VISUAL_FRAMERATE); //for visualizer updates



    addAndMakeVisible(knob);
    addAndMakeVisible(fallbackspeed_label);
    addAndMakeVisible(peakhold_label);
    addAndMakeVisible(none_peak_button);
    addAndMakeVisible(fast_peak_button);
    addAndMakeVisible(medium_peak_button);
    addAndMakeVisible(slow_peak_button);
    addAndMakeVisible(audioVisualizer);
    addAndMakeVisible(spectrumVisualizer);
    addAndMakeVisible(lowPassKnob);
    addAndMakeVisible(lowPass_label);
}


SpectrumAnalyzerAudioProcessorEditor::~SpectrumAnalyzerAudioProcessorEditor()
{
    for (int i = 0; i < 4; ++i)
    {
        buttons[i]->removeListener(this);  // Remove listener when editor is destroyed
    }

    stopTimer();  // Stop the timer when the editor is destroyed
}


//==============================================================================
void SpectrumAnalyzerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
}


void SpectrumAnalyzerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    double fontsize = fallbackspeed_label.getFont().getHeight();

    double x_pos_firstcol = 0.5 * getWidth();
    fallbackspeed_label.setBounds(x_pos_firstcol, 0.5 * getHeight(), ITEM_SIZE, fontsize);
    knob.setBounds(x_pos_firstcol, 0.5 * getHeight() + fontsize + PADDING, ITEM_SIZE, ITEM_SIZE);

    double x_pos_secondcol = x_pos_firstcol + ITEM_SIZE + PADDING;

    double x_pos_thirdcol = x_pos_secondcol + BUTTON_WIDTH + PADDING;

    peakhold_label.setBounds(x_pos_secondcol, 0.5 * getHeight(), BUTTON_WIDTH, fontsize);

    none_peak_button.setBounds(x_pos_secondcol, 0.5 * getHeight() + fontsize + PADDING, BUTTON_WIDTH, BUTTON_HEIGHT);
    fast_peak_button.setBounds(x_pos_secondcol, 0.5 * getHeight() + fontsize + PADDING + (BUTTON_HEIGHT + PADDING), BUTTON_WIDTH, BUTTON_HEIGHT);
    medium_peak_button.setBounds(x_pos_secondcol, 0.5 * getHeight() + fontsize + PADDING + 2 * (BUTTON_HEIGHT + PADDING), BUTTON_WIDTH, BUTTON_HEIGHT);
    slow_peak_button.setBounds(x_pos_secondcol, 0.5 * getHeight() + fontsize + PADDING + 3 * (BUTTON_HEIGHT + PADDING), BUTTON_WIDTH, BUTTON_HEIGHT);

    lowPass_label.setBounds(x_pos_thirdcol, 0.5 * getHeight(), ITEM_SIZE, fontsize);
    lowPassKnob.setBounds(x_pos_thirdcol, 0.5 * getHeight() + fontsize + PADDING, ITEM_SIZE, ITEM_SIZE);
}


void SpectrumAnalyzerAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    // Check if the button is being toggled on
    if (button->getToggleState()) {
        for (int i = 0; i < 4; i++) {
            if (buttons[i] != button && buttons[i]->getToggleState()) {
                // If another button is toggled on, turn it off
                buttons[i]->setToggleState(false, juce::dontSendNotification);
            }
        }
    }
}

void SpectrumAnalyzerAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &lowPassKnob)
    {
        // Pass the value from the low-pass knob to the processor
        audioProcessor.setLowPassFrequency((float)slider->getValue());
    }
}

void SpectrumAnalyzerAudioProcessorEditor::timerCallback()
{
    // Get the waveform path from the processor (for channel 0)
    waveformPath = audioProcessor.getWaveformPath(20000, 0, VISUALIZER_HEIGHT, VISUALIZER_WIDTH);  // channel 0

    // Update the visualizer with the new waveform path
    audioVisualizer->setWaveformPath(waveformPath);

    spectrumPath = audioProcessor.getSpectrumPath(knob.getValue(), 0, SPECTRUM_HEIGHT, SPECTRUM_WIDTH, getPeakHoldMode(), (float)lowPassKnob.getValue()); //channel 0
    spectrumVisualizer->setWaveformPath(spectrumPath);
}

int SpectrumAnalyzerAudioProcessorEditor::getPeakHoldMode() {
    if (none_peak_button.getToggleState()) {
        return 0;
    }
    else if (fast_peak_button.getToggleState()) {
        return 1;
    }
    else if (medium_peak_button.getToggleState()) {
        return 2;
    }
    else if (slow_peak_button.getToggleState()) {
        return 3;
    }
    else{
        return -1;
    }
}