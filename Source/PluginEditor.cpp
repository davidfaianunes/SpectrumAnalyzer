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

    fallbackspeed_label.setText("Fall-Back Speed", juce::NotificationType::dontSendNotification);
    peakhold_label.setText("Peak Hold", juce::NotificationType::dontSendNotification);


    none_peak_button.setButtonText("None");
    fast_peak_button.setButtonText("Fast");
    medium_peak_button.setButtonText("Medium");
    slow_peak_button.setButtonText("Slow");


    none_peak_button.setLookAndFeel(&customButtonLookAndFeel);
    fast_peak_button.setLookAndFeel(&customButtonLookAndFeel);
    medium_peak_button.setLookAndFeel(&customButtonLookAndFeel);
    slow_peak_button.setLookAndFeel(&customButtonLookAndFeel);

    addAndMakeVisible(knob);
    addAndMakeVisible(fallbackspeed_label);
    addAndMakeVisible(peakhold_label);
    addAndMakeVisible(none_peak_button);
    addAndMakeVisible(fast_peak_button);
    addAndMakeVisible(medium_peak_button);
    addAndMakeVisible(slow_peak_button);

    // Add listeners for real-time button state updates
    none_peak_button.addListener(this);
    fast_peak_button.addListener(this);
    medium_peak_button.addListener(this);
    slow_peak_button.addListener(this);
}


SpectrumAnalyzerAudioProcessorEditor::~SpectrumAnalyzerAudioProcessorEditor()
{
    none_peak_button.removeListener(this);
    fast_peak_button.removeListener(this);
    medium_peak_button.removeListener(this);
    slow_peak_button.removeListener(this);
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

    double x_pos_firstcol = 0.5 * getWidth() + PADDING;
    fallbackspeed_label.setBounds(x_pos_firstcol, 0.5 * getHeight(), ITEM_SIZE, fontsize);
    knob.setBounds(x_pos_firstcol, 0.5 * getHeight() + fontsize + PADDING, ITEM_SIZE, ITEM_SIZE);

    double x_pos_secondcol = x_pos_firstcol + ITEM_SIZE + PADDING;

    peakhold_label.setBounds(x_pos_secondcol, 0.5 * getHeight(), ITEM_SIZE, fontsize);

    none_peak_button.setBounds(x_pos_secondcol, 0.5 * getHeight() + fontsize + PADDING, ITEM_SIZE, BUTTON_HEIGHT);
    fast_peak_button.setBounds(x_pos_secondcol, 0.5 * getHeight() + fontsize + PADDING + (BUTTON_HEIGHT + PADDING), ITEM_SIZE, BUTTON_HEIGHT);
    medium_peak_button.setBounds(x_pos_secondcol, 0.5 * getHeight() + fontsize + PADDING + 2 * (BUTTON_HEIGHT + PADDING), ITEM_SIZE, BUTTON_HEIGHT);
    slow_peak_button.setBounds(x_pos_secondcol, 0.5 * getHeight() + fontsize + PADDING + 3 * (BUTTON_HEIGHT + PADDING), ITEM_SIZE, BUTTON_HEIGHT);
}


void SpectrumAnalyzerAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    bool turnedon = false;

    if (button->getToggleState() == true) {//if button was turned on, turn off all buttons
        audioProcessor.nonePeakButtonState->setValueNotifyingHost(false);
        audioProcessor.fastPeakButtonState->setValueNotifyingHost(false);
        audioProcessor.mediumPeakButtonState->setValueNotifyingHost(false);
        audioProcessor.slowPeakButtonState->setValueNotifyingHost(false);

        none_peak_button.setToggleState(false, juce::NotificationType::dontSendNotification);
        fast_peak_button.setToggleState(false, juce::NotificationType::dontSendNotification);
        medium_peak_button.setToggleState(false, juce::NotificationType::dontSendNotification);
        slow_peak_button.setToggleState(false, juce::NotificationType::dontSendNotification);

        turnedon = true;
    }

    // turn on that button again

    if (button == &none_peak_button)
    {
        if (turnedon) {
            audioProcessor.nonePeakButtonState->setValueNotifyingHost(true);
            none_peak_button.setToggleState(true, juce::NotificationType::dontSendNotification);
        }
        audioProcessor.nonePeakButtonState->setValueNotifyingHost(none_peak_button.getToggleState());
    }
    else if (button == &fast_peak_button)
    {
        if (turnedon) {
            audioProcessor.fastPeakButtonState->setValueNotifyingHost(true);
            fast_peak_button.setToggleState(true, juce::NotificationType::dontSendNotification);
        }
        audioProcessor.fastPeakButtonState->setValueNotifyingHost(fast_peak_button.getToggleState());
    }
    else if (button == &medium_peak_button)
    {
        if (turnedon) {
            audioProcessor.mediumPeakButtonState->setValueNotifyingHost(true);
            medium_peak_button.setToggleState(true, juce::NotificationType::dontSendNotification);
        }
        audioProcessor.mediumPeakButtonState->setValueNotifyingHost(medium_peak_button.getToggleState());
    }
    else if (button == &slow_peak_button)
    {
        if (turnedon) {
            audioProcessor.slowPeakButtonState->setValueNotifyingHost(true);
            slow_peak_button.setToggleState(true, juce::NotificationType::dontSendNotification);
        }
        audioProcessor.slowPeakButtonState->setValueNotifyingHost(slow_peak_button.getToggleState());
    }
}