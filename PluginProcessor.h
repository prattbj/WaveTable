/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Wave.h"
#include "Visualiser.h"
#include "LFO.h"

//==============================================================================
/**
*/
class WaveTableAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    WaveTableAudioProcessor();
    ~WaveTableAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    Visualiser& getWavetableVisualiser() { return wavetableVisualiser; }
    juce::AudioVisualiserComponent& getAudioVisualiser() { return audioVisualiser; }
    juce::AudioProcessorValueTreeState& getVTS() { return vts; }
    juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }
    LFO& getLFO() { return lfo; }
private:
    //==============================================================================
    Visualiser wavetableVisualiser;
    Wave wavetable;
    LFO lfo;
    LFOAttachment gainAttachment;
    juce::dsp::Gain<float> gain;
    juce::AudioProcessorValueTreeState vts;
    juce::MidiKeyboardState keyboardState;
    juce::Synthesiser synth;
    juce::AudioVisualiserComponent audioVisualiser;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveTableAudioProcessor)
};

class SynthSound : public juce::SynthesiserSound
{
public:
    SynthSound(Wave* wavetable) : wavetable(wavetable) {}
    //Frame* getFrames() { return frames; }
    Wave* getWavetable() { return wavetable; }

    bool appliesToNote(int)
    {
        return true;
    }
    bool appliesToChannel(int)
    {
        return true;
    }
    float getTableSize()
    {
        return 2048.f;
    }
private:
    Wave* wavetable;
};
class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice(juce::dsp::ProcessSpec spec) : waveTable(), sampleRate((float)spec.sampleRate), angleDelta(0.f), phase(), frameNumber(0), midiNoteNumber(0)
    {

    }
    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return (dynamic_cast<SynthSound*>(sound) != nullptr);
    }
    void startNote(int midiNoteNumber, float, juce::SynthesiserSound* sound, int) override
    {
        SynthSound* synthSound = dynamic_cast<SynthSound*>(sound);
        //waveTable = synthSound->getTable();
        synthSound->getWavetable()->getFrame(frameNumber, midiNoteNumber, waveTable);
        this->midiNoteNumber = midiNoteNumber;
        float hz = (float)juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        angleDelta = hz * synthSound->getTableSize() / sampleRate;

    }
    
    
    void renderNextBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples) override
    {
        if (waveTable)
        {
            float* dataL = buffer.getWritePointer(0);
            float* dataR = buffer.getWritePointer(1);
            for (int j = startSample; j < numSamples + startSample; j++)
            {
                dataL[j] += getNextSample(0);
                dataR[j] += getNextSample(1);
            }
        }
        
    }
    

    float SynthVoice::getNextSample(int channel)
    {

        int index0 = (int)phase[channel]; 
        int index1 = index0 == (2048.f - 1) ? 0 : index0 + 1;

        float frac = phase[channel] - (float)index0; 

        float* table = this->waveTable;
        float value0 = table[index0];
        float value1 = table[index1];

        float currentSample = value0 + frac * (value1 - value0); 

        if ((phase[channel] += angleDelta) > 2048.f)   
            phase[channel] -= 2048.f;

        return currentSample;
    }

    void stopNote(float, bool) override { for (auto& val : waveTable) { val = 0.0f; } }
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}
    void setFrameNumber(int n) { frameNumber = n; }
private:
    float waveTable[2048];
    float phase[2];
    float angleDelta;
    float sampleRate;
    int midiNoteNumber;
    int frameNumber;
};