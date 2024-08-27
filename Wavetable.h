/*
  ==============================================================================

    Wavetable.h
    Created: 22 Aug 2024 3:30:11pm
    Author:  benny

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include <fstream>
const int numSteps = 4;
class WaveTable 
{
public:
    /*WaveTable() : name("Analog Shapes"), frame(0.0f), current(1, 2048)
    {
        loadFunction([](float x) { return std::sin(x); });

        loadFunction([](float x)
            {
                return (x < 0) ? juce::jmap(x, float(-juce::MathConstants<double>::pi), 0.0f, -1.0f, 1.0f)
                    :
                    juce::jmap(x, 0.0f, float(juce::MathConstants<double>::pi), 1.0f, -1.0f);
            });

        loadFunction([](float x) { return juce::jmap(x, 0.0f, float(juce::MathConstants<double>::pi), 1.0f, -1.0f); });

        loadFunction([](float x)
            {
                return x > 0 ? -1.0f : 1.0f;
            });

    }*/

    WaveTable(juce::String path) : name("custom"), formatManager(), fft(11), end()//, lastCurrentFrame(0)
    {
        formatManager.registerBasicFormats();
        juce::File file(juce::File("D:").getChildFile("Samples").getChildFile("packs").getChildFile("Virtual Riot Sample Pack").getChildFile("Virtual_Riot_Sample_Pack").getChildFile("V_RIOT_tonal").getChildFile("V_RIOT_bass").getChildFile("V_RIOT_bass_one_shots").getChildFile("V_RIOT_bass_big_yoy_01_F.wav"));

        juce::AudioFormatReader* reader = formatManager.createReaderFor(file.createInputStream());
        juce::AudioBuffer<float> audioBuffer;
        audioBuffer.setSize(reader->numChannels, reader->lengthInSamples);
        reader->read(&audioBuffer, 0, reader->lengthInSamples, 0, true, true);
        int samples = reader->lengthInSamples;
        delete reader;

        int numFrames = samples / 2048;
        const float* read = audioBuffer.getReadPointer(0);
        

        int frameDistance = 256 / numFrames;
        for (int i = 0; i < numFrames; i++)
        {
            
            for (int j = 0; j < 2048; j++)
            {
                /*float sample1 = read[(i / frameDistance) * j];
                float sample2 = read[(i / frameDistance + 1) * j];
                float distance = juce::jmap((float)i, (float)(j / frameDistance), (float)(j / frameDistance + 1), 0.0f, 1.0f);
                sample1 *= distance;
                sample2 *= 1.0f - distance;
                end[i][0][j] = sample1 + sample2;*/
                end[i][0][j] = read[i * 2048 + j];
            }
        }

        float toTransform[4096] = {0.0f};
        for (int i = 0; i < 256; i++)
        {
            std::copy_n(end[i][0], 2048, toTransform);
            fft.performRealOnlyForwardTransform(toTransform, false);
            int endIndex = 48000.0f / juce::MidiMessage::getMidiNoteInHertz(0 * numSteps);
            for (int x = 0; x < 128 / numSteps; x++)
            {
                for (int j = 48000.0f / juce::MidiMessage::getMidiNoteInHertz(x * numSteps); j < endIndex + 1; j++)
                {
                    toTransform[j] = 0.0f;
                }
                endIndex = 48000.0f / juce::MidiMessage::getMidiNoteInHertz(x * numSteps);
                float transformed[4096];
                std::copy(std::begin(toTransform),std::end(toTransform), std::begin(transformed));
                fft.performRealOnlyInverseTransform(transformed);
                std::copy_n(transformed, 2048, end[i][x]);
            }
        }
        
    }
    
    void reset(juce::File file) 
    {
        
        juce::AudioFormatReader* reader = formatManager.createReaderFor(file);
        juce::AudioBuffer<float> audioBuffer;
        audioBuffer.setSize(reader->numChannels, reader->lengthInSamples);
        reader->read(&audioBuffer, 0, reader->lengthInSamples, 0, true, true);
        int samples = reader->lengthInSamples;
        delete reader;

        int numFrames = samples / 2048;
        if (numFrames > 256)
            numFrames = 256;
        const float* read = audioBuffer.getReadPointer(0);
        

        int frameDistance = 256 / numFrames;
        for (int i = 0; i < numFrames; i++)
        {

            for (int j = 0; j < 2048; j++)
            {
                /*float sample1 = read[(i / frameDistance) * j];
                float sample2 = read[(i / frameDistance + 1) * j];
                float distance = juce::jmap((float)i, (float)(j / frameDistance), (float)(j / frameDistance + 1), 0.0f, 1.0f);
                sample1 *= distance;
                sample2 *= 1.0f - distance;
                end[i][0][j] = sample1 + sample2;*/
                end[i][0][j] = read[i * 2048 + j];
            }
        }

        float toTransform[4096] = { 0.0f };
        for (int i = 0; i < 256; i++)
        {
            std::copy_n(end[i][0], 2048, toTransform);
            fft.performRealOnlyForwardTransform(toTransform, false);
            int endIndex = 48000.0f / juce::MidiMessage::getMidiNoteInHertz(0);
            for (int x = 0; x < 128 / numSteps; x++)
            {
                
                for (int j = 48000.0f / juce::MidiMessage::getMidiNoteInHertz(x * numSteps); j < endIndex; j++)
                {
                    toTransform[j] = 0.0f;
                }
                endIndex = 48000.0f / juce::MidiMessage::getMidiNoteInHertz(x * numSteps);
                float transformed[4096];
                std::copy(std::begin(toTransform), std::end(toTransform), std::begin(transformed));
                fft.performRealOnlyInverseTransform(transformed);
                std::copy_n(transformed, 2048, end[i][x]);
            }
        }
    }

    

    float * getEnd(float frame, float hz)
    {
        int z = (int)((12.0f * log2(hz / 440.0f) + 69.0f) / numSteps);
        
        return end[(int)frame][z];
    }

    float* getEnd(float frame)
    {
        return end[(int)frame][0];
    }
    ~WaveTable()
    {
       
    }
private:
    float end[256][128 / numSteps][2048];
    juce::dsp::FFT fft;
    juce::AudioFormatManager formatManager;
    juce::String name;
};