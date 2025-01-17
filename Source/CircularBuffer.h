#pragma once

#include <JuceHeader.h>
#include <vector>
#include <atomic>
#include <cassert>

/**
 * CircularBuffer class for managing a ring buffer of audio or other data.
 * @tparam T - The data type to be stored in the buffer (e.g., float, int).
 */
class CircularBuffer
{
public:
    explicit CircularBuffer(int numChannels, int capacity)
        : buffer(numChannels, capacity), writeIndex(0), readIndex(0), bufferSize(capacity)
    {
        assert(capacity > 0 && "Capacity must be greater than zero");
        buffer.clear(); // Ensure the buffer starts clean
    }

    void push(const float* data, int numSamples, int channel)
    {
        std::lock_guard<std::mutex> lock(bufferMutex);

        assert(data != nullptr && "Data pointer must not be null");
        assert(numSamples <= bufferSize && "Cannot push more data at a time than buffer capacity");
        assert(channel >= 0 && channel < buffer.getNumChannels() && "Invalid channel index");

        if (numSamples + writeIndex <= bufferSize) // Data fits completely within buffer
        {
            buffer.copyFrom(channel, writeIndex, data, numSamples);
        }
        else
        {
            // Compute how many samples to write at the end of the buffer
            int numSamplesAtEnd = bufferSize - writeIndex;
            buffer.copyFrom(channel, writeIndex, data, numSamplesAtEnd);

            // Compute how many samples to write at the beginning
            int numSamplesAtBeginning = numSamples - numSamplesAtEnd;
            buffer.copyFrom(channel, 0, data + numSamplesAtEnd, numSamplesAtBeginning);
        }

        // Update writeIndex
        writeIndex = (writeIndex + numSamples) % bufferSize;
    }

    void read(std::vector<float>& output, int numSamples, int channel)
    {
        std::lock_guard<std::mutex> lock(bufferMutex);

        assert(!output.empty() && "Output vector must not be empty");
        assert(numSamples <= bufferSize && "Cannot read more samples than buffer capacity");
        assert(channel >= 0 && channel < buffer.getNumChannels() && "Invalid channel index");

        // Resize the output vector to the number of samples requested
        output.resize(numSamples);

        // Compute readIndex to read the most recent samples
        readIndex = writeIndex - numSamples;
        if (readIndex < 0) readIndex += bufferSize;

        if (numSamples + readIndex <= bufferSize) // Data fits within a single block
        {
            copyBufferToVector(channel, readIndex, numSamples, output, 0);
        }
        else
        {
            // Data wraps around the buffer
            int numSamplesAtEnd = bufferSize - readIndex;
            copyBufferToVector(channel, readIndex, numSamplesAtEnd, output, 0);

            int numSamplesAtBeginning = numSamples - numSamplesAtEnd;
            copyBufferToVector(channel, 0, numSamplesAtBeginning, output, numSamplesAtEnd);
        }
    }

    void clear()
    {
        buffer.clear();
        writeIndex = 0;
        readIndex = 0;
    }

private:
    void copyBufferToVector(int channel, int readIndex, int numSamples, std::vector<float>& output, int startOn)
    {
        // Ensure the input parameters are valid
        assert(channel >= 0 && channel < buffer.getNumChannels());
        assert(readIndex >= 0 && readIndex + numSamples <= buffer.getNumSamples());
        assert(startOn >= 0);

        // Get a pointer to the data in the specified channel
        const float* channelData = buffer.getReadPointer(channel);

        // Copy the data
        for (int i = 0; i < numSamples; ++i)
        {
            output[startOn + i] = channelData[(readIndex + i) % bufferSize];
        }
    }

    juce::AudioBuffer<float> buffer; ///< The buffer for storing data.
    int writeIndex;                  ///< Index where the next data will be written.
    int readIndex;                   ///< Index where the next data will be read.
    int bufferSize;                  ///< Capacity of the buffer.
    std::mutex bufferMutex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CircularBuffer)
};
