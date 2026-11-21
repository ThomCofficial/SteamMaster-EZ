#pragma once
// SteamMaster EZ — Foundry Atelier
// An original, entirely self-contained JUCE vector skin.
// No embedded bitmaps, third-party artwork, fonts, images or DSP changes.
#include <juce_audio_utils/juce_audio_utils.h>
#include "MaterialAtlas.h"
#include <cmath>

namespace foundry
{
// Studio Finish: one accent per module, all hardware and typography neutral.
// Inspired by professional interface design principles, not third-party artwork.
inline const juce::Colour black  {0xff0b0f14};
inline const juce::Colour iron   {0xff181e26};
inline const juce::Colour ironHi {0xff313a45};
inline const juce::Colour bronze {0xff48535f};
inline const juce::Colour copper {0xff667482};
inline const juce::Colour gold   {0xffc5d0d9};
inline const juce::Colour ivory  {0xffeef3f6};
inline const juce::Colour muted  {0xff9aa7b3};
inline const juce::Colour teal   {0xff61cbd2};
inline const juce::Colour warm   {0xfff3a77e};

// Ten distinct instrument accent colours. Intentionally restrained: the
// original brass/iron/walnut material system remains identical in every row.
inline juce::Colour moduleAccent(int index)
{
    switch(index)
    {
        case 1: return juce::Colour(0xffb8d5e7); // platinum blue
        case 2: return juce::Colour(0xff78b7f1); // ocean blue
        case 3: return juce::Colour(0xff64d5ba); // sea green
        case 4: return juce::Colour(0xffedc182); // vintage amber
        case 5: return juce::Colour(0xffe28d93); // shadow rose
        case 6: return juce::Colour(0xfff2a36d); // warm drive
        case 7: return juce::Colour(0xff67d5df); // cyan M/S
        case 8: return juce::Colour(0xffbba2dd); // tape violet
        case 9: return juce::Colour(0xff83cdb8); // stereo mint
        case 10:return juce::Colour(0xffe6a7bf); // limiter pink
        default:return teal;
    }
}

inline void gradient(juce::Graphics& g, juce::Rectangle<float> a, float round,
                     juce::Colour upper, juce::Colour lower)
{
    juce::ColourGradient fill(upper,a.getCentreX(),a.getY(),lower,a.getCentreX(),a.getBottom(),false);
    fill.addColour(.49,upper.interpolatedWith(lower,.40f));
    fill.addColour(.63,lower.brighter(.09f));
    g.setGradientFill(fill);
    g.fillRoundedRectangle(a,round);
}

inline void screw(juce::Graphics& g,float x,float y,float r,bool diagonal=false)
{
    const auto a=juce::Rectangle<float>(x-r,y-r,2*r,2*r);
    g.setColour(juce::Colours::black.withAlpha(.77f));
    g.fillEllipse(a.translated(.4f,.9f).expanded(.7f));
    g.setGradientFill(juce::ColourGradient(gold,x-r,y-r,bronze,x+r,y+r,false));
    g.fillEllipse(a);
    g.setColour(ivory.withAlpha(.50f));g.drawEllipse(a.reduced(.5f),.6f);
    g.setColour(black.withAlpha(.95f));
    const float d=r*.53f;
    if(diagonal)g.drawLine(x-d,y-d,x+d,y+d,.8f);
    else g.drawLine(x-d,y,x+d,y,.8f);
}

inline void ornament(juce::Graphics& g,float x,float y,float w)
{
    g.setColour(gold.withAlpha(.56f));
    g.drawLine(x,y,x+w*.43f,y,.7f);
    g.drawLine(x+w*.57f,y,x+w,y,.7f);
    juce::Path diamond;
    diamond.startNewSubPath(x+w*.5f,y-2.5f);
    diamond.lineTo(x+w*.5f+3.f,y);
    diamond.lineTo(x+w*.5f,y+2.5f);
    diamond.lineTo(x+w*.5f-3.f,y);
    diamond.closeSubPath();
    g.setColour(copper);g.fillPath(diamond);
}

inline void gear(juce::Graphics& g,float cx,float cy,float r,float phase=0.f)
{
    juce::Path p;
    constexpr int n=12;
    for(int k=0;k<n*4;++k)
    {
        const float theta=phase+float(k)*juce::MathConstants<float>::twoPi/float(n*4);
        const float radius=(k%4==1||k%4==2)?r:r*.82f;
        const float x=cx+std::sin(theta)*radius;
        const float y=cy+std::cos(theta)*radius;
        if(k==0)p.startNewSubPath(x,y);else p.lineTo(x,y);
    }
    p.closeSubPath();
    g.setColour(black.withAlpha(.72f));g.fillPath(p,juce::AffineTransform::translation(0.f,1.2f));
    g.setGradientFill(juce::ColourGradient(gold,cx-r,cy-r,bronze,cx+r,cy+r,false));
    g.fillPath(p);
    g.setColour(ivory.withAlpha(.56f));g.strokePath(p,juce::PathStrokeType(.65f));
    g.setColour(iron);g.fillEllipse(cx-r*.60f,cy-r*.60f,r*1.20f,r*1.20f);
    g.setColour(copper);g.drawEllipse(cx-r*.61f,cy-r*.61f,r*1.22f,r*1.22f,1.f);
    for(int k=0;k<6;++k)
    {
        const float a=phase+k*juce::MathConstants<float>::twoPi/6.f;
        g.setColour(gold.withAlpha(.75f));
        g.drawLine(cx+std::sin(a)*r*.22f,cy+std::cos(a)*r*.22f,
                   cx+std::sin(a)*r*.54f,cy+std::cos(a)*r*.54f,1.25f);
    }
    screw(g,cx,cy,r*.16f,true);
}

inline void rivetedPlate(juce::Graphics& g,juce::Rectangle<float> a,float corner,
                         juce::Colour top,juce::Colour bottom)
{
    // Solid cast graphite chassis. Restrained grain, no gold outlines.
    g.setColour(black.withAlpha(.88f));
    g.fillRoundedRectangle(a.translated(0.f,1.5f).expanded(.5f),corner);
    gradient(g,a,corner,top,bottom);
    atelier::fill(g,a.reduced(.8f),corner,atelier::Material::forgedIron,.10f);
    g.setColour(juce::Colour(0xff48515d).withAlpha(.75f));
    g.drawRoundedRectangle(a.reduced(.5f),corner,1.f);
    g.setColour(ivory.withAlpha(.07f));
    g.drawLine(a.getX()+9.f,a.getY()+1.6f,a.getRight()-9.f,a.getY()+1.6f,.8f);
}

inline void lamp(juce::Graphics& g,float x,float y,bool on,juce::Colour colour=teal)
{
    g.setColour(black);g.fillEllipse(x-5.7f,y-5.7f,11.4f,11.4f);
    g.setColour(gold.withAlpha(.6f));g.drawEllipse(x-4.9f,y-4.9f,9.8f,9.8f,.95f);
    const auto light=on?colour:juce::Colour(0xff5d4c3b);
    if(on){g.setColour(light.withAlpha(.14f));g.fillEllipse(x-8.f,y-8.f,16.f,16.f);}
    g.setGradientFill(juce::ColourGradient(light.brighter(.4f),x-2.f,y-3.f,
                     light.darker(.7f),x+2.f,y+3.f,false));
    g.fillEllipse(x-3.2f,y-3.2f,6.4f,6.4f);
    g.setColour(ivory.withAlpha(on?.72f:.11f));g.fillEllipse(x-2.f,y-2.1f,2.4f,1.8f);
}

inline void valve(juce::Graphics& g,float x,float y,float r,bool active,float t)
{
    const auto a=juce::Rectangle<float>(x-r*.7f,y-r*1.26f,r*1.4f,r*2.52f);
    g.setColour(black);g.fillRoundedRectangle(a.expanded(1.f),r*.33f);
    gradient(g,a,r*.31f,juce::Colour(0xff344348),juce::Colour(0xff141c21));
    g.setColour(copper.withAlpha(.9f));g.drawRoundedRectangle(a,r*.31f,1.f);
    const auto filament=active?warm:juce::Colour(0xff72563d);
    g.setColour(filament.withAlpha(active?.20f:.06f));
    g.fillRoundedRectangle(a.reduced(2.f),r*.2f);
    g.setColour(filament);
    const float flick=active?(.8f+.2f*std::sin(t*3.5f)):.24f;
    g.drawLine(x-1.8f,y+r*.55f,x-1.8f,y-r*.47f,1.1f*flick);
    g.drawLine(x+1.8f,y+r*.55f,x+1.8f,y-r*.47f,1.1f*flick);
    g.setColour(gold.withAlpha(.65f));g.drawLine(x-r*.35f,y-r*.80f,x+r*.35f,y-r*.80f,.8f);
    g.setColour(bronze);g.fillRoundedRectangle(x-r*.78f,y+r*.97f,r*1.56f,r*.34f,2.f);
}

inline void tapeReel(juce::Graphics& g,float cx,float cy,float r,float phase)
{
    g.setColour(black);g.fillEllipse(cx-r-1.5f,cy-r+.6f,2*r+3.f,2*r+3.f);
    g.setGradientFill(juce::ColourGradient(gold,cx-r,cy-r,bronze,cx+r,cy+r,false));
    g.fillEllipse(cx-r,cy-r,2*r,2*r);
    g.setColour(iron);g.fillEllipse(cx-r*.76f,cy-r*.76f,1.52f*r,1.52f*r);
    for(int k=0;k<5;++k)
    {
        const float a=phase+float(k)*juce::MathConstants<float>::twoPi/5.f;
        g.setColour(copper);
        g.drawLine(cx+std::sin(a)*r*.25f,cy+std::cos(a)*r*.25f,
                   cx+std::sin(a)*r*.74f,cy+std::cos(a)*r*.74f,2.3f);
    }
    g.setColour(gold);g.drawEllipse(cx-r*.7f,cy-r*.7f,r*1.4f,r*1.4f,.7f);
    screw(g,cx,cy,r*.23f);
}

inline void icon(juce::Graphics& g,int index,float x,float y,bool active,float t)
{
    // Neutral line icons with exactly ONE module accent, no mixed gold/teal.
    const auto c=moduleAccent(index).withAlpha(active?.95f:.52f);
    g.setColour(c);
    switch(index)
    {
        case 1: // trim / gain
            g.drawEllipse(x-10.f,y-10.f,20.f,20.f,1.7f);
            g.drawLine(x,y-13.f,x,y-5.f,2.f);
            break;
        case 2: // twin compressor stages
            g.drawEllipse(x-12.f,y-9.f,15.f,18.f,1.5f);
            g.drawEllipse(x-2.f,y-9.f,15.f,18.f,1.5f);
            break;
        case 3: // four parametric bands
            for(int i=0;i<4;++i){
                const float xx=x-11.f+float(i)*7.f;
                g.drawLine(xx,y-11.f,xx,y+11.f,.8f);
                g.fillEllipse(xx-2.5f,y+std::sin(float(i)*1.6f)*5.f-2.5f,5.f,5.f);
            }
            break;
        case 4: // Pultec shelves
            g.drawLine(x-12.f,y+6.f,x-5.f,y+6.f,1.6f);
            g.drawLine(x-5.f,y+6.f,x+4.f,y-5.f,1.6f);
            g.drawLine(x+4.f,y-5.f,x+12.f,y-5.f,1.6f);
            break;
        case 5: // opto / discrete
            g.drawEllipse(x-11.f,y-10.f,22.f,20.f,1.3f);
            g.drawLine(x-9.f,y+6.f,x-3.f,y+1.f,1.7f);
            g.drawLine(x-3.f,y+1.f,x+2.f,y+3.f,1.7f);
            g.drawLine(x+2.f,y+3.f,x+9.f,y-7.f,1.7f);
            break;
        case 6: // harmonics
            for(int i=0;i<3;++i){
                const float xx=x-9.f+float(i)*9.f;
                g.drawLine(xx,y+9.f,xx,y-8.f+float(i)*3.f,2.f);
            }
            break;
        case 7: // M/S rings
            g.drawEllipse(x-13.f,y-10.f,21.f,21.f,1.7f);
            g.drawEllipse(x-5.f,y-10.f,21.f,21.f,1.7f);
            break;
        case 8: // small tape reels, still animated
            for(int i=0;i<2;++i){
                const float xx=x+(i==0?-8.f:8.f);
                g.drawEllipse(xx-7.f,y-7.f,14.f,14.f,1.5f);
                const float a=active?t*(i==0?1.f:-1.f):0.f;
                g.drawLine(xx+std::sin(a)*5.f,y-std::cos(a)*5.f,
                           xx-std::sin(a)*5.f,y+std::cos(a)*5.f,1.4f);
            }
            break;
        case 9: // stereo field
            g.drawEllipse(x-12.f,y-9.f,18.f,18.f,1.5f);
            g.drawEllipse(x-6.f,y-9.f,18.f,18.f,1.5f);
            break;
        case 10: // limiting
            for(int i=0;i<5;++i){
                const float h=5.f+float((i*7)%11);
                g.fillRoundedRectangle(x-12.f+float(i)*6.f,y+8.f-h,3.3f,h,1.f);
            }
            break;
        default:break;
    }
}

inline void moduleSurface(juce::Graphics& g,juce::Rectangle<float> a,int index,bool active,float t)
{
    const auto accent=moduleAccent(index);
    rivetedPlate(g,a,5.f,juce::Colour(0xff262e38),juce::Colour(0xff131922));

    // Continuous low-noise instrument bed. The colour identity lives in
    // exactly two places: the left strip and a single thin accent rule.
    const auto instrument=juce::Rectangle<float>(208.f,4.f,a.getWidth()-214.f,a.getHeight()-8.f);
    gradient(g,instrument,4.f,juce::Colour(0xff222a34),juce::Colour(0xff171d25));
    atelier::fill(g,instrument,3.f,atelier::Material::forgedIron,.06f);
    g.setColour(juce::Colour(0xff414b57).withAlpha(.60f));
    g.drawRoundedRectangle(instrument.reduced(.5f),3.5f,.75f);
    g.setColour(accent.withAlpha(.56f));
    g.fillRoundedRectangle(instrument.getX()+4.f,instrument.getY()+2.f,
                           instrument.getWidth()-8.f,1.6f,.7f);

    // Integrated module title area, not an ornate, competing metal plaque.
    const auto label=juce::Rectangle<float>(5.f,5.f,196.f,a.getHeight()-10.f);
    gradient(g,label,4.f,juce::Colour(0xff2b3440),juce::Colour(0xff19212b));
    g.setColour(accent.withAlpha(.09f));
    g.fillRoundedRectangle(label.reduced(1.f),3.f);
    g.setColour(juce::Colour(0xff586471).withAlpha(.45f));
    g.drawRoundedRectangle(label.reduced(.5f),4.f,.8f);
    g.setColour(accent);
    g.fillRoundedRectangle(label.getX()+1.5f,label.getY()+4.f,
                           3.5f,label.getHeight()-8.f,1.5f);
    g.setColour(accent.withAlpha(.70f));
    g.fillRoundedRectangle(17.f,32.f,126.f,1.5f,.5f);
    g.setColour(ivory.withAlpha(.10f));
    g.drawLine(17.f,49.f,147.f,49.f,.7f);

    const auto motif=juce::Rectangle<float>(158.f,17.f,30.f,30.f);
    g.setColour(juce::Colour(0xff10161d));
    g.fillRoundedRectangle(motif,6.f);
    g.setColour(accent.withAlpha(.48f));
    g.drawRoundedRectangle(motif.reduced(.5f),6.f,1.f);
    icon(g,index,173.f,32.f,active,t);
    g.setColour(juce::Colour(0xff0a0e13));
    g.drawLine(204.f,7.f,204.f,a.getBottom()-7.f,1.4f);
    g.setColour(ivory.withAlpha(.13f));
    g.drawLine(206.f,7.f,206.f,a.getBottom()-7.f,.7f);
    if(active){
        g.setColour(accent.withAlpha(.66f));
        g.fillRoundedRectangle(18.f,a.getBottom()-7.f,126.f,1.4f,.7f);
    }
}

inline void cell(juce::Graphics& g,juce::Rectangle<float> a)
{
    // Consistent neutral control wells; the knobs supply the ONLY hue.
    g.setColour(juce::Colour(0xff0c1015).withAlpha(.8f));
    g.fillRoundedRectangle(a.translated(0.f,1.f),4.f);
    gradient(g,a,4.f,juce::Colour(0xff252d36),juce::Colour(0xff171d24));
    g.setColour(juce::Colour(0xff56616e).withAlpha(.44f));
    g.drawRoundedRectangle(a.reduced(.5f),3.5f,.75f);
    g.setColour(ivory.withAlpha(.07f));
    g.drawLine(a.getX()+4.f,a.getY()+1.5f,a.getRight()-4.f,a.getY()+1.5f,.7f);
}

// The original material-synthesised knob is converted ONCE into neutral
// graphite/nickel. Its metallic highlights remain, but no golden pigment can
// conflict with the module accent ring. Never recolour per frame.
inline const juce::Image& studioKnob()
{
    static const juce::Image image=[]{
        auto img=atelier::knob().createCopy();
        juce::Image::BitmapData pixels(img,juce::Image::BitmapData::readWrite);
        const float cx=(float(img.getWidth())-1.f)*.5f;
        const float cy=(float(img.getHeight())-1.f)*.5f;
        for(int y=0;y<img.getHeight();++y)
            for(int x=0;x<img.getWidth();++x)
            {
                const auto c=pixels.getPixelColour(x,y);
                if(c.getAlpha()==0)continue;
                const float lum=.30f*float(c.getRed())+
                                .59f*float(c.getGreen())+
                                .11f*float(c.getBlue());
                const float dx=(float(x)-cx)/cx,dy=(float(y)-cy)/cy;
                const float radius=std::sqrt(dx*dx+dy*dy);
                const float v=radius>.78f ? (66.f+lum*.52f) : (21.f+lum*.46f);
                const auto r=juce::uint8(juce::jlimit(0.f,255.f,v));
                const auto gg=juce::uint8(juce::jlimit(0.f,255.f,v+4.f));
                const auto b=juce::uint8(juce::jlimit(0.f,255.f,v+8.f));
                pixels.setPixelColour(x,y,juce::Colour(r,gg,b).withAlpha(c.getFloatAlpha()));
            }
        return img;
    }();
    return image;
}

inline void rotary(juce::Graphics& g,int x,int y,int width,int height,float pos,
                   float startAngle,float endAngle,bool hovered,
                   juce::Colour accent=teal)
{
    const float cx=float(x)+float(width)*.5f,cy=float(y)+float(height)*.5f;
    const float half=juce::jmin(float(width),float(height))*.5f;
    const float r=juce::jmax(4.f,half-7.f);
    const float angle=startAngle+pos*(endAngle-startAngle);
    const float ring=r+3.f;
    juce::Path track,signal;
    track.addCentredArc(cx,cy,ring,ring,0.f,startAngle,endAngle,true);
    signal.addCentredArc(cx,cy,ring,ring,0.f,startAngle,angle,true);
    g.setColour(juce::Colour(0xff0b1016));
    g.strokePath(track,juce::PathStrokeType(4.4f));
    g.setColour(juce::Colour(0xff52606c));
    g.strokePath(track,juce::PathStrokeType(1.4f));
    g.setColour(accent.withAlpha(hovered?.26f:.12f));
    g.strokePath(signal,juce::PathStrokeType(5.2f));
    g.setColour(hovered?accent.brighter(.16f):accent);
    g.strokePath(signal,juce::PathStrokeType(2.2f));
    for(int k=0;k<=10;++k){
        const float theta=startAngle+(endAngle-startAngle)*float(k)/10.f;
        const float sn=std::sin(theta),cs=std::cos(theta);
        const float out=half-1.2f;
        const float in=out-(k%5==0?2.3f:1.1f);
        g.setColour(ivory.withAlpha(k%5==0?.55f:.24f));
        g.drawLine(cx+sn*in,cy-cs*in,cx+sn*out,cy-cs*out,k%5==0?.95f:.65f);
    }
    const auto disc=juce::Rectangle<float>(cx-r,cy-r,2*r,2*r);
    g.setColour(black.withAlpha(.86f));
    g.fillEllipse(disc.expanded(1.7f).translated(0.f,1.6f));
    g.drawImage(studioKnob(),disc);
    g.setColour(ivory.withAlpha(.21f));
    g.drawEllipse(disc.reduced(.6f),.7f);
    // Precision white pointer: neutral, never a second competing colour.
    const float tx=cx+std::sin(angle)*r*.68f;
    const float ty=cy-std::cos(angle)*r*.68f;
    g.setColour(black.withAlpha(.75f));
    g.drawLine(cx+.5f,cy+.5f,tx+.5f,ty+.5f,3.f);
    g.setColour(ivory);
    g.drawLine(cx,cy,tx,ty,1.7f);
    g.setColour(juce::Colour(0xff9faab5));
    g.fillEllipse(cx-1.3f,cy-1.3f,2.6f,2.6f);
}

inline void toggle(juce::Graphics& g,juce::Rectangle<float> a,bool on,bool highlighted,bool down,
                   juce::Colour accent=teal)
{
    // Sculpted aluminium toggle bezel with a recessed switch well.
    g.setColour(black.withAlpha(.70f));
    g.fillRoundedRectangle(a.translated(0.f,1.2f).expanded(.6f),4.5f);
    gradient(g,a,4.f,juce::Colour(0xff505e6c),juce::Colour(0xff202a34));
    g.setColour(ivory.withAlpha(.25f));
    g.drawRoundedRectangle(a.reduced(.55f),3.5f,.8f);
    const auto well=a.reduced(2.6f,3.1f);
    gradient(g,well,2.4f,juce::Colour(0xff080e14),juce::Colour(0xff19232c));
    g.setColour(black);
    g.drawRoundedRectangle(well,2.4f,.7f);
    const float switchX=on?well.getRight()-14.f:well.getX()+2.f;
    const auto rocker=juce::Rectangle<float>(switchX,well.getY()+1.6f,12.f,well.getHeight()-3.2f);
    gradient(g,rocker,2.4f,juce::Colour(0xffa8b6c3),juce::Colour(0xff465361));
    g.setColour(ivory.withAlpha(.52f));
    g.drawLine(rocker.getX()+2.f,rocker.getY()+2.f,rocker.getRight()-2.f,rocker.getY()+2.f,.7f);
    const float lampX=on?well.getX()+8.5f:well.getRight()-8.5f;
    if(on)
    {
        g.setColour(accent.withAlpha(highlighted?.25f:.12f));
        g.fillEllipse(lampX-5.5f,well.getCentreY()-5.5f,11.f,11.f);
    }
    g.setColour(on?accent:juce::Colour(0xff52606d));
    g.fillEllipse(lampX-2.6f,well.getCentreY()-2.6f,5.2f,5.2f);
    g.setColour(ivory.withAlpha(down?.72f:.94f));
    g.setFont(juce::Font(juce::FontOptions(9.f)));
    g.drawFittedText(on?"ON":"OFF",well.reduced(13.f,0.f).toNearestInt(),
                     juce::Justification::centred,1);
}

inline void combo(juce::Graphics& g,juce::Rectangle<float> a,bool pressed,
                  juce::Colour accent=teal)
{
    // Flush inset selector rather than a default-looking dropdown.
    g.setColour(black.withAlpha(.68f));
    g.fillRoundedRectangle(a.translated(0.f,1.f),3.7f);
    gradient(g,a,3.5f,juce::Colour(0xff34414e),juce::Colour(0xff19232d));
    g.setColour(juce::Colour(0xff738394).withAlpha(.55f));
    g.drawRoundedRectangle(a.reduced(.5f),3.f,.8f);
    g.setColour(ivory.withAlpha(.11f));
    g.drawLine(a.getX()+4.f,a.getY()+1.6f,a.getRight()-4.f,a.getY()+1.6f,.7f);
    const auto right=juce::Rectangle<float>(a.getRight()-17.5f,a.getY()+2.f,15.5f,a.getHeight()-4.f);
    g.setColour(black.withAlpha(.45f));
    g.fillRoundedRectangle(right,2.f);
    g.setColour(ivory.withAlpha(.13f));
    g.drawLine(right.getX(),right.getY()+2.f,right.getX(),right.getBottom()-2.f,.7f);
    const float xc=right.getCentreX(),yc=right.getCentreY();
    juce::Path arrow;
    arrow.startNewSubPath(xc-3.1f,yc-1.2f);
    arrow.lineTo(xc+3.1f,yc-1.2f);
    arrow.lineTo(xc,yc+2.5f);
    arrow.closeSubPath();
    g.setColour(pressed?accent:ivory.withAlpha(.80f));
    g.fillPath(arrow);
}

inline void pushButton(juce::Graphics& g,juce::Rectangle<float> a,juce::Colour base,bool hovered,bool down)
{
    g.setColour(black.withAlpha(.70f));
    g.fillRoundedRectangle(a.translated(0.f,1.5f),4.f);
    gradient(g,a,4.f,juce::Colour(0xff485562),juce::Colour(0xff222c36));
    if(base.getSaturation()>.20f)
    {
        g.setColour(base.withAlpha(down?.55f:.27f));
        g.fillRoundedRectangle(a.reduced(1.5f),3.f);
        g.setColour(base.withAlpha(.62f));
        g.fillRoundedRectangle(a.getX()+6.f,a.getBottom()-3.2f,a.getWidth()-12.f,1.5f,.75f);
    }
    g.setColour((hovered?ivory:juce::Colour(0xff8393a1)).withAlpha(.74f));
    g.drawRoundedRectangle(a.reduced(.5f),3.5f,.8f);
    g.setColour(ivory.withAlpha(.17f));
    g.drawLine(a.getX()+5.f,a.getY()+2.f,a.getRight()-5.f,a.getY()+2.f,.8f);
}

inline void header(juce::Graphics& g,float width)
{
    const auto area=juce::Rectangle<float>(2.f,2.f,width-4.f,65.f);
    rivetedPlate(g,area,7.f,juce::Colour(0xff303b47),juce::Colour(0xff131a22));
    // Recessed product signature with an optically lit lower edge.
    const auto brand=juce::Rectangle<float>(11.f,9.f,317.f,49.f);
    gradient(g,brand,5.f,juce::Colour(0xff2a3540),juce::Colour(0xff121a23));
    g.setColour(juce::Colour(0xff6e7d8b).withAlpha(.50f));
    g.drawRoundedRectangle(brand.reduced(.5f),4.5f,.85f);
    g.setGradientFill(juce::ColourGradient(teal.brighter(.1f),12.f,13.f,
                                          teal.darker(.55f),12.f,53.f,false));
    g.fillRoundedRectangle(13.f,14.f,3.5f,39.f,1.5f);
    g.setColour(ivory);
    g.setFont(juce::Font(juce::FontOptions(24.4f)));
    g.drawText("STEAMMASTER",25,11,240,29,juce::Justification::centredLeft);
    g.setColour(teal.brighter(.12f));
    g.drawText("EZ",264,11,52,29,juce::Justification::centredLeft);
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(9.f)));
    g.drawText("PRECISION MASTERING WORKS",26,39,278,13,juce::Justification::centredLeft);
    g.setColour(ivory.withAlpha(.08f));
    g.drawLine(22.f,57.f,313.f,57.f,.7f);

    // Engineering mark: three aligned circles form a unique, simple logo.
    const auto mark=juce::Rectangle<float>(347.f,23.f,25.f,25.f);
    g.setColour(juce::Colour(0xff0a131c));
    g.fillEllipse(mark);
    for(int k=0;k<3;++k)
    {
        const float radius=8.f-float(k)*2.2f;
        g.setColour(teal.withAlpha(.73f-float(k)*.15f));
        g.drawEllipse(mark.getCentreX()-radius,mark.getCentreY()-radius,
                      radius*2.f,radius*2.f,1.f);
    }
    g.setColour(ivory);
    g.setFont(juce::Font(juce::FontOptions(11.f)));
    g.drawText("THOMC",385,13,130,20,juce::Justification::centredLeft);
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(9.f)));
    g.drawText("STEAM ENGINEERING",385,34,143,17,juce::Justification::centredLeft);

    const auto controls=juce::Rectangle<float>(528.f,11.f,229.f,44.f);
    gradient(g,controls,5.f,juce::Colour(0xff283440),juce::Colour(0xff121b24));
    g.setColour(juce::Colour(0xff6c7a89).withAlpha(.50f));
    g.drawRoundedRectangle(controls.reduced(.5f),4.5f,.8f);

    // Original signature: a luminous steam-coil/ribbon sculpted from curves.
    // The shape is ornamental only, not a fake control or an audio waveform.
    const auto coil=juce::Rectangle<float>(777.f,9.f,92.f,49.f);
    gradient(g,coil,5.f,juce::Colour(0xff0c1824),juce::Colour(0xff101c27));
    g.setColour(juce::Colour(0xff4e6171).withAlpha(.40f));
    g.drawRoundedRectangle(coil.reduced(.6f),4.5f,.75f);
    const float cx=823.f,cy=33.5f;
    for(int k=4;k>=0;--k)
    {
        const float rx=15.f+float(k)*4.1f;
        const float ry=7.f+float(k)*2.9f;
        juce::Path ribbon;
        ribbon.startNewSubPath(cx-rx,cy);
        ribbon.cubicTo(cx-rx*.75f,cy-ry*1.7f,cx+rx*.67f,cy-ry*1.4f,cx+rx,cy);
        ribbon.cubicTo(cx+rx*.64f,cy+ry*1.6f,cx-rx*.69f,cy+ry*1.45f,cx-rx,cy);
        g.setColour(teal.withAlpha(.06f));
        g.strokePath(ribbon,juce::PathStrokeType(4.5f));
        g.setColour(teal.brighter(.14f).withAlpha(.38f+float(4-k)*.12f));
        g.strokePath(ribbon,juce::PathStrokeType(1.15f));
    }
    g.setColour(ivory.withAlpha(.5f));
    g.drawLine(866.f,17.f,866.f,50.f,.65f);

    const auto metering=juce::Rectangle<float>(942.f,8.f,490.f,51.f);
    gradient(g,metering,4.f,juce::Colour(0xff26333e),juce::Colour(0xff0c141d));
    g.setColour(juce::Colour(0xff5d6c7a).withAlpha(.55f));
    g.drawRoundedRectangle(metering.reduced(.5f),3.5f,.85f);
    g.setColour(ivory.withAlpha(.10f));
    g.drawLine(943.f,34.f,1430.f,34.f,.6f);
}

inline void meter(juce::Graphics& g,juce::Rectangle<float> a,float peak,float held)
{
    g.setColour(juce::Colour(0xff060c13));
    g.fillRoundedRectangle(a.expanded(1.1f),3.f);
    gradient(g,a,2.f,juce::Colour(0xff0b1b25),juce::Colour(0xff050c12));
    g.setColour(juce::Colour(0xff526778).withAlpha(.65f));
    g.drawRoundedRectangle(a.expanded(.4f),2.7f,.75f);
    const auto map=[](float v){
        return juce::jlimit(0.f,1.f,
            (juce::Decibels::gainToDecibels(juce::jmax(.000001f,v))+60.f)/60.f);
    };
    const float fraction=map(peak),hold=map(held);
    // Segmented optical LEDs, colour-coded ONLY for actual level warnings.
    constexpr int n=37;
    const float gap=1.1f;
    const float cellW=(a.getWidth()-2.f-gap*float(n-1))/float(n);
    for(int k=0;k<n;++k)
    {
        const float p=(float(k)+.5f)/float(n);
        const float db=-60.f+p*60.f;
        const auto c=db>-3.f?juce::Colour(0xfff68c83):
                     db>-10.f?juce::Colour(0xffe6bd76):teal;
        const auto led=juce::Rectangle<float>(a.getX()+1.f+float(k)*(cellW+gap),
                                              a.getY()+2.f,cellW,a.getHeight()-4.f);
        g.setColour(p<=fraction?c:juce::Colour(0xff1b2c37));
        g.fillRoundedRectangle(led,.55f);
        if(p<=fraction)
        {
            g.setColour(ivory.withAlpha(.19f));
            g.drawLine(led.getX()+.3f,led.getY()+.7f,
                       led.getRight()-.3f,led.getY()+.7f,.45f);
        }
    }
    g.setColour(ivory.withAlpha(.9f));
    const float xx=a.getX()+hold*a.getWidth();
    g.drawLine(xx,a.getY()+1.f,xx,a.getBottom()-1.f,1.f);
}

} // namespace foundry
