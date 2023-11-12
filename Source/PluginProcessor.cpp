/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicLCRDelayAudioProcessor::BasicLCRDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), treeState(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    treeState.addParameterListener("delayTimeID1", this);
    treeState.addParameterListener("delayTimeID2", this);
    treeState.addParameterListener("delayTimeID3", this);
    treeState.addParameterListener("delayFeedbackID", this);
    treeState.addParameterListener("wetLevelID", this);
    treeState.addParameterListener("dryLevelID", this);
}

BasicLCRDelayAudioProcessor::~BasicLCRDelayAudioProcessor()
{
    treeState.removeParameterListener("delayTimeID1", this);
    treeState.removeParameterListener("delayTimeID2", this);
    treeState.removeParameterListener("delayTimeID3", this);
    treeState.removeParameterListener("delayFeedbackID", this);
    treeState.removeParameterListener("wetLevelID", this);
    treeState.removeParameterListener("dryLevelID", this);
}

//==============================================================================
const juce::String BasicLCRDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

juce::AudioProcessorValueTreeState::ParameterLayout BasicLCRDelayAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.reserve(6);
    
    juce::NormalisableRange<float> delayTimeRange (0.f, 2000.f, 1.f, 1.f, false);
    params.push_back(std::make_unique<juce::AudioParameterFloat>("delayTimeID1", "left time", delayTimeRange, 250.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("delayTimeID2", "right time", delayTimeRange, 350.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("delayTimeID3", "center time", delayTimeRange, 450.f));
    
    juce::NormalisableRange<float> delayFeedbackRange (0.f, 100.f, 1.f, 1.f, false);
    params.push_back(std::make_unique<juce::AudioParameterFloat>("delayFeedbackID", "feedback", delayFeedbackRange, 30.f));
    
    juce::NormalisableRange<float> wetLevelRange (-60.f, 12.f, 1.f, 1.f, false);
    params.push_back(std::make_unique<juce::AudioParameterFloat>("wetLevelID", "wetLevel", wetLevelRange, -3.f));
    
    juce::NormalisableRange<float> dryLevelRange (-60.f, 12.f, 1.f, 1.f, false);
    params.push_back(std::make_unique<juce::AudioParameterFloat>("dryLevelID", "dryLevel", dryLevelRange, -3.f));
        
    return { params.begin(), params.end() };
}

void BasicLCRDelayAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    if (parameterID == "delayTimeID1")
    {
        params.left_mSec = newValue;
        lcrDelay.setLCRParameters(params);
    }
    
    if (parameterID == "delayTimeID2")
    {
        params.right_mSec = newValue;
        lcrDelay.setLCRParameters(params);
    }
    
    if (parameterID == "delayTimeID3")
    {
        params.center_mSec = newValue;
        lcrDelay.setLCRParameters(params);
    }
    
    if (parameterID == "delayFeedbackID")
    {
        params.feedback_Pct = newValue;
        lcrDelay.setLCRParameters(params);
    }
    
    if (parameterID == "wetLevelID")
    {
        params.wetLevel_dB = newValue;
        lcrDelay.setLCRParameters(params);
    }
    
    if (parameterID == "dryLevelID")
    {
        params.dryLevel_dB = newValue;
        lcrDelay.setLCRParameters(params);
    }
}

bool BasicLCRDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BasicLCRDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BasicLCRDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BasicLCRDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BasicLCRDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BasicLCRDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BasicLCRDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BasicLCRDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void BasicLCRDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BasicLCRDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    lcrDelay.reset(sampleRate);

    lcrDelay.createDelayBuffers(sampleRate, 2000);

    params.left_mSec = treeState.getRawParameterValue("delayTimeID1")->load();
    params.right_mSec = treeState.getRawParameterValue("delayTimeID2")->load();
    params.center_mSec = treeState.getRawParameterValue("delayTimeID3")->load();

    params.feedback_Pct = treeState.getRawParameterValue("delayFeedbackID")->load();

    params.dryLevel_dB = treeState.getRawParameterValue("dryLevelID")->load();
    params.wetLevel_dB = treeState.getRawParameterValue("wetLevelID")->load();
    
    lcrDelay.setLCRParameters(params);
}

void BasicLCRDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BasicLCRDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void BasicLCRDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    int numSamples = buffer.getNumSamples();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    for (int sample = 0; sample < numSamples; ++sample)
    {
        auto* xnL = buffer.getReadPointer(0);
        auto* ynL = buffer.getWritePointer(0);
        
        auto* xnR = buffer.getReadPointer(1);
        auto* ynR = buffer.getWritePointer(1);
        
        float inputs[2] = {xnL[sample], xnR[sample]};
        float outputs[2] = {0.0, 0.0};
        lcrDelay.processAudioFrame(inputs, outputs, 2, 2);
        
        ynL[sample] = outputs[0];
        ynR[sample] = outputs[1];
    }
}

//==============================================================================
bool BasicLCRDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BasicLCRDelayAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void BasicLCRDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BasicLCRDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicLCRDelayAudioProcessor();
}
