#include "PluginEditor.h"
#include "FoundryArt.h"
#include <BinaryData.h>
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
        setColour(juce::ComboBox::textColourId, foundry::ivory);
        setColour(juce::ComboBox::arrowColourId, foundry::ivory);
        setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff202932));
        setColour(juce::Label::textColourId, secondaryText);
        setColour(juce::Slider::textBoxTextColourId, foundry::ivory);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff111820));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff1c242d));
        setColour(juce::PopupMenu::textColourId, foundry::ivory);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff344755));
        setColour(juce::ScrollBar::thumbColourId, juce::Colour(0xff647686));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                          float pos, float start, float end, juce::Slider&) override
    {
        const auto img = juce::ImageCache::getFromMemory(
            BinaryData::SteamMaster_EZ_knob_standard_256_png,
            BinaryData::SteamMaster_EZ_knob_standard_256_pngSize);
        if (!img.isValid()) return;
        const float dia = juce::jmin(float(w), float(h)) * .92f;
        if (dia < 4.f) return;
        const auto tr = juce::AffineTransform::translation(
            -img.getWidth()*.5f, -img.getHeight()*.5f)
            .rotated(start + pos*(end-start) - juce::MathConstants<float>::pi)
            .scaled(dia / float(img.getWidth()))
            .translated(float(x) + w*.5f, float(y) + h*.5f);
        g.drawImageTransformed(img, tr, false);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool, bool) override
    {
        const auto img = button.getToggleState()
            ? juce::ImageCache::getFromMemory(BinaryData::toggle_small_on_png, BinaryData::toggle_small_on_pngSize)
            : juce::ImageCache::getFromMemory(BinaryData::toggle_small_off_png, BinaryData::toggle_small_off_pngSize);
        if (img.isValid()) g.drawImageWithin(img, 0, 0, button.getWidth(), button.getHeight(),
                                              juce::RectanglePlacement::stretchToFit, false);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool,
                      int, int, int, int, juce::ComboBox&) override
    {
        const auto img = width > 98
            ? juce::ImageCache::getFromMemory(BinaryData::dropdown_long_png, BinaryData::dropdown_long_pngSize)
            : juce::ImageCache::getFromMemory(BinaryData::dropdown_short_png, BinaryData::dropdown_short_pngSize);
        if (img.isValid()) g.drawImageWithin(img, 0, 0, width, height,
                                              juce::RectanglePlacement::stretchToFit, false);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour&, bool, bool) override
    {
        juce::Image img;
        const auto label=button.getButtonText();
        if (label=="A")
            img=button.getToggleState()
              ?juce::ImageCache::getFromMemory(BinaryData::A_ACTIVE_png,BinaryData::A_ACTIVE_pngSize)
              :juce::ImageCache::getFromMemory(BinaryData::A_INACTIVE_png,BinaryData::A_INACTIVE_pngSize);
        else if (label=="B")
            img=button.getToggleState()
              ?juce::ImageCache::getFromMemory(BinaryData::B_ACTIVE_png,BinaryData::B_ACTIVE_pngSize)
              :juce::ImageCache::getFromMemory(BinaryData::B_INACTIVE_png,BinaryData::B_INACTIVE_pngSize);
        else
            img=label=="CONFIRM"
              ?juce::ImageCache::getFromMemory(BinaryData::reset_active_png,BinaryData::reset_active_pngSize)
              :juce::ImageCache::getFromMemory(BinaryData::reset_inactive_png,BinaryData::reset_inactive_pngSize);
        if (img.isValid()) g.drawImageWithin(img,0,0,button.getWidth(),button.getHeight(),
                                              juce::RectanglePlacement::stretchToFit,false);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool) override
    {
        // Labels on the approved PNG buttons are already present.
        if (b.getButtonText()=="CONFIRM") {
            g.setColour(juce::Colours::white);
            g.setFont(juce::Font(juce::FontOptions(10.f)));
            g.drawText("CONFIRM",b.getLocalBounds(),juce::Justification::centred);
        }
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
                  juce::AudioProcessorParameterWithID& parameter,
                  juce::Colour accent)
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
        title.setColour(juce::Label::textColourId, foundry::muted);
        title.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(title);

        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*>(&parameter))
        {
            combo = std::make_unique<juce::ComboBox>();
            combo->setColour(juce::ComboBox::arrowColourId,accent);
            combo->addItemList(choice->choices, 1);
            combo->setTooltip(parameter.getName(256) + ": choose a stepped setting.");
            addAndMakeVisible(*combo);
            comboAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                state, id, *combo);
        }
        else if (dynamic_cast<juce::AudioParameterBool*>(&parameter) != nullptr)
        {
            toggle = std::make_unique<juce::ToggleButton>();
            toggle->setColour(juce::ToggleButton::tickColourId,accent);
            toggle->setClickingTogglesState(true);
            toggle->setTooltip(parameter.getName(256) + ": enable or bypass.");
            addAndMakeVisible(*toggle);
            buttonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
                state, id, *toggle);
        }
        else
        {
            knob = std::make_unique<juce::Slider>();
            knob->setColour(juce::Slider::rotarySliderFillColourId,accent);
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

    void paint(juce::Graphics&) override {} // The locked faceplate owns every panel background.

    void resized() override
    {
        const int w=getWidth(), h=getHeight();
        if(h < 54) {
            title.setBounds(1,0,w-2,12);
            if (knob) {
                knob->setTextBoxStyle(juce::Slider::TextBoxRight,false,42,15);
                knob->setBounds(2,12,w-4,h-13);
            }
            if (combo) combo->setBounds(3,15,w-6,h-18);
            if (toggle) toggle->setBounds(3,15,w-6,h-18);
        } else {
            title.setBounds(1,1,w-2,14);
            if (knob) knob->setBounds(2,15,w-4,h-17);
            if (combo) combo->setBounds(4,29,w-8,29);
            if (toggle) toggle->setBounds(4,29,w-8,29);
        }
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
                auto cell = std::make_unique<ParameterCell>(
                    state, *identified, foundry::moduleAccent(index));
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
        // DO NOT paint a second panel: the original faceplate is underneath.
        const auto accent=foundry::moduleAccent(index);
        g.setColour(juce::Colour(0xfff1e8dd));
        g.setFont(juce::Font(juce::FontOptions(14.3f)));
        g.drawFittedText(title,20,8,228,25,juce::Justification::centredLeft,1);
        g.setColour(juce::Colour(0xffb0a49b));
        g.setFont(juce::Font(juce::FontOptions(9.5f)));
        g.drawFittedText(description.toUpperCase(),20,34,227,14,
                         juce::Justification::centredLeft,1);
        g.setColour(isActive ? accent : juce::Colour(0xff9c8c81));
        g.setFont(juce::Font(juce::FontOptions(9.2f)));
        g.drawText(isActive?"ACTIVE":"BYPASSED",20,51,110,14,juce::Justification::centredLeft);
        if(index==7){
            g.setColour(juce::Colour(0xff91dfdb));
            g.setFont(juce::Font(juce::FontOptions(10.f)));
            g.drawText("MID",278,6,46,30,juce::Justification::centred);
            g.drawText("SIDE",278,46,46,30,juce::Justification::centred);
        }
        if(index==2||index==5||index==10){
            const float a = index==2 ? processor.oceanReductionDb.load(std::memory_order_relaxed)
                          : index==5 ? processor.shadowOpticalReductionDb.load(std::memory_order_relaxed)
                                     : processor.limiterReductionDb.load(std::memory_order_relaxed);
            g.setColour(accent);
            g.setFont(juce::Font(juce::FontOptions(9.f)));
            g.drawText("GR "+juce::String(std::abs(a),1)+" dB",124,51,124,15,
                       juce::Justification::centredRight);
        }
    }

    void resized() override
    {
        if(index==7 && controls.size()==16){
            for(int i=0;i<16;++i)
                controls[size_t(i)]->setBounds(324+(i%8)*138,2+(i/8)*40,132,39);
            return;
        }
        const int count=int(controls.size());
        if(index==3){
            for(int i=0;i<count;++i)
                controls[size_t(i)]->setBounds(controlLeft+i*62,8,60,getHeight()-13);
            return;
        }
        const int slot=usableWidth/juce::jmax(1,count);
        for(int i=0;i<count;++i)
            controls[size_t(i)]->setBounds(controlLeft+slot*i+1,3,
                  juce::jmax(40,slot-5),getHeight()-6);
    }

private:
    static constexpr int controlLeft = 251;
    static constexpr int usableWidth = 1176;
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

    // Locked rack layout: permanent control coordinates in 1440x1013 space.
    constexpr std::array<int,10> tops{82,183,275,368,456,545,632,711,795,875};
    constexpr std::array<int,10> heights{98,90,89,82,86,83,78,81,80,91};
    for(size_t i=0;i<modules.size() && i<tops.size();++i)
        modules[i]->setBounds(0,tops[i],designWidth,heights[i]);
    rackHeight=1013;
    designHeight=1013;
    rackCanvas.setBounds(0,0,designWidth,designHeight);
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
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff222b35),0.f,0.f,
                                          background,bounds.getRight(),bounds.getBottom(),false));
    g.fillAll();
    juce::Graphics::ScopedSaveState state(g);
    g.addTransform(juce::AffineTransform::scale(uiScale).translated(uiOffsetX,uiOffsetY));

    // Fully bespoke Foundry header; keep original coordinates/hit areas.
    const auto faceplate=juce::ImageCache::getFromMemory(BinaryData::Faceplate_png,BinaryData::Faceplate_pngSize);
    if(faceplate.isValid())
        g.drawImageWithin(faceplate,0,0,designWidth,designHeight,juce::RectanglePlacement::stretchToFit,false);

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
    g.setColour(juce::Colour(0xff414d5a));
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
    resetAll.setBounds(juce::Rectangle<float>(515.f,17.f,87.f,38.f).transformedBy(scaled).getSmallestIntegerContainer());
    compareA.setBounds(juce::Rectangle<float>(608.f,17.f,77.f,38.f).transformedBy(scaled).getSmallestIntegerContainer());
    compareB.setBounds(juce::Rectangle<float>(692.f,17.f,77.f,38.f).transformedBy(scaled).getSmallestIntegerContainer());
    compareA.setToggleState(currentComparison==0,juce::dontSendNotification);
    compareB.setToggleState(currentComparison==1,juce::dontSendNotification);
    repaint();
}
