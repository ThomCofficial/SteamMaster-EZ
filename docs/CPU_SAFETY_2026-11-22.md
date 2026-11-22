# SteamMaster EZ: CPU safety and crash-reproduction protocol

This branch is an **unreleased candidate**. Do not replace the existing working VST3 until CI passes and the host stress tests below have been checked locally.

## Changed code paths
- A module with fully settled zero wet amount now advances only its latency-matched dry delay, without processing inaudible wet DSP.
- The fully bypassed limiter uses a dry delay of exactly its reported latency instead of computing three 8x true-peak reconstruction paths per sample.
- The post-limiter 8x true-peak correction skips expensive reconstruction when its mix has fully reached zero; true peak detection skips work when TP is disabled.
- Repeated sample-rate-dependent exponentials are precomputed in prepareToPlay-equivalent setup.
- GOLD VCA reference comparison warms both units to equivalent threshold-smoothing state before asserting sample-level equivalence.
- Rack UI switches are capped and centred with code; the approved art is untouched.

## Automatic safety tests
- CTest: 14 existing native DSP tests plus SteamMasterCPUSafety.
- Windows VST3 workflow: complete CTest suite including the JUCE plugin-host smoke test, GUI screenshot, and CPU ratio logs.
- SteamMasterCPUSafety verifies exact limiter bypass / reported latency at 44.1, 48, 96, and 192 kHz, rapid 0/2/4/8x switches, per-module bypass/enabling, finite/safe audio, constant latency, and bypass-vs-active processing cost.

## Host test still required (use a fresh empty Cubase project)
1. Keep task manager open. Start at 44.1 or 48 kHz, 512 samples, oversampling Off, all modules bypassed.
2. Watch CPU with the editor closed and open for at least 2 minutes each. Open/close repeatedly and resize the editor; watch for retained CPU use.
3. Enable only one stage at a time, then all modules; test 2x and 4x. Try 8x only if temperatures and CPU remain stable. Repeat at 96 kHz.
4. Bypass all modules with 8x still selected: CPU should fall substantially after the crossfade. Check that input/output remain delayed and bit-identical after settle.
5. Test rapid bypass and oversampling automation. Confirm no audio spikes, GUI hangs, denormal stalls, or changing latency.
6. Duplicate the plugin one instance at a time (2, 4, 8) while monitoring CPU. If overload occurs, stop playback, do not stress further, and save the observed settings.
7. Unload the plugin, quit Cubase normally and verify background CPU returns to idle.

Do not force a power-button shutdown during a controlled test. If Windows remains responsive, stop audio, end Cubase from Task Manager, and capture its CPU usage, RAM, sample rate, buffer size, active modules and oversampling setting. The above avoids calling a successful CI build proof of PC host stability.
