/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
class WaveTableAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    WaveTableAudioProcessorEditor (WaveTableAudioProcessor&);
    ~WaveTableAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    WaveTableAudioProcessor& audioProcessor;
    Visualiser& visualiser;
    juce::Slider frameSlider;
    std::unique_ptr<SliderAttachment> visualiserAttachment;
    juce::MidiKeyboardComponent keyboard;
    juce::AudioVisualiserComponent& audioVisualiser;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveTableAudioProcessorEditor)
};

