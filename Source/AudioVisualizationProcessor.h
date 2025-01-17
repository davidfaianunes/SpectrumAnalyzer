#pragma once

#include <JuceHeader.h>
#include <vector>
#include <complex>
#include <cassert>
#include <cmath>
#include "CircularBuffer.h"
#define SPECTRUM_SCALING_FACTOR 5

class AudioVisualizationProcessor
{
public:
    explicit AudioVisualizationProcessor(int buffer_capacity, int channels)
    {
        buffer = new CircularBuffer(channels, buffer_capacity);
        peakHoldDelay[0] = 0;
        peakHoldDelay[1] = 1.0f;
        peakHoldDelay[2] = 2.0f;
        peakHoldDelay[3] = 5.0f;
    }

    ~AudioVisualizationProcessor()
    {
        delete buffer; // Ensure the buffer is properly deallocated
    }

    // Push audio data to the circular buffer
    void pushAudioData(const float* source, int numSamples, int channel)
    {
        buffer->push(source, numSamples, channel);
    }

    juce::Path getVisualizationPath(int numSamples, int channel, int height, int width)
    {
        juce::Path path; // Use a local path variable

        // Temporary buffer for reading data
        std::vector<float> tempBuffer(numSamples, 0.0f);

        buffer->read(tempBuffer, numSamples, channel); // Read the data into tempBuffer

        float x = 0.0f; // Initialize horizontal position tracker

        // Create the waveform path
        for (int i = 0; i < numSamples; ++i)
        {
            const float y = (tempBuffer[i] + 1.0f) * 0.5f * static_cast<float>(height); // Normalize to fit height
            if (i == 0)
                path.startNewSubPath(x, y);
            else
                path.lineTo(x, y);

            x += static_cast<float>(width) / numSamples; // Evenly space the waveform across the width
        }

        return path; // Return the local path
    }

    juce::Path AudioVisualizationProcessor::getSpectrumPath(int numSamples, int channel, int height, int width, int peakHoldMode, float lowPassFrequency)
    {
        juce::Path path; // Path to hold the visual representation

        if (peakHoldMode == -1) {
            return path;
        }


        // Ensure numSamples is a power of two
        numSamples = getPowerOfTwo(numSamples);

        // Temporary buffer for the audio data
        std::vector<float> tempBuffer(numSamples, 0.0f);

        // Read the audio data into the tempBuffer
        buffer->read(tempBuffer, numSamples, channel);

        // Create a buffer for FFT (real + imaginary parts)
        std::vector<float> fftData(numSamples * 2, 0.0f); // FFT input: real and imaginary parts interleaved

        // Copy the tempBuffer into the real part of fftData
        for (int i = 0; i < numSamples; ++i)
        {
            fftData[i] = tempBuffer[i];
        }

        // Perform FFT (real -> complex transform)
        juce::dsp::FFT fft(static_cast<int>(std::log2(numSamples))); // FFT size as a power of 2
        fft.performFrequencyOnlyForwardTransform(fftData.data());

        // Extract magnitudes from FFT data (use only the first half: positive frequencies)
        int numBins = numSamples / 2;
        std::vector<float> magnitudes(numBins, 0.0f);

        for (int i = 0; i < numBins; ++i)
        {
            float real = fftData[i];            // Real part
            float imaginary = fftData[numSamples + i]; // Imaginary part
            magnitudes[i] = std::sqrt(real * real + imaginary * imaginary);
        }

        if (peakHoldMode != 0) {
            float delay = peakHoldDelay[peakHoldMode];
            peaks.resize(numBins, 0.0f);
            timePassed.resize(numBins, 0.0f);

            lifetime = 30 * delay; //number of functioncalls in delayTime


            for (int i = 0; i < numBins;i++) {
                if (timePassed[i] >= lifetime) {
                    peaks[i] = magnitudes[i];
                    timePassed[i] = 0;
                }
                else {
                    peaks[i] = std::max(peaks[i], magnitudes[i]);
                    timePassed[i]++;
                }
            }
        }

        // Start drawing the spectrum
        float x;
        for (int i = 0; i < numBins; ++i)
        {
            // Increment x position for each frequency bin
            float x = width * std::log(i + 1) / std::log(numSamples);
            float y;
            if (peakHoldMode != 0) {
                y = height - (peaks[i] / SPECTRUM_SCALING_FACTOR);
            }
            else {
                y = height - (magnitudes[i] / SPECTRUM_SCALING_FACTOR);
            }

            // Map to visual space
            if (i == 0)
                path.startNewSubPath(x, y); // Start at the first point
            else
                path.lineTo(x, y); // Draw line to the next point

        }

        // Add a vertical line at the cutoff frequency
        if (lowPassFrequency > 0.0f) {
            // Calculate x position for the cutoff frequency
            int cutoffBin = static_cast<int>((lowPassFrequency * numSamples) / 20000);
            cutoffBin = std::clamp(cutoffBin, 0, numBins - 1); // Make sure the bin is within range

            float cutoffX = width * std::log(cutoffBin + 1) / std::log(numSamples);

            // Create a new subpath for the vertical line
            path.startNewSubPath(cutoffX, 0);  // Start at the top of the window
            path.lineTo(cutoffX, height); // End at the bottom of the window
        }

        return path; // Return the constructed path
    }




    void setSampleRate(int _sampleRate)
    {
        sampleRate = _sampleRate;
    }

private:
    int getPowerOfTwo(int numSamples)
    {
        int powers[25] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216 };
        for (int i = 0; i < 25; i++) {
            if (powers[i] > numSamples) {
                return powers[i - 1];
            }
        }
        // If no larger than numSamples
        return powers[24];
    }

    float peakHoldDelay[4];

    int sampleRate = 0;
    CircularBuffer* buffer;

    int lifetime;

    std::vector<float> peaks;
    std::vector<float> timePassed;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioVisualizationProcessor)
};
