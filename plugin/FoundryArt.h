#pragma once
// SteamMaster EZ — Foundry Atelier
// An original, entirely self-contained JUCE vector skin.
// No embedded bitmaps, third-party artwork, fonts, images or DSP changes.
#include <juce_audio_utils/juce_audio_utils.h>
#include <cmath>

namespace foundry
{
inline const juce::Colour black  {0xff100f0c};
inline const juce::Colour iron   {0xff1b2326};
inline const juce::Colour ironHi {0xff374247};
inline const juce::Colour bronze {0xff745136};
inline const juce::Colour copper {0xffca8d51};
inline const juce::Colour gold   {0xffe8c589};
inline const juce::Colour ivory  {0xfff5e6c9};
inline const juce::Colour muted  {0xffadaf9f};
inline const juce::Colour teal   {0xff63dec6};
inline const juce::Colour warm   {0xfff39d5d};

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
    g.setColour(juce::Colours::black.withAlpha(.63f));
    g.fillRoundedRectangle(a.translated(0.f,2.f),corner);
    gradient(g,a,corner,top,bottom);
    g.setColour(bronze.withAlpha(.94f));g.drawRoundedRectangle(a.reduced(.5f),corner,1.1f);
    g.setColour(gold.withAlpha(.30f));g.drawRoundedRectangle(a.reduced(2.f),corner-1.f,.7f);
    // Restrained horizontal brushed texture only, clipped safely away from corners.
    const float left=a.getX()+corner+3.f,right=a.getRight()-corner-3.f;
    if(right>left)
      for(float yy=a.getY()+corner+1.f;yy<a.getBottom()-corner-1.f;yy+=3.5f)
      {
          g.setColour((int(yy*2)%2?ivory:juce::Colours::black).withAlpha(.023f));
          g.drawLine(left,yy,right,yy,.55f);
      }
    const float ss=2.6f;
    screw(g,a.getX()+8.f,a.getY()+8.f,ss);
    screw(g,a.getRight()-8.f,a.getY()+8.f,ss,true);
    screw(g,a.getX()+8.f,a.getBottom()-8.f,ss,true);
    screw(g,a.getRight()-8.f,a.getBottom()-8.f,ss);
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
    // Each of the ten modules has its own distinct, physically plausible motif.
    const auto metal=active?gold:muted;
    switch(index)
    {
    case 1:
        gear(g,x,y,12.f);
        g.setColour(teal.withAlpha(.7f));g.drawEllipse(x-15.f,y-15.f,30.f,30.f,.8f);
        break;
    case 2:
        for(float dx:{-8.f,8.f})
        {
            g.setColour(bronze);g.fillRoundedRectangle(x+dx-5.f,y-13.f,10.f,25.f,2.f);
            g.setColour(gold);g.drawRoundedRectangle(x+dx-5.f,y-13.f,10.f,25.f,2.f,.8f);
            g.drawLine(x+dx,y-9.f,x+dx,y+7.f,2.f);
            lamp(g,x+dx,y+3.f,active,dx<0?teal:warm);
        }
        break;
    case 3:
        for(int k=0;k<4;++k)
        {
            const float dx=(k-1.5f)*7.f;
            g.setColour(bronze);g.drawLine(x+dx,y-11.f,x+dx,y+11.f,.8f);
            g.setColour(k%2==0?teal:gold);
            g.fillEllipse(x+dx-2.5f,y+(k==0?4.f:k==1?-5.f:k==2?-2.f:5.f)-2.5f,5.f,5.f);
        }
        break;
    case 4:
        for(int k=0;k<5;++k)
        {
            const float dx=(k-2.f)*5.3f;
            g.setColour(copper);
            g.drawEllipse(x+dx-4.f,y-8.f,8.f,16.f,1.4f);
        }
        break;
    case 5:
        valve(g,x-8.f,y,9.f,active,t);
        valve(g,x+8.f,y,9.f,active,t+.4f);
        break;
    case 6:
        valve(g,x-10.f,y,9.f,active,t);
        valve(g,x+10.f,y,9.f,active,t+.6f);
        g.setColour(gold);g.drawLine(x-2.f,y+11.f,x+2.f,y+11.f,1.f);
        break;
    case 7:
        g.setColour(gold);g.drawEllipse(x-13.f,y-12.f,23.f,23.f,1.25f);
        g.setColour(teal);g.drawEllipse(x-9.f,y-12.f,23.f,23.f,1.25f);
        g.setColour(metal);g.drawLine(x,y-15.f,x,y+15.f,.65f);
        break;
    case 8:
        tapeReel(g,x-11.f,y,11.f,active?t*.85f:0.f);
        tapeReel(g,x+11.f,y,11.f,active?-t*.85f:0.f);
        g.setColour(gold.withAlpha(.85f));g.drawLine(x-11.f,y+11.f,x+11.f,y+11.f,.8f);
        break;
    case 9:
        for(int side:{-1,1})
        {
            const float xx=x+float(side)*6.f;
            g.setColour(side>0?gold:teal);
            g.drawEllipse(xx-9.f,y-10.f,18.f,20.f,.85f);
            g.drawLine(xx+float(side)*5.f,y-5.f,
                       xx+float(side)*12.f,y-8.f,1.f);
        }
        break;
    case 10:
        for(int k=0;k<5;++k)
        {
            const float xx=x-13.f+float(k)*6.5f;
            g.setColour(k<3?teal:gold);
            g.fillRoundedRectangle(xx,y+(k%2==0?0.f:-5.f),3.5f,11.f,1.f);
        }
        break;
    default: break;
    }
}

inline void moduleSurface(juce::Graphics& g,juce::Rectangle<float> a,int index,bool active,float t)
{
    const auto accent=(index==2||index==5||index==10)?copper:
                      (index==3||index==7||index==9)?teal:gold;
    const auto top=index==4||index==8?juce::Colour(0xff50432e):
                   index==2||index==5?juce::Colour(0xff334446):ironHi;
    const auto bottom=index==4||index==8?juce::Colour(0xff241d16):iron;
    rivetedPlate(g,a,6.f,top,bottom);

    // Hardware chassis: a full-width recessed instrument bay behind actual
    // controls, not individual web-form-looking rectangles.
    const auto instrument=juce::Rectangle<float>(208.f,5.f,a.getWidth()-215.f,a.getHeight()-10.f);
    gradient(g,instrument,4.f,juce::Colour(0xff182023),juce::Colour(0xff101619));
    g.setColour(juce::Colour(0xff0b0e10));g.drawRoundedRectangle(instrument,4.f,1.8f);
    g.setColour(copper.withAlpha(.26f));g.drawLine(213.f,7.f,a.getRight()-10.f,7.f,.8f);
    g.setColour(juce::Colours::black.withAlpha(.35f));g.drawLine(213.f,a.getBottom()-8.f,a.getRight()-10.f,a.getBottom()-8.f,1.2f);

    const auto label=juce::Rectangle<float>(5.f,6.f,196.f,a.getHeight()-12.f);
    gradient(g,label,4.f,juce::Colour(0xff3a3123),juce::Colour(0xff1d211f));
    g.setColour(bronze);g.drawRoundedRectangle(label,4.f,1.2f);
    g.setColour(gold.withAlpha(.46f));g.drawRoundedRectangle(label.reduced(2.f),3.f,.5f);
    g.setColour(accent.withAlpha(.85f));g.fillRoundedRectangle(15.f,13.f,2.5f,27.f,1.f);
    g.setColour(black.withAlpha(.8f));g.fillRoundedRectangle(155.f,14.f,35.f,34.f,4.f);
    g.setColour(bronze);g.drawRoundedRectangle(155.f,14.f,35.f,34.f,4.f,.7f);
    icon(g,index,172.5f,31.f,active,t);
    screw(g,11.5f,9.f,2.f);screw(g,194.5f,9.f,2.f,true);
    screw(g,11.5f,a.getBottom()-9.f,2.f,true);
    screw(g,194.5f,a.getBottom()-9.f,2.f);
    g.setColour(gold.withAlpha(.4f));g.drawLine(21.f,48.f,151.f,48.f,.7f);
    g.setColour(juce::Colours::black.withAlpha(.65f));g.drawLine(205.f,7.f,205.f,a.getBottom()-7.f,2.f);
    g.setColour(copper.withAlpha(.75f));g.drawLine(207.f,8.f,207.f,a.getBottom()-8.f,.9f);

    // One intentional piece of machining per processor rather than a repeated skin.
    if(index==2||index==5) {
        g.setColour(copper.withAlpha(.35f));
        for(int i=0;i<5;++i)g.drawLine(1210.f+float(i)*7.f,9.f,1210.f+float(i)*7.f,14.f,.75f);
    } else if(index==8) {
        for(int i=0;i<7;++i){
            g.setColour(bronze.withAlpha(.27f));
            g.drawLine(1210.f+float(i)*5.f,12.f,1210.f+float(i)*5.f+3.f,8.f,.7f);
        }
    }
    if(active){
        g.setColour(accent.withAlpha(.4f));
        g.fillRoundedRectangle(4.f,a.getBottom()-4.f,193.f,1.2f,.4f);
    }
}

inline void cell(juce::Graphics& g,juce::Rectangle<float> a)
{
    // Slim raised bezels: keep the entire panel readable, not a card for every knob.
    g.setColour(juce::Colours::black.withAlpha(.32f));
    g.fillRoundedRectangle(a.translated(0.f,1.f),3.f);
    gradient(g,a,3.f,juce::Colour(0xff303639),juce::Colour(0xff1b2427));
    g.setColour(gold.withAlpha(.13f));g.drawRoundedRectangle(a.reduced(.6f),3.f,.65f);
    g.setColour(juce::Colours::black.withAlpha(.2f));
    g.drawLine(a.getX()+4.f,a.getBottom()-3.f,a.getRight()-4.f,a.getBottom()-3.f,.8f);
}

inline void rotary(juce::Graphics& g,int x,int y,int width,int height,float pos,
                   float startAngle,float endAngle,bool hovered)
{
    const float cx=float(x)+float(width)*.5f,cy=float(y)+float(height)*.5f;
    const float half=juce::jmin(float(width),float(height))*.5f;
    const float r=juce::jmax(4.f,half-7.2f);
    const float angle=startAngle+pos*(endAngle-startAngle);
    const float ring=r+2.8f;

    // Inscribed scales are not a PNG overlay: pointer and progress are live.
    juce::Path track,signal;
    track.addCentredArc(cx,cy,ring,ring,0.f,startAngle,endAngle,true);
    signal.addCentredArc(cx,cy,ring,ring,0.f,startAngle,angle,true);
    g.setColour(black);g.strokePath(track,juce::PathStrokeType(4.f));
    g.setColour(bronze.withAlpha(.88f));g.strokePath(track,juce::PathStrokeType(1.65f));
    g.setColour((hovered?teal.brighter(.2f):teal).withAlpha(.18f));
    g.strokePath(signal,juce::PathStrokeType(4.8f));
    g.setColour(hovered?teal.brighter(.15f):teal);
    g.strokePath(signal,juce::PathStrokeType(1.7f));
    for(int k=0;k<=10;++k)
    {
        const float theta=startAngle+(endAngle-startAngle)*float(k)/10.f;
        const float sn=std::sin(theta),cs=std::cos(theta);
        const float out=half-1.2f;
        const float in=out-(k%5==0?2.65f:1.25f);
        g.setColour((k%5==0?gold:muted).withAlpha(k%5==0?.85f:.56f));
        g.drawLine(cx+sn*in,cy-cs*in,cx+sn*out,cy-cs*out,k%5==0?1.f:.64f);
    }

    const auto disc=juce::Rectangle<float>(cx-r,cy-r,2*r,2*r);
    g.setColour(juce::Colours::black.withAlpha(.88f));
    g.fillEllipse(disc.expanded(1.1f).translated(0.f,1.7f));
    g.setGradientFill(juce::ColourGradient(gold,cx-r,cy-r,juce::Colour(0xff654323),cx+r,cy+r,false));
    g.fillEllipse(disc);
    g.setColour(ivory.withAlpha(.65f));g.drawEllipse(disc.reduced(.5f),.8f);

    // Radially engraved knurling around the machined bezel.
    for(int k=0;k<24;++k)
    {
        const float a=float(k)*juce::MathConstants<float>::twoPi/24.f;
        g.setColour(black.withAlpha(.66f));
        g.drawLine(cx+std::sin(a)*(r-2.9f),cy-std::cos(a)*(r-2.9f),
                   cx+std::sin(a)*(r-.7f),cy-std::cos(a)*(r-.7f),.7f);
    }
    auto face=disc.reduced(3.5f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff526066),cx-r*.7f,cy-r*.8f,
                                          juce::Colour(0xff151e22),cx+r*.7f,cy+r*.9f,false));
    g.fillEllipse(face);
    g.setColour(juce::Colour(0xff839294).withAlpha(.52f));
    g.drawEllipse(face.reduced(.45f),.8f);

    // Fine pointer engraved in the top plate, with real angular tracking.
    const float tx=cx+std::sin(angle)*r*.65f;
    const float ty=cy-std::cos(angle)*r*.65f;
    g.setColour(black.withAlpha(.88f));
    g.drawLine(cx+.6f,cy+.75f,tx+.6f,ty+.75f,3.6f);
    g.setColour(gold);g.drawLine(cx,cy,tx,ty,2.2f);
    g.setColour(ivory);g.fillEllipse(tx-1.35f,ty-1.35f,2.7f,2.7f);
    screw(g,cx,cy,1.65f,true);
}

inline void toggle(juce::Graphics& g,juce::Rectangle<float> a,bool on,bool highlighted,bool down)
{
    g.setColour(black.withAlpha(.80f));g.fillRoundedRectangle(a.translated(0.f,1.2f),4.f);
    gradient(g,a,4.f,juce::Colour(0xff6d634d),juce::Colour(0xff332b24));
    g.setColour(gold.withAlpha(.58f));g.drawRoundedRectangle(a,4.f,.8f);
    const auto pocket=a.reduced(2.7f,3.2f);
    gradient(g,pocket,2.2f,juce::Colour(0xff101719),juce::Colour(0xff070a0c));
    const float switchX=on?pocket.getRight()-10.5f:pocket.getX()+1.5f;
    const auto rocker=juce::Rectangle<float>(switchX,pocket.getY()+1.5f,9.f,pocket.getHeight()-3.f);
    gradient(g,rocker,2.f,on?juce::Colour(0xffe7d3a1):juce::Colour(0xff94806b),
             on?juce::Colour(0xff805e3a):juce::Colour(0xff46382a));
    g.setColour(ivory.withAlpha(.56f));g.drawLine(rocker.getX()+2.f,rocker.getY()+2.f,
                                                  rocker.getRight()-2.f,rocker.getY()+2.f,.8f);
    lamp(g,on?pocket.getX()+8.f:pocket.getRight()-8.f,pocket.getCentreY(),on,
         highlighted?teal.brighter(.3f):teal);
    g.setColour(ivory.withAlpha(down?.75f:.94f));
    g.setFont(juce::Font(juce::FontOptions(8.8f)));
    g.drawFittedText(on?"ON":"OFF",pocket.reduced(11.f,0.f).toNearestInt(),
                     juce::Justification::centred,1);
}

inline void combo(juce::Graphics& g,juce::Rectangle<float> a,bool pressed)
{
    g.setColour(black);g.fillRoundedRectangle(a.translated(0.f,1.f),3.5f);
    gradient(g,a,3.f,juce::Colour(0xff373d3c),juce::Colour(0xff1b2425));
    g.setColour(copper.withAlpha(.63f));g.drawRoundedRectangle(a.reduced(.5f),3.f,.9f);
    g.setColour(black);g.fillRoundedRectangle(a.getRight()-19.f,a.getY()+2.f,17.f,
                                             a.getHeight()-4.f,2.f);
    g.setColour(gold.withAlpha(.78f));g.drawLine(a.getRight()-20.f,a.getY()+3.f,
                                                  a.getRight()-20.f,a.getBottom()-3.f,.7f);
    const float xc=a.getRight()-10.5f,yc=a.getCentreY();
    juce::Path arrow;arrow.startNewSubPath(xc-3.4f,yc-1.1f);
    arrow.lineTo(xc+3.4f,yc-1.1f);arrow.lineTo(xc,yc+2.5f);arrow.closeSubPath();
    g.setColour(pressed?teal:gold);g.fillPath(arrow);
}

inline void pushButton(juce::Graphics& g,juce::Rectangle<float> a,juce::Colour base,bool hovered,bool down)
{
    g.setColour(juce::Colours::black.withAlpha(.7f));g.fillRoundedRectangle(a.translated(0.f,2.f),4.f);
    gradient(g,a,4.f,base.brighter(hovered?.23f:.12f),base.darker(down?.38f:.22f));
    g.setColour((hovered?ivory:gold).withAlpha(.8f));g.drawRoundedRectangle(a.reduced(.45f),4.f,1.f);
    g.setColour(gold.withAlpha(.45f));g.drawLine(a.getX()+3.f,a.getY()+2.f,
                                                 a.getRight()-3.f,a.getY()+2.f,.65f);
    screw(g,a.getX()+4.f,a.getCentreY(),1.2f);
    screw(g,a.getRight()-4.f,a.getCentreY(),1.2f,true);
}

inline void header(juce::Graphics& g,float width)
{
    const auto area=juce::Rectangle<float>(2.f,2.f,width-4.f,65.f);
    rivetedPlate(g,area,7.f,juce::Colour(0xff4b5350),juce::Colour(0xff202527));

    // Logo cast into a dark-green enamel brass fascia.
    const auto brand=juce::Rectangle<float>(12.f,8.f,312.f,51.f);
    gradient(g,brand,4.f,juce::Colour(0xff243f3b),juce::Colour(0xff0c1e1d));
    g.setColour(gold.withAlpha(.95f));g.drawRoundedRectangle(brand,4.f,1.5f);
    g.setColour(copper.withAlpha(.75f));g.drawRoundedRectangle(brand.reduced(2.f),3.f,.55f);
    g.setFont(juce::Font(juce::FontOptions(25.f)));
    g.setColour(black);g.drawText("STEAMMASTER",21,10,244,31,juce::Justification::centredLeft);
    g.setColour(ivory);g.drawText("STEAMMASTER",20,9,244,31,juce::Justification::centredLeft);
    g.setColour(gold);
    g.setFont(juce::Font(juce::FontOptions(24.f)));
    g.drawText("EZ",265,9,49,31,juce::Justification::centredLeft);
    g.setFont(juce::Font(juce::FontOptions(9.f)));
    g.setColour(muted);g.drawText("THE MASTERING FOUNDRY",22,40,273,13,
                                  juce::Justification::centredLeft);

    // Full engineered logo: a forged gear, pressure line and serial plaque.
    gear(g,352.f,33.f,19.f);
    g.setColour(gold.withAlpha(.72f));g.drawLine(376.f,32.f,405.f,32.f,1.25f);
    ornament(g,379.f,18.f,29.f);
    g.setFont(juce::Font(juce::FontOptions(10.5f)));
    g.setColour(ivory);g.drawText("THOMC",408,13,105,20,juce::Justification::centredLeft);
    g.setFont(juce::Font(juce::FontOptions(9.f)));
    g.setColour(muted);g.drawText("ATELIER No. 01",408,34,112,17,juce::Justification::centredLeft);

    // Heavy bevel around existing RESET, A/B interactive buttons.
    const auto controls=juce::Rectangle<float>(528.f,11.f,229.f,44.f);
    gradient(g,controls,5.f,juce::Colour(0xff171e20),juce::Colour(0xff0b1012));
    g.setColour(copper.withAlpha(.65f));g.drawRoundedRectangle(controls,5.f,.8f);
    screw(g,532.f,16.f,1.6f);screw(g,753.f,50.f,1.6f,true);

    const auto metering=juce::Rectangle<float>(942.f,8.f,490.f,51.f);
    gradient(g,metering,4.f,juce::Colour(0xff152123),juce::Colour(0xff0a1113));
    g.setColour(bronze);g.drawRoundedRectangle(metering,4.f,1.f);
    ornament(g,951.f,12.f,470.f);
}

inline void meter(juce::Graphics& g,juce::Rectangle<float> a,float peak,float held)
{
    g.setColour(black);g.fillRoundedRectangle(a.expanded(1.8f).translated(0.f,1.f),3.f);
    gradient(g,a,2.f,juce::Colour(0xff132829),juce::Colour(0xff071314));
    g.setColour(gold.withAlpha(.4f));g.drawRoundedRectangle(a.expanded(.6f),2.8f,.8f);
    const auto map=[](float v){
        return juce::jlimit(0.f,1.f,(juce::Decibels::gainToDecibels(juce::jmax(.000001f,v))+60.f)/60.f);
    };
    const float fraction=map(peak),hold=map(held);
    if(fraction>.0001f)
    {
        auto fill=a.reduced(1.1f);
        fill.setWidth(fill.getWidth()*fraction);
        g.setGradientFill(juce::ColourGradient(teal,fill.getX(),fill.getY(),
                         fraction>.93f?warm:gold,fill.getRight(),fill.getY(),false));
        g.fillRoundedRectangle(fill,1.5f);
    }
    for(float db:{-48.f,-24.f,-12.f,-6.f})
    {
        const float xx=a.getX()+(db+60.f)/60.f*a.getWidth();
        g.setColour(black.withAlpha(.48f));g.drawLine(xx,a.getY()+1.f,xx,a.getBottom()-1.f,.7f);
    }
    g.setColour(ivory.withAlpha(.85f));
    const float xx=a.getX()+hold*a.getWidth();
    g.drawLine(xx,a.getY()+1.f,xx,a.getBottom()-1.f,1.f);
}
} // namespace foundry
