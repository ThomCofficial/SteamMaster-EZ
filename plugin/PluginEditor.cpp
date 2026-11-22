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

const juce::Image& assembledApprovedFaceplate()
{
    // Approved artwork is composited exactly once, never on the audio thread.
    // Use all contiguous source intervals: earlier code cropped the middle
    // of panels and left black strips, shifting each next module into the wrong row.
    static const juce::Image image=[] {
        juce::Image out(juce::Image::ARGB,1672,941,true);
        juce::Graphics g(out);
        const auto src=juce::ImageCache::getFromMemory(
            BinaryData::Faceplate_png,BinaryData::Faceplate_pngSize);
        if(!src.isValid()) return out;
        // Boundaries are the ORIGINAL approved 1440 x 1013 rack coordinates.
        // All bands meet, so no source pixel/row is accidentally skipped.
        constexpr int srcY[]{0,82,183,275,368,456,545,632,711,795,875,970};
        constexpr int dstY[]{0,73,166,256,353,441,527,606,707,792,871,941};
        constexpr int srcX[]{0,300,1130,1440};
        constexpr int dstX[]{0,282,1347,1672};
        const auto xs=[&](int x){return int(std::round(
            double(src.getWidth())*double(x)/1440.));};
        const auto ys=[&](int y){return int(std::round(
            double(src.getHeight())*double(y)/1013.));};
        // Header gets its own cuts to keep the logo clear of RESET and A/B.
        constexpr int headerSrcX[]{0,555,610,775,944,1440};
        constexpr int headerDstX[]{0,555,700,834,996,1672};
        for(int col=0;col<5;++col) {
            const int sx=xs(headerSrcX[col]);
            g.drawImage(src,headerDstX[col],0,
                        headerDstX[col+1]-headerDstX[col],dstY[1],
                        sx,0,xs(headerSrcX[col+1])-sx,ys(srcY[1]),false);
        }
        for(int row=1;row<11;++row) {
            const int sy=ys(srcY[row]), sh=ys(srcY[row+1])-sy;
            for(int col=0;col<3;++col) {
                const int sx=xs(srcX[col]), sw=xs(srcX[col+1])-sx;
                g.drawImage(src,dstX[col],dstY[row],
                            dstX[col+1]-dstX[col],dstY[row+1]-dstY[row],
                            sx,sy,sw,sh,false);
            }
        }
        return out;
    }();
    return image;
}

class EzLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    EzLookAndFeel()
    {
        setColour(juce::ComboBox::textColourId, foundry::ivory);
        setColour(juce::ComboBox::arrowColourId, foundry::ivory);
        setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff15100e));
        setColour(juce::Label::textColourId, secondaryText);
        setColour(juce::Slider::textBoxTextColourId, foundry::ivory);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff110e0d));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff1c242d));
        setColour(juce::PopupMenu::textColourId, foundry::ivory);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff344755));
        setColour(juce::ScrollBar::thumbColourId, juce::Colour(0xff647686));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                          float pos, float start, float end, juce::Slider& slider) override
    {
        // Explicit asset mapping; never guess a dial's type from component size.
        const auto id=slider.getComponentID();
        const bool master=id=="global_input_db" || id=="global_output_db";
        const bool compact=id.startsWith("parametric_band_") || id.startsWith("bax_");
        const auto img=master
            ? juce::ImageCache::getFromMemory(BinaryData::SteamMaster_EZ_knob_master_512_png,
                                               BinaryData::SteamMaster_EZ_knob_master_512_pngSize)
            : compact
                ? juce::ImageCache::getFromMemory(BinaryData::SteamMaster_EZ_knob_compact_128_png,
                                                   BinaryData::SteamMaster_EZ_knob_compact_128_pngSize)
                : juce::ImageCache::getFromMemory(BinaryData::SteamMaster_EZ_knob_standard_256_png,
                                                   BinaryData::SteamMaster_EZ_knob_standard_256_pngSize);
        if(!img.isValid())return;
        // Larger hardware dial, still contained by its labelled JUCE cell.
        // Compensate for the horizontal text box and JUCE rotary drawing area.
        const float targetScale=master ? 1.16f : compact ? 1.18f : 1.17f;
        const float dia=juce::jmin(float(w)*.82f,float(h)*targetScale);
        if (dia < 4.f) return;
        const auto tr = juce::AffineTransform::translation(
            -img.getWidth()*.5f, -img.getHeight()*.5f)
            .rotated(start + pos*(end-start) - (start+end)*.5f)
            .scaled(dia / float(img.getWidth()))
            .translated(float(x) + w*.5f, float(y) + h*.5f);
        g.drawImageTransformed(img, tr, false);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool, bool) override
    {
        const auto id=button.getComponentID();
        const bool large=id=="global_in";
        // The True Peak switch is an ordinary toggle in the limiter rack.
        // The separate amber TP indicator is display-only in the header.
        const bool alt=id=="shadow_optical_in" || id=="shadow_discrete_in";
        const auto img=large
                ? (button.getToggleState()
                    ? juce::ImageCache::getFromMemory(BinaryData::toggle_large_on_png,BinaryData::toggle_large_on_pngSize)
                    : juce::ImageCache::getFromMemory(BinaryData::toggle_large_off_png,BinaryData::toggle_large_off_pngSize))
                : alt
                    ? (button.getToggleState()
                        ? juce::ImageCache::getFromMemory(BinaryData::toggle_small_on_alt_png,BinaryData::toggle_small_on_alt_pngSize)
                        : juce::ImageCache::getFromMemory(BinaryData::toggle_small_off_alt_png,BinaryData::toggle_small_off_alt_pngSize))
                    : (button.getToggleState()
                        ? juce::ImageCache::getFromMemory(BinaryData::toggle_small_on_png,BinaryData::toggle_small_on_pngSize)
                        : juce::ImageCache::getFromMemory(BinaryData::toggle_small_off_png,BinaryData::toggle_small_off_pngSize));
        if (img.isValid())
            g.drawImageWithin(img, 0, 0, button.getWidth(), button.getHeight(),
                              juce::RectanglePlacement::stretchToFit, false);
        // Accurate ON/OFF state text rendered live, not baked into a stretched sprite.
        g.setColour(button.getToggleState() ? juce::Colour(0xffffd08a) : juce::Colour(0xffc1b7aa));
        g.setFont(juce::Font(juce::FontOptions(large ? 10.f : 8.5f)));
        const auto labelArea=button.getLocalBounds().withTrimmedLeft(large ? 42 : 32).reduced(2,0);
        g.drawFittedText(button.getToggleState() ? "ON" : "OFF", labelArea,
                         juce::Justification::centred, 1);
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

    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        // Parameter values get the reference's restrained dark engraved boxes.
        // Labels/ticks remain live JUCE text, never part of a raster overlay.
        const bool isParameterValue=dynamic_cast<juce::Slider*>(
            label.getParentComponent())!=nullptr;
        const auto rect=label.getLocalBounds().toFloat().reduced(.6f);
        if(isParameterValue && rect.getWidth()>=24.f && rect.getHeight()>=12.f) {
            g.setColour(juce::Colour(0xff070707).withAlpha(.88f));
            g.fillRoundedRectangle(rect,1.8f);
            g.setColour(juce::Colour(0xff978679).withAlpha(.68f));
            g.drawRoundedRectangle(rect,1.8f,.85f);
        }
        g.setColour(label.findColour(juce::Label::textColourId));
        g.setFont(label.getFont());
        g.drawFittedText(label.getText(), label.getLocalBounds().reduced(2,0),
                         label.getJustificationType(), 1);
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
        label.setFont(juce::Font(juce::FontOptions(9.5f)));
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
            return juce::String(value / 1000.0, value >= 10000.0 ? 1 : 2) + " kHz";
        return wholeOrOneDecimal(value) + " Hz";
    }
    if (percentage) return wholeOrOneDecimal(value) + "%";
    if (milliseconds) return wholeOrOneDecimal(value) + " ms";
    if (decibels) return wholeOrOneDecimal(value)
                              + (id.contains("ceiling") ? " dBTP" : " dB");
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
        else if (id == "furnace_gold") name = "GOLD VCA";
        else if (id == "shadow_optical_threshold") name = "OPTICAL THRESH";
        else if (id == "shadow_discrete_threshold") name = "DISCRETE THRESH";
        else if (id == "shadow_discrete_recovery") name = "DISCRETE RECOVERY";
        else if (id == "shadow_discrete_attack") name = "DISCRETE ATTACK";
        else if (id == "shadow_discrete_gain") name = "DISCRETE GAIN";
        else if (id == "limiter_ceiling_dbtp") name = "CEILING dBTP";
        else if (name == "IN") name = "IN";
        else if (name.length() > 15) name = name.replace("Frequency", "Freq")
                                                 .replace("Threshold", "Thresh")
                                                 .replace("Attenuation", "Atten")
                                                 .replace("Oversampling", "OS");
        title.setText(name.toUpperCase(), juce::dontSendNotification);
        // Long Shadow and split Bax labels must remain entirely readable.
        const float labelSize=id.startsWith("shadow_") ? 9.15f
            : id.startsWith("bax_") ? 9.4f
            : id.startsWith("parametric_band_") ? 9.5f
            : 10.25f;
        title.setFont(juce::Font(juce::FontOptions(labelSize)));
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
            toggle->setComponentID(id);
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
            knob->setComponentID(id);
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
                knob->setTextBoxStyle(juce::Slider::TextBoxRight,false,44,16);
                knob->setBounds(2,10,w-4,h-11);
            }
            if (combo) {
                const int cw=juce::jmin(105, w-6);
                combo->setBounds((w-cw)/2,15,cw,h-18);
            }
            if (toggle) {
                const int tw=juce::jmin(95,w-2);
                toggle->setBounds((w-tw)/2,14,tw,juce::jmin(28,h-15));
            }
        } else {
            title.setBounds(1,1,w-2,14);
            if (knob) {
                // Rotary image + exact JUCE parameter value stay inside one cell.
                knob->setTextBoxStyle(juce::Slider::TextBoxBelow,false,juce::jmax(36,w-7),16);
                knob->setBounds(0,13,w,h-14);
            }
            if (combo) {
                const int cw=juce::jmin(126,w-2);
                combo->setBounds((w-cw)/2,30,cw,juce::jmin(28,h-31));
            }
            if (toggle) {
                const int tw=juce::jmin(100,w-2);
                toggle->setBounds((w-tw)/2,28,tw,juce::jmin(32,h-29));
            }
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

    int requiredHeight() const noexcept { return getHeight(); }

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
            // Invalidate ONLY the GR readouts, not the entire 1672-pixel rack.
            if (index == 2 || index == 5)
                repaint(juce::Rectangle<int>(119, 43, 164, 37));
            else if (index == 10)
                repaint(juce::Rectangle<int>(120, 48, 142, 22));
            // Animation remains GUI-only: no animation calculations on the audio thread.
            if (isActive && (index == 5 || index == 6))
                repaint(juce::Rectangle<int>(154, 13, 55, 42));
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
        if(index==1) {
            // Crisp original hardware power mark, with dark isolation from the
            // wave graphics. The approved faceplate itself is untouched.
            g.setColour(juce::Colour(0xff100d0a).withAlpha(.88f));
            g.fillEllipse(240.f,16.f,39.f,39.f);
            g.setColour(juce::Colour(0xffffc47a));
            juce::Path powerArc;
            powerArc.addCentredArc(259.5f,35.5f,15.4f,15.4f,0.f,
                                  .65f,juce::MathConstants<float>::twoPi-.65f,true);
            g.strokePath(powerArc,juce::PathStrokeType(2.3f));
            g.drawLine(259.5f,15.f,259.5f,36.f,2.6f);
        }
        if(index==7) {
            // Original approved MID/SIDE button artwork with live bypass state.
            const auto mid=processor.state.getRawParameterValue("bax_mid_in");
            const auto side=processor.state.getRawParameterValue("bax_side_in");
            const bool midOn=mid && mid->load(std::memory_order_relaxed)>.5f;
            const bool sideOn=side && side->load(std::memory_order_relaxed)>.5f;
            const auto midImage=midOn
                ? juce::ImageCache::getFromMemory(BinaryData::mid_active_png,BinaryData::mid_active_pngSize)
                : juce::ImageCache::getFromMemory(BinaryData::mid_inactive_png,BinaryData::mid_inactive_pngSize);
            const auto sideImage=sideOn
                ? juce::ImageCache::getFromMemory(BinaryData::side_active_png,BinaryData::side_active_pngSize)
                : juce::ImageCache::getFromMemory(BinaryData::side_inactive_png,BinaryData::side_inactive_pngSize);
            if(midImage.isValid())
                g.drawImageWithin(midImage,293,6,83,35,juce::RectanglePlacement::stretchToFit,false);
            if(sideImage.isValid())
                g.drawImageWithin(sideImage,293,50,83,35,juce::RectanglePlacement::stretchToFit,false);
        }
        if(index==3) {
            // Four independently outlined EQ bands, kept clear of all hit targets.
            const juce::Colour colors[]{juce::Colour(0xff8c9750),juce::Colour(0xff75d3c5),
                                         juce::Colour(0xff7fb782),juce::Colour(0xff7fbd93)};
            const int lefts[]{512,790,1066,1340};
            const int rights[]{788,1064,1338,1657};
            for(int i=0;i<4;++i) {
                g.setColour(colors[i].withAlpha(.85f));
                g.drawRoundedRectangle(juce::Rectangle<float>(float(lefts[i]),3.f,
                    float(rights[i]-lefts[i]),float(getHeight()-6)),5.f,1.1f);
            }
        }
        // Subtle hardware separators, matching reference group boundaries.
        if(index==1 || index==2 || index==4 || index==5 || index==6 || index==10) {
            g.setColour(juce::Colour(0xff87786d).withAlpha(.42f));
            const int dividers1[]{595,921,1220};
            const int dividers2[]{912};
            const int dividers4[]{411,639,865,1112,1345};
            const int dividers5[]{388,588,779,979,1192,1456};
            const int dividers6[]{441,866,1160,1455};
            const int dividers10[]{428,582,728,843,1021,1161,1316,1458};
            const int* marks=nullptr;int count=0;
            if(index==1){marks=dividers1;count=3;}
            else if(index==2){marks=dividers2;count=1;}
            else if(index==4){marks=dividers4;count=5;}
            else if(index==5){marks=dividers5;count=6;}
            else if(index==6){marks=dividers6;count=4;}
            else {marks=dividers10;count=8;}
            for(int i=0;i<count;++i)
                g.drawLine(float(marks[i]),9.f,float(marks[i]),float(getHeight()-9),.8f);
        }
        if(index==2 || index==5) {
            // Two independent, genuinely metered DSP gain-reduction readouts.
            // All calculations happen on the JUCE GUI thread at the timer rate.
            const bool dynamics=index==2;
            const auto* aOn=processor.state.getRawParameterValue(
                dynamics ? "ocean_in" : "shadow_optical_in");
            const auto* bOn=processor.state.getRawParameterValue(
                dynamics ? "furnace_in" : "shadow_discrete_in");
            const auto* shadowOn=dynamics ? nullptr
                : processor.state.getRawParameterValue("shadow_in");
            const bool parent=!shadowOn || shadowOn->load(std::memory_order_relaxed)>.5f;
            const bool firstEnabled=parent && aOn
                && aOn->load(std::memory_order_relaxed)>.5f;
            const bool secondEnabled=parent && bOn
                && bOn->load(std::memory_order_relaxed)>.5f;
            const float firstRaw=dynamics
                ? processor.oceanReductionDb.load(std::memory_order_relaxed)
                : processor.shadowOpticalReductionDb.load(std::memory_order_relaxed);
            const float secondRaw=dynamics
                ? processor.furnaceReductionDb.load(std::memory_order_relaxed)
                : processor.shadowDiscreteReductionDb.load(std::memory_order_relaxed);
            const auto dbValue=[](float r,bool enabled)->float {
                return enabled && std::isfinite(r)
                    ? juce::jlimit(0.f,36.f,std::abs(r)) : 0.f;
            };
            const float first=dbValue(firstRaw,firstEnabled);
            const float second=dbValue(secondRaw,secondEnabled);
            const auto firstColour=dynamics ? juce::Colour(0xff53d9f6)
                                               : juce::Colour(0xffff95ab);
            const auto secondColour=dynamics ? juce::Colour(0xffffc176)
                                                : juce::Colour(0xfff4a0d7);
            g.setColour(juce::Colour(0xff08131b).withAlpha(.90f));
            g.fillRoundedRectangle(juce::Rectangle<float>(120.f,43.f,160.f,35.f),4.f);
            const auto drawRow=[&](const juce::String& name,float db,int y,
                                   juce::Colour light) {
                g.setFont(juce::Font(juce::FontOptions(8.6f)));
                g.setColour(juce::Colour(0xffd5d7df));
                g.drawFittedText(name,123,y,43,12,juce::Justification::centredLeft,1);
                const int lit=juce::jlimit(0,10,int(std::round(db*10.f/18.f)));
                for(int seg=0;seg<10;++seg) {
                    g.setColour(seg<lit ? light : light.withAlpha(.16f));
                    g.fillRoundedRectangle(juce::Rectangle<float>(
                        168.f+float(seg)*5.1f,float(y)+4.f,3.9f,5.f),.8f);
                }
                g.setFont(juce::Font(juce::FontOptions(9.f)));
                g.setColour(light);
                g.drawFittedText(juce::String(db,1)+"dB",221,y,55,12,
                                 juce::Justification::centredRight,1);
            };
            drawRow(dynamics ? "OCEAN" : "OPT",first,46,firstColour);
            drawRow(dynamics ? "FURNACE" : "DISC",second,62,secondColour);
        } else if(index==10) {
            const float raw=processor.limiterReductionDb.load(std::memory_order_relaxed);
            const float db=std::isfinite(raw) ? juce::jlimit(0.f,36.f,std::abs(raw)) : 0.f;
            g.setColour(accent);
            g.setFont(juce::Font(juce::FontOptions(9.f)));
            g.drawText("GR "+juce::String(db,1)+" dB",124,51,124,15,
                       juce::Justification::centredRight);
        }
    }

    void resized() override
    {
        // Reference artwork coordinates (1672 x 941), never reflowed on resize.
        // Index lookup follows APVTS parameter insertion order, and is verified
        // by the host smoke test for all 109 parameter controls.
        const auto place=[&](int i,int cx,int width,int top=2,int height=-1) {
            if(i<0 || i>=int(controls.size())) return;
            if(height<0)height=getHeight()-5;
            controls[size_t(i)]->setBounds(cx-width/2,top,width,height);
        };
        const auto row=[&](std::initializer_list<int> centers,int width,int top=2,int height=-1) {
            int i=0;
            for(const auto cx:centers)place(i++,cx,width,top,height);
            jassert(i==int(controls.size()));
        };
        switch(index) {
            case 1:
                jassert(controls.size()==4);
                place(0,438,112,3,80);
                place(1,737,124,1,81);
                place(2,1063,124,1,81);
                place(3,1425,173,3,79);
                break;
            case 2:
                row({343,427,512,604,699,785,868,
                     957,1052,1138,1233,1325,1417,1508,1598},79);
                break;
            case 3:
                // IN/MIX/OUT + 4 × (TYPE/FREQUENCY/Q/GAIN), always one row.
                row({326,397,467,552,621,689,754,
                     825,897,965,1034,
                     1101,1171,1237,1304,
                     1377,1450,1523,1596},67,4,getHeight()-8);
                for(int i:{3,7,11,15}) {
                    const int cx=(i==3?552:i==7?825:i==11?1101:1377);
                    place(i,cx,79,4,getHeight()-8);
                }
                break;
            case 4:
                row({346,465,582,697,805,925,1047,1162,1293,1448,1570},96);
                break;
            case 5:
                row({337,427,527,627,720,815,914,1017,1107,1204,1293,1416,1569},90);
                break;
            case 6:
                row({360,498,642,787,928,1082,1237,1395,1554},112);
                break;
            case 7:
                jassert(controls.size()==16);
                for(int i=0;i<16;++i) {
                    const int xs[]{432,598,759,874,1033,1171,1361,1530};
                    const int col=i%8;
                    // 118 px for Bax gain/output: sufficient knob face and editable value.
                    const int w=col==0 ? 98 : (col==3||col==5||col==7 ? 118 : 108);
                    place(i,xs[col],w,5+(i/8)*44,39);
                }
                break;
            case 8:
                row({360,516,661,811,955,1084,1214},116);
                break;
            case 9:
                row({353,558,736,924,1112,1263},140);
                break;
            case 10:
                row({357,501,649,770,890,1082,1212,1374,1550},114);
                break;
            default:
                break;
        }
    }

private:
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
    constexpr std::array<int,10> tops{73,166,256,353,441,527,606,707,792,871};
    constexpr std::array<int,10> heights{84,82,89,80,81,74,94,78,76,70};
    for(size_t i=0;i<modules.size() && i<tops.size();++i)
        modules[i]->setBounds(0,tops[i],designWidth,heights[i]);
    rackHeight=941;
    designHeight=941;
    rackCanvas.setBounds(0,0,designWidth,designHeight);
    // The rack and all its interactive child components get one shared transform.
    setResizable(true, true);
    setResizeLimits(900, 510, 2200, 1240);
    setSize(1672, 941);
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

    // Locked original artwork; one affine transform for faceplate and UI controls.
    // One inexpensive blit for the faceplate; dynamic controls are separate.
    g.drawImageAt(assembledApprovedFaceplate(),0,0,false);

    // Stereo image goniometer: the fan is approved artwork; the actual moving
    // trace uses lock-free post-DSP samples and is clipped to the instrument.
    {
        juce::Graphics::ScopedSaveState scopeState(g);
        constexpr float cx=1464.f, baseY=843.f;
        const juce::Rectangle<int> scopeBounds(1332,789,273,59);
        g.reduceClipRegion(scopeBounds);
        const auto count=juce::jmin<unsigned>(
            processor.scopeCount.load(std::memory_order_acquire),
            unsigned(SteamMasterProcessor::kScopePoints));
        g.setColour(juce::Colour(0xffcf7af6).withAlpha(.48f));
        bool hasPrevious=false;
        juce::Point<float> prev;
        for(unsigned i=0;i<count;++i) {
            const float l=processor.scopeLeft[i].load(std::memory_order_relaxed);
            const float r=processor.scopeRight[i].load(std::memory_order_relaxed);
            if(!std::isfinite(l)||!std::isfinite(r)) {hasPrevious=false;continue;}
            const float mid=juce::jlimit(-1.f,1.f,(l+r)*.5f);
            const float side=juce::jlimit(-1.f,1.f,(l-r)*.5f);
            const juce::Point<float> point(cx+side*111.f,baseY-mid*39.f);
            if(hasPrevious && point.getDistanceFrom(prev)<30.f)
                g.drawLine(juce::Line<float>(prev,point),1.25f);
            prev=point;
            hasPrevious=true;
        }
    }

    // Keep the exact approved baked reels stationary until isolated transparent
    // reel sprites are approved. A tiny moving rim glint indicates tape activity
    // without the duplicate spokes and rotated tape-bed textures seen in tests.
    if (auto* tape=processor.state.getRawParameterValue("tape_in");
        tape && tape->load(std::memory_order_relaxed)>.5f) {
        juce::Graphics::ScopedSaveState clip(g);
        g.reduceClipRegion(juce::Rectangle<int>(1322,696,306,79));
        const auto highlight=[&](float cx,float phase) {
            juce::Path rim;
            rim.addCentredArc(cx,740.f,51.f,42.f,0.f,
                               phase,phase+.36f,true);
            g.setColour(juce::Colour(0xffffc17d).withAlpha(.64f));
            g.strokePath(rim,juce::PathStrokeType(2.1f));
        };
        highlight(1390.f,reelPhase);
        highlight(1575.f,-reelPhase);
    }

    const int x = 1090;
    g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.setColour(secondaryText);
    g.drawText("INPUT PEAK", x, 6, 100, 15, juce::Justification::centredLeft);
    g.drawText("OUTPUT PEAK", x + 238, 6, 110, 15, juce::Justification::centredLeft);
    const float ceiling = processor.state.getRawParameterValue("limiter_ceiling_dbtp")->load(std::memory_order_relaxed);
    const float actualTP = processor.limiterMeasuredTruePeak.load(std::memory_order_relaxed);
    const bool limiterOn = processor.state.getRawParameterValue("limiter_in")->load(std::memory_order_relaxed) > 0.5f;
    const float actualTPDb = juce::Decibels::gainToDecibels(juce::jmax(0.000001f, actualTP));
    const bool warning = limiterOn && actualTPDb > ceiling + 0.05f;
    const bool tpEnabled=processor.state.getRawParameterValue("limiter_true_peak")
        ->load(std::memory_order_relaxed)>.5f;
    const auto led=tpEnabled
        ? juce::ImageCache::getFromMemory(BinaryData::led_amber_png,BinaryData::led_amber_pngSize)
        : juce::ImageCache::getFromMemory(BinaryData::led_off_png,BinaryData::led_off_pngSize);
    g.setColour(warning ? juce::Colour(0xfff28a75) : secondaryText);
    g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.drawText("TP",987,26,22,18,juce::Justification::centredLeft);
    if(led.isValid())
        g.drawImageWithin(led,1009,27,16,16,juce::RectanglePlacement::centred,false);
    g.drawText(limiterOn ? juce::String(actualTPDb,1) : juce::String("--"),
               1026,26,53,18,juce::Justification::centredLeft);
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

    // A 25 Hz GUI timer animates only visible telemetry. No animation code
    // touches the audio callback; these two local repaints avoid whole-rack
    // redraws and stop immediately once the editor is destroyed.
    if(const auto* tape=processor.state.getRawParameterValue("tape_in");
       tape && tape->load(std::memory_order_relaxed)>.5f) {
        reelPhase+=.19f;
        if(reelPhase>juce::MathConstants<float>::twoPi)
            reelPhase-=juce::MathConstants<float>::twoPi;
        const auto reelRegion=juce::Rectangle<float>(1320.f,695.f,309.f,81.f)
            .transformedBy(juce::AffineTransform::scale(uiScale)
                             .translated(uiOffsetX,uiOffsetY));
        repaint(reelRegion.getSmallestIntegerContainer());
    }
    const auto scopeRegion=juce::Rectangle<float>(1331.f,789.f,274.f,60.f)
        .transformedBy(juce::AffineTransform::scale(uiScale)
                         .translated(uiOffsetX,uiOffsetY));
    repaint(scopeRegion.getSmallestIntegerContainer());

    const auto meters = juce::Rectangle<float>(850.f, 2.f, float(designWidth - 850), 63.f)
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
    resetAll.setBounds(juce::Rectangle<float>(586.f,16.f,87.f,37.f).transformedBy(scaled).getSmallestIntegerContainer());
    compareA.setBounds(juce::Rectangle<float>(699.f,16.f,68.f,37.f).transformedBy(scaled).getSmallestIntegerContainer());
    compareB.setBounds(juce::Rectangle<float>(771.f,16.f,68.f,37.f).transformedBy(scaled).getSmallestIntegerContainer());
    compareA.setToggleState(currentComparison==0,juce::dontSendNotification);
    compareB.setToggleState(currentComparison==1,juce::dontSendNotification);
    repaint();
}
