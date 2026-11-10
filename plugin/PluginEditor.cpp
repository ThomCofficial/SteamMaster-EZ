#include "PluginEditor.h"
#include <algorithm>
#include <cmath>

namespace
{
constexpr auto background = juce::Colour(0xff0a1019);
constexpr auto panelDark = juce::Colour(0xff111d2a);
constexpr auto panelLight = juce::Colour(0xff1c2b3a);
constexpr auto copper = juce::Colour(0xffd4a76f);
constexpr auto aqua = juce::Colour(0xff66d9c6);
constexpr auto secondaryText = juce::Colour(0xff8fa6b7);

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
        g.setFont(juce::Font(juce::FontOptions(11.5f)));
        g.drawFittedText(enabled ? "ACTIVE" : "BYPASS", area.toNearestInt(),
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
        const float x = width - 16.f, y = height * 0.5f;
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
        label.setBounds(9, 1, box.getWidth() - 33, box.getHeight() - 2);
        label.setFont(juce::Font(juce::FontOptions(12.f)));
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
        title.setFont(juce::Font(juce::FontOptions(10.5f)));
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
            knob->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 22);
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
        title.setBounds(5, 7, w - 10, 19);
        if (knob) knob->setBounds(7, 26, w - 14, h - 31);
        if (combo) combo->setBounds(9, 54, w - 18, 30);
        if (toggle) toggle->setBounds(12, 52, w - 24, 32);
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

class SteamMasterEditor::ModulePanel final : public juce::Component
{
public:
    struct Group
    {
        juce::String section;
        std::vector<std::unique_ptr<ParameterCell>> controls;
    };

    ModulePanel(juce::AudioProcessor& p, juce::AudioProcessorValueTreeState& state,
                const juce::String& name, const juce::String& subtitle,
                std::initializer_list<const char*> sections, int number)
        : title(name), description(subtitle), index(number)
    {
        for (const char* section : sections)
        {
            Group group;
            group.section = section;
            for (auto* parameter : p.getParameters())
            {
                auto* identified = dynamic_cast<juce::AudioProcessorParameterWithID*>(parameter);
                if (identified == nullptr ||
                    !identified->getName(256).startsWith(group.section + " / "))
                    continue;
                auto cell = std::make_unique<ParameterCell>(state, *identified);
                addAndMakeVisible(*cell);
                group.controls.push_back(std::move(cell));
            }
            groups.push_back(std::move(group));
        }
    }

    int requiredHeight(int width) const
    {
        const auto columns = columnsForWidth(width);
        int y = 71;
        for (const auto& group : groups)
        {
            if (groups.size() > 1) y += 26;
            y += int((group.controls.size() + size_t(columns) - 1) / size_t(columns)) * cellHeight;
            y += 12;
        }
        return y + 3;
    }

    void paint(juce::Graphics& g) override
    {
        const auto area = getLocalBounds().toFloat().reduced(2.f);
        g.setGradientFill(juce::ColourGradient(panelLight, area.getX(), area.getY(),
                                               panelDark, area.getRight(), area.getBottom(), false));
        g.fillRoundedRectangle(area, 11.f);
        g.setColour(juce::Colour(0xff344554));
        g.drawRoundedRectangle(area, 11.f, 1.f);

        g.setColour(index % 2 == 0 ? copper : aqua);
        g.fillRoundedRectangle(juce::Rectangle<float>(15.f, 18.f, 3.f, 31.f), 1.f);
        g.setColour(juce::Colour(0xffeef3f4));
        g.setFont(juce::Font(juce::FontOptions(20.f)));
        g.drawFittedText(title, 27, 13, getWidth() - 125, 28,
                         juce::Justification::centredLeft, 1);
        g.setColour(secondaryText);
        g.setFont(juce::Font(juce::FontOptions(10.5f)));
        g.drawFittedText(description.toUpperCase(), 28, 40, getWidth() - 115, 19,
                         juce::Justification::centredLeft, 1);
        g.setColour(copper.withAlpha(0.75f));
        g.setFont(juce::Font(juce::FontOptions(12.f)));
        g.drawText(juce::String(index).paddedLeft('0', 2), getWidth() - 60, 20, 34, 22,
                   juce::Justification::centredRight);

        if (groups.size() > 1)
        {
            int y = 71;
            for (const auto& group : groups)
            {
                g.setColour(secondaryText);
                g.setFont(juce::Font(juce::FontOptions(11.5f)));
                g.drawText(group.section.toUpperCase(), 27, y, getWidth() - 54, 19,
                           juce::Justification::centredLeft);
                y += 26;
                const auto columns = columnsForWidth(getWidth());
                y += int((group.controls.size() + size_t(columns) - 1) / size_t(columns)) * cellHeight;
                y += 12;
            }
        }
    }

    void resized() override
    {
        const int cols = columnsForWidth(getWidth());
        const int cellW = (getWidth() - 42) / cols;
        int y = 71;
        for (auto& group : groups)
        {
            if (groups.size() > 1) y += 26;
            for (int i = 0; i < int(group.controls.size()); ++i)
            {
                const int x = 21 + (i % cols) * cellW;
                const int rowY = y + (i / cols) * cellHeight;
                group.controls[size_t(i)]->setBounds(x, rowY, cellW - 5, cellHeight - 5);
            }
            y += int((group.controls.size() + size_t(cols) - 1) / size_t(cols)) * cellHeight;
            y += 12;
        }
    }

private:
    static constexpr int cellHeight = 123;
    int columnsForWidth(int width) const { return juce::jlimit(3, 8, (width - 42) / 146); }
    juce::String title;
    juce::String description;
    int index = 0;
    std::vector<Group> groups;
};

SteamMasterEditor::SteamMasterEditor(SteamMasterProcessor& p)
    : juce::AudioProcessorEditor(&p), processor(p),
      skin(std::make_unique<EzLookAndFeel>())
{
    setLookAndFeel(skin.get());
    setOpaque(true);
    viewport.setViewedComponent(&rackCanvas, false);
    viewport.setScrollBarsShown(true, false);
    viewport.setScrollBarThickness(7);
    addAndMakeVisible(viewport);

    const auto addModule = [&](const char* title, const char* tagline,
                               std::initializer_list<const char*> parts)
    {
        auto module = std::make_unique<ModulePanel>(processor, processor.state,
                                                    title, tagline, parts,
                                                    int(modules.size()) + 1);
        rackCanvas.addAndMakeVisible(*module);
        modules.push_back(std::move(module));
    };

    addModule("Master Control", "Signal integrity  /  gain staging  /  oversampling", {"Global"});
    addModule("Ocean & Furnace", "Dual analog-style dynamics engines", {"Ocean", "Furnace"});
    addModule("Parametric Equalizer", "Four fully independent precision bands", {"Parametric"});
    addModule("Pultec Equalizer", "Broad musical tone shaping", {"Pultec"});
    addModule("Shadow Compressor", "Optical and discrete compression stages", {"Shadow"});
    addModule("Exciter", "Dual-band harmonic character", {"Exciter"});
    addModule("Baxandall Mid / Side", "Independent M/S tone control  /  linked routing", {"Bax Mid", "Bax Side"});
    addModule("Tape Machine", "Studio  /  Vintage  /  Modern saturation", {"Tape"});
    addModule("Stereo Image", "Mono Maker  /  width  /  routing", {"Stereo"});
    addModule("Master Limiter", "Final output safety  /  true-peak control", {"Limiter"});

    setResizable(true, true);
    setResizeLimits(790, 520, 1800, 1400);
    setSize(1130, 820);
    startTimerHz(25);
}

SteamMasterEditor::~SteamMasterEditor()
{
    stopTimer();
    viewport.setViewedComponent(nullptr, false);
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
    g.setColour(copper);
    g.fillRect(0, 0, getWidth(), 3);

    g.setColour(juce::Colour(0xfff3f3ee));
    g.setFont(juce::Font(juce::FontOptions(27.f)));
    g.drawText("STEAMMASTER", 24, 15, 245, 36, juce::Justification::centredLeft);
    g.setColour(aqua);
    g.drawText("EZ", 263, 15, 55, 36, juce::Justification::centredLeft);
    g.setColour(secondaryText);
    g.setFont(juce::Font(juce::FontOptions(11.f)));
    g.drawText("PRECISION MASTERING  //  NATIVE JUCE EDITION", 27, 52, 420, 19,
               juce::Justification::centredLeft);

    const int x = getWidth() - 352;
    g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.setColour(secondaryText);
    g.drawText("INPUT", x, 12, 65, 18, juce::Justification::centredLeft);
    g.drawText("OUTPUT", x + 167, 12, 80, 18, juce::Justification::centredLeft);
    for (int i = 0; i < 4; ++i)
    {
        const int pair = i / 2, channel = i % 2;
        const int sx = x + pair * 167;
        const int sy = 34 + channel * 21;
        g.setColour(secondaryText);
        g.drawText(channel == 0 ? "L" : "R", sx, sy, 13, 13, juce::Justification::centredLeft);
        drawMeter(g, juce::Rectangle<float>(float(sx + 17), float(sy + 2), 131.f, 9.f),
                  meterLevel[size_t(i)]);
    }

    g.setColour(juce::Colour(0xff354854));
    g.drawLine(22.f, 87.f, float(getWidth() - 22), 87.f, 1.f);
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
    repaint(juce::Rectangle<int>(getWidth() - 365, 8, 355, 74));
}

void SteamMasterEditor::resized()
{
    viewport.setBounds(15, 100, getWidth() - 30, getHeight() - 111);
    const int contentWidth = juce::jmax(450, viewport.getWidth() - 12);
    int y = 0;
    for (auto& module : modules)
    {
        const int moduleHeight = module->requiredHeight(contentWidth);
        module->setBounds(0, y, contentWidth, moduleHeight);
        y += moduleHeight + 10;
    }
    rackCanvas.setSize(contentWidth, y + 10);
}
