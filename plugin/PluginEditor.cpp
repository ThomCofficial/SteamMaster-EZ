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
        const float cx = x + width * 0.5f, cy = y + height * 0.5f;
        const float radius = juce::jmin(float(width), float(height)) * 0.365f;
        const float angle = startAngle + pos * (endAngle - startAngle);
        const auto ring = radius + 4.f;

        juce::Path track, progress;
        track.addCentredArc(cx, cy, ring, ring, 0.f, startAngle, endAngle, true);
        progress.addCentredArc(cx, cy, ring, ring, 0.f, startAngle, angle, true);
        g.setColour(juce::Colour(0xff354652));
        g.strokePath(track, juce::PathStrokeType(3.2f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
        g.setColour(aqua);
        g.strokePath(progress, juce::PathStrokeType(3.6f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

        const auto disc = juce::Rectangle<float>(cx-radius, cy-radius, radius*2, radius*2);
        g.setColour(juce::Colour(0xff071019));
        g.fillEllipse(disc.expanded(1.5f));
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff536777), cx-radius, cy-radius,
                                              juce::Colour(0xff152432), cx+radius, cy+radius, false));
        g.fillEllipse(disc);
        g.setColour(copper.withAlpha(0.55f));
        g.drawEllipse(disc.reduced(1.4f), 0.8f);
        const float inner = radius*0.75f;
        g.setColour(copper);
        g.drawLine(cx, cy, cx+std::sin(angle)*inner, cy-std::cos(angle)*inner, 3.f);
        g.setColour(juce::Colour(0xff0d1a24));
        g.fillEllipse(cx-2.5f, cy-2.5f, 5.f, 5.f);
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


// Retain the full-precision DSP parameters; only the compact readout changes.
juce::String formatParameterValue(const juce::String& id, double value)
{
    const bool frequency = id.contains("freq") || id.endsWith("_hz");
    const bool percentage = id.contains("mix") || id.contains("percent")
                         || id == "limiter_stereo_link";
    const bool milliseconds = id.endsWith("_ms") || id == "ocean_attack_ms"
                           || id == "limiter_lookahead_ms";
    const bool decibels = id.endsWith("_db") || id.contains("gain")
                       || id.contains("threshold") || id.contains("ceiling")
                       || id.contains("atten") || id == "tape_drive_db";
    const auto wholeOrOneDecimal = [](double x)
    {
        return juce::String(x, std::abs(x - std::round(x)) < 0.045 ? 0 : 1);
    };

    if (frequency)
    {
        if (value >= 1000.0)
            return juce::String(value / 1000.0, value >= 10000.0 ? 1 : 2) + "k";
        return wholeOrOneDecimal(value) + "Hz";
    }
    if (percentage) return wholeOrOneDecimal(value) + "%";
    if (milliseconds) return wholeOrOneDecimal(value) + "ms";
    if (decibels) return wholeOrOneDecimal(value) + "dB";
    if (id.endsWith("_q")) return juce::String(value, 2);
    return wholeOrOneDecimal(value);
}

double parseParameterValue(const juce::String& id, const juce::String& text)
{
    const auto entered = text.trim().toLowerCase();
    double value = entered.getDoubleValue();
    if ((id.contains("freq") || id.endsWith("_hz")) && entered.containsChar('k'))
        value *= 1000.0;
    return value;
}

class ParameterCell final : public juce::Component
{
public:
    ParameterCell(juce::AudioProcessorValueTreeState& state,
                  juce::AudioProcessorParameterWithID& parameter)
    {
        const auto id = parameter.paramID;
        auto name = parameter.getName(256).fromFirstOccurrenceOf(" / ", false, false);
        if (id.startsWith("parametric_band_"))
        {
            const auto number = id.fromFirstOccurrenceOf("band_", false, false)
                                  .upToFirstOccurrenceOf("_", false, false);
            const auto kind = name.fromLastOccurrenceOf(" ", false, false);
            name = "B" + number + " " + kind;
        }
        else if (id == "global_in") name = "MASTER IN";
        else if (name == "IN") name = "IN";
        else if (name.length() > 15) name = name.replace("Frequency", "Freq")
                                                 .replace("Threshold", "Thresh")
                                                 .replace("Attenuation", "Atten")
                                                 .replace("Oversampling", "OS");
        title.setText(name.toUpperCase(), juce::dontSendNotification);
        title.setFont(juce::Font(juce::FontOptions(10.1f)));
        setComponentID(id);
        setTooltip(parameter.getName(256));
        title.setJustificationType(juce::Justification::centred);
        title.setColour(juce::Label::textColourId, secondaryText);
        title.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(title);

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
            knob->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 17);
            knob->setScrollWheelEnabled(true);
            knob->textFromValueFunction = [id](double v) { return formatParameterValue(id, v); };
            knob->valueFromTextFunction = [id](const juce::String& text)
            {
                return parseParameterValue(id, text);
            };
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
        if (knob) knob->setBounds(3, 16, w - 6, h - 18);
        if (combo) combo->setBounds(4, 25, w - 8, 27);
        if (toggle) toggle->setBounds(4, 24, w - 8, 29);
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
// Every module gets a permanent, fixed coordinate layout. The compact parametric
// row has 19 cells in one line; no control can spill into another row.
class SteamMasterEditor::ModulePanel final : public juce::Component
{
public:
    ModulePanel(SteamMasterProcessor& engine, juce::AudioProcessorValueTreeState& state,
                const juce::String& name, const juce::String& subtitle,
                std::initializer_list<const char*> sections, int number)
        : processor(engine), title(name), description(subtitle), index(number)
    {
        for (const auto* section : sections)
        {
            const int first = int(controls.size());
            for (auto* parameter : processor.getParameters())
            {
                auto* identified = dynamic_cast<juce::AudioProcessorParameterWithID*>(parameter);
                if (identified == nullptr ||
                    !identified->getName(256).startsWith(juce::String(section) + " / "))
                    continue;
                if (identified->getName(256).endsWith(" / IN"))
                {
                    if (auto* flag = state.getRawParameterValue(identified->paramID))
                        activeFlags.push_back(flag);
                }
                auto cell = std::make_unique<ParameterCell>(state, *identified);
                addAndMakeVisible(*cell);
                controls.push_back(std::move(cell));
            }
            if (first != int(controls.size()))
                groups.emplace_back(juce::String(section), first);
        }
        jassert(!controls.empty());
    }

    int requiredHeight() const noexcept { return index == 3 ? 84 : 72; }

    void refreshStatus()
    {
        bool next = false;
        for (auto* flag : activeFlags)
            next = next || flag->load(std::memory_order_relaxed) > 0.5f;
        if (next != isActive)
        {
            isActive = next;
            for (auto& control : controls)
                control->setAlpha(isActive ? 1.f : 0.84f);
            repaint();
        }
        else if (index == 2 || index == 5 || index == 10)
        {
            // Small gain-reduction displays are read-only telemetry.
            repaint(juce::Rectangle<int>(15, 52, 169, 18));
        }
    }

    void paint(juce::Graphics& g) override
    {
        const auto area = getLocalBounds().toFloat().reduced(1.0f);
        g.setGradientFill(juce::ColourGradient(panelLight, 0.f, 0.f,
                                               panelDark, float(getWidth()), float(getHeight()), false));
        g.fillRoundedRectangle(area, 8.f);
        g.setColour(isActive ? aqua.withAlpha(0.65f) : juce::Colour(0xff344554));
        g.drawRoundedRectangle(area, 8.f, isActive ? 1.3f : 0.8f);

        // Solid rack identity block + illuminated channel status.
        g.setColour(isActive ? aqua : copper.withAlpha(0.42f));
        g.fillRoundedRectangle(juce::Rectangle<float>(9.f, 12.f, 3.f,
                                float(getHeight() - 24)), 1.5f);
        g.setColour(isActive ? aqua : juce::Colour(0xff465764));
        g.fillEllipse(juce::Rectangle<float>(164.f, 14.f, 8.f, 8.f));
        g.setColour(juce::Colour(0xfff0f4f5));
        g.setFont(juce::Font(juce::FontOptions(16.8f)));
        g.drawFittedText(title, 18, 9, 147, 25, juce::Justification::centredLeft, 1);
        g.setColour(secondaryText);
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.drawFittedText(description.toUpperCase(), 18, 34, 158, 16,
                         juce::Justification::centredLeft, 1);
        g.setColour(isActive ? aqua : juce::Colour(0xff8298a6));
        g.setFont(juce::Font(juce::FontOptions(9.5f)));
        g.drawText(isActive ? "ACTIVE" : "BYPASSED", 18, 52, 92, 16,
                   juce::Justification::centredLeft);

        const auto printReduction = [&](const juce::String& label, float value)
        {
            const auto reduction = std::abs(value);
            return label + juce::String(reduction, 1) + "dB";
        };

        juce::String reduction;
        if (index == 2)
            reduction = printReduction("O ", processor.oceanReductionDb.load(std::memory_order_relaxed))
                      + "  " + printReduction("F ", processor.furnaceReductionDb.load(std::memory_order_relaxed));
        else if (index == 5)
            reduction = printReduction("O ", processor.shadowOpticalReductionDb.load(std::memory_order_relaxed))
                      + "  " + printReduction("D ", processor.shadowDiscreteReductionDb.load(std::memory_order_relaxed));
        else if (index == 10)
            reduction = printReduction("GR ", processor.limiterReductionDb.load(std::memory_order_relaxed));
        if (reduction.isNotEmpty())
        {
            g.setColour(copper);
            g.drawFittedText(reduction, 94, 52, 91, 16,
                             juce::Justification::centredRight, 1);
        }

        g.setColour(juce::Colour(0xff3b4c5b));
        g.drawLine(float(controlLeft - 8), 7.f,
                   float(controlLeft - 8), float(getHeight() - 7), 1.f);

        // Parametric EQ: group all 4 controls for each of 4 bands on ONE row.
        if (index == 3)
        {
            constexpr int bandStart = 3;
            constexpr int firstX = controlLeft + 2;
            constexpr int pitch = 64;
            for (int band = 0; band < 4; ++band)
            {
                const float x = float(firstX + (bandStart + band * 4) * pitch - 3);
                const auto bandArea = juce::Rectangle<float>(x, 6.f, float(pitch * 4 - 2), 71.f);
                g.setColour((band % 2 == 0 ? aqua : copper).withAlpha(0.055f));
                g.fillRoundedRectangle(bandArea, 6.f);
                g.setColour((band % 2 == 0 ? aqua : copper).withAlpha(0.42f));
                g.drawRoundedRectangle(bandArea, 6.f, 0.8f);
            }
        }
        else if (groups.size() > 1)
        {
            const int count = int(controls.size());
            const float slot = float(usableWidth) / float(count);
            for (size_t i = 1; i < groups.size(); ++i)
            {
                const float x = float(controlLeft) + slot * float(groups[i].second);
                g.setColour(copper.withAlpha(0.45f));
                g.drawLine(x - 2.f, 12.f, x - 2.f, float(getHeight() - 12), 1.5f);
            }
        }
    }

    void resized() override
    {
        // Only the fixed virtual design determines control rectangles.
        const int count = int(controls.size());
        if (index == 3)
        {
            constexpr int firstX = controlLeft + 2;
            constexpr int pitch = 64;
            for (int i = 0; i < count; ++i)
                controls[size_t(i)]->setBounds(firstX + i * pitch, 17, 60, 57);
        }
        else
        {
            const int slot = usableWidth / juce::jmax(1, count);
            const int cellWidth = juce::jlimit(60, 106, slot - 7);
            for (int i = 0; i < count; ++i)
            {
                const int x = controlLeft + slot * i + (slot - cellWidth) / 2;
                controls[size_t(i)]->setBounds(x, 7, cellWidth, 58);
            }
        }
    }

private:
    static constexpr int controlLeft = 208;
    static constexpr int usableWidth = 1220;
    SteamMasterProcessor& processor;
    juce::String title, description;
    int index = 0;
    bool isActive = false;
    std::vector<std::unique_ptr<ParameterCell>> controls;
    std::vector<std::pair<juce::String, int>> groups;
    std::vector<std::atomic<float>*> activeFlags;
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
        module->refreshStatus();
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
                                 float peak, float held) const
{
    g.setColour(juce::Colour(0xff091720));
    g.fillRoundedRectangle(area, 3.f);
    const auto asPosition = [](float amplitude)
    {
        const float dB = juce::Decibels::gainToDecibels(juce::jmax(0.000001f, amplitude));
        return juce::jlimit(0.f, 1.f, (dB + 60.f) / 60.f);
    };
    const float fraction = asPosition(peak), heldFraction = asPosition(held);
    if (fraction > 0.001f)
    {
        auto fill = area.reduced(1.f);
        fill.setWidth((fill.getWidth() - 1.f) * fraction);
        g.setGradientFill(juce::ColourGradient(aqua, fill.getX(), fill.getY(),
                            fraction > 0.94f ? juce::Colour(0xffef8d67) : copper,
                            fill.getRight(), fill.getBottom(), false));
        g.fillRoundedRectangle(fill, 2.f);
    }
    for (const float dB : {-48.f, -24.f, -12.f, -6.f})
    {
        const float x = area.getX() + (dB + 60.f)/60.f*area.getWidth();
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.drawLine(x, area.getY(), x, area.getBottom(), 0.8f);
    }
    if (heldFraction > 0.01f)
    {
        const float x = area.getX() + heldFraction*area.getWidth();
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.drawLine(x, area.getY() + 1.f, x, area.getBottom()-1.f, 1.2f);
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

    const int x = designWidth - 485;
    g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.setColour(secondaryText);
    g.drawText("INPUT PEAK", x, 6, 100, 15, juce::Justification::centredLeft);
    g.drawText("OUTPUT PEAK", x + 238, 6, 110, 15, juce::Justification::centredLeft);
    for (int i = 0; i < 4; ++i)
    {
        const int pair = i / 2, channel = i % 2;
        const int sx = x + pair * 238;
        const int sy = 24 + channel * 18;
        g.setColour(secondaryText);
        g.drawText(channel == 0 ? "L" : "R", sx, sy, 13, 13,
                   juce::Justification::centredLeft);
        drawMeter(g, juce::Rectangle<float>(float(sx + 16), float(sy + 1), 150.f, 11.f),
                  meterLevel[size_t(i)], meterPeakHold[size_t(i)]);
        const float peakDb = juce::Decibels::gainToDecibels(
            juce::jmax(0.000001f, meterPeakHold[size_t(i)]));
        g.setColour(peakDb > -0.1f ? juce::Colour(0xfff29575) : juce::Colours::white);
        g.setFont(juce::Font(juce::FontOptions(10.f)));
        g.drawFittedText(peakDb < -59.f ? "-inf" : juce::String(peakDb, 1),
                         sx + 169, sy, 42, 13, juce::Justification::centredRight, 1);
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
    {
        meterLevel[i] = target[i] > meterLevel[i]
                            ? target[i]
                            : 0.78f * meterLevel[i] + 0.22f * target[i];
        if (target[i] >= meterPeakHold[i])
        {
            meterPeakHold[i] = target[i];
            holdTicks[i] = 0;
        }
        else if (++holdTicks[i] > 45)
            meterPeakHold[i] = juce::jmax(target[i], meterPeakHold[i] * 0.94f);
    }
    for (auto& module : modules) module->refreshStatus();

    const auto meters = juce::Rectangle<float>(float(designWidth - 499), 2.f, 495.f, 63.f)
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
