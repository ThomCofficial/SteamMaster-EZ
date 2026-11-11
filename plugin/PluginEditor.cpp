#include "PluginEditor.h"
#include <algorithm>
#include <cmath>

namespace
{
const auto background = juce::Colour(0xff0a1019);
const auto panelDark = juce::Colour(0xff111d2a);
const auto panelLight = juce::Colour(0xff1c2b3a);
const auto copper = juce::Colour(0xffd4a76f);
const auto aqua = juce::Colour(0xff66d9c6);
const auto secondaryText = juce::Colour(0xff8fa6b7);

class EzLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    EzLookAndFeel()
    {
        setColour(juce::ComboBox::textColourId, juce::Colours::white);
        setColour(juce::ComboBox::arrowColourId, copper);
        setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff192838));
        setColour(juce::Label::textColourId, secondaryText);
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe8edf0));
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff142331));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff15212d));
        setColour(juce::PopupMenu::textColourId, juce::Colours::white);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff315d66));
        setColour(juce::ScrollBar::thumbColourId, juce::Colour(0xff44606a));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float pos, float startAngle, float endAngle, juce::Slider&) override
    {
        const float cx = x + width * 0.5f;
        const float cy = y + height * 0.5f;
        const float radius = juce::jmin(float(width), float(height)) * 0.365f;
        const float angle = startAngle + pos * (endAngle - startAngle);
        const auto ring = radius + 4.f;

        juce::Path track;
        track.addCentredArc(cx, cy, ring, ring, 0.f, startAngle, endAngle, true);
        g.setColour(juce::Colour(0xff354652));
        g.strokePath(track, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

        juce::Path active;
        active.addCentredArc(cx, cy, ring, ring, 0.f, startAngle, angle, true);
        g.setColour(aqua.withAlpha(0.90f));
        g.strokePath(active, juce::PathStrokeType(3.4f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

        const auto disc = juce::Rectangle<float>(cx - radius, cy - radius,
                                                  radius * 2.f, radius * 2.f);
        g.setColour(juce::Colour(0xff071019));
        g.fillEllipse(disc.expanded(1.5f));
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff455664), cx - radius, cy - radius,
                                              juce::Colour(0xff172635), cx + radius, cy + radius, false));
        g.fillEllipse(disc);
        g.setColour(juce::Colour(0xff5c7080));
        g.drawEllipse(disc.reduced(1.3f), 1.0f);

        const float inner = radius * 0.67f;
        const float tx = cx + std::sin(angle) * inner;
        const float ty = cy - std::cos(angle) * inner;
        g.setColour(copper);
        g.drawLine(cx, cy, tx, ty, 3.2f);
        g.setColour(juce::Colour(0xff111f2a));
        g.fillEllipse(cx - 3.0f, cy - 3.0f, 6.0f, 6.0f);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool highlighted, bool) override
    {
        const auto area = button.getLocalBounds().toFloat().reduced(6.f, 4.f);
        const bool enabled = button.getToggleState();
        const auto fill = enabled ? juce::Colour(0xff164f4b) : juce::Colour(0xff263441);
        g.setColour(fill.brighter(highlighted ? 0.12f : 0.f));
        g.fillRoundedRectangle(area, 7.f);
        g.setColour(enabled ? aqua : juce::Colour(0xff586b78));
        g.drawRoundedRectangle(area.reduced(0.5f), 7.f, 1.2f);
        g.setFont(juce::Font(juce::FontOptions(9.2f)));
        g.drawFittedText(enabled ? "ON" : "OFF", area.toNearestInt(),
                         juce::Justification::centred, 1);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool,
                      int, int, int, int, juce::ComboBox&) override
    {
        const auto rect = juce::Rectangle<float>(0.f, 0.f, float(width), float(height));
        g.setColour(juce::Colour(0xff1a2a38));
        g.fillRoundedRectangle(rect.reduced(0.5f), 6.f);
        g.setColour(juce::Colour(0xff46616e));
        g.drawRoundedRectangle(rect.reduced(0.7f), 6.f, 1.f);
        const float x = width - 11.f, y = height * 0.5f;
        juce::Path arrow;
        arrow.startNewSubPath(x - 4.f, y - 2.f);
        arrow.lineTo(x + 4.f, y - 2.f);
        arrow.lineTo(x, y + 3.f);
        arrow.closeSubPath();
        g.setColour(copper);
        g.fillPath(arrow);
    }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds(5, 1, box.getWidth() - 19, box.getHeight() - 2);
        label.setFont(juce::Font(juce::FontOptions(10.f)));
    }
};

class ParameterCell final : public juce::Component
{
public:
    ParameterCell(juce::AudioProcessorValueTreeState& state,
                  juce::AudioProcessorParameterWithID& parameter)
    {
        const auto name = parameter.getName(256).fromFirstOccurrenceOf(" / ", false, false);
        title.setText(name.toUpperCase(), juce::dontSendNotification);
        title.setFont(juce::Font(juce::FontOptions(9.5f)));
        title.setJustificationType(juce::Justification::centred);
        title.setColour(juce::Label::textColourId, secondaryText);
        title.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(title);

        const auto id = parameter.paramID;
        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*>(&parameter))
        {
            combo = std::make_unique<juce::ComboBox>();
            combo->addItemList(choice->choices, 1);
            combo->setTooltip(id);
            addAndMakeVisible(*combo);
            comboAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                state, id, *combo);
        }
        else if (dynamic_cast<juce::AudioParameterBool*>(&parameter) != nullptr)
        {
            toggle = std::make_unique<juce::ToggleButton>();
            toggle->setClickingTogglesState(true);
            toggle->setTooltip(id);
            addAndMakeVisible(*toggle);
            buttonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
                state, id, *toggle);
        }
        else
        {
            knob = std::make_unique<juce::Slider>();
            knob->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            knob->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 62, 16);
            knob->setScrollWheelEnabled(true);
            knob->setTooltip(id);
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(&parameter))
                knob->setDoubleClickReturnValue(true,
                    ranged->convertFrom0to1(parameter.getDefaultValue()));
            addAndMakeVisible(*knob);
            sliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                state, id, *knob);
        }
    }

    void paint(juce::Graphics& g) override
    {
        const auto area = getLocalBounds().toFloat().reduced(2.f);
        g.setColour(juce::Colour(0xff152331));
        g.fillRoundedRectangle(area, 8.f);
        g.setColour(juce::Colour(0xff26394a));
        g.drawRoundedRectangle(area, 8.f, 0.8f);
    }

    void resized() override
    {
        const int w = getWidth();
        const int h = getHeight();
        title.setBounds(2, 3, w - 4, 13);
        if (knob) knob->setBounds(3, 13, w - 6, h - 16);
        if (combo) combo->setBounds(4, 20, w - 8, 25);
        if (toggle) toggle->setBounds(4, 18, w - 8, 27);
    }

private:
    juce::Label title;
    std::unique_ptr<juce::Slider> knob;
    std::unique_ptr<juce::ComboBox> combo;
    std::unique_ptr<juce::ToggleButton> toggle;
    // Attachments deliberately destroyed before the controls they reference.
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachment;
};
} // namespace


// Each control has one permanent coordinate on this rack. The panel never
// changes its column count or rearranges knobs based on window width.
class SteamMasterEditor::ModulePanel final : public juce::Component
{
public:
    ModulePanel(juce::AudioProcessor& processor,
                juce::AudioProcessorValueTreeState& state,
                const juce::String& name, const juce::String& subtitle,
                std::initializer_list<const char*> sections, int number)
        : title(name), description(subtitle), index(number)
    {
        for (const auto* section : sections)
        {
            const int firstIndex = int(controls.size());
            for (auto* parameter : processor.getParameters())
            {
                auto* identified = dynamic_cast<juce::AudioProcessorParameterWithID*>(parameter);
                if (identified == nullptr ||
                    !identified->getName(256).startsWith(juce::String(section) + " / "))
                    continue;

                auto cell = std::make_unique<ParameterCell>(state, *identified);
                addAndMakeVisible(*cell);
                controls.push_back(std::move(cell));
            }
            if (firstIndex != int(controls.size()))
                groups.emplace_back(juce::String(section), firstIndex);
        }
        jassert(!controls.empty());
    }

    int requiredHeight() const noexcept
    {
        const int rows = (int(controls.size()) + columns - 1) / columns;
        return topMargin + rows * cellHeight + bottomMargin;
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);
        g.setGradientFill(juce::ColourGradient(panelLight, 0.f, 0.f,
                                               panelDark, float(getWidth()), float(getHeight()),
                                               false));
        g.fillRoundedRectangle(bounds, 8.f);
        g.setColour(juce::Colour(0xff344554));
        g.drawRoundedRectangle(bounds, 8.f, 0.8f);

        // Slim hardware-style sidebar, with the rack identity preserved.
        g.setColour(index % 2 == 0 ? copper : aqua);
        g.fillRoundedRectangle(juce::Rectangle<float>(9.f, 13.f, 3.f,
                                float(getHeight() - 26)), 1.5f);
        g.setColour(juce::Colour(0xffedf2f3));
        g.setFont(juce::Font(juce::FontOptions(17.f)));
        g.drawFittedText(title, 18, 10, 172, 25,
                         juce::Justification::centredLeft, 1);
        g.setColour(secondaryText);
        g.setFont(juce::Font(juce::FontOptions(10.5f)));
        g.drawFittedText(description.toUpperCase(), 18, 36, 171, 27,
                         juce::Justification::topLeft, 2);
        g.setColour(copper.withAlpha(0.6f));
        g.setFont(juce::Font(juce::FontOptions(10.f)));
        g.drawText(juce::String(index).paddedLeft('0', 2), 154, 8, 28, 16,
                   juce::Justification::centredRight);

        g.setColour(juce::Colour(0xff3b4c5b));
        g.drawLine(float(controlLeft - 8), 8.f,
                   float(controlLeft - 8), float(getHeight() - 8), 1.f);

        // Visually group Ocean/Furnace and Mid/Side without moving controls.
        for (size_t i = 1; i < groups.size(); ++i)
        {
            const int firstIndex = groups[i].second;
            const int col = firstIndex % columns;
            if (col != 0)
            {
                const float x = float(controlLeft + col * cellWidth - 2);
                const float y = float(topMargin + (firstIndex / columns) * cellHeight);
                g.setColour(copper.withAlpha(0.55f));
                g.drawLine(x, y + 5.f, x, y + cellHeight - 8.f, 1.5f);
            }
        }
    }

    void resized() override
    {
        // Geometry defined in a fixed 1440-unit space, never by current width.
        for (int i = 0; i < int(controls.size()); ++i)
        {
            const int x = controlLeft + (i % columns) * cellWidth;
            const int y = topMargin + (i / columns) * cellHeight;
            controls[size_t(i)]->setBounds(x, y, cellWidth - 4, cellHeight - 2);
        }
    }

private:
    static constexpr int columns = 18;
    static constexpr int cellWidth = 68;
    static constexpr int cellHeight = 61;
    static constexpr int controlLeft = 208;
    static constexpr int topMargin = 9;
    static constexpr int bottomMargin = 8;

    juce::String title;
    juce::String description;
    int index = 0;
    std::vector<std::unique_ptr<ParameterCell>> controls;
    std::vector<std::pair<juce::String, int>> groups;
};

SteamMasterEditor::SteamMasterEditor(SteamMasterProcessor& p)
    : juce::AudioProcessorEditor(&p), processor(p),
      skin(std::make_unique<EzLookAndFeel>())
{
    setLookAndFeel(skin.get());
    setOpaque(true);
    addAndMakeVisible(rackCanvas);

    const auto addModule = [&](const char* title, const char* tagline,
                               std::initializer_list<const char*> parts)
    {
        auto module = std::make_unique<ModulePanel>(processor, processor.state,
                                                    title, tagline, parts,
                                                    int(modules.size()) + 1);
        rackCanvas.addAndMakeVisible(*module);
        modules.push_back(std::move(module));
    };

    addModule("Master Control", "GAIN / OVERSAMPLING", {"Global"});
    addModule("Ocean & Furnace", "DUAL DYNAMICS", {"Ocean", "Furnace"});
    addModule("Parametric EQ", "FOUR BANDS", {"Parametric"});
    addModule("Pultec EQ", "MUSICAL SHELVES", {"Pultec"});
    addModule("Shadow Compressor", "OPTICAL / DISCRETE", {"Shadow"});
    addModule("Exciter", "HARMONIC DETAIL", {"Exciter"});
    addModule("Baxandall M/S", "MID / SIDE SHELVES", {"Bax Mid", "Bax Side"});
    addModule("Tape Machine", "ANALOG CHARACTER", {"Tape"});
    addModule("Stereo Image", "WIDTH / MONO MAKER", {"Stereo"});
    addModule("Master Limiter", "FINAL OUTPUT", {"Limiter"});

    int y = 0;
    for (auto& module : modules)
    {
        const int height = module->requiredHeight();
        module->setBounds(0, y, designWidth, height);
        y += height + moduleGap;
    }
    rackHeight = y - moduleGap;
    designHeight = headerHeight + rackHeight + footerHeight;
    rackCanvas.setBounds(0, headerHeight, designWidth, rackHeight);
    // The rack and all its interactive child components get one shared transform.
    setResizable(true, true);
    setResizeLimits(900, 580, 1800, 1200);
    setSize(1220, 810);
    startTimerHz(25);
}

SteamMasterEditor::~SteamMasterEditor()
{
    stopTimer();
    modules.clear();
    setLookAndFeel(nullptr);
}

void SteamMasterEditor::drawMeter(juce::Graphics& g, juce::Rectangle<float> area,
                                 float peak) const
{
    g.setColour(juce::Colour(0xff10212c));
    g.fillRoundedRectangle(area, 3.f);
    const float dB = juce::Decibels::gainToDecibels(juce::jmax(0.000001f, peak));
    const float fraction = juce::jlimit(0.f, 1.f, (dB + 60.f) / 60.f);
    if (fraction > 0.001f)
    {
        auto fill = area.reduced(1.f);
        fill.setWidth((fill.getWidth() - 1.f) * fraction);
        g.setGradientFill(juce::ColourGradient(aqua, fill.getX(), fill.getY(),
                           fraction > 0.94f ? juce::Colour(0xffee9068) : copper,
                           fill.getRight(), fill.getBottom(), false));
        g.fillRoundedRectangle(fill, 2.f);
    }
    g.setColour(juce::Colour(0xff364e5a));
    g.drawRoundedRectangle(area, 3.f, 0.8f);
}

void SteamMasterEditor::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff122230), 0.f, 0.f,
                                          background, bounds.getRight(), bounds.getBottom(), false));
    g.fillAll();
    juce::Graphics::ScopedSaveState state(g);
    g.addTransform(juce::AffineTransform::scale(uiScale).translated(uiOffsetX, uiOffsetY));
    g.setColour(copper);
    g.fillRect(0, 0, designWidth, 3);

    g.setColour(juce::Colour(0xfff3f3ee));
    g.setFont(juce::Font(juce::FontOptions(26.f)));
    g.drawText("STEAMMASTER", 19, 11, 245, 35, juce::Justification::centredLeft);
    g.setColour(aqua);
    g.drawText("EZ", 261, 11, 55, 35, juce::Justification::centredLeft);
    g.setColour(secondaryText);
    g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.drawText("COMPACT MASTERING RACK  /  ALL MODULES VISIBLE", 22, 44, 500, 18,
               juce::Justification::centredLeft);

    const int x = designWidth - 352;
    g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.setColour(secondaryText);
    g.drawText("INPUT", x, 8, 65, 16, juce::Justification::centredLeft);
    g.drawText("OUTPUT", x + 167, 8, 80, 16, juce::Justification::centredLeft);
    for (int i = 0; i < 4; ++i)
    {
        const int pair = i / 2, channel = i % 2;
        const int sx = x + pair * 167;
        const int sy = 25 + channel * 17;
        g.setColour(secondaryText);
        g.drawText(channel == 0 ? "L" : "R", sx, sy, 13, 12,
                   juce::Justification::centredLeft);
        drawMeter(g, juce::Rectangle<float>(float(sx + 17), float(sy + 2), 131.f, 8.f),
                  meterLevel[size_t(i)]);
    }
    g.setColour(juce::Colour(0xff354854));
    g.drawLine(10.f, float(headerHeight - 3), float(designWidth - 10),
               float(headerHeight - 3), 1.f);
}

void SteamMasterEditor::timerCallback()
{
    const std::array<float, 4> target{
        processor.inputPeakL.load(std::memory_order_relaxed),
        processor.inputPeakR.load(std::memory_order_relaxed),
        processor.outputPeakL.load(std::memory_order_relaxed),
        processor.outputPeakR.load(std::memory_order_relaxed)
    };
    for (size_t i = 0; i < meterLevel.size(); ++i)
        meterLevel[i] = target[i] > meterLevel[i]
                            ? target[i]
                            : 0.78f * meterLevel[i] + 0.22f * target[i];

    const auto meters = juce::Rectangle<float>(float(designWidth - 367), 2.f, 360.f, 63.f)
                          .transformedBy(juce::AffineTransform::scale(uiScale)
                                         .translated(uiOffsetX, uiOffsetY));
    repaint(meters.toNearestInt());
}

void SteamMasterEditor::resized()
{
    if (designHeight <= 0) return;
    constexpr float margin = 8.f;
    const float availableW = juce::jmax(1.f, float(getWidth()) - margin * 2.f);
    const float availableH = juce::jmax(1.f, float(getHeight()) - margin * 2.f);
    uiScale = juce::jmin(availableW / float(designWidth),
                          availableH / float(designHeight));
    uiOffsetX = (float(getWidth()) - float(designWidth) * uiScale) * 0.5f;
    uiOffsetY = (float(getHeight()) - float(designHeight) * uiScale) * 0.5f;

    // Scale the *entire* fixed-coordinate rack (including controls and hit tests).
    // No scrollbars, no responsive reflow, no per-knob repositioning.
    rackCanvas.setTransform(juce::AffineTransform::scale(uiScale)
                                .translated(uiOffsetX, uiOffsetY));
    repaint();
}
