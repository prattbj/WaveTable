/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WaveTableAudioProcessorEditor::WaveTableAudioProcessorEditor (WaveTableAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), visualiser(p.getWavetableVisualiser()), 
    keyboard(p.getKeyboardState(), juce::KeyboardComponentBase::Orientation::horizontalKeyboard),
    audioVisualiser(p.getAudioVisualiser())
{

    setSize (400, 150);
    addAndMakeVisible(visualiser);

    //addAndMakeVisible(frameSlider);
    visualiser.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    visualiser.setRange(0.0, 255.0);
    visualiserAttachment.reset(new SliderAttachment(p.getVTS(), "frame", visualiser));
    visualiser.setValue(*p.getVTS().getRawParameterValue("frame"));
    visualiser.addListener((juce::Slider::Listener*)&p.getWavetableVisualiser());
    visualiser.setSliderSnapsToMousePosition(false);

    addAndMakeVisible(keyboard);
    //addAndMakeVisible(audioVisualiser);
}

WaveTableAudioProcessorEditor::~WaveTableAudioProcessorEditor()
{
}

//==============================================================================
void WaveTableAudioProcessorEditor::paint (juce::Graphics&)
{
    
    
}

void WaveTableAudioProcessorEditor::resized()
{
    visualiser.setBounds(0, 0, getWidth(), getHeight() * 2 / 3);
    //frameSlider.setBounds(0, getHeight() * 2 / 5, getWidth(), 20);
    keyboard.setBounds(0, getHeight()  * 2 / 3, getWidth(), getHeight() / 3);
    //audioVisualiser.setBounds(0, getHeight() * 2 / 5 + 20, getWidth(), getHeight() / 5);
}
