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
                          float pos, float startAngle, float endAngle, juce::Slider& knob) override
    {
        // Individually machined titanium knob with a recessed, illuminated encoder ring.
        // Native JUCE slider handles all mouse gestures and APVTS automation.
        const float cx=x+width*.5f, cy=y+height*.5f;
        const float r=juce::jmin(float(width),float(height))*.353f;
        const float angle=startAngle+pos*(endAngle-startAngle);
        const auto brass=juce::Colour(0xffcaa77a);
        const auto glow=knob.isMouseOverOrDragging() ? juce::Colour(0xff8cf8e2) : aqua;

        g.setColour(juce::Colours::black.withAlpha(.34f));
        g.fillEllipse(cx-r-2.f,cy-r+3.f,(r+2.f)*2.f,(r+2.f)*2.f);
        const float ring=r+5.f;
        juce::Path groove, progress;
        groove.addCentredArc(cx,cy,ring,ring,0.f,startAngle,endAngle,true);
        progress.addCentredArc(cx,cy,ring,ring,0.f,startAngle,angle,true);
        g.setColour(juce::Colour(0xff050d14));
        g.strokePath(groove,juce::PathStrokeType(4.6f));
        g.setColour(juce::Colour(0xff344a59));
        g.strokePath(groove,juce::PathStrokeType(2.4f));
        g.setColour(glow.withAlpha(.18f));
        g.strokePath(progress,juce::PathStrokeType(6.4f));
        g.setColour(glow);
        g.strokePath(progress,juce::PathStrokeType(2.6f));

        for(int i=0;i<=10;++i)
        {
            const float a=startAngle+float(i)*(endAngle-startAngle)/10.f;
            const float s=std::sin(a), c=std::cos(a);
            const float outer=ring+6.f;
            const float inner=outer-(i%5==0?3.3f:1.7f);
            g.setColour((i%5==0 ? brass : juce::Colour(0xff70818c)).withAlpha(.78f));
            g.drawLine(cx+s*inner,cy-c*inner,cx+s*outer,cy-c*outer,i%5==0?1.15f:.7f);
        }

        const auto rim=juce::Rectangle<float>(cx-r,cy-r,2.f*r,2.f*r);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff82919b),cx-r,cy-r,
                                               juce::Colour(0xff162430),cx+r,cy+r,false));
        g.fillEllipse(rim);
        g.setColour(juce::Colour(0xffcfb28a).withAlpha(.52f));
        g.drawEllipse(rim.reduced(.7f),1.f);
        const auto face=rim.reduced(3.1f);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff394b5b),cx-r*.5f,cy-r*.8f,
                                               juce::Colour(0xff0e1b27),cx+r*.7f,cy+r*.8f,false));
        g.fillEllipse(face);
        g.setColour(juce::Colour(0xffaebfc5).withAlpha(.23f));
        g.drawEllipse(face.reduced(.7f),.8f);

        const float inner=r*.72f;
        const float tx=cx+std::sin(angle)*inner, ty=cy-std::cos(angle)*inner;
        g.setColour(juce::Colours::black.withAlpha(.65f));
        g.drawLine(cx+1.f,cy+1.f,tx+1.f,ty+1.f,3.6f);
        g.setColour(brass.brighter(.19f));
        g.drawLine(cx,cy,tx,ty,2.5f);
        g.setColour(juce::Colour(0xff162531));
        g.fillEllipse(cx-2.5f,cy-2.5f,5.f,5.f);
        g.setColour(juce::Colour(0xff84939e).withAlpha(.55f));
        g.drawEllipse(cx-2.5f,cy-2.5f,5.f,5.f,.8f);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool highlighted, bool down) override
    {
        const auto a=button.getLocalBounds().toFloat().reduced(5.f,3.f);
        const bool enabled=button.getToggleState();
        const auto led=enabled ? aqua : juce::Colour(0xff677d86);
        g.setColour(juce::Colours::black.withAlpha(.35f));
        g.fillRoundedRectangle(a.translated(0.f,2.f),5.f);
        g.setGradientFill(juce::ColourGradient(
            enabled?juce::Colour(0xff315e60):juce::Colour(0xff344451),a.getX(),a.getY(),
            enabled?juce::Colour(0xff13353a):juce::Colour(0xff14212d),a.getX(),a.getBottom(),false));
        g.fillRoundedRectangle(a,5.f);
        g.setColour(enabled ? led.withAlpha(.68f) : juce::Colour(0xff607483));
        g.drawRoundedRectangle(a.reduced(.6f),5.f,1.f);
        g.setColour(enabled?led:juce::Colour(0xff172632));
        g.fillEllipse(a.getX()+6.f,a.getCentreY()-2.2f,4.4f,4.4f);
        g.setColour(juce::Colour(0xffebf2f1).withAlpha(highlighted?1.f:.86f));
        g.setFont(juce::Font(juce::FontOptions(9.3f)));
        g.drawFittedText(enabled?"ON":"OFF",a.withTrimmedLeft(11.f).toNearestInt(),
                         juce::Justification::centred,1);
        if(down){g.setColour(juce::Colours::black.withAlpha(.17f));g.fillRoundedRectangle(a,5.f);}
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int, int, int, int, juce::ComboBox& box) override
    {
        auto a=juce::Rectangle<float>(.5f,.5f,float(width)-1.f,float(height)-1.f);
        g.setColour(juce::Colours::black.withAlpha(.33f));
        g.fillRoundedRectangle(a.translated(0.f,1.8f),5.f);
        g.setGradientFill(juce::ColourGradient(
            juce::Colour(0xff374a59),a.getX(),a.getY(),
            juce::Colour(0xff13232f),a.getX(),a.getBottom(),false));
        g.fillRoundedRectangle(a,5.f);
        g.setColour(juce::Colour(0xff7d91a0).withAlpha(box.isMouseOverOrDragging()?.78f:.42f));
        g.drawRoundedRectangle(a.reduced(.6f),5.f,1.f);
        const float x=width-11.f,y=height*.5f;
        juce::Path arrow;
        arrow.startNewSubPath(x-3.3f,y-1.f);
        arrow.lineTo(x+3.3f,y-1.f);
        arrow.lineTo(x,y+2.8f);
        arrow.closeSubPath();
        g.setColour(isButtonDown?aqua:copper);
        g.fillPath(arrow);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour&, bool hovered, bool pressed) override
    {
        auto a=button.getLocalBounds().toFloat().reduced(1.3f,2.f);
        const auto base=button.findColour(juce::TextButton::buttonColourId);
        g.setColour(juce::Colours::black.withAlpha(.48f));
        g.fillRoundedRectangle(a.translated(0.f,2.f),6.f);
        g.setGradientFill(juce::ColourGradient(
           base.brighter(hovered?.43f:.26f),a.getX(),a.getY(),
           base.darker(pressed?.35f:.18f),a.getX(),a.getBottom(),false));
        g.fillRoundedRectangle(a,6.f);
        g.setColour((hovered?aqua:copper).withAlpha(hovered?.88f:.32f));
        g.drawRoundedRectangle(a.reduced(.6f),6.f,1.1f);
        g.setColour(juce::Colours::white.withAlpha(.12f));
        g.drawLine(a.getX()+5.f,a.getY()+2.5f,a.getRight()-5.f,a.getY()+2.5f,.8f);
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
        title.setJustificationType(juce::Justification::centred);
        title.setColour(juce::Label::textColourId, secondaryText);
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
        const auto area=getLocalBounds().toFloat().reduced(1.5f);
        g.setColour(juce::Colours::black.withAlpha(.22f));
        g.fillRoundedRectangle(area.translated(0.f,1.5f),5.5f);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff20313e),area.getX(),area.getY(),
                                              juce::Colour(0xff101d29),area.getRight(),area.getBottom(),false));
        g.fillRoundedRectangle(area,5.5f);
        g.setColour(juce::Colour(0xff718696).withAlpha(.24f));
        g.drawRoundedRectangle(area.reduced(.5f),5.5f,.8f);
        g.setColour(juce::Colour(0xffc6a77a).withAlpha(.12f));
        g.drawLine(area.getX()+5.f,area.getY()+2.f,area.getRight()-5.f,area.getY()+2.f,.8f);
    }

    void resized() override
    {
        const int w = getWidth();
        const int h = getHeight();
        title.setBounds(2, 3, w - 4, 13);
        if (knob) knob->setBounds(3, 16, w - 6, h - 18);
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
        else if (index == 2 || index == 5 || index == 10)
        {
            // Small gain-reduction displays are read-only telemetry.
            repaint(juce::Rectangle<int>(15, 52, 169, 34));
        }
    }

    void paint(juce::Graphics& g) override
    {
        const auto area=getLocalBounds().toFloat().reduced(1.f);
        const auto tint=(index==2||index==5||index==10) ? copper :
                        ((index==3||index==4||index==7) ? aqua :
                         juce::Colour(0xffd0ae86));
        g.setColour(juce::Colours::black.withAlpha(.40f));
        g.fillRoundedRectangle(area.translated(0.f,1.5f),8.f);
        g.setGradientFill(juce::ColourGradient(
            juce::Colour(0xff31424d),0.f,0.f,
            juce::Colour(0xff111c28),float(getWidth()),float(getHeight()),false));
        g.fillRoundedRectangle(area,8.f);
        // A restrained, brushed-titanium highlight, kept behind the controls.
        g.setColour(juce::Colours::white.withAlpha(.065f));
        g.drawLine(10.f,2.f,float(getWidth()-10),2.f,.85f);
        g.setColour(juce::Colours::black.withAlpha(.35f));
        g.drawLine(9.f,float(getHeight()-3),float(getWidth()-9),float(getHeight()-3),.9f);
        g.setColour(isActive?tint.withAlpha(.78f):juce::Colour(0xff566979).withAlpha(.68f));
        g.drawRoundedRectangle(area.reduced(.5f),8.f,isActive?1.3f:.85f);
        g.setColour(tint.withAlpha(isActive?.12f:.035f));
        g.fillRoundedRectangle(juce::Rectangle<float>(3.f,5.f,196.f,float(getHeight()-10)),6.f);

        // Solid rack identity block + illuminated channel status.

        g.setColour(isActive ? tint : tint.withAlpha(0.42f));
        g.fillRoundedRectangle(juce::Rectangle<float>(9.f, 12.f, 3.f,
                                float(getHeight() - 24)), 1.5f);
        g.setColour(juce::Colours::black.withAlpha(.68f));
        g.fillEllipse(juce::Rectangle<float>(163.f,13.f,10.f,10.f));
        g.setColour(isActive?tint:juce::Colour(0xff465764));
        g.fillEllipse(juce::Rectangle<float>(165.f,15.f,6.f,6.f));
        if(isActive)
        {
            g.setColour(tint.withAlpha(.20f));
            g.drawEllipse(juce::Rectangle<float>(161.f,11.f,14.f,14.f),1.8f);
        }
        g.setColour(juce::Colour(0xfff0f4f5));
        g.setFont(juce::Font(juce::FontOptions(16.8f)));
        g.drawFittedText(title, 18, 9, 147, 25, juce::Justification::centredLeft, 1);
        g.setColour(secondaryText.brighter(.10f));
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
            const auto drawGR = [&](float value, float x, float width) {
                g.setColour(juce::Colour(0xff0b1925));
                g.fillRoundedRectangle(x, 74.f, width, 6.f, 2.f);
                g.setColour(isActive ? aqua : secondaryText.withAlpha(0.45f));
                g.fillRoundedRectangle(x, 74.f, width * juce::jlimit(0.f, 1.f, std::abs(value) / 18.f), 6.f, 2.f);
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
    setSize(1220, 835);
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
    // Bezel, recessed black glass and subtle illumination around the VU strip.
    g.setColour(juce::Colours::black.withAlpha(.52f));
    g.fillRoundedRectangle(area.expanded(2.f).translated(0.f,1.f),4.f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff132a35),
                                          area.getX(),area.getY(),
                                          juce::Colour(0xff07121d),
                                          area.getX(),area.getBottom(),false));
    g.fillRoundedRectangle(area,3.f);
    g.setColour(juce::Colour(0xff637c88).withAlpha(.42f));
    g.drawRoundedRectangle(area.expanded(1.f),3.f,.9f);
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
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff3bcbaa),
                            fill.getX(),fill.getY(),
                            fraction>.94f?juce::Colour(0xfff2a072):juce::Colour(0xffe1bb83),
                            fill.getRight(),fill.getBottom(),false));
        g.fillRoundedRectangle(fill,2.f);
        g.setColour(juce::Colours::white.withAlpha(.23f));
        g.drawLine(fill.getX()+1.f,fill.getY()+1.f,fill.getRight()-1.f,fill.getY()+1.f,.7f);
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
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff14232e),0.f,0.f,
                                          background,bounds.getRight(),bounds.getBottom(),false));
    g.fillAll();
    juce::Graphics::ScopedSaveState state(g);
    g.addTransform(juce::AffineTransform::scale(uiScale).translated(uiOffsetX,uiOffsetY));

    // Fixed 1440-unit header: the complete hardware rack still scales together.
    const auto header=juce::Rectangle<float>(2.f,2.f,float(designWidth-4),65.f);
    g.setColour(juce::Colours::black.withAlpha(.36f));
    g.fillRoundedRectangle(header.translated(0.f,2.f),8.f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff344955),0.f,0.f,
                                          juce::Colour(0xff101b27),0.f,68.f,false));
    g.fillRoundedRectangle(header,8.f);
    g.setColour(juce::Colour(0xffb89b75).withAlpha(.74f));
    g.fillRect(4,2,designWidth-8,2);
    g.setColour(juce::Colour(0xffa2b4ba).withAlpha(.23f));
    g.drawRoundedRectangle(header.reduced(.6f),8.f,1.f);
    g.setColour(juce::Colours::black.withAlpha(.30f));
    g.drawLine(5.f,67.f,float(designWidth-5),67.f,1.1f);

    // Engraved brand type, understated gold trim and turquoise edition name.
    g.setColour(juce::Colours::black.withAlpha(.70f));
    g.setFont(juce::Font(juce::FontOptions(26.f)));
    g.drawText("STEAMMASTER",20,13,246,34,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xfff1eee5));
    g.drawText("STEAMMASTER",19,11,246,34,juce::Justification::centredLeft);
    g.setColour(aqua);
    g.setFont(juce::Font(juce::FontOptions(25.f)));
    g.drawText("EZ",263,11,52,34,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xffabbecc));
    g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.drawText("PRECISION MASTERING  /  NATIVE DIGITAL EDITION",22,44,490,16,
               juce::Justification::centredLeft);
    g.setColour(copper.withAlpha(.60f));
    g.drawLine(320.f,17.f,320.f,51.f,.8f);
    g.setColour(juce::Colour(0xff152530));
    for(const float xScrew : {11.f,float(designWidth-11)})
    {
        g.fillEllipse(xScrew-2.8f,32.f,5.6f,5.6f);
        g.setColour(juce::Colour(0xffa9a29a).withAlpha(.8f));
        g.drawEllipse(xScrew-2.8f,32.f,5.6f,5.6f,.9f);
        g.drawLine(xScrew-1.5f,35.f,xScrew+1.5f,34.f,.7f);
        g.setColour(juce::Colour(0xff152530));
    }

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
                       currentComparison == 0 ? juce::Colour(0xff185b59) : juce::Colour(0xff263a49));
    compareB.setColour(juce::TextButton::buttonColourId,
                       currentComparison == 1 ? juce::Colour(0xff185b59) : juce::Colour(0xff263a49));
    repaint();
}
