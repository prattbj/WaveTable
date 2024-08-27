/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WaveTableAudioProcessor::WaveTableAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    wavetable(), wavetableVisualiser(wavetable), vts(*this, nullptr, juce::Identifier("WaveTable"), 
        {
            std::make_unique<juce::AudioParameterInt>("frame", "Frame", 0, 255, 0)
        }),
    keyboardState(),
    audioVisualiser(1)
{
    audioVisualiser.setBufferSize(80);
    keyboardState.addListener(&wavetableVisualiser);
}

WaveTableAudioProcessor::~WaveTableAudioProcessor()
{
}

//==============================================================================
const juce::String WaveTableAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WaveTableAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool WaveTableAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool WaveTableAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double WaveTableAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int WaveTableAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int WaveTableAudioProcessor::getCurrentProgram()
{
    return 0;
}

void WaveTableAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String WaveTableAudioProcessor::getProgramName (int index)
{
    return {};
}

void WaveTableAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void WaveTableAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    
    juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 };

    for (int i = 0; i < 16; i++)
    {
        synth.addVoice(new SynthVoice(spec));
    }
    synth.addSound(new SynthSound(wavetable.getFrames()));
    synth.setCurrentPlaybackSampleRate(sampleRate);
    audioVisualiser.clear();
    audioVisualiser.setSamplesPerBlock(samplesPerBlock);
}

void WaveTableAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool WaveTableAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void WaveTableAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    for (int i = 0; i < synth.getNumVoices(); i++)
    {
        ((SynthVoice*)synth.getVoice(i))->setFrameNumber((int)*vts.getRawParameterValue("frame"));
    }
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    audioVisualiser.pushBuffer(buffer);
}

//==============================================================================
bool WaveTableAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* WaveTableAudioProcessor::createEditor()
{
    return new WaveTableAudioProcessorEditor (*this);
}

//==============================================================================
void WaveTableAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    std::unique_ptr<juce::XmlElement> xml = std::make_unique<juce::XmlElement>("WAVETABLE");
    xml->setAttribute(juce::Identifier("filename"), wavetable.getFile().getFullPathName());
    copyXmlToBinary(*xml, destData);
}

void WaveTableAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName("WAVETABLE"))
        {
            wavetable.initializeWaveTable(juce::File(xmlState->getStringAttribute("filename")));
        }
    }
            
    
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WaveTableAudioProcessor();
}
