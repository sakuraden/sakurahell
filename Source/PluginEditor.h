#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class HellLook : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos,
                          float a0, float a1, juce::Slider&) override;
};

class SakuraHellAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit SakuraHellAudioProcessorEditor(SakuraHellAudioProcessor&);
    ~SakuraHellAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    struct Knob
    {
        juce::Slider s;
        juce::Label l;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
    };
    struct Particle { float x, y, vx, vy, life, size; bool acid; };

    void setupKnob(Knob&, const juce::String& id, const juce::String& text, const juce::String& suffix);
    void timerCallback() override;
    void spawn(float x, float y, float spread, float up, int count, bool acid = false);

    void drawBackgroundMask(juce::Graphics&);
    void drawStage(juce::Graphics&);
    void drawCharacter(juce::Graphics&, float cx, float floorY, float scaleY, float lift,
                       float headDX, float headDY, float headRot);

    SakuraHellAudioProcessor& proc;
    HellLook look;
    Knob pitch, crush, hell, doom, output;

    std::vector<Particle> particles;
    juce::Random rnd;
    float smile = 0.f;      // smoothed loudness 0..1 -> mask smile
    float pool = 0.f;       // pink blood on the floor
    float shake = 0.f;

    const juce::Colour pink { 0xffff2d95 };
    const juce::Colour acidPink { 0xffff7ad9 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SakuraHellAudioProcessorEditor)
};
