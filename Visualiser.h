/*
  ==============================================================================

    Visualiser.h
    Created: 26 Aug 2024 8:26:47pm
    Author:  benny

  ==============================================================================
*/

#pragma once
class Visualiser : public juce::FileDragAndDropTarget, public juce::Slider, public juce::MidiKeyboardState::Listener
{
public:
    Visualiser(Wave& wavetable) : wavetable(wavetable), midiNoteNumber(0)
    {

    }
    bool isInterestedInFileDrag(const juce::StringArray& files) override
    {
        return files.begin()->endsWith(".wav");
    }
    void filesDropped(const juce::StringArray& files, int, int) override
    {
        juce::File file(*files.begin());
        wavetable.initializeWaveTable(file);
        repaint();
    }
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        float* wave = (*wavetable.getFrame((int)getValue()))[midiNoteNumber / numSteps];

        g.setColour(juce::Colours::blue);

        if (wave)
        {
            for (int i = 1; i < 2048; i++)
            {
                g.drawLine(
                    getWidth() * (i - 1) / 2048.0f, 
                    getHeight() / 2.0f * wave[i - 1] + getHeight() / 2.0f, 
                    getWidth() * (i) / 2048.0f, 
                    getHeight() / 2.0f * wave[i] + getHeight() / 2.0f
                );
            }
        }
        if (getThumbBeingDragged() >= 0)
        {
            g.setColour(juce::Colours::green);
            g.setOpacity(0.75f);
            g.fillRect(
                (float)getWidth() * (float)getValue() / 256.f,
                0.0f,
                5.f,
                (float)getHeight()
                );
            
        }
    }
    void resized() override {}
    
    void handleNoteOn(juce::MidiKeyboardState*, int, int n, float)
    {
        midiNoteNumber = n;
        repaint();
    }
    void handleNoteOff(juce::MidiKeyboardState*, int, int, float)
    {
        
    }
    void valueChanged() override
    {
        repaint();
    }

    
private:
    int midiNoteNumber;
    Wave& wavetable;
};