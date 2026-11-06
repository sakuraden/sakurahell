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
    struct Decal { float x, y, size, alpha; bool wall; };
    struct Petal { float x, y, vx, vy, rot, vr, size; };
    struct Expr { float fear = 0.f; int kind = 0; bool dead = false; float cut = 0.f; };

    void setupKnob(Knob&, const juce::String& id, const juce::String& text, const juce::String& suffix);
    void timerCallback() override;
    void spawn(float x, float y, float spread, float up, int count, bool acid = false);
    Expr currentExpr() const;
    juce::Rectangle<float> stageRect() const { return { 30.f, 96.f, 660.f, 400.f }; }

    void drawBackgroundMask(juce::Graphics&);
    void drawPetals(juce::Graphics&);
    void drawHeader(juce::Graphics&);
    void drawStage(juce::Graphics&);
    void drawDeck(juce::Graphics&);
    void drawFace(juce::Graphics&, const Expr&);
    void drawCharacter(juce::Graphics&, float cx, float floorY, float scaleY, float lift,
                       float headDX, float headDY, float headRot, const Expr&);
    void drawPortrait(juce::Graphics&, juce::Rectangle<float>, const Expr&);
    void drawSpeaker(juce::Graphics&, juce::Rectangle<float>, bool facingRight);
    void drawLedPanel(juce::Graphics&, juce::Rectangle<float>);
    void drawHazard(juce::Graphics&, juce::Rectangle<float>);
    void drawChain(juce::Graphics&, float x, float y0, float y1, float phase);

    SakuraHellAudioProcessor& proc;
    HellLook look;
    Knob pitch, crush, hell, doom, output;

    std::vector<Particle> particles;
    std::vector<Decal> decals;
    std::vector<Petal> petals;
    juce::Random rnd;
    int tick = 0;
    float smile = 0.f;
    float pool = 0.f;
    float shake = 0.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SakuraHellAudioProcessorEditor)
};
