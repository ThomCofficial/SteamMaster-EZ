# SteamMaster EZ – approved artwork restoration

The previous CPU-safety branch accidentally inherited the procedural Foundry UI from `ez/sculpted-console-v1`. That branch has no `assets/` folder.

This restoration branch is based on the **tested CPU-safety DSP** (`75787a4aa5174ce38fbee01a592dea34252a1522`) and brings back the exact visual implementation from `ez/approved-artwork-v1`.

- Restored all 35 approved PNG assets byte-for-byte using the original Git tree `96643d4342331d4f46b76f15f8d2e1742081cdc9`.
- Restored the approved `plugin/PluginEditor.cpp`, with `Faceplate.png`, golden rotary sprites, toggle sprites, dropdown sprites and A/B/reset PNG buttons.
- Restored the `juce_add_binary_data(SteamMasterEZArt)` packaging and linking.
- Kept all current `src/` DSP, CPU-safety changes, APVTS 109-parameter fix, reliability/stress tests and corrected Windows CI workflow **unchanged**.
- Geometry continues to be deterministic in JUCE and the raster artwork is never regenerated; all controls remain real.
- This is a standalone branch for test and preview. Do not install over a working copy until the Windows VST3 CI host test, DSP suite and visual screenshot have been verified.

**UI quality gate:** Confirm the actual JUCE screenshot on the Windows workflow has the black/brass artwork and not the blue-grey procedural Foundry panels. Compilation alone is not visual proof.
