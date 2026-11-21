#include "PluginEditor.h"
#include "FoundryArt.h"
#include <algorithm>
#include <cmath>

namespace
{
const auto background=foundry::black;
const auto panelDark=foundry::iron;
const auto panelLight=foundry::ironHi;
const auto copper=foundry::gold;
const auto aqua=foundry::teal;
const auto secondaryText=foundry::muted;

class EzLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    EzLookAndFeel()
    {
        setColour(juce::ComboBox::textColourId, juce::Colours::white);
        setColour(juce::ComboBox::arrowColourId, copper);
        setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1b292b));
        setColour(juce::Label::textColourId, secondaryText);
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xfff5e6c9));
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff182224));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff1a2222));
        setColour(juce::PopupMenu::textColourId, juce::Colours::white);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff435747));
        setColour(juce::ScrollBar::thumbColourId, juce::Colour(0xffad8951));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float pos, float startAngle, float endAngle, juce::Slider& knob) override
    {
        foundry::rotary(g,x,y,width,height,pos,startAngle,endAngle,
                        knob.isMouseOverOrDragging());
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool highlighted, bool down) override
    {
        foundry::toggle(g,button.getLocalBounds().toFloat().reduced(4.f,3.f),
                        button.getToggleState(),highlighted,down);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool pressed,
                      int, int, int, int, juce::ComboBox&) override
    {
        foundry::combo(g,juce::Rectangle<float>(.5f,.5f,float(width)-1.f,
                                                float(height)-1.f),pressed);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour&, bool hovered, bool pressed) override
    {
        const auto base=button.findColour(juce::TextButton::buttonColourId);
        foundry::pushButton(g,button.getLocalBounds().toFloat().reduced(1.f,2.f),
                            base,hovered,pressed);
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
    if (std::abs(value) < .0005) value=0.; // Round near-zero floating-point residue.
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
        title.setFont(juce::Font(juce::FontOptions(10.8f)));
        setComponentID(id);
        title.setJustificationType(juce::Justification::centred);
        title.setColour(juce::Label::textColourId, foundry::ivory);
        title.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(title);

        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*>(&parameter))
        {
            combo = std::make_unique<juce::ComboBox>();
            combo->addItemList(choice->choices, 1);
            combo->setTooltip(parameter.getName(256) + ": choose a stepped setting.");
            addAndMakeVisible(*combo);
            comboAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                state, id, *combo);
        }
        else if (dynamic_cast<juce::AudioParameterBool*>(&parameter) != nullptr)
        {
            toggle = std::make_unique<juce::ToggleButton>();
            toggle->setClickingTogglesState(true);
            toggle->setTooltip(parameter.getName(256) + ": enable or bypass.");
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
            knob->setTooltip(parameter.getName(256) + ": drag to adjust; double-click to reset.");
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(&parameter))
                knob->setDoubleClickReturnValue(true,
                    ranged->convertFrom0to1(parameter.getDefaultValue()));
            addAndMakeVisible(*knob);
            sliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                state, id, *knob);
            // Apply value formatting after the APVTS attachment sets up the slider.
            knob->textFromValueFunction = [id](double v) { return formatParameterValue(id, v); };
            knob->valueFromTextFunction = [id](const juce::String& text) { return parseParameterValue(id, text); };
            knob->updateText();
        }
    }

    void paint(juce::Graphics& g) override
    {
        foundry::cell(g,getLocalBounds().toFloat().reduced(1.5f));
    }

    void resized() override
    {
        const int w = getWidth();
        const int h = getHeight();
        title.setBounds(2, 2, w - 4, 13);
        if (knob) knob->setBounds(2, 14, w - 4, h - 16);
        if (combo) combo->setBounds(4, 31, w - 8, 29);
        if (toggle) toggle->setBounds(4, 31, w - 8, 29);
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

    int requiredHeight() const noexcept { return index == 3 ? 98 : 88; }

    void refreshStatus()
    {
        bool next = false;
        for (auto* flag : activeFlags)
            next = next || flag->load(std::memory_order_relaxed) > 0.5f;
        if (next != isActive)
        {
            isActive = next;
            for (auto& control : controls)
                control->setAlpha(isActive ? 1.f : 0.90f);
            repaint();
        }
        else
        {
            if (index == 2 || index == 5 || index == 10)
                repaint(juce::Rectangle<int>(15, 52, 169, 34));
            // The decorative valve filaments and tape hubs animate only while
            // the unit is active. Keep repaints local to the nameplate.
            if (isActive && (index == 5 || index == 6 || index == 8))
                repaint(juce::Rectangle<int>(154, 13, 38, 36));
        }
    }

    void paint(juce::Graphics& g) override
    {
        const auto area=getLocalBounds().toFloat().reduced(1.f);
        const float t=float(juce::Time::getMillisecondCounterHiRes()*.001);
        foundry::moduleSurface(g,area,index,isActive,t);

        // Actual engraved brass title plaque: warm dark lettering belongs
        // on the bright metal, unlike the old white-on-brown digital label.
        g.setColour(juce::Colour(0xff30180b));
        g.setFont(juce::Font(juce::FontOptions(14.1f)));
        g.drawFittedText(title,22,11,127,23,juce::Justification::centredLeft,1);
        g.setColour(foundry::muted);
        g.setFont(juce::Font(juce::FontOptions(9.5f)));
        g.drawFittedText(description.toUpperCase(),19,36,135,14,
                         juce::Justification::centredLeft,1);
        g.setColour(isActive?foundry::teal:foundry::muted.withAlpha(.6f));
        g.setFont(juce::Font(juce::FontOptions(9.3f)));
        g.drawText(isActive?"ACTIVE":"BYPASSED",19,52,92,15,
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
            g.setColour(foundry::gold);
            g.drawFittedText(reduction, 94, 52, 91, 16,
                             juce::Justification::centredRight, 1);
            const auto drawGR = [&](float value, float x, float width) {
                g.setColour(foundry::black);
                g.fillRoundedRectangle(x,74.f,width,5.f,2.f);
                const float amount=width*juce::jlimit(0.f,1.f,std::abs(value)/18.f);
                if(amount>.05f){
                    g.setGradientFill(juce::ColourGradient(isActive?foundry::teal:foundry::bronze,
                                      x,74.f,foundry::gold,x+amount,79.f,false));
                    g.fillRoundedRectangle(x,74.f,amount,5.f,1.6f);
                }
                g.setColour(foundry::bronze);
                g.drawRoundedRectangle(x,74.f,width,5.f,2.f,.7f);
            };
            if (index == 2) {
                drawGR(processor.oceanReductionDb.load(std::memory_order_relaxed), 18.f, 77.f);
                drawGR(processor.furnaceReductionDb.load(std::memory_order_relaxed), 103.f, 77.f);
            } else if (index == 5) {
                drawGR(processor.shadowOpticalReductionDb.load(std::memory_order_relaxed), 18.f, 77.f);
                drawGR(processor.shadowDiscreteReductionDb.load(std::memory_order_relaxed), 103.f, 77.f);
            } else if (index == 10) {
                drawGR(processor.limiterReductionDb.load(std::memory_order_relaxed), 18.f, 162.f);
            }
        }

        g.setColour(juce::Colour(0xff3b4c5b));
        g.drawLine(float(controlLeft - 8), 7.f,
                   float(controlLeft - 8), float(getHeight() - 7), 1.f);

        if (index == 7 && groups.size() == 2) {
            g.setColour(aqua.withAlpha(0.7f));
            g.setFont(juce::Font(juce::FontOptions(10.f)));
            g.drawText("MID", controlLeft + 8, 2, 45, 13, juce::Justification::centredLeft);
            g.drawText("SIDE", controlLeft + 615, 2, 45, 13, juce::Justification::centredLeft);
        }
        // Parametric EQ: group all 4 controls for each of 4 bands on ONE row.
        if (index == 3)
        {
            constexpr int bandStart = 3;
            constexpr int firstX = controlLeft + 2;
            constexpr int pitch = 64;
            for (int band = 0; band < 4; ++band)
            {
                const float x = float(firstX + (bandStart + band * 4) * pitch - 3);
                const auto bandArea = juce::Rectangle<float>(x, 6.f, float(pitch * 4 - 2), 85.f);
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
                controls[size_t(i)]->setBounds(firstX + i * pitch, 18, 60, 74);
        }
        else
        {
            const int slot = usableWidth / juce::jmax(1, count);
            const int cellWidth = juce::jlimit(60, 106, slot - 7);
            for (int i = 0; i < count; ++i)
            {
                const int x = controlLeft + slot * i + (slot - cellWidth) / 2;
                controls[size_t(i)]->setBounds(x, 8, cellWidth, 75);
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
    addAndMakeVisible(compareA);
    addAndMakeVisible(compareB);
    addAndMakeVisible(resetAll);
    resetAll.setTooltip("Reset all parameters to their default values. Requires two clicks to confirm.");
    resetAll.onClick = [this]
    {
        if (resetConfirmTicks == 0)
        {
            resetConfirmTicks = 100; // 4-second confirmation window at 25 Hz
            resetAll.setButtonText("CONFIRM");
            return;
        }
        resetConfirmTicks = 0;
        resetAll.setButtonText("RESET");
        for (auto* parameter : processor.getParameters())
            parameter->setValueNotifyingHost(parameter->getDefaultValue());
    };
    compareA.setComponentID("ez_compare_a");
    compareB.setComponentID("ez_compare_b");
    resetAll.setComponentID("ez_reset");
    compareA.setTooltip("Compare A: recall the stored A settings.");
    compareB.setTooltip("Compare B: recall the stored B settings.");
    compareA.onClick = [this] { switchComparison(0); };
    compareB.onClick = [this] { switchComparison(1); };
    for (auto* param : processor.getParameters()) compareState[0].push_back(param->getValue());
    compareValid[0] = true;

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
    setSize(1370, 960);
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
    foundry::meter(g,area,peak,held);
}

void SteamMasterEditor::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff273534),0.f,0.f,
                                          background,bounds.getRight(),bounds.getBottom(),false));
    g.fillAll();
    juce::Graphics::ScopedSaveState state(g);
    g.addTransform(juce::AffineTransform::scale(uiScale).translated(uiOffsetX,uiOffsetY));

    // Fully bespoke Foundry header; keep original coordinates/hit areas.
    foundry::header(g,float(designWidth));

    const int x = designWidth - 485;
    g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.setColour(secondaryText);
    g.drawText("INPUT PEAK", x, 6, 100, 15, juce::Justification::centredLeft);
    g.drawText("OUTPUT PEAK", x + 238, 6, 110, 15, juce::Justification::centredLeft);
    const float ceiling = processor.state.getRawParameterValue("limiter_ceiling_dbtp")->load(std::memory_order_relaxed);
    const float actualTP = processor.limiterMeasuredTruePeak.load(std::memory_order_relaxed);
    const bool limiterOn = processor.state.getRawParameterValue("limiter_in")->load(std::memory_order_relaxed) > 0.5f;
    const float actualTPDb = juce::Decibels::gainToDecibels(juce::jmax(0.000001f, actualTP));
    const bool warning = limiterOn && actualTPDb > ceiling + 0.05f;
    g.setColour(warning ? juce::Colour(0xfff28a75) : secondaryText);
    g.setFont(juce::Font(juce::FontOptions(11.f)));
    const auto label = limiterOn ? ("TP " + juce::String(actualTPDb, 1) + " dBTP") : juce::String("TP --");
    g.drawText(label, 768, 23, 167, 26, juce::Justification::centredRight);
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
    if (resetConfirmTicks > 0 && --resetConfirmTicks == 0)
        resetAll.setButtonText("RESET");
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

    const auto meters = juce::Rectangle<float>(767.f, 2.f, float(designWidth - 767), 63.f)
                          .transformedBy(juce::AffineTransform::scale(uiScale)
                                         .translated(uiOffsetX, uiOffsetY));
    repaint(meters.toNearestInt());
}

void SteamMasterEditor::switchComparison(int target)
{
    if (target < 0 || target > 1 || target == currentComparison) return;
    const auto& parameters = processor.getParameters();
    auto& current = compareState[size_t(currentComparison)];
    current.clear();
    current.reserve(size_t(parameters.size()));
    for (auto* parameter : parameters) current.push_back(parameter->getValue());
    compareValid[size_t(currentComparison)] = true;
    if (!compareValid[size_t(target)])
    {
        compareState[size_t(target)] = current;
        compareValid[size_t(target)] = true;
    }
    else
    {
        const auto& snapshot = compareState[size_t(target)];
        if (snapshot.size() == size_t(parameters.size()))
            for (int i = 0; i < parameters.size(); ++i)
                parameters[i]->setValueNotifyingHost(snapshot[size_t(i)]);
    }
    currentComparison = target;
    resized();
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
    const auto scaled = juce::AffineTransform::scale(uiScale)
                            .translated(uiOffsetX, uiOffsetY);
    resetAll.setBounds(juce::Rectangle<float>(535.f, 18.f, 87.f, 31.f)
                          .transformedBy(scaled).getSmallestIntegerContainer());
    compareA.setBounds(juce::Rectangle<float>(635.f, 18.f, 54.f, 31.f)
                           .transformedBy(scaled).getSmallestIntegerContainer());
    compareB.setBounds(juce::Rectangle<float>(694.f, 18.f, 54.f, 31.f)
                           .transformedBy(scaled).getSmallestIntegerContainer());
    compareA.setColour(juce::TextButton::buttonColourId,
                       currentComparison == 0 ? juce::Colour(0xff24685e) : juce::Colour(0xff40392c));
    compareB.setColour(juce::TextButton::buttonColourId,
                       currentComparison == 1 ? juce::Colour(0xff24685e) : juce::Colour(0xff40392c));
    repaint();
}
