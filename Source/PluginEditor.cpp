#include "PluginEditor.h"

using namespace juce;

static const Colour kPink(0xffff2d95);

void HellLook::drawRotarySlider(Graphics& g, int x, int y, int w, int h, float pos,
                                float a0, float a1, Slider&)
{
    auto r = Rectangle<float>((float) x, (float) y, (float) w, (float) h).reduced(6.f);
    float d = jmin(r.getWidth(), r.getHeight());
    auto c = r.getCentre();
    float rad = d / 2.f;
    float ang = a0 + pos * (a1 - a0);

    g.setColour(Colour(0xff1b1320));
    g.fillEllipse(c.x - rad, c.y - rad, rad * 2, rad * 2);
    g.setColour(Colour(0xff3a2a44));
    g.drawEllipse(c.x - rad, c.y - rad, rad * 2, rad * 2, 2.f);

    Path arc;
    arc.addCentredArc(c.x, c.y, rad - 4, rad - 4, 0.f, a0, ang, true);
    g.setColour(kPink.withAlpha(0.35f));
    g.strokePath(arc, PathStrokeType(9.f, PathStrokeType::curved, PathStrokeType::rounded));
    g.setColour(kPink);
    g.strokePath(arc, PathStrokeType(4.f, PathStrokeType::curved, PathStrokeType::rounded));

    Path p;
    p.addRoundedRectangle(-2.f, -rad + 8, 4.f, rad * 0.45f, 2.f);
    g.setColour(Colours::white);
    g.fillPath(p, AffineTransform::rotation(ang).translated(c.x, c.y));
}

SakuraHellAudioProcessorEditor::SakuraHellAudioProcessorEditor(SakuraHellAudioProcessor& p)
    : AudioProcessorEditor(&p), proc(p)
{
    setSize(720, 820);
    setupKnob(crush,  "crush",  "PRESS  /  CRUSH", "");
    setupKnob(hell,   "hell",   "BLADE  /  HELL", "");
    setupKnob(pitch,  "pitch",  "SPIKES  /  PITCH", " st");
    setupKnob(doom,   "doom",   "ACID  /  DOOM", "");
    setupKnob(output, "output", "OUTPUT", " dB");
    startTimerHz(30);
}

SakuraHellAudioProcessorEditor::~SakuraHellAudioProcessorEditor()
{
    for (auto* k : { &pitch, &crush, &hell, &doom, &output }) k->s.setLookAndFeel(nullptr);
}

void SakuraHellAudioProcessorEditor::setupKnob(Knob& k, const String& id, const String& text, const String& suffix)
{
    k.s.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    k.s.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 18);
    k.s.setTextValueSuffix(suffix);
    k.s.setNumDecimalPlacesToDisplay(id == "pitch" || id == "output" ? 1 : 2);
    k.s.setLookAndFeel(&look);
    k.s.setColour(Slider::textBoxTextColourId, Colours::white);
    k.s.setColour(Slider::textBoxOutlineColourId, Colours::transparentBlack);
    addAndMakeVisible(k.s);
    k.l.setText(text, dontSendNotification);
    k.l.setJustificationType(Justification::centred);
    k.l.setFont(Font(FontOptions(13.f, Font::bold)));
    k.l.setColour(Label::textColourId, kPink);
    addAndMakeVisible(k.l);
    k.att = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(proc.apvts, id, k.s);
}

void SakuraHellAudioProcessorEditor::resized()
{
    const int y = 560, size = 130, labelH = 20;
    const int xs[4] = { 30, 205, 380, 555 };
    Knob* ks[4] = { &crush, &hell, &pitch, &doom };
    for (int i = 0; i < 4; ++i)
    {
        ks[i]->l.setBounds(xs[i] - 10, y, size + 20, labelH);
        ks[i]->s.setBounds(xs[i], y + labelH, size, size + 20);
    }
    output.l.setBounds(260, 725, 200, labelH);
    output.s.setBounds(310, 745, 100, 65);
}

void SakuraHellAudioProcessorEditor::spawn(float x, float y, float spread, float up, int count, bool acid)
{
    for (int i = 0; i < count && particles.size() < 700; ++i)
        particles.push_back({ x, y,
                              (rnd.nextFloat() - 0.5f) * spread,
                              -up * (0.4f + rnd.nextFloat()),
                              1.f, 2.f + rnd.nextFloat() * 5.f, acid });
}

void SakuraHellAudioProcessorEditor::timerCallback()
{
    const float target = jlimit(0.f, 1.f, proc.level.load() * 1.6f);
    smile += (target - smile) * (target > smile ? 0.5f : 0.12f);

    const float press = (float) crush.s.getValue();
    const float blade = (float) hell.s.getValue();
    const float spikes = jlimit(0.f, 1.f, std::abs((float) pitch.s.getValue()) / 24.f);
    const float acidV = (float) doom.s.getValue();

    const float cx = 360.f, floorY = 90.f + 420.f - 30.f;
    const float scaleY = 1.f - 0.85f * press;

    if (press > 0.05f)
        spawn(cx + (rnd.nextFloat() - 0.5f) * 100.f, floorY - 12.f, 16.f * press, 10.f * press, (int) (press * 7));
    if (blade > 0.5f)
        spawn(cx, floorY - 160.f, 12.f, 11.f * blade, (int) ((blade - 0.5f) * 14));
    if (spikes > 0.08f)
        spawn(cx + (rnd.nextFloat() - 0.5f) * 80.f, floorY - spikes * 60.f, 8.f, 8.f * spikes, (int) (spikes * 5));
    if (acidV > 0.04f)
        spawn(cx + (rnd.nextFloat() - 0.5f) * acidV * 50.f, 100.f, 2.f, -1.f, (int) (acidV * 4) + 1, true);

    for (auto& p : particles)
    {
        p.x += p.vx; p.y += p.vy; p.vy += 0.55f; p.life -= 0.01f;
        if (p.y >= floorY - 4.f) { pool += 0.03f * p.size; p.life = 0.f; }
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(),
                                   [](const Particle& p) { return p.life <= 0.f; }), particles.end());

    pool = jlimit(0.f, 26.f, pool - 0.04f);
    shake = (press > 0.7f || blade > 0.5f) ? (rnd.nextFloat() - 0.5f) * 2.5f * jmax(press, blade) : 0.f;
    repaint();
}

void SakuraHellAudioProcessorEditor::drawBackgroundMask(Graphics& g)
{
    g.fillAll(Colour(0xff09050d));
    const float W = (float) getWidth();
    const float cx = W / 2.f;

    // big face of the mask
    ColourGradient grad(Colour(0xff2a2140), cx, 250.f, Colour(0xff120b1c), cx, 760.f, false);
    g.setGradientFill(grad);
    g.fillEllipse(cx - 340.f, 10.f, 680.f, 800.f);
    g.setColour(Colour(0xff4a3a66));
    g.drawEllipse(cx - 340.f, 10.f, 680.f, 800.f, 3.f);

    // eyes (narrow noh-style slits)
    g.setColour(Colour(0xff050207));
    for (int s = -1; s <= 1; s += 2)
    {
        Path eye;
        eye.startNewSubPath(cx + s * 70.f, 250.f);
        eye.quadraticTo(cx + s * 150.f, 215.f, cx + s * 230.f, 255.f);
        eye.quadraticTo(cx + s * 150.f, 240.f, cx + s * 70.f, 250.f);
        g.fillPath(eye);
    }

    // cheeks
    g.setColour(kPink.withAlpha(0.10f + 0.15f * smile));
    g.fillEllipse(cx - 290.f, 420.f, 130.f, 80.f);
    g.fillEllipse(cx + 160.f, 420.f, 130.f, 80.f);

    // mouth: the louder, the wider the smile
    const float w = 70.f + smile * 215.f;
    const float yM = 640.f;
    const float depth = 10.f + smile * 150.f;
    Path mouth;
    mouth.startNewSubPath(cx - w, yM - smile * 40.f);
    mouth.quadraticTo(cx, yM - 8.f, cx + w, yM - smile * 40.f);
    mouth.quadraticTo(cx, yM + depth * 1.6f, cx - w, yM - smile * 40.f);
    mouth.closeSubPath();

    g.setColour(Colour(0xff2a0010));
    g.fillPath(mouth);
    {
        Graphics::ScopedSaveState ss(g);
        g.reduceClipRegion(mouth);
        g.setColour(Colour(0xffeee8f5));
        const int teeth = 14;
        for (int i = 0; i < teeth; ++i)
        {
            float tx = cx - w + (2.f * w) * i / teeth;
            g.fillRect(tx + 2.f, yM - 50.f, 2.f * w / teeth - 4.f, 16.f + smile * 26.f);
        }
    }
    g.setColour(kPink.withAlpha(0.35f));
    g.strokePath(mouth, PathStrokeType(9.f));
    g.setColour(kPink);
    g.strokePath(mouth, PathStrokeType(3.f));
}

void SakuraHellAudioProcessorEditor::drawCharacter(Graphics& g, float cx, float floorY, float scaleY,
                                                   float lift, float headDX, float headDY, float headRot)
{
    Graphics::ScopedSaveState ss(g);
    g.addTransform(AffineTransform::scale(1.f, scaleY, cx, floorY).translated(0.f, -lift));
    auto P = [&](float x, float y) { return Point<float>(cx + x, floorY + y); };

    // shoes
    g.setColour(Colour(0xff0d0d0d));
    g.fillRoundedRectangle(cx - 36, floorY - 9, 28, 9, 3);
    g.fillRoundedRectangle(cx + 8, floorY - 9, 28, 9, 3);

    // flared jeans
    g.setColour(Colour(0xff5b7fa8));
    for (int s = -1; s <= 1; s += 2)
    {
        Path leg;
        leg.startNewSubPath(P(s * 1.f, -96)); leg.lineTo(P(s * 21.f, -96));
        leg.lineTo(P(s * 36.f, -9)); leg.lineTo(P(s * 7.f, -9));
        leg.closeSubPath();
        g.fillPath(leg);
    }

    // arms
    g.setColour(Colour(0xff111111));
    Path armL; armL.startNewSubPath(P(-26, -152)); armL.lineTo(P(-46, -122)); armL.lineTo(P(-34, -98));
    Path armR; armR.startNewSubPath(P(26, -152));  armR.lineTo(P(52, -178));  armR.lineTo(P(36, -200));
    g.strokePath(armL, PathStrokeType(11.f, PathStrokeType::curved, PathStrokeType::rounded));
    g.strokePath(armR, PathStrokeType(11.f, PathStrokeType::curved, PathStrokeType::rounded));
    g.setColour(Colour(0xffe8c4a8));
    g.fillEllipse(P(-34, -98).x - 5, P(-34, -98).y - 5, 10, 10);
    g.fillEllipse(P(36, -200).x - 5, P(36, -200).y - 5, 10, 10);

    // jacket + striped tee + belt
    g.setColour(Colour(0xff111111));
    g.fillRoundedRectangle(cx - 27, floorY - 162, 54, 68, 8);
    g.setColour(Colour(0xff9fc3df));
    g.fillRect(cx - 6.f, floorY - 162.f, 12.f, 24.f);
    g.setColour(Colour(0xff1d2f40));
    for (int i = 0; i < 5; ++i) g.fillRect(cx - 6, floorY - 160 + i * 5.f, 12.f, 1.5f);
    g.setColour(Colour(0xffb02a4a));
    g.fillRect(cx - 24.f, floorY - 100.f, 48.f, 7.f);

    // head with mask (can detach)
    {
        Graphics::ScopedSaveState hs(g);
        g.addTransform(AffineTransform::rotation(headRot, cx, floorY - 162).translated(headDX, headDY));
        g.setColour(Colour(0xff4a3426));
        g.fillEllipse(cx - 22, floorY - 208, 44, 50);          // hair
        g.setColour(Colour(0xffdfe6f5));
        g.fillEllipse(cx - 17, floorY - 203, 34, 42);          // mask
        g.setColour(Colour(0xffff9f6a).withAlpha(0.6f));
        g.fillEllipse(cx - 15, floorY - 184, 9, 8);
        g.fillEllipse(cx + 6, floorY - 184, 9, 8);
        g.setColour(Colour(0xff222222));
        g.drawLine(cx - 11, floorY - 188, cx - 4, floorY - 188, 1.5f);
        g.drawLine(cx + 4, floorY - 188, cx + 11, floorY - 188, 1.5f);
        g.setColour(Colour(0xffd01030));
        g.fillEllipse(cx - 5, floorY - 174, 10, 5);             // lips
        g.drawLine(cx + 15, floorY - 192, cx + 18, floorY - 166, 2.f);  // red tassel
    }
}

void SakuraHellAudioProcessorEditor::drawStage(Graphics& g)
{
    const Rectangle<float> stage(30.f, 90.f, 660.f, 420.f);
    const float cx = stage.getCentreX();
    const float floorY = stage.getBottom() - 30.f;
    const float press = (float) crush.s.getValue();
    const float blade = (float) hell.s.getValue();
    const float spikes = jlimit(0.f, 1.f, std::abs((float) pitch.s.getValue()) / 24.f);
    const float acidV = (float) doom.s.getValue();

    g.setColour(Colour(0xff0b0710).withAlpha(0.72f));
    g.fillRoundedRectangle(stage, 10.f);
    g.setColour(Colour(0xff3a2a44));
    g.drawRoundedRectangle(stage, 10.f, 2.f);

    Graphics::ScopedSaveState ss(g);
    g.reduceClipRegion(stage.toNearestInt());
    g.addTransform(AffineTransform::translation(shake, 0.f));

    // floor
    g.setColour(Colour(0xff2a2230));
    g.fillRect(stage.getX(), floorY, stage.getWidth(), 30.f);

    const float scaleY = 1.f - 0.85f * press;
    const float spikeH = spikes * 60.f;
    const float lift = spikeH * 0.9f;

    // blade progress
    const float bladeIn = jmin(1.f, blade * 2.f);
    const float cut = jlimit(0.f, 1.f, (blade - 0.5f) * 2.f);
    const float headDX = cut * 70.f;
    const float headDY = jmin(150.f, cut * cut * 190.f);
    const float headRot = cut * 1.3f;

    drawCharacter(g, cx, floorY, scaleY, lift, headDX, headDY, headRot);

    // acid overlay
    if (acidV > 0.01f)
    {
        g.setColour(acidPink.withAlpha(acidV * 0.55f));
        g.fillRoundedRectangle(cx - 34, floorY - 205.f * scaleY - lift, 68.f, 205.f * scaleY, 14.f);
        g.setColour(acidPink.withAlpha(0.85f));
        float sw = 4.f + acidV * 26.f;
        g.fillRoundedRectangle(cx - sw / 2, stage.getY(), sw, floorY - 205.f * scaleY - lift - stage.getY() + 20.f, sw / 2);
        // pipe
        g.setColour(Colour(0xff555060));
        g.fillRect(cx - 28, stage.getY(), 56.f, 14.f);
    }

    // press
    {
        const float topY = stage.getY() + 20.f;
        const float charTop = floorY - 215.f * scaleY - lift;
        const float t = jlimit(0.f, 1.f, press * 6.f);
        const float plateBottom = topY * (1.f - t) + charTop * t;
        g.setColour(Colour(0xff6b6677));
        g.fillRect(cx - 12, stage.getY(), 24.f, jmax(0.f, plateBottom - 26.f - stage.getY()));
        ColourGradient pg(Colour(0xffb9b4c4), cx, plateBottom - 26.f, Colour(0xff5b5668), cx, plateBottom, false);
        g.setGradientFill(pg);
        g.fillRoundedRectangle(cx - 105, plateBottom - 26.f, 210.f, 26.f, 4.f);
        g.setColour(Colour(0xff2b2733));
        for (int i = 0; i < 9; ++i) g.fillRect(cx - 98 + i * 24.f, plateBottom - 6.f, 12.f, 6.f);
    }

    // blade
    if (blade > 0.01f)
    {
        const float by = floorY - 164.f * scaleY - lift;
        const float bx = jmap(bladeIn, stage.getX() - 150.f, cx + 4.f);
        Path b;
        b.startNewSubPath(bx - 240.f, by - 7); b.lineTo(bx, by - 7);
        b.lineTo(bx + 26.f, by + 1); b.lineTo(bx, by + 9); b.lineTo(bx - 240.f, by + 9);
        b.closeSubPath();
        g.setColour(Colour(0xffcfd0da)); g.fillPath(b);
        g.setColour(Colours::white.withAlpha(0.8f)); g.drawLine(bx - 240.f, by - 4, bx + 20.f, by - 1, 1.5f);
    }

    // spikes
    if (spikes > 0.01f)
    {
        g.setColour(Colour(0xff9a97a6));
        for (int i = -5; i <= 5; ++i)
        {
            Path s;
            float sx = cx + i * 14.f;
            s.startNewSubPath(sx - 7, floorY); s.lineTo(sx, floorY - spikeH - (i % 2 == 0 ? 8.f : 0.f)); s.lineTo(sx + 7, floorY);
            s.closeSubPath();
            g.fillPath(s);
        }
    }

    // pool of neon pink blood
    if (pool > 0.5f)
    {
        g.setColour(kPink);
        g.fillRoundedRectangle(cx - 170.f, floorY - pool, 340.f, pool + 4.f, 8.f);
        g.setColour(Colours::white.withAlpha(0.25f));
        g.fillRoundedRectangle(cx - 150.f, floorY - pool + 3.f, 120.f, 3.f, 2.f);
    }

    // particles
    for (auto& p : particles)
    {
        g.setColour((p.acid ? acidPink : kPink).withAlpha(jlimit(0.2f, 1.f, p.life * 1.5f)));
        g.fillEllipse(p.x - p.size / 2, p.y - p.size / 2, p.size, p.size * (p.acid ? 1.6f : 1.f));
    }
}

void SakuraHellAudioProcessorEditor::paint(Graphics& g)
{
    drawBackgroundMask(g);

    g.setColour(kPink);
    g.setFont(Font(FontOptions(40.f, Font::bold)));
    g.drawText("SAKURAHELL", 30, 24, 660, 50, Justification::centredLeft);
    g.setColour(Colours::white.withAlpha(0.5f));
    g.setFont(Font(FontOptions(12.f)));
    g.drawText("EXECUTION UNIT  //  LOUDER = WIDER SMILE", 30, 62, 660, 20, Justification::centredLeft);

    drawStage(g);
}
