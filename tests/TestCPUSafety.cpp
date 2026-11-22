#include "steammaster/Mastering.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace steammaster;

namespace {
void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
void configure(MasteringRack& rack, bool active, OversamplingMode mode) {
    GlobalParameters global{};
    global.in = true;
    global.oversampling = mode;
    rack.setGlobal(global);
    OceanParameters ocean{};
    ocean.common.in = active;
    rack.firstSix.ocean.set(ocean);
    FurnaceParameters furnace{};
    furnace.common.in = active;
    rack.firstSix.furnace.set(furnace);
    ParametricParameters eq{};
    eq.common.in = active;
    rack.firstSix.parametric.set(eq);
    PultecParameters pultec{};
    pultec.common.in = active;
    rack.firstSix.pultec.set(pultec);
    ShadowParameters shadow{};
    shadow.common.in = active;
    shadow.opticalIn = active;
    shadow.discreteIn = active;
    rack.firstSix.shadow.set(shadow);
    ExciterParameters exciter{};
    exciter.common.in = active;
    exciter.lowDrive = 8.;
    exciter.highDrive = 8.;
    rack.firstSix.exciter.set(exciter);
    BaxandallParameters bax{};
    for (auto& row : bax.rows) row.in = active;
    rack.baxandall.set(bax);
    TapeParameters tape{};
    tape.common.in = active;
    tape.driveDb = 3.;
    rack.tape.set(tape);
    LimiterParameters limiter{};
    limiter.in = active;
    limiter.truePeak = true;
    limiter.thresholdDb = -8.;
    rack.limiter.set(limiter);
}

void checkExactLimiterBypass(double fs) {
    auto limiter = std::make_unique<MasterLimiter>();
    limiter->prepare(fs);
    LimiterParameters settings{};
    settings.in = false;
    settings.truePeak = true; // Must still be bypassed, even if TP is selected.
    limiter->set(settings);
    const auto delay = limiter->latencySamples();
    check(delay < 4096, "invalid bypass latency");
    const std::size_t length = static_cast<std::size_t>(fs * .19);
    std::vector<Stereo> source(length);
    double error = 0.;
    for (std::size_t i=0; i<length; ++i) {
        const double phase = 2.*kPi*901.*static_cast<double>(i)/fs;
        source[i] = { .31*std::sin(phase), .29*std::cos(phase) };
        const auto y=limiter->process(source[i]);
        check(std::isfinite(y.l) && std::isfinite(y.r), "limiter bypass nonfinite");
        if (i > static_cast<std::size_t>(fs*.07) && i > delay) {
            error = std::max(error,std::abs(y.l-source[i-delay].l));
            error = std::max(error,std::abs(y.r-source[i-delay].r));
        }
    }
    std::cout << "LIMITER_BYPASS sample_rate=" << fs
              << " latency=" << delay << " max_error=" << error << "\n";
    check(error < 1.e-8, "bypassed limiter altered dry audio or changed latency");
}

void rapidSwitchStress(double fs) {
    auto rack = std::make_unique<MasteringRack>();
    rack->prepare(fs);
    const auto latency = rack->latencySamples();
    double checksum = 0.;
    for (int block=0; block<72; ++block) {
        const auto mode = static_cast<OversamplingMode>(block%4);
        configure(*rack,(block%3)!=0,mode);
        if(block%12==11) rack->reset();
        for (int i=0; i<256; ++i) {
            const double n=static_cast<double>(block*256+i);
            const Stereo input{.29*std::sin(2.*kPi*1373.*n/fs),
                               .19*std::cos(2.*kPi*2411.*n/fs)};
            const auto y=rack->process(input);
            check(std::isfinite(y.l) && std::isfinite(y.r), "rapid toggles generated NaN/Inf");
            check(std::abs(y.l)<=4.01 && std::abs(y.r)<=4.01, "runaway audio during toggles");
            checksum += y.l*.00003 + y.r*.00007;
        }
        check(rack->latencySamples()==latency, "latency changed with oversampling/toggles");
    }
    std::cout << "RAPID_SWITCH sample_rate=" << fs << " checksum=" << checksum << "\n";
}

// Comparative timing on the same machine eliminates cross-runner GHz assumptions.
// This is not a guarantee of low Cubase CPU: that needs testing on the user's PC.
double bench(bool active) {
    constexpr double fs=48000.;
    auto rack=std::make_unique<MasteringRack>();
    rack->prepare(fs);
    configure(*rack,active,OversamplingMode::X8);
    constexpr int warm=4096;
    constexpr int count=24000;
    std::vector<Stereo> source(warm+count);
    for(std::size_t i=0;i<source.size();++i) {
        const double angle=2.*kPi*997.*static_cast<double>(i)/fs;
        source[i]={.22*std::sin(angle),.21*std::cos(angle)};
    }
    for(int i=0;i<warm;++i) rack->process(source[static_cast<std::size_t>(i)]);
    volatile double sink=0.;
    const auto start=std::chrono::steady_clock::now();
    for(int i=warm;i<warm+count;++i) {
        const auto y=rack->process(source[static_cast<std::size_t>(i)]);
        sink+=y.l*.001+y.r*.002;
    }
    const auto seconds=std::chrono::duration<double>(
        std::chrono::steady_clock::now()-start).count();
    check(std::isfinite(sink), "benchmark nonfinite");
    std::cout << "BENCHMARK active=" << active
              << " elapsed_s=" << seconds
              << " cpu_to_realtime=" << seconds/(double(count)/fs) << "\n";
    return seconds;
}
} // namespace

int main() {
    try {
        for(double fs : {44100.,48000.,96000.,192000.}) {
            checkExactLimiterBypass(fs);
            rapidSwitchStress(fs);
        }
        const auto idle=bench(false);
        const auto working=bench(true);
        check(idle < working*.80,
              "fully bypassed 8x rack did not show meaningful CPU reduction");
        std::cout << "CPU_SAFETY_PASS relative_speedup=" << working/idle << "x\n";
    } catch (const std::exception& e) {
        std::cerr << "CPU_SAFETY_FAIL: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
