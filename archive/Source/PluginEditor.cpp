#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include <array>
#include <cmath>
#include <vector>

namespace scrubbles
{
namespace
{
const juce::Colour background { 0xff151a1b };
const juce::Colour panel { 0xff1d2425 };
const juce::Colour border { 0xff34403f };
const juce::Colour ink { 0xffeee9df };
const juce::Colour muted { 0xff9ba9a5 };
const juce::Colour teal { 0xff82d8bd };
const juce::Colour amber { 0xffe6b67b };

juce::Font font (float size, bool bold = false)
{
    return juce::Font (juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain));
}

class InstrumentLook final : public juce::LookAndFeel_V4
{
public:
    InstrumentLook()
    {
        setColour (juce::Slider::textBoxTextColourId, ink);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxHighlightColourId, teal.withAlpha (0.3f));
        setColour (juce::ComboBox::backgroundColourId, background);
        setColour (juce::ComboBox::textColourId, ink);
        setColour (juce::ComboBox::outlineColourId, border);
        setColour (juce::ComboBox::arrowColourId, teal);
        setColour (juce::PopupMenu::backgroundColourId, panel);
        setColour (juce::PopupMenu::textColourId, ink);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, teal.withAlpha (0.2f));
        setColour (juce::PopupMenu::highlightedTextColourId, teal);
        setColour (juce::TextButton::textColourOffId, ink);
        setColour (juce::TextButton::textColourOnId, teal);
        setColour (juce::TooltipWindow::backgroundColourId, ink);
        setColour (juce::TooltipWindow::textColourId, background);
        setColour (juce::TooltipWindow::outlineColourId, border);
    }

    juce::Font getTextButtonFont (juce::TextButton&, int) override { return font (14.0f, true); }
    juce::Font getComboBoxFont (juce::ComboBox&) override { return font (14.0f); }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                               bool highlighted, bool down) override
    {
        const auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
        const auto active = button.getToggleState();
        auto colour = active ? teal.withAlpha (0.13f) : panel;
        if (highlighted || down)
            colour = colour.brighter (down ? 0.12f : 0.06f);
        g.setColour (colour);
        g.fillRoundedRectangle (bounds, 8.0f);
        g.setColour (active ? teal.withAlpha (0.55f) : border);
        g.drawRoundedRectangle (bounds, 8.0f, 1.0f);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float proportion,
                           float startAngle, float endAngle, juce::Slider& slider) override
    {
        const auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                                   static_cast<float> (width), static_cast<float> (height)).reduced (7.0f);
        const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre = bounds.getCentre();
        const auto angle = startAngle + proportion * (endAngle - startAngle);
        juce::Path track;
        track.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, startAngle, endAngle, true);
        g.setColour (border);
        g.strokePath (track, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path fill;
        fill.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, startAngle, angle, true);
        g.setColour (slider.isMouseOverOrDragging() ? teal.brighter (0.18f) : teal);
        g.strokePath (fill, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const auto face = juce::Rectangle<float> (radius * 1.54f, radius * 1.54f).withCentre (centre);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff394442), face.getTopLeft(),
                                               juce::Colour (0xff242d2c), face.getBottomRight(), false));
        g.fillEllipse (face);
        g.setColour (ink.withAlpha (0.13f));
        g.drawEllipse (face.reduced (0.5f), 1.0f);
        const auto tip = centre + juce::Point<float> (std::sin (angle), -std::cos (angle)) * radius * 0.59f;
        const auto inner = centre + juce::Point<float> (std::sin (angle), -std::cos (angle)) * radius * 0.33f;
        g.setColour (ink);
        g.drawLine ({ inner, tip }, 2.0f);
    }
};

class ContourView final : public juce::Component, public juce::SettableTooltipClient
{
public:
    explicit ContourView (ScrubblesProcessor& owner) : processor (owner)
    {
        setName ("contour");
        setTitle ("Read-only contour and draggable playhead ruler");
        setDescription ("One shared playhead. Drag the lower ruler to scrub; the curve itself is read-only.");
        setTooltip ("Drag the lower handle or click its ruler to take over the playhead. Release to resume from there.");
        setWantsKeyboardFocus (true);
    }

    ~ContourView() override { processor.releasePlayheadGrab(); }

    bool isGrabbed() const noexcept { return grabbed; }

    void update (Seed newSeed, float position, bool isRunning, bool isReverse)
    {
        if (seed != newSeed)
        {
            seed = newSeed;
            rebuildCurve();
        }
        readPosition = juce::jlimit (0.0f, 1.0f, position);
        running = isRunning;
        reverse = isReverse;
        repaint();
    }

    void resized() override
    {
        graph = { 28.0f, 9.0f, static_cast<float> (getWidth()) - 56.0f, 144.0f };
        ruler = { graph.getX(), graph.getBottom() + 18.0f, graph.getWidth(), 24.0f };
        rebuildCurve();
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (background.withAlpha (0.5f));
        g.fillRoundedRectangle (graph.expanded (10.0f, 7.0f), 5.0f);
        g.setColour (border.withAlpha (0.5f));
        for (int line = 0; line <= 4; ++line)
        {
            const auto x = graph.getX() + graph.getWidth() * static_cast<float> (line) / 4.0f;
            g.drawVerticalLine (juce::roundToInt (x), graph.getY(), graph.getBottom());
        }
        for (int line = 0; line <= 2; ++line)
        {
            const auto y = graph.getY() + graph.getHeight() * static_cast<float> (line) / 2.0f;
            g.drawHorizontalLine (juce::roundToInt (y), graph.getX(), graph.getRight());
        }

        g.setGradientFill (juce::ColourGradient (teal.withAlpha (0.18f), graph.getTopLeft(),
                                               teal.withAlpha (0.015f), graph.getBottomLeft(), false));
        g.fillPath (area);
        g.setColour (teal);
        g.strokePath (curve, juce::PathStrokeType (2.25f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const float dashes[] { 2.0f, 4.0f };
        g.setColour (amber.withAlpha (0.6f));
        g.drawDashedLine ({ graph.getTopLeft(), graph.getBottomLeft() }, dashes, 2, 1.0f);
        g.drawDashedLine ({ graph.getTopRight(), graph.getBottomRight() }, dashes, 2, 1.0f);

        const auto x = graph.getX() + readPosition * graph.getWidth();
        const auto y = graph.getBottom() - contourValue (seed, readPosition) * graph.getHeight();
        g.setColour (teal.withAlpha (0.08f));
        g.fillRect (x - 4.0f, graph.getY(), 8.0f, ruler.getCentreY() - graph.getY());
        g.setColour (teal.withAlpha (0.75f));
        g.drawLine (x, graph.getY(), x, ruler.getCentreY(), 1.0f);
        g.setColour (background);
        g.fillEllipse (x - 5.0f, y - 5.0f, 10.0f, 10.0f);
        g.setColour (ink);
        g.fillEllipse (x - 3.0f, y - 3.0f, 6.0f, 6.0f);

        g.setColour (teal.withAlpha (grabbed ? 0.12f : 0.055f));
        g.fillRoundedRectangle (ruler, 6.0f);
        g.setColour (teal.withAlpha (0.16f));
        g.drawRoundedRectangle (ruler, 6.0f, 1.0f);
        g.setColour (muted.withAlpha (0.6f));
        for (int tick = 0; tick <= 32; ++tick)
        {
            const auto tickX = ruler.getX() + ruler.getWidth() * static_cast<float> (tick) / 32.0f;
            const auto tickHeight = tick % 8 == 0 ? 8.0f : 4.0f;
            g.drawLine (tickX, ruler.getCentreY() - tickHeight * 0.5f,
                        tickX, ruler.getCentreY() + tickHeight * 0.5f, 1.0f);
        }
        g.setColour (grabbed ? ink : teal);
        g.fillRoundedRectangle (x - 11.0f, ruler.getY() + 1.0f, 22.0f, 22.0f, 5.0f);
        g.setColour (background);
        g.drawLine (x - 2.0f, ruler.getY() + 7.0f, x - 2.0f, ruler.getBottom() - 7.0f, 1.0f);
        g.drawLine (x + 2.0f, ruler.getY() + 7.0f, x + 2.0f, ruler.getBottom() - 7.0f, 1.0f);

        g.setFont (font (10.5f));
        g.setColour (muted);
        const auto captionY = juce::roundToInt (ruler.getBottom() + 5.0f);
        g.drawText ("0%", 28, captionY, 65, 15, juce::Justification::centredLeft);
        g.setColour (amber.withAlpha (0.85f));
        g.drawText ("100% / WRAP", getWidth() - 133, captionY, 105, 15, juce::Justification::centredRight);
        g.setColour (grabbed ? teal : muted);
        const auto caption = grabbed ? "HOLDING  /  release to " + juce::String (running ? "resume" : "stay here")
                                     : "DRAG THE HANDLE TO SCRUB  /  " + juce::String (running ? (reverse ? "running in reverse" : "free running") : "travel stopped");
        g.drawText (caption, 110, captionY, getWidth() - 245, 15, juce::Justification::centred);
    }

    void mouseMove (const juce::MouseEvent& event) override
    {
        setMouseCursor (ruler.expanded (8.0f).contains (event.position)
                            ? juce::MouseCursor::LeftRightResizeCursor : juce::MouseCursor::NormalCursor);
    }

    void mouseDown (const juce::MouseEvent& event) override
    {
        if (! event.mods.isLeftButtonDown() || ! ruler.expanded (8.0f).contains (event.position))
            return;
        grabbed = true;
        moveGrab (event.position.x);
    }

    void mouseDrag (const juce::MouseEvent& event) override
    {
        if (grabbed)
            moveGrab (event.position.x);
    }

    void mouseUp (const juce::MouseEvent& event) override
    {
        if (! grabbed)
            return;
        moveGrab (event.position.x);
        processor.setPlayheadGrab (false, lastGesturePosition);
        grabbed = false;
        repaint();
    }

    bool keyPressed (const juce::KeyPress& key) override
    {
        if (key.getKeyCode() != juce::KeyPress::leftKey && key.getKeyCode() != juce::KeyPress::rightKey)
            return false;
        const auto direction = key.getKeyCode() == juce::KeyPress::leftKey ? -1.0f : 1.0f;
        const auto increment = key.getModifiers().isShiftDown() ? 0.05f : 0.005f;
        lastGesturePosition = juce::jlimit (0.0f, 1.0f, readPosition + direction * increment);
        processor.setPlayheadGrab (true, lastGesturePosition);
        processor.setPlayheadGrab (false, lastGesturePosition);
        return true;
    }

private:
    void moveGrab (float x)
    {
        lastGesturePosition = juce::jlimit (0.0f, 1.0f, (x - ruler.getX()) / ruler.getWidth());
        processor.setPlayheadGrab (true, lastGesturePosition);
        readPosition = lastGesturePosition;
        repaint();
    }

    void rebuildCurve()
    {
        curve.clear();
        area.clear();
        if (graph.isEmpty())
            return;

        std::vector<double> breaks { 0.0, 1.0 };
        switch (seed)
        {
            case Seed::Ramp: break;
            case Seed::Triangle: breaks = { 0.0, 0.5, 1.0 }; break;
            case Seed::Needle: breaks = { 0.0, 0.35, 0.4, 0.5, 1.0 }; break;
            case Seed::Staircase: breaks = { 0.0, 0.25, 0.5, 0.75, 1.0 }; break;
            case Seed::Gate: breaks = { 0.0, 0.25, 0.75, 1.0 }; break;
            case Seed::Landscape: breaks = { 0.0, 0.16, 0.22, 0.28, 0.36, 0.46, 0.58, 1.0 }; break;
        }
        const auto point = [this] (double position, double valuePosition)
        {
            return juce::Point<float> (graph.getX() + static_cast<float> (position) * graph.getWidth(),
                                      graph.getBottom() - contourValue (seed, valuePosition) * graph.getHeight());
        };
        curve.startNewSubPath (point (0.0, 0.0));
        for (size_t interval = 1; interval < breaks.size(); ++interval)
        {
            const auto start = breaks[interval - 1];
            const auto end = breaks[interval];
            const auto steps = juce::jmax (2, static_cast<int> (std::ceil ((end - start) * 512.0)));
            for (int step = 1; step < steps; ++step)
            {
                const auto p = start + (end - start) * static_cast<double> (step) / static_cast<double> (steps);
                curve.lineTo (point (p, p));
            }
            // Sample both sides at the same x so hard edges are vertical, never diagonal approximations.
            curve.lineTo (point (end, std::nextafter (end, start)));
            curve.lineTo (point (end, end));
        }
        area = curve;
        area.lineTo (graph.getBottomRight());
        area.lineTo (graph.getBottomLeft());
        area.closeSubPath();
    }

    ScrubblesProcessor& processor;
    Seed seed = Seed::Landscape;
    juce::Rectangle<float> graph, ruler;
    juce::Path curve, area;
    float readPosition = 0.0f;
    float lastGesturePosition = 0.0f;
    bool grabbed = false;
    bool running = true;
    bool reverse = false;
};
} // namespace

class ScrubblesEditor::Content final : public juce::Component, private juce::Timer
{
public:
    explicit Content (ScrubblesProcessor& owner)
        : processor (owner), contour (owner), keyboard (owner.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
          tooltips (this, 650)
    {
        setName ("Scrubbles instrument");
        setOpaque (true);
        setLookAndFeel (&look);
        addAndMakeVisible (contour);
        seedSelector.setName (params::seed);
        seedSelector.setTitle ("Seed contour");
        seedSelector.setTooltip ("Change the shape without moving the shared playhead.");
        for (size_t index = 0; index < params::seedNames.size(); ++index)
            seedSelector.addItem (params::seedNames[index], static_cast<int> (index) + 1);
        addAndMakeVisible (seedSelector);
        seedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            processor.apvts, params::seed, seedSelector);

        const std::array<const char*, 5> ids { params::position, params::rate, params::depth, params::cutoff, params::resonance };
        const std::array<const char*, 5> titles { "Position", "Rate", "Depth", "Base cutoff", "Resonance" };
        const std::array<const char*, 5> hints {
            "Phase offset, not the playhead. Moves the contour relative to accumulated travel, even when stopped.",
            "Free-running cycles per second. Stop freezes travel without changing this speed.",
            "Signed cutoff excursion in octaves. Zero leaves cutoff at its base; negative depth closes the filter.",
            "Cutoff at contour value zero. The moving contour adds Depth in octaves above or below this base.",
            "Filter emphasis around the moving cutoff."
        };
        for (size_t index = 0; index < knobs.size(); ++index)
        {
            auto& knob = knobs[index];
            knob.setName (ids[index]);
            knob.setTitle (titles[index]);
            knob.setTooltip (hints[index]);
            knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            knob.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                      juce::MathConstants<float>::pi * 2.75f, true);
            knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 120, 23);
            addAndMakeVisible (knob);
            labels[index].setText (titles[index], juce::dontSendNotification);
            labels[index].setFont (font (13.0f, true));
            labels[index].setColour (juce::Label::textColourId, ink);
            labels[index].setJustificationType (juce::Justification::centred);
            addAndMakeVisible (labels[index]);
            knobAttachments[index] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
                processor.apvts, ids[index], knob);
        }

        configureButton (runButton, params::running, "Stop", "Freeze automatic travel. Notes still release, and Position still scrubs.");
        configureButton (reverseButton, params::reverse, "Reverse", "Reverse automatic travel without moving the current playhead.");
        configureButton (latchButton, params::latch, "Latch chord", "Hold a C minor chord for hands-free exploration. Turn off to release it; MIDI remains playable.");
        runAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.apvts, params::running, runButton);
        reverseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.apvts, params::reverse, reverseButton);
        latchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.apvts, params::latch, latchButton);

        keyboard.setName ("keyboard");
        keyboard.setTitle ("Playable MIDI keyboard");
        keyboard.setAvailableRange (36, 84);
        keyboard.setLowestVisibleKey (36);
        keyboard.setKeyPressBaseOctave (4);
        keyboard.setScrollButtonsVisible (false);
        keyboard.setVelocity (0.75f, true);
        keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xffd9dbd2));
        keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff202829));
        keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, panel);
        keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, teal.withAlpha (0.4f));
        keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, teal.withAlpha (0.8f));
        keyboard.setColour (juce::MidiKeyboardComponent::textLabelColourId, juce::Colour (0xff53615b));
        keyboard.setColour (juce::MidiKeyboardComponent::shadowColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (keyboard);

        seedParameter = processor.apvts.getRawParameterValue (params::seed);
        runningParameter = processor.apvts.getRawParameterValue (params::running);
        reverseParameter = processor.apvts.getRawParameterValue (params::reverse);
        timerCallback();
        startTimerHz (30);
    }

    ~Content() override
    {
        stopTimer();
        processor.releasePlayheadGrab();
        setLookAndFeel (nullptr);
    }

    void resized() override
    {
        seedSelector.setBounds (738, 100, 176, 30);
        contour.setBounds (24, 136, 912, 222);
        constexpr int columnWidth = 178;
        for (size_t index = 0; index < knobs.size(); ++index)
        {
            const auto x = 35 + static_cast<int> (index) * columnWidth;
            labels[index].setBounds (x, 383, columnWidth, 21);
            knobs[index].setBounds (x + 22, 405, columnWidth - 44, 106);
        }
        runButton.setBounds (24, 546, 112, 40);
        reverseButton.setBounds (146, 546, 112, 40);
        latchButton.setBounds (268, 546, 138, 40);
        keyboard.setBounds (38, 630, 884, 66);
        keyboard.setKeyWidth (884.0f / 29.0f);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (background);
        g.setColour (ink);
        g.setFont (font (29.0f, true).withExtraKerningFactor (0.065f));
        g.drawText ("scrubbles", 25, 18, 300, 36, juce::Justification::centredLeft);
        g.setColour (muted);
        g.setFont (font (12.0f));
        g.drawText ("One contour. A shared motion. Your hands on the sound.", 27, 54, 600, 20, juce::Justification::centredLeft);

        const auto held = contour.isGrabbed();
        const auto statusColour = held ? amber : (isRunning ? teal : muted);
        g.setColour (statusColour.withAlpha (0.09f));
        g.fillRoundedRectangle (788.0f, 29.0f, 148.0f, 30.0f, 15.0f);
        g.setColour (statusColour);
        g.fillEllipse (802.0f, 41.0f, 6.0f, 6.0f);
        g.setFont (font (11.0f, true));
        g.drawText (held ? "HOLDING" : (isRunning ? "FREE RUNNING" : "STOPPED"), 816, 29, 111, 30, juce::Justification::centredLeft);

        drawCard (g, { 24.0f, 88.0f, 912.0f, 270.0f });
        drawCard (g, { 24.0f, 372.0f, 912.0f, 160.0f });
        drawCard (g, { 24.0f, 600.0f, 912.0f, 110.0f });
        g.setFont (font (11.0f, true));
        g.setColour (ink);
        g.drawText ("FILTER CONTOUR", 44, 101, 150, 27, juce::Justification::centredLeft);
        g.setColour (muted);
        g.setFont (font (10.0f));
        g.drawText ("READ-ONLY", 184, 101, 92, 27, juce::Justification::centredLeft);
        g.drawText ("SEED", 696, 101, 38, 27, juce::Justification::centredLeft);
        g.setColour (teal);
        g.setFont (font (12.0f));
        g.drawText ("READ " + params::positionText (actualPosition, 0), 330, 101, 144, 27, juce::Justification::centredLeft);
        g.setColour (ink);
        g.drawText (params::cutoffText (actualCutoff, 0), 510, 101, 139, 27, juce::Justification::centredRight);
        g.setFont (font (9.5f));
        g.setColour (muted);
        g.drawText ("ACTUAL CUTOFF", 463, 101, 83, 27, juce::Justification::centredLeft);

        const std::array<const char*, 5> subtitles { "PHASE OFFSET", "CYCLES / SECOND", "SIGNED OCTAVES", "CONTOUR BASE", "FILTER EMPHASIS" };
        g.setFont (font (9.0f));
        for (size_t index = 0; index < subtitles.size(); ++index)
            g.drawText (subtitles[index], 35 + static_cast<int> (index) * 178, 512, 178, 15, juce::Justification::centred);
        g.setColour (border.withAlpha (0.65f));
        g.drawVerticalLine (568, 393.0f, 511.0f);

        g.setColour (ink);
        g.setFont (font (12.0f));
        g.drawText ("Try a latched chord, then grab the playhead.", 426, 544, 490, 22, juce::Justification::centredLeft);
        g.setColour (muted);
        g.setFont (font (10.5f));
        g.drawText ("Position is an offset. The ruler takes direct control. Double-click a knob to reset.", 426, 566, 490, 18, juce::Justification::centredLeft);

        g.setColour (ink);
        g.setFont (font (10.5f, true));
        g.drawText ("PLAY NOTES", 40, 605, 120, 22, juce::Justification::centredLeft);
        g.setColour (muted);
        g.setFont (font (10.5f));
        g.drawText ("MIDI or click keys  /  focus keyboard for A-K  /  latch holds C minor", 195, 605, 724, 22, juce::Justification::centredRight);
    }

private:
    static void drawCard (juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        g.setColour (panel);
        g.fillRoundedRectangle (bounds, 11.0f);
        g.setColour (border.withAlpha (0.65f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), 11.0f, 1.0f);
    }

    void configureButton (juce::TextButton& button, const char* name, const char* title, const char* hint)
    {
        button.setName (name);
        button.setButtonText (title);
        button.setTooltip (hint);
        button.setClickingTogglesState (true);
        addAndMakeVisible (button);
    }

    void timerCallback() override
    {
        const auto rawSeed = seedParameter->load (std::memory_order_relaxed);
        const auto selectedSeed = std::isfinite (rawSeed) ? juce::jlimit (0, 5, juce::roundToInt (rawSeed)) : 5;
        isRunning = runningParameter->load (std::memory_order_relaxed) >= 0.5f;
        actualPosition = processor.displayPosition.load (std::memory_order_relaxed);
        actualCutoff = processor.displayCutoff.load (std::memory_order_relaxed);
        contour.update (static_cast<Seed> (selectedSeed), actualPosition, isRunning,
                        reverseParameter->load (std::memory_order_relaxed) >= 0.5f);
        runButton.setButtonText (isRunning ? "Stop" : "Run");
        repaint (788, 29, 148, 30);
        repaint (320, 100, 340, 30);
    }

    InstrumentLook look;
    ScrubblesProcessor& processor;
    ContourView contour;
    juce::ComboBox seedSelector;
    std::array<juce::Slider, 5> knobs;
    std::array<juce::Label, 5> labels;
    juce::TextButton runButton, reverseButton, latchButton;
    juce::MidiKeyboardComponent keyboard;
    juce::TooltipWindow tooltips;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> seedAttachment;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 5> knobAttachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> runAttachment, reverseAttachment, latchAttachment;
    std::atomic<float>* seedParameter = nullptr;
    std::atomic<float>* runningParameter = nullptr;
    std::atomic<float>* reverseParameter = nullptr;
    bool isRunning = true;
    float actualPosition = 0.0f;
    float actualCutoff = 250.0f;
};

ScrubblesEditor::ScrubblesEditor (ScrubblesProcessor& processor)
    : AudioProcessorEditor (processor), content (std::make_unique<Content> (processor))
{
    setName ("Scrubbles");
    setResizable (false, false);
    addAndMakeVisible (*content);
    setSize (960, 720);
}

ScrubblesEditor::~ScrubblesEditor() = default;

void ScrubblesEditor::resized()
{
    content->setBounds (getLocalBounds());
}
} // namespace scrubbles
