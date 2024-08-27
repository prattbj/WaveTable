/*
  ==============================================================================

    Harmonics.h
    Created: 22 Aug 2024 3:32:40pm
    Author:  benny

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "Frame.h"
class Harmonics
{
public:
    Harmonics(juce::File file) : fft(11), toTransform({0.0f}), transformer({0.0f})
    {
        formatManager.registerBasicFormats();

        //Read the file
        juce::AudioFormatReader* reader = formatManager.createReaderFor(file);
        juce::AudioBuffer<float> audioBuffer;
        audioBuffer.setSize(reader->numChannels, (int)reader->lengthInSamples);
        reader->read(&audioBuffer, 0, (int)reader->lengthInSamples, (juce::int64)0, true, true);
        int samples = (int)reader->lengthInSamples;
        delete reader;

        //Number of key frames we will get from the file (max 256)
        numFrames = samples / 2048;
        if (numFrames > 256)
            numFrames = 256;

        

        const float* read = audioBuffer.getReadPointer(0);


        harmonics.reserve(numFrames);
        for (int i = 0; i < numFrames; i++)
        {
            harmonics.push_back(std::array<float, 4096>());
            for (int j = 0; j < 2048; j++)
            {
                harmonics[i][j] = read[i * 2048 + j];
            }
        }

        //float toTransform[4096] = { 0.0f };
        for (int i = 0; i < numFrames; i++)
        {
            //std::copy_n(harmonics[i], 2048, toTransform);
            fft.performRealOnlyForwardTransform(harmonics[i].data(), false);
        }
    }
    
    void fillFrame(Frame& frame, int frameNumber)
    {
        
        
        interpolate(frameNumber, toTransform.data());
        //float transformer[4096] = { 0.0f };
        //128 is the number of midi keys. every numSteps keys, we bandlimit the wavetable so that it is anti-aliased.
        float endIndex = 4096;
        for (int i = 0; i < 128 / numSteps; i++)
        {
            //We need to clear everything above the highest frequency band
            float highestFrequencyBand = 48000.0f / (float)juce::MidiMessage::getMidiNoteInHertz(i * numSteps);
            
            for (int j = (int)highestFrequencyBand; j < endIndex; j++)
            {
                toTransform[j] = 0.0f;
            }
            endIndex = highestFrequencyBand > 4096.f ? 4096.f : highestFrequencyBand;
            
            std::copy_n(toTransform.data(), 4096, transformer.data());
            fft.performRealOnlyInverseTransform(transformer.data());
            std::copy_n(transformer.data(), 2048, frame[i]);
        }
    }

    inline void interpolate(int currentFrame, float * arrToFill)
    {
        float scaledIndex = (currentFrame / 255.f) * (float)(numFrames - 1);
        int lowerIndex = (int)scaledIndex;
        int upperIndex = lowerIndex + 1 > numFrames - 1 ? numFrames - 1: lowerIndex + 1;
        float ratio = scaledIndex - lowerIndex;
        for (int i = 0; i < 2048; i++)
        {
            arrToFill[i * 2] = (1 - ratio) * harmonics[lowerIndex][i * 2] + ratio * harmonics[upperIndex][i * 2];
            arrToFill[i * 2 + 1] = (1 - ratio) * harmonics[lowerIndex][i * 2 + 1] + ratio * harmonics[upperIndex][i * 2 + 1];
        }
    }

private:
    juce::dsp::FFT fft;
    juce::AudioFormatManager formatManager;
    std::vector<std::array<float,4096>> harmonics;
    int numFrames;
    std::array<float, 4096> toTransform;
    std::array<float, 4096> transformer;
};