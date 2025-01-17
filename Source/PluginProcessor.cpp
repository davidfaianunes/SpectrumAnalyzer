/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#define NUM_CHANNELS 3
#define BUFFER_CAPACITY 80000
#define TEMPORARY_FALLBACK_SPEED 0.5 //FIXME

//==============================================================================
SpectrumAnalyzerAudioProcessor::SpectrumAnalyzerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    audioVisualizationProcessor = new AudioVisualizationProcessor(BUFFER_CAPACITY, NUM_CHANNELS);
    sampleRate = 0;
}

SpectrumAnalyzerAudioProcessor::~SpectrumAnalyzerAudioProcessor()
{
}

//==============================================================================
const juce::String SpectrumAnalyzerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpectrumAnalyzerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SpectrumAnalyzerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SpectrumAnalyzerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SpectrumAnalyzerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpectrumAnalyzerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpectrumAnalyzerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpectrumAnalyzerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SpectrumAnalyzerAudioProcessor::getProgramName (int index)
{
    return {};
}

void SpectrumAnalyzerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SpectrumAnalyzerAudioProcessor::prepareToPlay (double _sampleRate, int samplesPerBlock)
{
    sampleRate = _sampleRate;
    audioVisualizationProcessor->setSampleRate((int)_sampleRate);
    lastSamples.resize(getTotalNumInputChannels(), 0.0f); // One state per channel
}

void SpectrumAnalyzerAudioProcessor::releaseResources()
{
    delete audioVisualizationProcessor;
    audioVisualizationProcessor = nullptr;
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpectrumAnalyzerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SpectrumAnalyzerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    blockSize = buffer.getNumSamples(); // Get the number of samples in the current block

    // Clear any extra output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, blockSize);

    // Allocate and sum data
    float* summedData = new float[blockSize];
    std::fill(summedData, summedData + blockSize, 0.0f);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        const auto* channelData = buffer.getReadPointer(channel);

        for (int sampleIndex = 0; sampleIndex < blockSize; ++sampleIndex)
        {
            summedData[sampleIndex] += channelData[sampleIndex];
        }
    }

    if (audioVisualizationProcessor != nullptr)
    {
        audioVisualizationProcessor->pushAudioData(summedData, blockSize, 0);
    }

    delete[] summedData;

    // Apply the low-pass filter
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        applyLowpassFilter(channelData, blockSize, lowPassCutoffFrequency, sampleRate, channel);
    }

}


juce::Path SpectrumAnalyzerAudioProcessor::getWaveformPath(int numSamples, int channel, int height, int width) {
    return audioVisualizationProcessor->getVisualizationPath(numSamples, channel, height, width);
}

juce::Path SpectrumAnalyzerAudioProcessor::getSpectrumPath(double fallbackSpeed, int channel, int height, int width, int peakHoldMode, float lowPassFrequency) {
    double numSamples = (double)sampleRate * fallbackSpeed;
    return audioVisualizationProcessor->getSpectrumPath((int)numSamples, channel, height, width, peakHoldMode, lowPassFrequency);
}

//==============================================================================
bool SpectrumAnalyzerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SpectrumAnalyzerAudioProcessor::createEditor()
{
    return new SpectrumAnalyzerAudioProcessorEditor (*this);
}

//==============================================================================
void SpectrumAnalyzerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SpectrumAnalyzerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectrumAnalyzerAudioProcessor();
}

void SpectrumAnalyzerAudioProcessor::applyLowpassFilter(float* data, int numSamples, float cutoffFrequency, double sampleRate, int channel)
{
    if (channel >= lastSamples.size() || sampleRate <= 0.0f) // Validate input
        return;

    float rc = 1.0f / (cutoffFrequency * 2.0f * juce::MathConstants<float>::pi);
    float dt = 1.0f / static_cast<float>(sampleRate);
    float alpha = dt / (rc + dt);

    for (int i = 0; i < numSamples; ++i)
    {
        data[i] = alpha * data[i] + (1.0f - alpha) * lastSamples[channel];
        lastSamples[channel] = data[i]; // Update the state
    }
}

void SpectrumAnalyzerAudioProcessor::setLowPassFrequency(float frequency)
{
    lowPassCutoffFrequency = frequency;
}