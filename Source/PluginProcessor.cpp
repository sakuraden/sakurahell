#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout SakuraHellAudioProcessor::createLayout()
{
    using P = juce::AudioParameterFloat;
    using R = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    l.add(std::make_unique<P>(juce::ParameterID{ "pitch", 1 },  "Pitch",  R(-24.f, 12.f, 0.01f), 0.f));
    l.add(std::make_unique<P>(juce::ParameterID{ "crush", 1 },  "Crush",  R(0.f, 1.f, 0.001f), 0.f));
    l.add(std::make_unique<P>(juce::ParameterID{ "hell", 1 },   "Hell",   R(0.f, 1.f, 0.001f), 0.f));
    l.add(std::make_unique<P>(juce::ParameterID{ "doom", 1 },   "Doom",   R(0.f, 1.f, 0.001f), 0.f));
    l.add(std::make_unique<P>(juce::ParameterID{ "output", 1 }, "Output", R(-24.f, 6.f, 0.1f), 0.f));
    return l;
}

SakuraHellAudioProcessor::SakuraHellAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "STATE", createLayout())
{
}

bool SakuraHellAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    return l.getMainOutputChannelSet() == l.getMainInputChannelSet()
        && (l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
         || l.getMainOutputChannelSet() == juce::AudioChannelSet::mono());
}

void SakuraHellAudioProcessor::prepareToPlay(double sampleRate, int)
{
    sr = sampleRate;
    for (auto& s : shifter) { s.prepare(); s.grain = (float) (sr * 0.055); }
    reverb.reset();
    reverb.setSampleRate(sr);
}

void SakuraHellAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int nCh = juce::jmin(2, buffer.getNumChannels());
    const int n = buffer.getNumSamples();

    const float pitchSt = apvts.getRawParameterValue("pitch")->load();
    const float crush   = apvts.getRawParameterValue("crush")->load();
    const float hell    = apvts.getRawParameterValue("hell")->load();
    const float doom    = apvts.getRawParameterValue("doom")->load();
    const float outDb   = apvts.getRawParameterValue("output")->load();

    const float ratio  = std::pow(2.f, pitchSt / 12.f);
    const float levels = std::pow(2.f, (16.f - crush * 14.f) - 1.f);
    const int   hold   = 1 + (int) (crush * 40.f);
    const float drive  = 1.f + hell * 40.f;
    const float ringMix = hell * 0.6f;
    const float ringInc = juce::MathConstants<float>::twoPi * (70.f + hell * 200.f) / (float) sr;
    const float outGain = juce::Decibels::decibelsToGain(outDb);
    const bool pitchOn = std::abs(pitchSt) > 0.01f;

    float phaseEnd = ringPhase;
    for (int ch = 0; ch < nCh; ++ch)
    {
        auto* d = buffer.getWritePointer(ch);
        float ph = ringPhase;
        for (int i = 0; i < n; ++i)
        {
            float x = d[i];
            float shifted = shifter[ch].process(x, pitchOn ? ratio : 1.f);
            x = pitchOn ? shifted : x;

            if (holdCount[ch] <= 0) { held[ch] = std::round(x * levels) / levels; holdCount[ch] = hold; }
            --holdCount[ch];
            x = held[ch];

            float dist = std::tanh(x * drive);
            float rm = dist * std::sin(ph);
            x = (dist * (1.f - ringMix) + rm * ringMix) / (1.f + hell * 0.3f);
            ph += ringInc;
            if (ph > juce::MathConstants<float>::twoPi) ph -= juce::MathConstants<float>::twoPi;

            d[i] = x;
        }
        phaseEnd = ph;
    }
    ringPhase = phaseEnd;

    juce::Reverb::Parameters rp;
    rp.roomSize = 0.6f + doom * 0.4f;
    rp.damping = 0.25f;
    rp.wetLevel = doom * 0.55f;
    rp.dryLevel = 1.f - doom * 0.35f;
    rp.width = 1.f;
    reverb.setParameters(rp);
    if (nCh == 2) reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), n);
    else if (nCh == 1) reverb.processMono(buffer.getWritePointer(0), n);

    buffer.applyGain(outGain);

    float peak = 0.f;
    for (int ch = 0; ch < nCh; ++ch) peak = juce::jmax(peak, buffer.getMagnitude(ch, 0, n));
    level.store(juce::jmax(peak, level.load() * 0.9f));
}

juce::AudioProcessorEditor* SakuraHellAudioProcessor::createEditor()
{
    return new SakuraHellAudioProcessorEditor(*this);
}

void SakuraHellAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary(*xml, dest);
}

void SakuraHellAudioProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SakuraHellAudioProcessor();
}
