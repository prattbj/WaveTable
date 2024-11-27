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
    audioVisualiser(p.getAudioVisualiser()),
    lfo(p.getLFO())
{

    setSize (600, 400);
    addAndMakeVisible(visualiser);

    //addAndMakeVisible(frameSlider);
    visualiser.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    visualiser.setRange(0.0, 255.0);
    visualiserAttachment.reset(new SliderAttachment(p.getVTS(), "frame", visualiser));
    visualiser.setValue(*p.getVTS().getRawParameterValue("frame"));
    visualiser.addListener((juce::Slider::Listener*)&p.getWavetableVisualiser());
    visualiser.setSliderSnapsToMousePosition(false);
    visualiser.setMouseDragSensitivity(visualiser.getWidth());
    
    addAndMakeVisible(keyboard);
    addAndMakeVisible(lfo);
    

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
    visualiser.setBounds(0, 0, getWidth(), getHeight() * 2 / 4);
    //frameSlider.setBounds(0, getHeight() * 2 / 5, getWidth(), 20);
    lfo.setBounds(0, getHeight() * 2 / 4, getWidth(), getHeight() * 1 / 4);
    keyboard.setBounds(0, getHeight()  * 3 / 4, getWidth(), getHeight() / 4);
    //audioVisualiser.setBounds(0, getHeight() * 2 / 5 + 20, getWidth(), getHeight() / 5);
}
