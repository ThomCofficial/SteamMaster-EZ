#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include <array>
#include <memory>
#include <vector>

// Native JUCE UI. No image assets, no tabs, no static knob overlays.
class SteamMasterEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit SteamMasterEditor(SteamMasterProcessor&);
    ~SteamMasterEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    class ModulePanel;
    void timerCallback() override;
    void drawMeter(juce::Graphics&, juce::Rectangle<float>, float) const;

    SteamMasterProcessor& processor;
    std::unique_ptr<juce::LookAndFeel_V4> skin;
    juce::Component rackCanvas;
    static constexpr int designWidth = 1440;
    static constexpr int headerHeight = 70;
    static constexpr int footerHeight = 8;
    static constexpr int moduleGap = 5;
    int rackHeight = 0;
    int designHeight = 0;
    float uiScale = 1.f;
    float uiOffsetX = 0.f, uiOffsetY = 0.f;
    std::vector<std::unique_ptr<ModulePanel>> modules;
    std::array<float, 4> meterLevel{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SteamMasterEditor)
};
