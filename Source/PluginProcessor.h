/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Fifo/Fifo.h"

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

// biquad filter, which is a 2nd-order IIR filter.
using Filter = juce::dsp::IIR::Filter<float>;

//When you design a 4th-order filter, it doesn't mean that you're stacking filters with increasing orders (like 1st, 2nd, 3rd, and 4th). Instead, you break the higher-order filter into multiple 2nd-order sections (biquads). This is a standard way to design high-order filters.
//Let's say you select 24 dB/octave in your filter slope selector, which corresponds to a 4th-order Butterworth filter:
//You will get two sets of coefficients from the Butterworth design function in JUCE. These coefficients define two biquad filters (2nd-order filters).
//You then apply these two filters in series, meaning the audio signal first passes through the first 2nd-order filter and then through the second 2nd-order filter.
//This cascading creates the desired 4th-order response, or a 24 dB/octave roll-off.

using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

using Coefficients = Filter::CoefficientsPtr;
void updateCoefficients(Coefficients& old, const Coefficients& replacements);

Coefficients makePeakFilter(const ChainSettings& chainSettings, double sampleRate);

template<int Index, typename ChainType, typename CoefficientType>
void update(ChainType& chain, const CoefficientType& coefficients)
{
    updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
    chain.template setBypassed<Index>(false);
}

template<typename ChainType, typename CoefficientType>
void updateCutFilter(ChainType& chain,
                     const CoefficientType& coefficients,
                     const Slope& slope)
{
    chain.template setBypassed<0>(true);
    chain.template setBypassed<1>(true);
    chain.template setBypassed<2>(true);
    chain.template setBypassed<3>(true);
    
    switch( slope )
    {
        case Slope_48:
        {
            update<3>(chain, coefficients);
        }
        case Slope_36:
        {
            update<2>(chain, coefficients);
        }
        case Slope_24:
        {
            update<1>(chain, coefficients);
        }
        case Slope_12:
        {
            update<0>(chain, coefficients);
        }
    }
}

inline auto makeLowCutFilter(const ChainSettings& chainSettings, double sampleRate )
{
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,
                                                                                       sampleRate,
                                                                                       2 * (chainSettings.lowCutSlope + 1));
}

inline auto makeHighCutFilter(const ChainSettings& chainSettings, double sampleRate )
{
    return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq,
                                                                                      sampleRate,
                                                                                      2 * (chainSettings.highCutSlope + 1));
}
//==============================================================================
/**
*/
class SimpleEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SimpleEQAudioProcessor();
    ~SimpleEQAudioProcessor() override;

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

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameters", createParameterLayout()};
    
    using BlockType = juce::AudioBuffer<float>;
    SingleChannelSampleFifo<BlockType> leftChannelFifo { Channel::Left };
    SingleChannelSampleFifo<BlockType> rightChannelFifo { Channel::Right };
private:
    MonoChain leftChain, rightChain;
    
    void updatePeakFilter(const ChainSettings& chainSettings);

    
    
    
    void updateLowCutFilters(const ChainSettings& chainSettings);
    void updateHighCutFilters(const ChainSettings& chainSettings);
    
    void updateFilters();
    
    juce::dsp::Oscillator<float> osc;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessor)
};
