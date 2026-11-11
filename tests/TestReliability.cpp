#include "steammaster/Mastering.hpp"
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace steammaster;
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
std::uint64_t rng=0x51A7E5BADC0FFEEULL;
double randomSample() noexcept {
 rng^=rng<<13;rng^=rng>>7;rng^=rng<<17;
 return double((rng>>11)&0x1fffff)/1048576.-1.;
}
std::uint64_t checksum(double x,std::uint64_t h) noexcept {
 const auto q=static_cast<std::int64_t>(std::llround(x*10000000.));
 const auto u=static_cast<std::uint64_t>(q);
 return (h^u)*1099511628211ULL;
}
void limiterStress(double fs) {
 auto p=std::make_unique<MasterLimiter>();p->prepare(fs);
 const auto latency=p->latencySamples();
 require(latency>0 && latency<4096,"invalid limiter latency");
 std::uint64_t h=1469598103934665603ULL;
 const int samples=static_cast<int>(fs*.55);
 for(int i=0;i<samples;++i){
  if(i%197==0){
   LimiterParameters q;
   q.mode=LimiterMode((i/197)%3);q.truePeak=(i/197)%2==0;
   q.in=(i/197)%5!=0;q.autoRelease=(i/197)%3!=0;
   q.thresholdDb=-24.+double((i/197)%25);q.ceilingDbTP=-2.+double((i/197)%3);
   q.lookaheadMs=.1+4.9*double((i/197)%17)/16.;
   q.stereoLink=double((i/197)%11)/10.;
   q.releaseMs=10.+double((i/197)%97)*10.;
   p->set(q);
  }
  const double x=i%137==0?3.8:(i%211==0?-3.8:randomSample());
  const auto y=p->process({x,randomSample()});
  require(std::isfinite(y.l)&&std::isfinite(y.r),"limiter nonfinite");
  require(std::abs(y.l)<5. && std::abs(y.r)<5.,"limiter runaway");
  h=checksum(y.l,h);h=checksum(y.r,h);
 }
 require(p->latencySamples()==latency,"limiter latency changed during automation");
 std::cout<<"limiter "<<fs<<"Hz samples="<<samples<<" checksum="<<h<<"\n";
}
void rackLifecycle(double fs){
 auto rack=std::make_unique<MasteringRack>();
 rack->prepare(fs);
 const auto latency=rack->latencySamples();
 require(latency>0 && latency<4096,"rack latency invalid");
 for(int pass=0;pass<4;++pass){
  if(pass)rack->reset();
  for(int i=0;i<static_cast<int>(fs*.10);++i){
   const double phase=2.*kPi*997.*double(i)/fs;
   const auto x=Stereo{.9*std::sin(phase),.9*std::cos(phase)};
   const auto y=rack->process(x);
   require(std::isfinite(y.l)&&std::isfinite(y.r),"rack lifecycle nonfinite");
  }
  require(rack->latencySamples()==latency,"rack latency drift");
 }
 require(rack->invalidEvents()==0,"rack invalid DSP event");
}
void benchmark(double fs){
 auto rack=std::make_unique<MasteringRack>();rack->prepare(fs);
 GlobalParameters g;g.oversampling=OversamplingMode::X8;rack->setGlobal(g);
 LimiterParameters l;l.in=true;l.mode=LimiterMode::Modern;l.truePeak=true;
 l.thresholdDb=-14.;l.autoRelease=true;rack->limiter.set(l);
 constexpr int n=48000;
 double sink=0.;
 const auto start=std::chrono::steady_clock::now();
 for(int i=0;i<n;++i){
  const double a=2.*kPi*440.*double(i)/fs;
  const auto y=rack->process({.5*std::sin(a),.5*std::cos(a)});
  sink+=y.l*.00001+y.r*.00002;
 }
 const auto elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
 const double realtime=n/fs;
 std::cout<<"BENCHMARK fs="<<fs<<" samples="<<n<<" elapsed_s="<<elapsed
          <<" realtime_ratio="<<elapsed/realtime<<" checksum="<<sink<<"\n";
 require(std::isfinite(sink),"benchmark output invalid");
 // Deliberately no timing pass/fail threshold: CI machines are not stable
 // CPU benchmarks. Compare ratios only on the same hardware/configuration.
}
}
int main(){
 try{
  for(double fs:{44100.,48000.,96000.,192000.,384000.}){
   limiterStress(fs);
   rackLifecycle(fs);
  }
  benchmark(48000.);
  benchmark(96000.);
  std::cout<<"PASS safety, automation, reset, sample-rate and CPU diagnostics\n";
 }catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<"\n";return 1;}
}
