#include "PluginEditor.h"
#include <algorithm>
#include <cmath>

namespace
{
// Entirely procedural artwork: fixed native controls, no external assets.
const auto background = juce::Colour(0xff110f0c);
const auto panelDark = juce::Colour(0xff201e19);
const auto panelLight = juce::Colour(0xff413b2f);
const auto copper = juce::Colour(0xffd8ae71);
const auto aqua = juce::Colour(0xff68d5b7);
const auto secondaryText = juce::Colour(0xffc5b99e);
const auto ivory = juce::Colour(0xfff7ead0);
const auto brassDark = juce::Colour(0xff63401f);
const auto brassLight = juce::Colour(0xffe8c98b);

void metalScrew(juce::Graphics& g, float x, float y, float radius, bool diagonal=false)
{
    const auto a=juce::Rectangle<float>(x-radius,y-radius,radius*2.f,radius*2.f);
    g.setColour(juce::Colours::black.withAlpha(.72f));
    g.fillEllipse(a.expanded(.8f).translated(0.f,.6f));
    juce::ColourGradient brass(brassLight,x-radius,y-radius,brassDark,x+radius,y+radius,false);
    brass.addColour(.48,juce::Colour(0xffa98045));
    g.setGradientFill(brass);
    g.fillEllipse(a);
    g.setColour(juce::Colour(0xff241b12));
    const float d=radius*.60f;
    g.drawLine(x-d,y+(diagonal?d:0.f),x+d,y-(diagonal?d:0.f),.9f);
    g.setColour(ivory.withAlpha(.34f));
    g.drawEllipse(a.reduced(.4f),.6f);
}

// A restrained brushed-metal plate, rendered in the existing logical coordinates.
void ironPlate(juce::Graphics& g, juce::Rectangle<float> a, float radius,
               juce::Colour top, juce::Colour bottom)
{
    juce::ColourGradient metal(top,a.getX(),a.getY(),bottom,a.getX(),a.getBottom(),false);
    metal.addColour(.29,top.darker(.12f));
    metal.addColour(.72,bottom.brighter(.10f));
    g.setGradientFill(metal);
    g.fillRoundedRectangle(a,radius);
    // Low-contrast horizontal machining marks; no bitmap or per-frame allocation.
    const auto left=a.getX()+radius+1.f, right=a.getRight()-radius-1.f;
    if(right>left)
        for(int i=1;float(i)<a.getHeight()-radius;++i)
        {
            g.setColour((i%4==0?juce::Colours::black:ivory).withAlpha(i%4==0?.07f:.018f));
            g.drawHorizontalLine(int(a.getY())+i,left,right);
        }
    g.setColour(brassDark);
    g.drawRoundedRectangle(a.reduced(.45f),radius,.9f);
    g.setColour(brassLight.withAlpha(.25f));
    g.drawRoundedRectangle(a.reduced(1.8f),juce::jmax(1.f,radius-1.f),.7f);
}

void brassGear(juce::Graphics& g,float x,float y,float r)
{
    juce::Path gear;
    constexpr int teeth=12;
    for(int i=0;i<teeth*4;++i){
        const float a=float(i)*juce::MathConstants<float>::twoPi/float(teeth*4);
        const float rr=(i%4==1||i%4==2)?r:r*.81f;
        const float px=x+std::cos(a)*rr,py=y+std::sin(a)*rr;
        if(i==0)gear.startNewSubPath(px,py);else gear.lineTo(px,py);
    }
    gear.closeSubPath();
    g.setColour(juce::Colours::black.withAlpha(.5f));
    g.fillPath(gear,juce::AffineTransform::translation(0.f,1.5f));
    juce::ColourGradient metal(brassLight,x-r,y-r,brassDark,x+r,y+r,false);
    metal.addColour(.5,copper);
    g.setGradientFill(metal);
    g.fillPath(gear);
    g.setColour(brassLight.withAlpha(.7f));g.strokePath(gear,juce::PathStrokeType(.55f));
    g.setColour(background);g.fillEllipse(x-r*.56f,y-r*.56f,r*1.12f,r*1.12f);
    g.setColour(copper);g.drawEllipse(x-r*.58f,y-r*.58f,r*1.16f,r*1.16f,.8f);
    for(int i=0;i<6;++i){
        const float a=float(i)*juce::MathConstants<float>::twoPi/6.f;
        g.drawLine(x+std::cos(a)*r*.23f,y+std::sin(a)*r*.23f,
                   x+std::cos(a)*r*.53f,y+std::sin(a)*r*.53f,1.7f);
    }
    metalScrew(g,x,y,r*.18f,true);
}

class EzLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    EzLookAndFeel()
    {
        setColour(juce::ComboBox::textColourId, juce::Colours::white);
        setColour(juce::ComboBox::arrowColourId, copper);
        setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2b261e));
        setColour(juce::Label::textColourId, secondaryText);
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xfff7ead0));
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff211a12));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff29231a));
        setColour(juce::PopupMenu::textColourId, juce::Colours::white);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff50452d));
        setColour(juce::ScrollBar::thumbColourId, juce::Colour(0xff8b7049));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float pos, float startAngle, float endAngle, juce::Slider& knob) override
    {
        const float cx=x+width*.5f, cy=y+height*.5f;
        const float available=juce::jmin(float(width),float(height))*.5f;
        const float r=juce::jmax(4.f,available-7.f);
        const float angle=startAngle+pos*(endAngle-startAngle);
        const auto glow=knob.isMouseOverOrDragging()?aqua.brighter(.25f):aqua;
        const auto disc=juce::Rectangle<float>(cx-r,cy-r,r*2.f,r*2.f);
        const float ring=r+3.1f;
        g.setColour(juce::Colours::black.withAlpha(.65f));
        g.fillEllipse(disc.expanded(1.9f).translated(0.f,1.4f));

        // Recessed engraved scale and lit enamel arc, inside the native bounds.
        juce::Path track,progress;
        track.addCentredArc(cx,cy,ring,ring,0.f,startAngle,endAngle,true);
        progress.addCentredArc(cx,cy,ring,ring,0.f,startAngle,angle,true);
        g.setColour(juce::Colour(0xff100c08));
        g.strokePath(track,juce::PathStrokeType(3.0f));
        g.setColour(brassDark);
        g.strokePath(track,juce::PathStrokeType(1.5f));
        if(pos>.0001f){
            g.setColour(glow.withAlpha(.22f));g.strokePath(progress,juce::PathStrokeType(4.2f));
            g.setColour(glow);g.strokePath(progress,juce::PathStrokeType(1.6f));
        }
        for(int i=0;i<=10;++i){
            const float a=startAngle+float(i)*(endAngle-startAngle)/10.f;
            const float sn=std::sin(a),cs=std::cos(a);
            const float outer=juce::jmin(available-.7f,ring+3.0f);
            const float inner=outer-(i%5==0?2.f:1.f);
            g.setColour((i%5==0?brassLight:secondaryText).withAlpha(.75f));
            g.drawLine(cx+sn*inner,cy-cs*inner,cx+sn*outer,cy-cs*outer,i%5==0?1.f:.65f);
        }
        // Multi-stop brass rim and rotating knurling around a dark Bakelite cap.
        juce::ColourGradient rim(brassLight,cx-r,cy-r,brassDark,cx+r,cy+r,false);
        rim.addColour(.28,juce::Colour(0xffa46e32));rim.addColour(.52,brassLight);
        rim.addColour(.72,juce::Colour(0xff8d5826));
        g.setGradientFill(rim);g.fillEllipse(disc);
        g.setColour(juce::Colour(0xfff1d394).withAlpha(.63f));g.drawEllipse(disc.reduced(.55f),.7f);
        for(int i=0;i<28;++i){
            const float a=angle+float(i)*juce::MathConstants<float>::twoPi/28.f;
            const float sn=std::sin(a),cs=std::cos(a);
            g.setColour(juce::Colours::black.withAlpha(.40f));
            g.drawLine(cx+sn*(r-2.5f),cy+cs*(r-2.5f),cx+sn*(r-.65f),cy+cs*(r-.65f),.6f);
        }
        const auto face=disc.reduced(3.f);
        juce::ColourGradient cap(juce::Colour(0xff5a4430),cx-r,cy-r,
                                 juce::Colour(0xff15120e),cx+r,cy+r,false);
        cap.addColour(.45,juce::Colour(0xff30261c));
        g.setGradientFill(cap);g.fillEllipse(face);
        g.setColour(brassLight.withAlpha(.58f));g.drawEllipse(face.reduced(.45f),.65f);
        const float inner=r*.63f;
        const float tx=cx+std::sin(angle)*inner,ty=cy-std::cos(angle)*inner;
        g.setColour(juce::Colours::black.withAlpha(.8f));
        g.drawLine(cx+.6f,cy+.8f,tx+.6f,ty+.8f,2.7f);
        g.setColour(ivory);g.drawLine(cx,cy,tx,ty,1.7f);
        metalScrew(g,cx,cy,1.6f,true);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool highlighted, bool down) override
    {
        const auto a=button.getLocalBounds().toFloat().reduced(5.f,3.f);
        const bool enabled=button.getToggleState();
        const auto led=enabled ? aqua : juce::Colour(0xff786a52);
        g.setColour(juce::Colours::black.withAlpha(.35f));
        g.fillRoundedRectangle(a.translated(0.f,2.f),5.f);
        g.setGradientFill(juce::ColourGradient(
            enabled?juce::Colour(0xff335d4d):juce::Colour(0xff67513a),a.getX(),a.getY(),
            enabled?juce::Colour(0xff142f28):juce::Colour(0xff2d2114),a.getX(),a.getBottom(),false));
        g.fillRoundedRectangle(a,5.f);
        g.setColour(enabled ? led.withAlpha(.68f) : juce::Colour(0xff96713c));
        g.drawRoundedRectangle(a.reduced(.6f),5.f,1.f);
        g.setColour(enabled?led:juce::Colour(0xff211b11));
        g.fillEllipse(a.getX()+6.f,a.getCentreY()-2.2f,4.4f,4.4f);
        g.setColour(juce::Colour(0xfffff0d1).withAlpha(highlighted?1.f:.86f));
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
            juce::Colour(0xff66513a),a.getX(),a.getY(),
            juce::Colour(0xff292017),a.getX(),a.getBottom(),false));
        g.fillRoundedRectangle(a,5.f);
        g.setColour(juce::Colour(0xffc6a16b).withAlpha(box.isMouseOverOrDragging()?.78f:.42f));
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
        g.setColour(brassLight.withAlpha(.35f));
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
        g.setColour(juce::Colours::black.withAlpha(.38f));
        g.fillRoundedRectangle(area.translated(0.f,1.1f),4.f);
        ironPlate(g,area,4.f,juce::Colour(0xff3a3124),juce::Colour(0xff211b14));
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
        const auto tint=(index==2||index==5||index==10)?copper:
                        ((index==3||index==4||index==7)?aqua:brassLight);
        g.setColour(juce::Colours::black.withAlpha(.62f));
        g.fillRoundedRectangle(area.translated(0.f,1.5f),7.f);
        ironPlate(g,area,7.f,panelLight,panelDark);
        g.setColour(tint.withAlpha(isActive?.65f:.23f));
        g.drawRoundedRectangle(area.reduced(2.5f),5.f,.85f);

        // Lacquered nameplate with double brass trim and real status lamps.
        const auto plate=juce::Rectangle<float>(5.f,5.f,193.f,float(getHeight()-10));
        ironPlate(g,plate,5.f,juce::Colour(0xff54412b),juce::Colour(0xff231b13));
        metalScrew(g,10.f,9.f,2.2f,true);
        metalScrew(g,192.f,9.f,2.2f);
        metalScrew(g,10.f,float(getHeight()-9),2.2f);
        metalScrew(g,192.f,float(getHeight()-9),2.2f,true);
        // Rack fastening points sit beyond the existing control cells.
        metalScrew(g,float(getWidth()-6),9.f,2.6f,true);
        metalScrew(g,float(getWidth()-6),float(getHeight()-9),2.6f);
        // Copper rail separation; no layout changes.
        g.setColour(brassDark);
        g.drawLine(202.f,7.f,202.f,float(getHeight()-7),2.f);
        g.setColour(brassLight.withAlpha(.35f));
        g.drawLine(203.f,8.f,203.f,float(getHeight()-8),.65f);

        // Solid rack identity block + illuminated channel status.

        g.setColour(isActive ? tint : tint.withAlpha(0.42f));
        g.fillRoundedRectangle(juce::Rectangle<float>(9.f, 12.f, 3.f,
                                float(getHeight() - 24)), 1.5f);
        g.setColour(juce::Colours::black.withAlpha(.68f));
        g.fillEllipse(juce::Rectangle<float>(163.f,13.f,10.f,10.f));
        g.setColour(isActive?tint:juce::Colour(0xff675843));
        g.fillEllipse(juce::Rectangle<float>(165.f,15.f,6.f,6.f));
        if(isActive)
        {
            g.setColour(tint.withAlpha(.20f));
            g.drawEllipse(juce::Rectangle<float>(161.f,11.f,14.f,14.f),1.8f);
        }
        g.setColour(juce::Colour(0xfff6e6c4));
        g.setFont(juce::Font(juce::FontOptions(16.8f)));
        g.drawFittedText(title, 18, 9, 147, 25, juce::Justification::centredLeft, 1);
        g.setColour(secondaryText.brighter(.10f));
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.drawFittedText(description.toUpperCase(), 18, 34, 158, 16,
                         juce::Justification::centredLeft, 1);
        g.setColour(isActive ? aqua : juce::Colour(0xffb4a58c));
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
                g.setColour(juce::Colour(0xff18140d));
                g.fillRoundedRectangle(x, 74.f, width, 6.f, 2.f);
                const float fill=width*juce::jlimit(0.f,1.f,std::abs(value)/18.f);
                if(fill>.01f){
                    g.setGradientFill(juce::ColourGradient(
                        isActive?brassLight:secondaryText.withAlpha(.45f),x,74.f,
                        isActive?juce::Colour(0xffad6a2c):brassDark,x,80.f,false));
                    g.fillRoundedRectangle(x,74.f,fill,6.f,2.f);
                }
                g.setColour(brassLight.withAlpha(.38f));
                g.drawRoundedRectangle(x,74.f,width,6.f,2.f,.6f);
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

        g.setColour(juce::Colour(0xff8c6638));
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
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff292116),
                                          area.getX(),area.getY(),
                                          juce::Colour(0xff100e09),
                                          area.getX(),area.getBottom(),false));
    g.fillRoundedRectangle(area,3.f);
    g.setColour(juce::Colour(0xffac864a).withAlpha(.42f));
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
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff58c89c),
                            fill.getX(),fill.getY(),
                            fraction>.94f?juce::Colour(0xffeb8250):juce::Colour(0xffe7b557),
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
    g.setColour(juce::Colour(0xff936e37));
    g.drawRoundedRectangle(area, 3.f, 0.8f);
}

void SteamMasterEditor::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff2a241a),0.f,0.f,
                                          background,bounds.getRight(),bounds.getBottom(),false));
    g.fillAll();
    juce::Graphics::ScopedSaveState state(g);
    g.addTransform(juce::AffineTransform::scale(uiScale).translated(uiOffsetX,uiOffsetY));

    // Brass nameplate, oxidised iron and a crafted cog insignia. All original
    // meter and button locations stay untouched.
    const auto header=juce::Rectangle<float>(2.f,2.f,float(designWidth-4),65.f);
    g.setColour(juce::Colours::black.withAlpha(.55f));
    g.fillRoundedRectangle(header.translated(0.f,2.f),7.f);
    ironPlate(g,header,7.f,juce::Colour(0xff56432e),juce::Colour(0xff1f1a13));
    g.setColour(brassLight.withAlpha(.75f));g.drawLine(9.f,4.f,float(designWidth-9),4.f,.9f);
    g.setColour(brassDark);g.drawLine(9.f,65.f,float(designWidth-9),65.f,1.5f);

    const auto brand=juce::Rectangle<float>(13.f,8.f,309.f,51.f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff42301d),13.f,8.f,
                                          juce::Colour(0xff1c1710),13.f,59.f,false));
    g.fillRoundedRectangle(brand,4.f);
    g.setColour(copper.withAlpha(.75f));g.drawRoundedRectangle(brand,4.f,1.f);
    g.setColour(brassLight.withAlpha(.24f));g.drawRoundedRectangle(brand.reduced(2.f),3.f,.6f);
    g.setFont(juce::Font(juce::FontOptions(26.f)));
    g.setColour(juce::Colours::black.withAlpha(.85f));
    g.drawText("STEAMMASTER",20,11,243,34,juce::Justification::centredLeft);
    g.setColour(brassLight);
    g.drawText("STEAMMASTER",19,10,243,34,juce::Justification::centredLeft);
    g.setColour(aqua);
    g.setFont(juce::Font(juce::FontOptions(25.f)));
    g.drawText("EZ",263,11,52,34,juce::Justification::centredLeft);
    g.setColour(ivory.withAlpha(.85f));
    g.setFont(juce::Font(juce::FontOptions(9.6f)));
    g.drawText("PRECISION MASTERING WORKS",22,43,290,13,juce::Justification::centredLeft);

    brassGear(g,356.f,34.f,19.f);
    g.setColour(brassLight);
    g.setFont(juce::Font(juce::FontOptions(11.5f)));
    g.drawText("THOMC ENGINEERING",385,17,139,16,juce::Justification::centredLeft);
    g.setColour(secondaryText);
    g.setFont(juce::Font(juce::FontOptions(9.f)));
    g.drawText("PREMIUM STEAM EDITION",385,33,139,15,juce::Justification::centredLeft);
    metalScrew(g,8.f,33.f,2.5f,true);
    metalScrew(g,float(designWidth-8),33.f,2.5f);
    // Small decorative copper rail behind the A/B strip, preserving hit areas.
    g.setColour(brassDark);
    g.drawRoundedRectangle(529.f,12.f,225.f,43.f,6.f,1.2f);
    g.setColour(brassLight.withAlpha(.22f));
    g.drawRoundedRectangle(531.f,14.f,221.f,39.f,5.f,.7f);

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
    g.setColour(juce::Colour(0xffaa783c));
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
                       currentComparison == 0 ? juce::Colour(0xff2b5a45) : juce::Colour(0xff473421));
    compareB.setColour(juce::TextButton::buttonColourId,
                       currentComparison == 1 ? juce::Colour(0xff2b5a45) : juce::Colour(0xff473421));
    repaint();
}
