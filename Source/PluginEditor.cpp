#include "PluginEditor.h"

using namespace juce;

namespace
{
const Colour kPink(0xffff2d95);
const Colour kPinkSoft(0xffff7ad9);
const Colour kLips(0xffd4173f);
constexpr float K = 1.2f;   // character scale

inline Rectangle<float> R(float x, float y, float w, float h) { return { x, y, w, h }; }
inline Point<float> polar(Point<float> c, float r, float a) { return { c.x + r * std::sin(a), c.y - r * std::cos(a) }; }
}

//==============================================================================
void HellLook::drawRotarySlider(Graphics& g, int x, int y, int w, int h, float pos,
                                float a0, float a1, Slider&)
{
    auto area = Rectangle<float>((float) x, (float) y, (float) w, (float) h).reduced(4.f);
    const float rad = jmin(area.getWidth(), area.getHeight()) / 2.f;
    const auto c = area.getCentre();
    const float ang = a0 + pos * (a1 - a0);
    const float arcR = rad - 4.f;
    const float body = rad * 0.72f;

    Path track;
    track.addCentredArc(c.x, c.y, arcR, arcR, 0.f, a0, a1, true);
    g.setColour(Colour(0xff2a1f33));
    g.strokePath(track, PathStrokeType(7.f, PathStrokeType::curved, PathStrokeType::rounded));

    Path arc;
    arc.addCentredArc(c.x, c.y, arcR, arcR, 0.f, a0, jmax(a0 + 0.001f, ang), true);
    g.setColour(kPink.withAlpha(0.25f));
    g.strokePath(arc, PathStrokeType(13.f, PathStrokeType::curved, PathStrokeType::rounded));
    g.setColour(kPink);
    g.strokePath(arc, PathStrokeType(5.f, PathStrokeType::curved, PathStrokeType::rounded));

    g.setColour(Colour(0xff6b5f78));
    for (int i = 0; i <= 10; ++i)
    {
        float a = a0 + (a1 - a0) * (float) i / 10.f;
        auto p1 = polar(c, body + 3.f, a);
        auto p2 = polar(c, body + 8.f, a);
        g.drawLine(p1.x, p1.y, p2.x, p2.y, 1.5f);
    }

    g.setColour(Colours::black.withAlpha(0.5f));
    g.fillEllipse(c.x - body + 2.f, c.y - body + 4.f, body * 2.f, body * 2.f);
    ColourGradient mg(Colour(0xff5a5068), c.x - body * 0.5f, c.y - body * 0.6f,
                      Colour(0xff130e18), c.x + body, c.y + body, true);
    g.setGradientFill(mg);
    g.fillEllipse(c.x - body, c.y - body, body * 2.f, body * 2.f);
    g.setColour(Colour(0xff0a0710));
    g.drawEllipse(c.x - body, c.y - body, body * 2.f, body * 2.f, 2.f);

    g.setColour(Colours::black.withAlpha(0.35f));
    for (int i = 0; i < 28; ++i)
    {
        float a = (float) i * MathConstants<float>::twoPi / 28.f;
        auto p1 = polar(c, body - 5.f, a);
        auto p2 = polar(c, body - 0.5f, a);
        g.drawLine(p1.x, p1.y, p2.x, p2.y, 1.f);
    }
    g.setColour(Colour(0xff1d1624));
    g.fillEllipse(c.x - body * 0.55f, c.y - body * 0.55f, body * 1.1f, body * 1.1f);

    auto q1 = polar(c, body * 0.25f, ang);
    auto q2 = polar(c, body * 0.9f, ang);
    g.setColour(kPink.withAlpha(0.35f));
    g.drawLine(q1.x, q1.y, q2.x, q2.y, 6.f);
    g.setColour(Colours::white);
    g.drawLine(q1.x, q1.y, q2.x, q2.y, 2.5f);
}

//==============================================================================
SakuraHellAudioProcessorEditor::SakuraHellAudioProcessorEditor(SakuraHellAudioProcessor& p)
    : AudioProcessorEditor(&p), proc(p)
{
    setSize(720, 880);
    setupKnob(crush,  "crush",  "PRESS  /  CRUSH", "");
    setupKnob(hell,   "hell",   "BLADE  /  HELL", "");
    setupKnob(pitch,  "pitch",  "SPIKES  /  PITCH", " st");
    setupKnob(doom,   "doom",   "ACID  /  DOOM", "");
    setupKnob(output, "output", "OUTPUT", " dB");

    for (int i = 0; i < 38; ++i)
        petals.push_back({ rnd.nextFloat() * 720.f, rnd.nextFloat() * 880.f,
                           (rnd.nextFloat() - 0.4f) * 0.8f, 0.6f + rnd.nextFloat() * 1.2f,
                           rnd.nextFloat() * 6.28f, (rnd.nextFloat() - 0.5f) * 0.1f,
                           0.7f + rnd.nextFloat() * 0.9f });
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
    const int y = 548, size = 130;
    const int xs[4] = { 30, 205, 380, 555 };
    Knob* ks[4] = { &crush, &hell, &pitch, &doom };
    for (int i = 0; i < 4; ++i)
    {
        ks[i]->l.setBounds(xs[i] - 10, y, size + 20, 20);
        ks[i]->s.setBounds(xs[i], y + 20, size, size + 20);
    }
    output.l.setBounds(270, 718, 180, 20);
    output.s.setBounds(300, 738, 120, 120);
}

//==============================================================================
SakuraHellAudioProcessorEditor::Expr SakuraHellAudioProcessorEditor::currentExpr() const
{
    Expr e;
    const float p = (float) crush.s.getValue();
    const float b = (float) hell.s.getValue();
    const float s = jlimit(0.f, 1.f, std::abs((float) pitch.s.getValue()) / 24.f);
    const float a = (float) doom.s.getValue();
    const float m = jmax(jmax(p, b), jmax(s, a));
    e.fear = jlimit(0.f, 1.f, m * 1.3f);
    if (m > 0.03f)
    {
        if (m == p) e.kind = 1;
        else if (m == b) e.kind = 2;
        else if (m == s) e.kind = 3;
        else e.kind = 4;
    }
    e.cut = jlimit(0.f, 1.f, (b - 0.5f) * 2.f);
    e.dead = (p > 0.93f) || (b > 0.55f) || (s > 0.9f) || (a > 0.92f);
    return e;
}

void SakuraHellAudioProcessorEditor::spawn(float x, float y, float spread, float up, int count, bool acid)
{
    for (int i = 0; i < count && particles.size() < 800; ++i)
        particles.push_back({ x, y,
                              (rnd.nextFloat() - 0.5f) * spread,
                              -up * (0.4f + rnd.nextFloat()),
                              1.f, 2.f + rnd.nextFloat() * 5.f, acid });
}

void SakuraHellAudioProcessorEditor::timerCallback()
{
    ++tick;
    const float target = jlimit(0.f, 1.f, proc.level.load() * 1.6f);
    smile += (target - smile) * (target > smile ? 0.5f : 0.12f);

    const float press = (float) crush.s.getValue();
    const float blade = (float) hell.s.getValue();
    const float spikes = jlimit(0.f, 1.f, std::abs((float) pitch.s.getValue()) / 24.f);
    const float acidV = (float) doom.s.getValue();

    const auto st = stageRect();
    const float cx = st.getCentreX();
    const float floorY = st.getBottom() - 36.f;

    if (press > 0.05f)
        spawn(cx + (rnd.nextFloat() - 0.5f) * 120.f, floorY - 14.f, 18.f * press, 11.f * press, (int) (press * 8));
    if (blade > 0.5f)
        spawn(cx, floorY - 164.f * K, 12.f, 11.f * blade, (int) ((blade - 0.5f) * 14));
    if (spikes > 0.08f)
        spawn(cx + (rnd.nextFloat() - 0.5f) * 90.f, floorY - spikes * 62.f, 8.f, 8.f * spikes, (int) (spikes * 5));
    if (acidV > 0.04f)
        spawn(cx + (rnd.nextFloat() - 0.5f) * acidV * 50.f, st.getY() + 20.f, 2.f, -1.f, (int) (acidV * 4) + 1, true);

    for (auto& p : particles)
    {
        p.x += p.vx; p.y += p.vy; p.vy += 0.55f; p.life -= 0.01f;
        if (p.y >= floorY - 4.f)
        {
            pool += 0.03f * p.size;
            if (decals.size() < 160)
                decals.push_back({ p.x, floorY + 3.f + rnd.nextFloat() * 22.f, p.size * 1.2f, 1.f, false });
            p.life = 0.f;
        }
        else if (!p.acid && p.life < 0.9f && p.y < floorY - 20.f && rnd.nextFloat() < 0.012f && decals.size() < 160)
        {
            decals.push_back({ p.x, p.y, p.size * 1.5f, 1.f, true });
            p.life = 0.f;
        }
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(),
                                   [](const Particle& p) { return p.life <= 0.f; }), particles.end());

    for (auto& d : decals) d.alpha -= 0.0012f;
    decals.erase(std::remove_if(decals.begin(), decals.end(),
                                [](const Decal& d) { return d.alpha <= 0.f; }), decals.end());

    for (auto& p : petals)
    {
        p.y += p.vy * (1.f + smile * 2.f);
        p.x += p.vx + std::sin(p.y * 0.02f + p.rot) * 0.6f;
        p.rot += p.vr;
        if (p.y > 892.f) { p.y = -12.f; p.x = rnd.nextFloat() * 720.f; }
    }

    pool = jlimit(0.f, 26.f, pool - 0.04f);
    shake = (press > 0.7f || blade > 0.5f) ? (rnd.nextFloat() - 0.5f) * 2.5f * jmax(press, blade) : 0.f;
    repaint();
}

//==============================================================================
void SakuraHellAudioProcessorEditor::drawBackgroundMask(Graphics& g)
{
    const float W = (float) getWidth();
    const float cx = W * 0.5f;
    const float t = (float) tick;

    g.fillAll(Colour(0xff08040b));
    {
        ColourGradient aura(kPink.withAlpha(0.18f + 0.25f * smile), cx, 420.f,
                            Colours::transparentBlack, cx, 1000.f, true);
        g.setGradientFill(aura);
        g.fillRect(getLocalBounds());
    }

    // messy dark hair behind the mask
    g.setColour(Colour(0xff1a110e));
    g.fillEllipse(cx - 365.f, -30.f, 730.f, 880.f);
    for (int i = 0; i < 18; ++i)
    {
        float a = -2.9f + (float) i * 0.17f;
        float rx = cx + std::cos(a) * 365.f;
        float ry = 410.f + std::sin(a) * 440.f + std::sin(t * 0.05f + (float) i) * 3.f;
        g.fillEllipse(rx - 30.f, ry - 30.f, 60.f, 70.f);
    }
    for (int s = -1; s <= 1; s += 2)
        for (int i = 0; i < 4; ++i)
            g.fillEllipse(cx + (float) s * 345.f - 35.f, 420.f + (float) i * 110.f, 70.f, 150.f);

    // red strings
    for (int s = -1; s <= 1; s += 2)
    {
        float sw = std::sin(t * 0.04f + (float) s) * 6.f;
        Path str;
        str.startNewSubPath(cx + (float) s * 300.f, 250.f);
        str.quadraticTo(cx + (float) s * 345.f + sw, 520.f, cx + (float) s * 330.f + sw * 1.5f, 800.f);
        g.setColour(Colour(0xffd4172f));
        g.strokePath(str, PathStrokeType(6.f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setColour(Colour(0xffff6a7f));
        g.strokePath(str, PathStrokeType(1.8f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setColour(kLips);
        g.fillEllipse(cx + (float) s * 330.f + sw * 1.5f - 8.f, 792.f, 16.f, 16.f);
    }

    // the mask face
    const float fy = 30.f, fh = 830.f, fw = 640.f;
    Path face;
    face.addEllipse(cx - fw / 2.f, fy, fw, fh);
    ColourGradient fg(Colour(0xfff3f4fb), cx - 100.f, fy + 120.f, Colour(0xff98a3c8), cx + 200.f, fy + fh, false);
    g.setGradientFill(fg);
    g.fillPath(face);
    ColourGradient sh(Colours::transparentBlack, cx, 420.f, Colour(0xff4a5280).withAlpha(0.5f), cx + 330.f, 420.f, true);
    g.setGradientFill(sh);
    g.fillPath(face);
    g.setColour(Colour(0xff6f7aa0));
    g.strokePath(face, PathStrokeType(3.f));

    // high painted brows (noh style)
    g.setColour(Colour(0xff2a1d26).withAlpha(0.9f));
    for (int s = -1; s <= 1; s += 2)
    {
        Path b;
        b.addEllipse(-34.f, -9.f, 68.f, 18.f);
        g.fillPath(b, AffineTransform::rotation((float) s * 0.2f)
                          .translated(cx + (float) s * 125.f, 150.f - smile * 14.f));
    }

    // eyes: narrow crescents, happier when loud
    for (int s = -1; s <= 1; s += 2)
    {
        const float ex = cx + (float) s * 150.f;
        g.setColour(kPink.withAlpha(0.22f));
        g.fillEllipse(ex - 90.f, 250.f - smile * 30.f, 180.f, 40.f);
        Path eye;
        eye.startNewSubPath(ex - 80.f * 1.f, 310.f);
        eye.quadraticTo(ex, 262.f - smile * 30.f, ex + 80.f, 310.f);
        eye.quadraticTo(ex, 284.f - smile * 20.f, ex - 80.f, 310.f);
        eye.closeSubPath();
        g.setColour(Colour(0xff1a1220));
        g.fillPath(eye);
    }

    // nose
    Path nose;
    nose.startNewSubPath(cx, 360.f);
    nose.quadraticTo(cx - 10.f, 450.f, cx + 14.f, 470.f);
    g.setColour(Colour(0xff3a4270).withAlpha(0.25f));
    g.strokePath(nose, PathStrokeType(4.f, PathStrokeType::curved, PathStrokeType::rounded));

    // blush
    g.setColour(Colour(0xffff9a86).withAlpha(0.35f + smile * 0.25f));
    g.fillEllipse(cx - 270.f, 460.f, 170.f, 105.f);
    g.fillEllipse(cx + 100.f, 460.f, 170.f, 105.f);

    // mouth: the louder, the wider the smile
    const float cyM = 705.f;
    const float halfW = 55.f + smile * 185.f;
    const float cornerY = cyM - smile * 55.f;
    const float yU = cyM - 14.f + (1.f - smile) * 6.f;
    const float yL = cyM + 30.f + smile * 170.f;
    Path lips;
    lips.startNewSubPath(cx - halfW, cornerY);
    lips.quadraticTo(cx, yU, cx + halfW, cornerY);
    lips.quadraticTo(cx, yL, cx - halfW, cornerY);
    lips.closeSubPath();
    g.setColour(kLips);
    g.fillPath(lips);
    if (smile > 0.12f)
    {
        const float k = 0.9f;
        Path in;
        in.startNewSubPath(cx - halfW * k, cornerY + 4.f);
        in.quadraticTo(cx, yU + 14.f, cx + halfW * k, cornerY + 4.f);
        in.quadraticTo(cx, yL - 16.f, cx - halfW * k, cornerY + 4.f);
        in.closeSubPath();
        g.setColour(Colour(0xff2a0010));
        g.fillPath(in);
        {
            Graphics::ScopedSaveState ss(g);
            g.reduceClipRegion(in, AffineTransform());
            g.setColour(Colour(0xfff1ecf4));
            const int nT = 12;
            const float tw = 2.f * halfW * k / (float) nT;
            for (int i = 0; i < nT; ++i)
                g.fillRoundedRectangle(cx - halfW * k + tw * (float) i + 2.f, yU + 4.f, tw - 4.f, 16.f + smile * 34.f, 4.f);
            g.setColour(kPinkSoft.withAlpha(0.9f));
            g.fillEllipse(cx - 80.f * smile - 20.f, yL - 90.f, 160.f * smile + 40.f, 90.f);
        }
    }
    g.setColour(kPink.withAlpha(smile * 0.35f));
    g.strokePath(lips, PathStrokeType(14.f));
    g.setColour(Colour(0xff7a0a22));
    g.strokePath(lips, PathStrokeType(4.f));
    g.setColour(Colours::white.withAlpha(0.25f));
    g.drawLine(cx - 20.f, yU + 3.f, cx + 20.f, yU + 3.f, 2.f);

    // dim the whole mask so the UI stays readable
    g.setColour(Colour(0xff08040b).withAlpha(0.45f));
    g.fillRect(getLocalBounds());
}

void SakuraHellAudioProcessorEditor::drawPetals(Graphics& g)
{
    for (auto& p : petals)
    {
        Path pt;
        pt.startNewSubPath(0.f, -5.f);
        pt.quadraticTo(5.f, -4.f, 3.f, 3.f);
        pt.lineTo(0.f, 1.5f);
        pt.lineTo(-3.f, 3.f);
        pt.quadraticTo(-5.f, -4.f, 0.f, -5.f);
        pt.closeSubPath();
        auto tf = AffineTransform::scale(p.size).rotated(p.rot).translated(p.x, p.y);
        g.setColour(Colour(0xffffb3d1).withAlpha(0.7f));
        g.fillPath(pt, tf);
    }
}

void SakuraHellAudioProcessorEditor::drawHeader(Graphics& g)
{
    const int j = (tick % 53 < 2) ? 3 : 1;
    g.setFont(Font(FontOptions(46.f, Font::bold)));
    g.setColour(Colour(0xff3df5ff).withAlpha(0.55f));
    g.drawText("SAKURAHELL", 30 - 3 * j, 16, 500, 56, Justification::centredLeft);
    g.setColour(kPink.withAlpha(0.9f));
    g.drawText("SAKURAHELL", 30 + 3 * j, 16, 500, 56, Justification::centredLeft);
    g.setColour(Colours::white);
    g.drawText("SAKURAHELL", 30, 16, 500, 56, Justification::centredLeft);

    g.setFont(Font(FontOptions(11.f)));
    g.setColour(Colours::white.withAlpha(0.55f));
    g.drawText(String(L"\u685c\u5730\u7344") + "  //  EXECUTION UNIT 07  //  THE LOUDER, THE WIDER THE SMILE",
               32, 66, 600, 18, Justification::centredLeft);

    const int lit = (int) (smile * 22.f);
    for (int i = 0; i < 22; ++i)
    {
        Colour c = i < 14 ? Colour(0xff39ff7a) : (i < 19 ? Colour(0xffffd23a) : kPink);
        g.setColour(i < lit ? c : Colour(0xff241a2e));
        g.fillRect(R(470.f + (float) i * 10.f, 30.f, 7.f, 22.f));
    }
    g.setFont(Font(FontOptions(9.f, Font::bold)));
    g.setColour(Colours::white.withAlpha(0.5f));
    g.drawText("LEVEL", 470, 54, 60, 12, Justification::centredLeft);
}

//==============================================================================
void SakuraHellAudioProcessorEditor::drawHazard(Graphics& g, Rectangle<float> r)
{
    Graphics::ScopedSaveState s(g);
    g.reduceClipRegion(r.toNearestInt());
    g.setColour(Colour(0xff181414));
    g.fillRect(r);
    g.setColour(Colour(0xffe8b923));
    for (float x = r.getX() - r.getHeight(); x < r.getRight(); x += 16.f)
    {
        Path p;
        p.startNewSubPath(x, r.getBottom());
        p.lineTo(x + 8.f, r.getBottom());
        p.lineTo(x + 8.f + r.getHeight(), r.getY());
        p.lineTo(x + r.getHeight(), r.getY());
        p.closeSubPath();
        g.fillPath(p);
    }
}

void SakuraHellAudioProcessorEditor::drawChain(Graphics& g, float x, float y0, float y1, float phase)
{
    const int n = (int) ((y1 - y0) / 9.f);
    for (int i = 0; i < n; ++i)
    {
        const float f = (float) i / (float) jmax(1, n);
        const float xx = x + std::sin((float) tick * 0.04f + phase) * 8.f * f;
        const float yy = y0 + (float) i * 9.f;
        g.setColour(Colour(0xff8d889a));
        if (i % 2 == 0) g.drawEllipse(xx - 3.f, yy, 6.f, 11.f, 1.6f);
        else g.fillEllipse(xx - 1.5f, yy, 3.f, 11.f);
    }
}

void SakuraHellAudioProcessorEditor::drawSpeaker(Graphics& g, Rectangle<float> r, bool facingRight)
{
    g.setColour(Colour(0xff15121b));
    g.fillRoundedRectangle(r, 7.f);
    g.setColour(Colour(0xff3a3445));
    g.drawRoundedRectangle(r, 7.f, 2.f);
    g.setColour(Colour(0xff6b6577));
    for (int i = 0; i < 4; ++i)
    {
        float sx = (i % 2 == 0) ? r.getX() + 7.f : r.getRight() - 7.f;
        float sy = (i < 2) ? r.getY() + 7.f : r.getBottom() - 7.f;
        g.fillEllipse(sx - 2.5f, sy - 2.5f, 5.f, 5.f);
    }

    const float tcx = r.getCentreX(), tcy = r.getY() + 24.f;
    const float tr = 9.f + smile * 1.5f;
    g.setColour(Colour(0xff08060b));
    g.fillEllipse(tcx - tr - 3.f, tcy - tr - 3.f, (tr + 3.f) * 2.f, (tr + 3.f) * 2.f);
    ColourGradient tg(Colour(0xff6d6779), tcx - tr * 0.4f, tcy - tr * 0.4f, Colour(0xff1b1722), tcx + tr, tcy + tr, true);
    g.setGradientFill(tg);
    g.fillEllipse(tcx - tr, tcy - tr, tr * 2.f, tr * 2.f);

    const float wcx = tcx, wcy = r.getY() + 88.f;
    const float wr = 28.f + smile * 4.f;
    g.setColour(Colour(0xff2b2733));
    g.fillEllipse(wcx - 34.f, wcy - 34.f, 68.f, 68.f);
    g.setColour(Colour(0xff08060b));
    g.fillEllipse(wcx - wr - 3.f, wcy - wr - 3.f, (wr + 3.f) * 2.f, (wr + 3.f) * 2.f);
    ColourGradient cg(Colour(0xff4a4458), wcx - wr * 0.4f, wcy - wr * 0.4f, Colour(0xff0e0b12), wcx + wr, wcy + wr, true);
    g.setGradientFill(cg);
    g.fillEllipse(wcx - wr, wcy - wr, wr * 2.f, wr * 2.f);
    g.setColour(Colour(0xff0a0810));
    g.fillEllipse(wcx - 9.f, wcy - 9.f, 18.f, 18.f);
    g.setColour(Colours::white.withAlpha(0.18f));
    g.drawEllipse(wcx - wr * 0.65f, wcy - wr * 0.65f, wr * 1.3f, wr * 1.3f, 1.f);

    const float dir = facingRight ? MathConstants<float>::halfPi : -MathConstants<float>::halfPi;
    for (int i = 0; i < 3; ++i)
    {
        float rr = 42.f + (float) i * 13.f + std::fmod((float) tick * 0.8f, 13.f);
        g.setColour(kPink.withAlpha(jlimit(0.f, 1.f, smile * 0.9f - (float) i * 0.2f) * 0.5f));
        Path w;
        w.addCentredArc(wcx, wcy, rr, rr, 0.f, dir - 0.7f, dir + 0.7f, true);
        g.strokePath(w, PathStrokeType(2.f));
    }
}

void SakuraHellAudioProcessorEditor::drawLedPanel(Graphics& g, Rectangle<float> r)
{
    g.setColour(Colour(0xff041208));
    g.fillRoundedRectangle(r, 4.f);
    g.setColour(Colour(0xff1f5a2e));
    g.drawRoundedRectangle(r, 4.f, 1.5f);
    const int bars = 14;
    const float bw = (r.getWidth() - 12.f) / (float) bars;
    for (int i = 0; i < bars; ++i)
    {
        float v = jlimit(0.f, 1.f, smile * (0.55f + 0.45f * std::sin((float) tick * 0.35f + (float) i * 0.8f)) * 1.05f + 0.04f);
        int lit = (int) (v * 8.f + 0.5f);
        for (int s = 0; s < 8; ++s)
        {
            Colour c = s < 5 ? Colour(0xff35ff6a) : (s < 7 ? Colour(0xffffd23a) : kPink);
            g.setColour(s < lit ? c : c.withAlpha(0.10f));
            g.fillRect(R(r.getX() + 6.f + (float) i * bw, r.getBottom() - 6.f - (float) (s + 1) * 6.f, bw - 2.f, 5.f));
        }
    }
}

//==============================================================================
void SakuraHellAudioProcessorEditor::drawFace(Graphics& g, const Expr& e)
{
    const float f = e.fear;
    const float t = (float) tick;
    const float hh = 2.5f + f * 4.5f;   // half height of open eye

    // hair
    g.setColour(Colour(0xff3d2a20));
    g.fillEllipse(-24.f, -27.f, 48.f, 50.f);
    for (int i = 0; i < 7; ++i)
    {
        float a = -2.7f + (float) i * 0.45f;
        float sw = std::sin(t * 0.08f + (float) i) * (1.f + f * 2.f);
        g.fillEllipse(std::cos(a) * 23.f - 4.f + sw, std::sin(a) * 25.f - 4.f, 8.f, 9.f);
    }
    g.fillEllipse(-26.f, 0.f, 9.f, 22.f);
    g.fillEllipse(17.f, 0.f, 9.f, 22.f);

    // mask
    Path face;
    face.addEllipse(-17.f, -21.f, 34.f, 42.f);
    ColourGradient mg(Colour(0xfff4f5fc), -8.f, -14.f, Colour(0xffb9c2de), 12.f, 18.f, false);
    g.setGradientFill(mg);
    g.fillPath(face);
    if (e.kind == 4)
    {
        g.setColour(kPink.withAlpha(f * 0.35f));
        g.fillPath(face);
    }
    g.setColour(Colour(0xff8f98b8));
    g.strokePath(face, PathStrokeType(0.7f));

    // brows
    const float worry = jlimit(0.f, 1.f, f * 1.5f);
    for (int s = -1; s <= 1; s += 2)
    {
        Path b;
        b.addRoundedRectangle(-3.f, -1.1f, 6.f, 2.2f, 1.1f);
        g.setColour(Colour(0xff2a1d1a));
        g.fillPath(b, AffineTransform::rotation((float) s * 0.45f * worry)
                          .translated((float) s * 7.5f, -13.f - worry * 3.f));
    }

    // eyes
    for (int s = -1; s <= 1; s += 2)
    {
        const float ex = (float) s * 6.5f, ey = -4.f;
        g.setColour(Colour(0xff1d1420));
        if (e.dead)
        {
            g.drawLine(ex - 3.f, ey - 3.f, ex + 3.f, ey + 3.f, 1.4f);
            g.drawLine(ex - 3.f, ey + 3.f, ex + 3.f, ey - 3.f, 1.4f);
        }
        else if (f < 0.08f)
        {
            Path c;
            c.startNewSubPath(ex - 4.5f, ey + 1.f);
            c.quadraticTo(ex, ey - 4.f, ex + 4.5f, ey + 1.f);
            g.strokePath(c, PathStrokeType(1.5f, PathStrokeType::curved, PathStrokeType::rounded));
        }
        else if (e.kind == 4)
        {
            Path c;
            c.startNewSubPath(ex + (float) s * 3.f, ey - 3.f);
            c.lineTo(ex - (float) s * 3.f, ey);
            c.lineTo(ex + (float) s * 3.f, ey + 3.f);
            g.strokePath(c, PathStrokeType(1.5f, PathStrokeType::curved, PathStrokeType::rounded));
        }
        else
        {
            const float h2 = (e.kind == 1) ? hh + 1.5f : hh;
            g.setColour(Colours::white);
            g.fillEllipse(ex - 5.5f, ey - h2, 11.f, h2 * 2.f);
            g.setColour(Colour(0xff1d1420));
            g.drawEllipse(ex - 5.5f, ey - h2, 11.f, h2 * 2.f, 0.8f);
            const float pr = jmax(1.1f, 2.2f - f * 1.0f);
            const float px = ex + std::sin(t * 0.5f) * f * 0.8f;
            const float py = (e.kind == 3) ? ey - h2 * 0.45f : ey;
            g.fillEllipse(px - pr, py - pr, pr * 2.f, pr * 2.f);
            g.setColour(Colours::white);
            g.fillEllipse(px - pr * 0.2f, py - pr * 0.7f, pr * 0.6f, pr * 0.6f);
        }
    }

    // nose
    Path n;
    n.startNewSubPath(0.5f, -1.f);
    n.quadraticTo(-1.2f, 4.5f, 1.f, 5.2f);
    g.setColour(Colour(0xff7a82a8).withAlpha(0.6f));
    g.strokePath(n, PathStrokeType(0.7f));

    // blush
    g.setColour((e.kind == 4 ? kPink : Colour(0xffff8f7a)).withAlpha(0.35f + f * 0.25f));
    g.fillEllipse(-15.f, 3.f, 9.f, 6.f);
    g.fillEllipse(6.f, 3.f, 9.f, 6.f);

    // mouth
    if (f < 0.08f)
    {
        g.setColour(kLips);
        g.fillEllipse(-4.f, 11.f, 8.f, 3.2f);
        Path sm;
        sm.startNewSubPath(-5.f, 11.f);
        sm.quadraticTo(0.f, 15.f, 5.f, 11.f);
        g.setColour(Colour(0xff8a1028));
        g.strokePath(sm, PathStrokeType(0.7f));
    }
    else if (e.kind == 4)
    {
        Path z;
        z.startNewSubPath(-6.f, 13.f);
        for (int i = 1; i <= 6; ++i) z.lineTo(-6.f + (float) i * 2.f, 13.f + ((i % 2) ? 1.3f : -1.3f));
        g.setColour(kLips);
        g.strokePath(z, PathStrokeType(1.5f, PathStrokeType::curved, PathStrokeType::rounded));
    }
    else
    {
        const float mw = 2.5f + f * 4.5f, mh = 1.5f + f * 8.f;
        g.setColour(Colour(0xff3a0012));
        g.fillEllipse(-mw, 12.f, mw * 2.f, mh * 2.f);
        g.setColour(Colour(0xfff1ecf4));
        g.fillRect(R(-mw * 0.7f, 12.f, mw * 1.4f, 1.6f));
        g.setColour(kPinkSoft);
        g.fillEllipse(-mw * 0.6f, 12.f + mh * 1.1f, mw * 1.2f, mh * 0.8f);
        g.setColour(kLips);
        g.drawEllipse(-mw - 0.5f, 11.5f, mw * 2.f + 1.f, mh * 2.f + 1.f, 1.1f);
    }

    // sweat
    if (f > 0.25f)
    {
        g.setColour(Colour(0xffa6dcff).withAlpha(0.85f));
        for (int i = 0; i < 3; ++i)
        {
            float x = -13.f + (float) i * 13.f;
            float y = std::fmod(t * 0.4f + (float) i * 7.f, 22.f) - 18.f;
            Path d;
            d.addEllipse(x - 1.2f, y, 2.4f, 3.2f);
            d.addTriangle(x, y - 3.f, x - 1.1f, y + 0.5f, x + 1.1f, y + 0.5f);
            g.fillPath(d);
        }
    }
    // tears
    if (f > 0.5f)
    {
        g.setColour(Colour(0xffa6dcff).withAlpha(0.8f));
        for (int s = -1; s <= 1; s += 2)
        {
            float len = 4.f + std::fmod(t * 0.6f, 10.f);
            g.drawLine((float) s * 7.5f, -1.f, (float) s * 8.f, -1.f + len, 1.2f);
        }
    }
    // cracks
    if (f > 0.65f && (e.kind == 1 || e.dead))
    {
        Path c;
        c.startNewSubPath(-2.f, -21.f);
        c.lineTo(1.f, -14.f); c.lineTo(-1.5f, -9.f); c.lineTo(2.f, -4.f); c.lineTo(0.f, 2.f);
        c.startNewSubPath(-1.5f, -9.f);
        c.lineTo(-7.f, -7.f); c.lineTo(-9.f, -2.f);
        g.setColour(Colour(0xff2b2540));
        g.strokePath(c, PathStrokeType(0.8f + f * 0.6f));
    }
    // gore on the mask
    if (f > 0.8f)
    {
        g.setColour(kPink.withAlpha(0.9f));
        g.fillEllipse(4.f, -12.f, 5.f, 4.f);
        g.fillEllipse(-10.f, 5.f, 4.f, 6.f);
        g.fillEllipse(-3.f, -19.f, 3.f, 3.f);
        g.fillRoundedRectangle(5.f, -9.f, 1.4f, 9.f, 0.7f);
    }
    // red tassel
    for (int i = 0; i < 2; ++i)
    {
        float sw = std::sin(t * 0.15f + (float) i) * (1.5f + f * 3.f);
        Path s;
        s.startNewSubPath(15.f + (float) i * 1.5f, -12.f);
        s.quadraticTo(20.f + sw, 2.f, 17.f + (float) i * 2.f + sw, 18.f);
        g.setColour(Colour(0xffd4172f));
        g.strokePath(s, PathStrokeType(1.2f));
        g.fillEllipse(16.f + (float) i * 2.f + sw, 17.f, 2.2f, 2.2f);
    }
}

void SakuraHellAudioProcessorEditor::drawPortrait(Graphics& g, Rectangle<float> r, const Expr& e)
{
    g.setColour(Colour(0xff050a0c));
    g.fillRoundedRectangle(r, 6.f);
    {
        Graphics::ScopedSaveState ss(g);
        Path clip;
        clip.addRoundedRectangle(r.getX(), r.getY(), r.getWidth(), r.getHeight(), 6.f);
        g.reduceClipRegion(clip, AffineTransform());

        ColourGradient bg(Colour(0xff17313a), r.getCentreX(), r.getCentreY(), Colour(0xff060c10), r.getX(), r.getY(), true);
        g.setGradientFill(bg);
        g.fillRect(r);
        {
            Graphics::ScopedSaveState fs(g);
            const float shk = e.fear * std::sin((float) tick * 1.7f) * 2.5f;
            g.addTransform(AffineTransform::scale(3.1f).translated(r.getCentreX() + shk, r.getCentreY() + 4.f));
            drawFace(g, e);
        }
        g.setColour(kPink.withAlpha(e.fear * 0.18f));
        g.fillRect(r);
        g.setColour(Colours::black.withAlpha(0.22f));
        for (float y = r.getY(); y < r.getBottom(); y += 3.f) g.drawLine(r.getX(), y, r.getRight(), y, 1.f);
        if (e.fear > 0.45f && (tick % 9) < 2)
        {
            g.setColour(Colours::white.withAlpha(0.14f));
            g.fillRect(R(r.getX(), r.getY() + (float) ((tick * 13) % 120), r.getWidth(), 8.f));
        }

        auto ecg = R(r.getX(), r.getBottom() - 26.f, r.getWidth(), 26.f);
        g.setColour(Colours::black.withAlpha(0.65f));
        g.fillRect(ecg);
        Path p;
        bool first = true;
        const float speed = 1.5f + e.fear * 4.f;
        for (float x = 0.f; x <= ecg.getWidth(); x += 2.f)
        {
            float ph = std::fmod((x + (float) tick * speed) / 38.f, 1.f);
            float v = 0.f;
            if (!e.dead)
            {
                if (ph < 0.08f) v = -(ph / 0.08f);
                else if (ph < 0.14f) v = -1.f + (ph - 0.08f) / 0.06f * 1.8f;
                else if (ph < 0.2f) v = 0.8f - (ph - 0.14f) / 0.06f * 0.8f;
            }
            float y = ecg.getCentreY() - v * 8.f;
            if (first) { p.startNewSubPath(ecg.getX() + x, y); first = false; }
            else p.lineTo(ecg.getX() + x, y);
        }
        g.setColour(e.dead ? Colour(0xffff3b3b) : Colour(0xff39ff7a));
        g.strokePath(p, PathStrokeType(1.4f));
        g.setFont(Font(FontOptions(9.f, Font::bold)));
        g.drawText(e.dead ? String("BPM 000") : "BPM " + String((int) (72.f + e.fear * 118.f)),
                   R(r.getX() + 6.f, r.getBottom() - 38.f, 80.f, 12.f), Justification::centredLeft);
    }
    g.setColour(kPink);
    g.drawRoundedRectangle(r, 6.f, 2.f);
    g.setFont(Font(FontOptions(9.f, Font::bold)));
    g.setColour(Colours::white.withAlpha(0.85f));
    g.drawText("CAM-02 // SUBJECT 07", R(r.getX() + 6.f, r.getY() + 4.f, 120.f, 12.f), Justification::centredLeft);
    if ((tick / 15) % 2 == 0)
    {
        g.setColour(Colour(0xffff3b3b));
        g.fillEllipse(r.getRight() - 14.f, r.getY() + 6.f, 7.f, 7.f);
    }
}

//==============================================================================
void SakuraHellAudioProcessorEditor::drawCharacter(Graphics& g, float cx, float floorY, float scaleY, float lift,
                                                   float headDX, float headDY, float headRot, const Expr& e)
{
    Graphics::ScopedSaveState ss(g);
    g.addTransform(AffineTransform::scale(K, K * scaleY, cx, floorY).translated(0.f, -lift));
    auto P = [&](float x, float y) { return Point<float>(cx + x, floorY + y); };
    const float t = (float) tick;
    const float fear = e.fear;
    const float wave = std::sin(t * 0.55f) * fear * 7.f;
    const float sway = std::sin(t * 0.1f) * (1.f - fear) * 2.5f;
    const Colour skin(0xffe8c4a8);

    // shadow
    g.setColour(Colours::black.withAlpha(0.45f));
    g.fillEllipse(cx - 40.f, floorY - 4.f, 80.f, 8.f);

    // shoes
    for (int s = -1; s <= 1; s += 2)
    {
        float sx = (s < 0) ? cx - 37.f : cx + 7.f;
        g.setColour(Colour(0xff0d0d0d));
        g.fillRoundedRectangle(sx, floorY - 11.f, 30.f, 11.f, 3.f);
        g.setColour(Colour(0xffdedede));
        g.fillRect(R(sx, floorY - 3.f, 30.f, 3.f));
        g.setColour(Colours::white.withAlpha(0.25f));
        g.fillRect(R(sx + 4.f, floorY - 9.f, 10.f, 1.5f));
    }

    // flared jeans
    for (int s = -1; s <= 1; s += 2)
    {
        Path leg;
        leg.startNewSubPath(P((float) s * 1.f, -96.f));
        leg.lineTo(P((float) s * 21.f, -96.f));
        leg.lineTo(P((float) s * 37.f, -10.f));
        leg.lineTo(P((float) s * 6.f, -10.f));
        leg.closeSubPath();
        g.setColour(Colour(0xff5b7fa8));
        g.fillPath(leg);
        g.setColour(Colour(0xff7d9cc2).withAlpha(0.45f));
        g.fillEllipse(cx + (float) s * 14.f - 7.f, floorY - 70.f, 14.f, 18.f);
        g.setColour(Colour(0xff3f5f88));
        g.drawLine(cx + (float) s * 3.f, floorY - 94.f, cx + (float) s * 11.f, floorY - 12.f, 1.f);
        g.drawLine(cx + (float) s * 8.f, floorY - 50.f, cx + (float) s * 18.f, floorY - 47.f, 1.f);
        g.drawLine(cx + (float) s * 10.f, floorY - 32.f, cx + (float) s * 22.f, floorY - 30.f, 1.f);
    }

    // orange rail sign held at the hip
    {
        Path sign;
        sign.addRectangle(-17.f, -13.f, 34.f, 26.f);
        auto tf = AffineTransform::rotation(-0.25f).translated(cx - 42.f, floorY - 82.f);
        g.setColour(Colour(0xffe8742a));
        g.fillPath(sign, tf);
        Path marks;
        marks.startNewSubPath(-11.f, -6.f); marks.lineTo(11.f, -6.f);
        marks.startNewSubPath(-9.f, 4.f);   marks.lineTo(9.f, 4.f);
        g.setColour(Colour(0xff1a1a1a));
        g.strokePath(marks, PathStrokeType(2.2f), tf);
        g.setColour(Colour(0xff7a3a16));
        g.strokePath(sign, PathStrokeType(1.5f), tf);
    }

    // jacket
    g.setColour(Colour(0xff111111));
    g.fillRoundedRectangle(cx - 27.f, floorY - 162.f, 54.f, 68.f, 8.f);
    g.setColour(Colours::white.withAlpha(0.05f));
    g.fillRoundedRectangle(cx - 27.f, floorY - 162.f, 20.f, 68.f, 8.f);
    g.setColour(Colour(0xff1b1b1b));
    g.fillRect(R(cx - 27.f, floorY - 100.f, 54.f, 6.f));
    g.setColour(Colour(0xff050505));
    for (int i = 0; i < 9; ++i) g.drawLine(cx - 24.f + (float) i * 6.f, floorY - 100.f, cx - 24.f + (float) i * 6.f, floorY - 94.f, 1.f);
    // tee
    g.setColour(Colour(0xffb9d2e6));
    g.fillRect(R(cx - 7.f, floorY - 162.f, 14.f, 26.f));
    g.setColour(Colour(0xff3b5a78));
    for (int i = 0; i < 6; ++i) g.fillRect(R(cx - 7.f, floorY - 160.f + (float) i * 4.5f, 14.f, 1.6f));
    // zipper
    g.setColour(Colour(0xff9a97a6));
    g.drawLine(cx, floorY - 136.f, cx, floorY - 96.f, 1.2f);
    // collar
    g.setColour(Colour(0xff0a0a0a));
    {
        Path collar;
        collar.addTriangle(cx - 7.f, floorY - 164.f, cx - 16.f, floorY - 158.f, cx - 7.f, floorY - 140.f);
        collar.addTriangle(cx + 7.f, floorY - 164.f, cx + 16.f, floorY - 158.f, cx + 7.f, floorY - 140.f);
        g.fillPath(collar);
    }
    // logo
    g.setColour(Colours::white.withAlpha(0.9f));
    g.drawLine(cx + 12.f, floorY - 139.f, cx + 14.f, floorY - 143.f, 1.5f);
    g.drawLine(cx + 15.f, floorY - 139.f, cx + 17.f, floorY - 146.f, 1.5f);
    g.drawLine(cx + 18.f, floorY - 139.f, cx + 20.f, floorY - 149.f, 1.5f);

    // belt + buckle
    g.setColour(Colour(0xffb02a4a));
    g.fillRect(R(cx - 25.f, floorY - 100.f, 50.f, 7.f));
    g.setColour(Colour(0xff6a1328));
    for (int i = 0; i < 12; ++i) g.drawLine(cx - 24.f + (float) i * 4.f, floorY - 100.f, cx - 22.f + (float) i * 4.f, floorY - 93.f, 1.f);
    g.setColour(Colour(0xffd6d6de));
    g.fillRect(R(cx - 3.f, floorY - 102.f, 7.f, 10.f));

    // hip pouch + bandana
    g.setColour(Colour(0xff0e0e0e));
    g.fillRoundedRectangle(cx + 22.f, floorY - 96.f, 14.f, 20.f, 3.f);
    Path ban;
    ban.addTriangle(cx + 24.f, floorY - 96.f, cx + 40.f, floorY - 96.f, cx + 36.f, floorY - 70.f);
    g.setColour(Colours::white);
    g.fillPath(ban);
    g.setColour(Colour(0xff111111));
    g.fillEllipse(cx + 28.f, floorY - 92.f, 3.f, 3.f);
    g.fillEllipse(cx + 33.f, floorY - 88.f, 3.f, 3.f);
    g.fillEllipse(cx + 32.f, floorY - 80.f, 2.5f, 2.5f);

    // arms with three-stripe sleeves
    Path armL, armR;
    armL.startNewSubPath(P(-26.f, -152.f));
    armL.lineTo(P(-46.f, -122.f));
    armL.lineTo(P(-34.f, -96.f + wave * 0.3f));
    armR.startNewSubPath(P(26.f, -152.f));
    armR.lineTo(P(52.f + sway, -178.f));
    armR.lineTo(P(36.f + wave, -200.f + wave * 0.4f));
    for (auto* a : { &armL, &armR })
    {
        g.setColour(Colour(0xff111111));
        g.strokePath(*a, PathStrokeType(11.f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setColour(Colours::white);
        g.strokePath(*a, PathStrokeType(8.5f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setColour(Colour(0xff111111));
        g.strokePath(*a, PathStrokeType(7.f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setColour(Colours::white);
        g.strokePath(*a, PathStrokeType(4.f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setColour(Colour(0xff111111));
        g.strokePath(*a, PathStrokeType(2.5f, PathStrokeType::curved, PathStrokeType::rounded));
    }
    // hands with fingers
    {
        auto h1 = P(-34.f, -96.f + wave * 0.3f);
        auto h2 = P(36.f + wave, -200.f + wave * 0.4f);
        g.setColour(skin);
        g.fillEllipse(h1.x - 5.f, h1.y - 5.f, 10.f, 10.f);
        g.fillEllipse(h2.x - 5.f, h2.y - 5.f, 10.f, 10.f);
        for (int i = 0; i < 3; ++i)
        {
            g.drawLine(h1.x - 3.f + (float) i * 3.f, h1.y + 3.f, h1.x - 4.f + (float) i * 3.5f, h1.y + 10.f, 2.f);
            g.drawLine(h2.x - 3.f + (float) i * 3.f, h2.y - 3.f, h2.x - 4.f + (float) i * 3.5f, h2.y - 10.f, 2.f);
        }
    }

    // gore on the clothes
    if (fear > 0.1f && e.kind != 4)
    {
        const float pos[5][4] = { { -10.f, -130.f, 12.f, 9.f }, { 8.f, -110.f, 10.f, 14.f }, { -18.f, -70.f, 9.f, 12.f },
                                  { 14.f, -60.f, 11.f, 8.f }, { 0.f, -150.f, 8.f, 8.f } };
        const int n = (int) (fear * 5.f + 0.5f);
        g.setColour(kPink.withAlpha(0.9f));
        for (int i = 0; i < n && i < 5; ++i)
            g.fillEllipse(cx + pos[i][0], floorY + pos[i][1], pos[i][2], pos[i][3]);
    }

    // neck, stump, head
    g.setColour(skin);
    g.fillRect(R(cx - 4.f, floorY - 168.f, 8.f, 8.f));
    if (e.cut > 0.02f)
    {
        g.setColour(kPink);
        g.fillEllipse(cx - 6.f, floorY - 168.f, 12.f, 7.f);
    }
    {
        Graphics::ScopedSaveState hs(g);
        g.addTransform(AffineTransform::rotation(headRot, cx, floorY - 162.f).translated(headDX, headDY));
        g.addTransform(AffineTransform::translation(cx, floorY - 184.f));
        drawFace(g, e);
    }
}

//==============================================================================
void SakuraHellAudioProcessorEditor::drawStage(Graphics& g)
{
    const auto stage = stageRect();
    const float cx = stage.getCentreX();
    const float floorY = stage.getBottom() - 36.f;
    const auto ex = currentExpr();
    const float press = (float) crush.s.getValue();
    const float blade = (float) hell.s.getValue();
    const float spikes = jlimit(0.f, 1.f, std::abs((float) pitch.s.getValue()) / 24.f);
    const float acidV = (float) doom.s.getValue();
    const float t = (float) tick;

    ColourGradient bg(Colour(0xff17121d).withAlpha(0.92f), cx, stage.getY(),
                      Colour(0xff0a070e).withAlpha(0.95f), cx, stage.getBottom(), false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(stage, 10.f);
    g.setColour(Colour(0xff3a2a44));
    g.drawRoundedRectangle(stage, 10.f, 2.f);

    Graphics::ScopedSaveState clipState(g);
    {
        Path clip;
        clip.addRoundedRectangle(stage.getX(), stage.getY(), stage.getWidth(), stage.getHeight(), 10.f);
        g.reduceClipRegion(clip, AffineTransform());
    }
    g.addTransform(AffineTransform::translation(shake, 0.f));

    // wall tiles
    g.setColour(Colours::white.withAlpha(0.04f));
    for (float x = stage.getX(); x < stage.getRight(); x += 44.f) g.drawLine(x, stage.getY(), x, floorY, 1.f);
    for (float y = stage.getY(); y < floorY; y += 44.f) g.drawLine(stage.getX(), y, stage.getRight(), y, 1.f);
    // pipe
    g.setColour(Colour(0xff2a2631));
    g.fillRect(R(stage.getX(), stage.getY() + 30.f, stage.getWidth(), 8.f));
    g.setColour(Colours::white.withAlpha(0.12f));
    g.fillRect(R(stage.getX(), stage.getY() + 31.f, stage.getWidth(), 1.5f));
    g.setColour(Colour(0xff4a4456));
    for (float x = stage.getX() + 60.f; x < stage.getRight(); x += 120.f) g.fillRect(R(x, stage.getY() + 27.f, 6.f, 14.f));
    // vents
    for (int s = -1; s <= 1; s += 2)
    {
        auto v = R(cx + (float) s * 130.f - 20.f, stage.getY() + 60.f, 40.f, 26.f);
        g.setColour(Colours::black.withAlpha(0.5f));
        g.fillRoundedRectangle(v, 3.f);
        g.setColour(Colour(0xff3a3445));
        for (int i = 0; i < 5; ++i) g.drawLine(v.getX() + 3.f, v.getY() + 4.f + (float) i * 4.5f, v.getRight() - 3.f, v.getY() + 4.f + (float) i * 4.5f, 1.5f);
    }
    drawHazard(g, R(stage.getX(), stage.getY(), stage.getWidth(), 10.f));

    // chains
    drawChain(g, cx - 150.f, stage.getY() + 10.f, stage.getY() + 130.f, 0.f);
    drawChain(g, cx + 150.f, stage.getY() + 10.f, stage.getY() + 170.f, 2.f);

    // screens
    drawPortrait(g, R(42.f, 108.f, 150.f, 150.f), ex);
    drawLedPanel(g, R(562.f, 108.f, 116.f, 64.f));

    // warning lamps
    {
        const bool on = ex.fear > 0.25f && ((tick / 6) % 2 == 0);
        const Point<float> lamps[2] = { { 118.f, 284.f }, { 620.f, 196.f } };
        for (auto& l : lamps)
        {
            g.setColour(Colour(0xff2a2631));
            g.fillRect(R(l.x - 12.f, l.y + 8.f, 24.f, 6.f));
            if (on)
            {
                ColourGradient lg(Colour(0xffff3b3b).withAlpha(0.55f), l.x, l.y, Colours::transparentBlack, l.x + 38.f, l.y, true);
                g.setGradientFill(lg);
                g.fillEllipse(l.x - 38.f, l.y - 38.f, 76.f, 76.f);
            }
            g.setColour(on ? Colour(0xffff4040) : Colour(0xff5a1414));
            g.fillEllipse(l.x - 9.f, l.y - 9.f, 18.f, 18.f);
        }
    }

    // floor
    {
        ColourGradient fgrad(Colour(0xff2b2432), cx, floorY, Colour(0xff0f0b14), cx, stage.getBottom(), false);
        g.setGradientFill(fgrad);
        g.fillRect(R(stage.getX(), floorY, stage.getWidth(), stage.getBottom() - floorY));
        g.setColour(Colour(0xff4a4056));
        g.drawLine(stage.getX(), floorY, stage.getRight(), floorY, 2.f);
        g.setColour(Colours::black.withAlpha(0.35f));
        for (float x = stage.getX() + 6.f; x < stage.getRight(); x += 14.f) g.drawLine(x, floorY + 3.f, x, stage.getBottom(), 3.f);
    }

    // speakers
    drawSpeaker(g, R(44.f, floorY - 150.f, 92.f, 150.f), true);
    drawSpeaker(g, R(584.f, floorY - 150.f, 92.f, 150.f), false);

    // spotlight
    {
        Path cone;
        cone.startNewSubPath(cx - 40.f, stage.getY());
        cone.lineTo(cx + 40.f, stage.getY());
        cone.lineTo(cx + 150.f, floorY);
        cone.lineTo(cx - 150.f, floorY);
        cone.closeSubPath();
        ColourGradient sg(Colours::white.withAlpha(0.13f), cx, stage.getY(), Colours::white.withAlpha(0.f), cx, floorY, false);
        g.setGradientFill(sg);
        g.fillPath(cone);
    }

    // blood decals (stay for a while)
    for (auto& d : decals)
    {
        g.setColour(kPink.withAlpha(jlimit(0.f, 1.f, d.alpha) * 0.9f));
        if (d.wall)
        {
            g.fillEllipse(d.x - d.size / 2.f, d.y - d.size / 2.f, d.size, d.size);
            g.fillRoundedRectangle(d.x - d.size * 0.16f, d.y, d.size * 0.32f, d.size * 2.4f, d.size * 0.16f);
        }
        else
            g.fillEllipse(d.x - d.size, d.y, d.size * 2.f, d.size * 0.45f);
    }

    // character
    const float scaleY = 1.f - 0.85f * press;
    const float spikeH = spikes * 62.f;
    const float bounce = (press < 0.2f) ? smile * 5.f * std::abs(std::sin(t * 0.35f)) : 0.f;
    const float lift = spikeH * 0.9f + bounce;
    const float bladeIn = jmin(1.f, blade * 2.f);
    const float cut = ex.cut;
    const float headDX = cut * 70.f;
    const float headDY = jmin(150.f, cut * cut * 190.f);
    const float headRot = cut * 1.3f;

    drawCharacter(g, cx, floorY, scaleY, lift, headDX, headDY, headRot, ex);

    const float charTop = floorY - 215.f * K * scaleY - lift;

    // acid
    if (acidV > 0.01f)
    {
        g.setColour(kPinkSoft.withAlpha(acidV * 0.55f));
        g.fillRoundedRectangle(cx - 38.f, charTop + 10.f, 76.f, floorY - charTop - 10.f - lift, 16.f);
        const float sw = 4.f + acidV * 26.f;
        g.setColour(kPinkSoft.withAlpha(0.85f));
        g.fillRoundedRectangle(cx - sw / 2.f, stage.getY() + 14.f, sw, charTop - stage.getY(), sw / 2.f);
        g.setColour(Colours::white.withAlpha(0.35f));
        g.fillRect(R(cx - sw / 2.f + 2.f, stage.getY() + 14.f, 2.f, charTop - stage.getY()));
        g.setColour(Colour(0xff555060));
        g.fillRect(R(cx - 32.f, stage.getY() + 8.f, 64.f, 16.f));
        g.setColour(Colour(0xff3a3645));
        g.fillRect(R(cx - 38.f, stage.getY() + 6.f, 8.f, 20.f));
        g.fillRect(R(cx + 30.f, stage.getY() + 6.f, 8.f, 20.f));
        g.setColour(Colour(0xffc02040));
        g.drawEllipse(cx + 44.f, stage.getY() + 6.f, 18.f, 18.f, 3.f);
        g.drawLine(cx + 53.f, stage.getY() + 6.f, cx + 53.f, stage.getY() + 24.f, 2.f);
        g.drawLine(cx + 44.f, stage.getY() + 15.f, cx + 62.f, stage.getY() + 15.f, 2.f);
    }

    // press
    {
        const float topY = stage.getY() + 26.f;
        const float t01 = jlimit(0.f, 1.f, press * 6.f);
        const float plateBottom = jmap(t01, topY + 30.f, charTop);
        ColourGradient cg(Colour(0xff8a8598), cx - 38.f, stage.getY(), Colour(0xff3a3645), cx + 38.f, stage.getY(), false);
        g.setGradientFill(cg);
        g.fillRoundedRectangle(cx - 38.f, stage.getY() + 10.f, 76.f, 46.f, 4.f);
        g.setColour(Colour(0xff6b6677));
        g.fillRect(R(cx - 11.f, stage.getY() + 56.f, 22.f, jmax(0.f, plateBottom - 26.f - (stage.getY() + 56.f))));
        g.setColour(Colours::white.withAlpha(0.25f));
        g.fillRect(R(cx - 8.f, stage.getY() + 56.f, 3.f, jmax(0.f, plateBottom - 26.f - (stage.getY() + 56.f))));
        ColourGradient pg(Colour(0xffb9b4c4), cx, plateBottom - 26.f, Colour(0xff5b5668), cx, plateBottom, false);
        g.setGradientFill(pg);
        g.fillRoundedRectangle(cx - 105.f, plateBottom - 26.f, 210.f, 26.f, 4.f);
        g.setColour(Colour(0xff2b2733));
        for (int i = 0; i < 9; ++i) g.fillRect(R(cx - 98.f + (float) i * 24.f, plateBottom - 6.f, 12.f, 6.f));
        g.setColour(Colour(0xff35303f));
        g.fillEllipse(cx - 98.f, plateBottom - 22.f, 6.f, 6.f);
        g.fillEllipse(cx + 92.f, plateBottom - 22.f, 6.f, 6.f);
        drawHazard(g, R(cx - 105.f, plateBottom - 14.f, 210.f, 7.f));
    }

    // blade
    if (blade > 0.01f)
    {
        const float by = floorY - 164.f * K - lift;
        const float bx = jmap(bladeIn, stage.getX() - 150.f, cx + 4.f);
        Path b;
        b.startNewSubPath(bx - 240.f, by - 7.f);
        b.lineTo(bx, by - 7.f);
        b.lineTo(bx + 26.f, by + 1.f);
        b.lineTo(bx, by + 9.f);
        b.lineTo(bx - 240.f, by + 9.f);
        b.closeSubPath();
        g.setColour(Colour(0xffcfd0da));
        g.fillPath(b);
        g.setColour(Colours::white.withAlpha(0.8f));
        g.drawLine(bx - 240.f, by - 4.f, bx + 20.f, by - 1.f, 1.5f);
        g.setColour(Colour(0xff4a4658));
        g.fillRect(R(bx - 250.f, by - 12.f, 22.f, 26.f));
        if (cut > 0.02f)
        {
            g.setColour(kPink);
            g.drawLine(bx - 60.f, by + 8.f, bx + 22.f, by + 1.f, 2.f);
        }
    }

    // spikes
    if (spikes > 0.01f)
    {
        g.setColour(Colour(0xff3a3645));
        g.fillRect(R(cx - 92.f, floorY - 6.f, 184.f, 8.f));
        for (int i = -5; i <= 5; ++i)
        {
            Path s;
            float sx = cx + (float) i * 14.f;
            float tip = floorY - spikeH - (i % 2 == 0 ? 8.f : 0.f);
            s.startNewSubPath(sx - 7.f, floorY - 5.f);
            s.lineTo(sx, tip);
            s.lineTo(sx + 7.f, floorY - 5.f);
            s.closeSubPath();
            g.setColour(Colour(0xff9a97a6));
            g.fillPath(s);
            g.setColour(Colours::white.withAlpha(0.35f));
            g.drawLine(sx - 2.f, floorY - 8.f, sx, tip + 3.f, 1.f);
            if (spikes > 0.3f)
            {
                Path bl;
                bl.startNewSubPath(sx - 3.f, tip + (floorY - tip) * 0.3f);
                bl.lineTo(sx, tip);
                bl.lineTo(sx + 3.f, tip + (floorY - tip) * 0.3f);
                bl.closeSubPath();
                g.setColour(kPink);
                g.fillPath(bl);
            }
        }
    }

    // pool of neon pink blood
    if (pool > 0.5f)
    {
        ColourGradient glow(kPink.withAlpha(0.28f * pool / 26.f), cx, floorY, Colours::transparentBlack, cx + 260.f, floorY, true);
        g.setGradientFill(glow);
        g.fillRect(R(stage.getX(), floorY - 120.f, stage.getWidth(), 120.f));
        ColourGradient pgrad(kPink.brighter(0.2f), cx, floorY - pool, kPink.darker(0.5f), cx, floorY + 4.f, false);
        g.setGradientFill(pgrad);
        g.fillRoundedRectangle(cx - 170.f, floorY - pool, 340.f, pool + 4.f, 8.f);
        g.setColour(Colours::white.withAlpha(0.25f));
        g.fillRoundedRectangle(cx - 150.f, floorY - pool + 3.f, 120.f, 3.f, 2.f);
        g.setColour(Colours::white.withAlpha(0.35f));
        for (int i = 0; i < 6; ++i)
        {
            float bxp = cx - 120.f + (float) i * 45.f;
            float byp = floorY - pool * 0.5f + std::sin(t * 0.1f + (float) i) * 2.f;
            g.drawEllipse(bxp, byp, 5.f, 5.f, 1.f);
        }
    }

    // particles
    for (auto& p : particles)
    {
        g.setColour((p.acid ? kPinkSoft : kPink).withAlpha(jlimit(0.2f, 1.f, p.life * 1.5f)));
        g.fillEllipse(p.x - p.size / 2.f, p.y - p.size / 2.f, p.size, p.size * (p.acid ? 1.6f : 1.f));
        if (p.size > 4.f)
        {
            g.setColour(Colours::white.withAlpha(0.4f));
            g.fillEllipse(p.x - p.size * 0.2f, p.y - p.size * 0.3f, p.size * 0.25f, p.size * 0.25f);
        }
    }

    // vignette
    ColourGradient vg(Colours::transparentBlack, cx, stage.getCentreY(), Colours::black.withAlpha(0.6f),
                      stage.getX(), stage.getY(), true);
    g.setGradientFill(vg);
    g.fillRect(stage);
}

void SakuraHellAudioProcessorEditor::drawDeck(Graphics& g)
{
    const auto deck = R(20.f, 520.f, 680.f, 340.f);
    g.setColour(Colour(0xff0b0710).withAlpha(0.55f));
    g.fillRoundedRectangle(deck, 12.f);
    g.setColour(Colour(0xff3a2a44));
    g.drawRoundedRectangle(deck, 12.f, 2.f);

    const Point<float> corners[4] = { { deck.getX() + 14.f, deck.getY() + 14.f }, { deck.getRight() - 14.f, deck.getY() + 14.f },
                                      { deck.getX() + 14.f, deck.getBottom() - 14.f }, { deck.getRight() - 14.f, deck.getBottom() - 14.f } };
    for (auto& c : corners)
    {
        g.setColour(Colour(0xff6b6577));
        g.fillEllipse(c.x - 4.f, c.y - 4.f, 8.f, 8.f);
        g.setColour(Colour(0xff1a1620));
        g.drawLine(c.x - 3.f, c.y + 1.f, c.x + 3.f, c.y - 1.f, 1.2f);
    }
    drawHazard(g, R(deck.getX() + 40.f, deck.getBottom() - 12.f, deck.getWidth() - 80.f, 6.f));

    const float xs[4] = { 30.f, 205.f, 380.f, 555.f };
    for (int i = 0; i < 4; ++i)
    {
        auto plate = R(xs[i] - 8.f, 538.f, 146.f, 176.f);
        g.setColour(Colours::black.withAlpha(0.35f));
        g.fillRoundedRectangle(plate, 10.f);
        g.setColour(Colour(0xff2d2236));
        g.drawRoundedRectangle(plate, 10.f, 1.5f);
    }
    auto op = R(285.f, 712.f, 150.f, 142.f);
    g.setColour(Colours::black.withAlpha(0.35f));
    g.fillRoundedRectangle(op, 10.f);
    g.setColour(Colour(0xff2d2236));
    g.drawRoundedRectangle(op, 10.f, 1.5f);
}

void SakuraHellAudioProcessorEditor::paint(Graphics& g)
{
    drawBackgroundMask(g);
    drawPetals(g);
    drawHeader(g);
    drawStage(g);
    drawDeck(g);
}
