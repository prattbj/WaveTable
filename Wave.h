/*
  ==============================================================================

    Wave.h
    Created: 22 Aug 2024 5:33:09pm
    Author:  benny

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

#include "Frame.h"
#include "Harmonics.h"



class Wave
{
public:
    Wave() : harmonics(nullptr), file(), frames()
    {

    }

    void initializeWaveTable(juce::File file)
    {
        if (file.exists())
        {
            if (harmonics)
                harmonics.get();
            this->file = file;
            harmonics = std::make_unique<Harmonics>(file);

            for (int i = 0; i < 256; i++)
            {
                harmonics->fillFrame(frames[i], i);
            }
        }
    }
    Harmonics* getHarmonics() { return harmonics.get(); }
    void getFrame(int frame, int midiNoteNumber, float* arrToFill) 
    { 
        if (harmonics) 
            harmonics->getFrame(frame, midiNoteNumber, arrToFill);
        else
            memset(arrToFill, 0.f, 2048);
    }
    Frame* getFrames() { return frames.data(); }
    juce::File getFile() { return file; }
private:
    juce::File file;
    std::array<Frame, 256> frames;
    std::unique_ptr<Harmonics> harmonics;
};