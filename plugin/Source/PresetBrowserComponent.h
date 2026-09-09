/*
  ==============================================================================
    PresetBrowserComponent — MPC3000 build STUB.

    The original component is the Akai MPC3000 preset/file manager (SysEx banks,
    VFX cartridges via ensoniq_vfx_cartridge, "cart:eeprom", osram injection,
    vfxcart.h). None of that exists on the mpc3000 machine, so this is replaced
    by a minimal stub that preserves ONLY the public interface PluginEditor
    references (all no-ops), with no Ensoniq-specific includes. The "File
    Manager" button therefore opens an empty placeholder panel.
  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class PresetBrowserComponent : public juce::Component
{
public:
    explicit PresetBrowserComponent(EnsoniqSD1AudioProcessor& p) : audioProcessor(p) {}
    ~PresetBrowserComponent() override = default;

    // --- interface used by PluginEditor ---
    std::function<void()> onClose;

    bool isSYXProcessing      = false;
    bool wasWriteSinglePreset = false;
    bool wasSYXPreview        = false;
    bool wasSYXImport         = false;
    bool wasSEQImport         = false;
    bool isErrorState         = false;
    bool wasPresetPreviewed   = false;

    void showClosingProgress(const juce::String&, double) {}
    void saveStateToProcessor()      {}
    void restoreStateFromProcessor() {}

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff202020));
        g.setColour(juce::Colours::grey);
        g.setFont(juce::FontOptions(16.0f));
        g.drawText("File manager is not part of the MPC3000 build.",
                   getLocalBounds(), juce::Justification::centred, true);
    }

private:
    EnsoniqSD1AudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowserComponent)
};
