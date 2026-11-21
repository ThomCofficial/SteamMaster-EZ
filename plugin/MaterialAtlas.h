#pragma once
// SteamMaster EZ - Original material synthesis / rendered hardware components.
// Produces actual cached ARGB material bitmaps and independent knob sprites.
// There are no external art files, no additional licence, no runtime allocation
// on the DSP thread and no new dependencies. All drawing is in the JUCE GUI.
#include <juce_audio_utils/juce_audio_utils.h>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace atelier
{
enum class Material { forgedIron, smokedWalnut, oldCopper, satinBrass, enamel };
inline int wrap(int n, int m) { n %= m; return n < 0 ? n+m : n; }
inline float unitHash(int x,int y,int seed)
{
    std::uint32_t z=std::uint32_t(x)*0x9e3779b9u^std::uint32_t(y)*0x85ebca6bu
                    ^std::uint32_t(seed)*0xc2b2ae35u;
    z^=z>>16; z*=0x7feb352du; z^=z>>15; z*=0x846ca68bu; z^=z>>16;
    return float(z&0xffffu)/65535.f;
}
inline float lerp(float a,float b,float t){return a+(b-a)*t;}
inline float noise(float x,float y,int period,int seed)
{
    // Periodic smooth value noise: all generated texture tiles wrap seamlessly.
    const int ix=int(std::floor(x)), iy=int(std::floor(y));
    float tx=x-float(ix),ty=y-float(iy);
    tx=tx*tx*(3.f-2.f*tx);ty=ty*ty*(3.f-2.f*ty);
    const float a=unitHash(wrap(ix,period),wrap(iy,period),seed);
    const float b=unitHash(wrap(ix+1,period),wrap(iy,period),seed);
    const float c=unitHash(wrap(ix,period),wrap(iy+1,period),seed);
    const float d=unitHash(wrap(ix+1,period),wrap(iy+1,period),seed);
    return lerp(lerp(a,b,tx),lerp(c,d,tx),ty)*2.f-1.f;
}
inline float smooth(float a,float b,float x)
{
    const float t=juce::jlimit(0.f,1.f,(x-a)/(b-a));
    return t*t*(3.f-2.f*t);
}
inline juce::uint8 channel(float x) {
    return juce::uint8(juce::jlimit(0.f,255.f,x));
}
inline juce::Image makeMaterial(Material material)
{
    constexpr int N=256;
    juce::Image img(juce::Image::ARGB,N,N,true);
    juce::Image::BitmapData px(img,juce::Image::BitmapData::writeOnly);
    const int kind=int(material);
    for(int y=0;y<N;++y)for(int x=0;x<N;++x)
    {
        const float fx=float(x)/N,fy=float(y)/N;
        const float broad=noise(fx*4.f,fy*4.f,4,19+kind*31);
        const float cloud=noise(fx*12.f,fy*12.f,12,37+kind*31);
        const float grit=noise(fx*48.f,fy*48.f,48,53+kind*31);
        const float fine=unitHash(x,y,kind+801)*2.f-1.f;
        const float pits=unitHash(x/2,y/2,kind+113);
        float R=0,G=0,B=0;
        switch(material)
        {
            case Material::forgedIron:
            {
                // Deep iron, oxidised micropits and marbled mineral grain.
                const float dark=23.f*broad+12.f*cloud+7.f*grit+8.f*fine;
                const float oxidation=smooth(.76f,.92f,pits+cloud*.20f);
                R=37.f+dark+oxidation*34.f;
                G=40.f+dark*.91f+oxidation*14.f;
                B=39.f+dark*.83f+oxidation*3.f;
                // Directional finishing scratches, always subpixel and subtle.
                const float scar=unitHash(x/17,y,kind+99);
                if(scar>.992f){R+=20;G+=16;B+=10;}
                break;
            }
            case Material::smokedWalnut:
            {
                // Long fibrous dark wood grain, knots and waxed specular ridges.
                const float warp=7.f*noise(fx*3.f,fy*3.f,3,421);
                const float grain=std::sin(fy*72.f+warp+2.f*cloud);
                const float narrow=std::sin(fy*350.f+3.f*broad+cloud);
                const float ridges=grain*12.f+narrow*5.f+cloud*9.f+broad*13.f+fine*3.f;
                const float pore=(pits>.92f)?-9.f:0.f;
                R=65.f+ridges+pore;
                G=36.f+ridges*.61f+pore*.5f;
                B=21.f+ridges*.34f;
                break;
            }
            case Material::oldCopper:
            {
                const float patina=smooth(.37f,.86f,broad*.48f+cloud*.48f+grit*.08f+.5f);
                const float brushed=8.f*std::sin(fy*412.f)+9.f*grit+7.f*fine;
                R=115.f+16.f*broad+brushed-64.f*patina;
                G=61.f+11.f*cloud+brushed*.63f+26.f*patina;
                B=35.f+9.f*grit+brushed*.46f+26.f*patina;
                break;
            }
            case Material::satinBrass:
            {
                const float hair=std::sin(fy*1100.f+cloud*2.f)*4.f;
                const float shine=16.f*broad+9.f*cloud+4.f*grit+3.f*fine+hair;
                R=170.f+shine; G=125.f+shine*.83f; B=70.f+shine*.59f;
                break;
            }
            case Material::enamel:
            {
                const float fleck=3.f*fine+2.f*cloud;
                R=18.f+fleck; G=38.f+fleck*1.2f; B=37.f+fleck;
                break;
            }
        }
        px.setPixelColour(x,y,juce::Colour(channel(R),channel(G),channel(B)));
    }
    return img;
}
inline const juce::Image& material(Material m)
{
    static const auto iron=makeMaterial(Material::forgedIron);
    static const auto walnut=makeMaterial(Material::smokedWalnut);
    static const auto copper=makeMaterial(Material::oldCopper);
    static const auto brass=makeMaterial(Material::satinBrass);
    static const auto enamel=makeMaterial(Material::enamel);
    switch(m){
        case Material::forgedIron:return iron;
        case Material::smokedWalnut:return walnut;
        case Material::oldCopper:return copper;
        case Material::satinBrass:return brass;
        case Material::enamel:return enamel;
    }
    return iron;
}
inline void fill(juce::Graphics& g,juce::Rectangle<float> a,float corner,
                 Material m,float opacity=1.f)
{
    // Tile image fill respects exact rounded corners, resizing and JUCE's
    // existing coordinate transform. No static PNG overlay of controls.
    g.setTiledImageFill(material(m),0,0,opacity);
    g.fillRoundedRectangle(a,corner);
}
inline juce::Image makeKnob()
{
    constexpr int N=256;
    constexpr float mid=(N-1)*.5f;
    juce::Image img(juce::Image::ARGB,N,N,true);
    juce::Image::BitmapData px(img,juce::Image::BitmapData::writeOnly);
    for(int y=0;y<N;++y)for(int x=0;x<N;++x)
    {
        const float dx=(float(x)-mid)/(mid-1.f),dy=(float(y)-mid)/(mid-1.f);
        const float r=std::sqrt(dx*dx+dy*dy);
        if(r>1.f)continue;
        const float theta=std::atan2(dy,dx);
        const float tangent=std::cos(theta+2.28f);
        const float north=std::cos(theta+1.38f);
        const float grain=unitHash(x,y,625)*2.f-1.f;
        const float brush=std::sin(r*855.f+noise(float(x)/32.f,float(y)/32.f,8,117)*.3f)*3.f;
        float R=0,G=0,B=0;
        if(r>.86f)
        {
            // Individually knurled outer ridges with a curved, hard specular strip.
            const float teeth=std::cos(theta*64.f);
            const float glint=std::pow(juce::jmax(0.f,tangent),9.f);
            const float low=std::pow(juce::jmax(0.f,-north),6.f);
            const float b=35.f*north+22.f*teeth+85.f*glint-24.f*low+grain*5.f;
            R=165.f+b;G=112.f+b*.93f;B=55.f+b*.76f;
        }
        else if(r>.79f)
        {
            const float light=28.f*north+22.f*tangent;
            R=89.f+light;G=61.f+light*.76f;B=32.f+light*.55f;
        }
        else if(r>.70f)
        {
            const float light=30.f*north+39.f*std::pow(juce::jmax(0.f,tangent),5.f)+brush+grain*5.f;
            R=194.f+light;G=141.f+light*.95f;B=70.f+light*.70f;
        }
        else if(r>.645f)
        {
            R=22.f+8.f*north;G=24.f+7.f*north;B=23.f+6.f*north;
        }
        else if(r>.12f)
        {
            // Deep blackened nickel / Bakelite with anisotropic polished grain.
            const float curved=std::pow(juce::jmax(0.f,-.7f*dx-.7f*dy),2.f);
            const float streak=std::sin(r*530.f+theta*.2f)*2.8f+brush*.35f;
            const float light=46.f*curved+14.f*north+streak+grain*3.f;
            R=31.f+light*.88f;G=34.f+light;B=35.f+light;
        }
        else
        {
            const float light=14.f*north+20.f*tangent;
            R=125.f+light;G=102.f+light*.9f;B=66.f+light*.8f;
        }
        // Edge antialiasing and undercut dark contour.
        if(r>.983f){const float edge=(1.f-r)/.017f;R*=edge;G*=edge;B*=edge;}
        px.setPixelColour(x,y,juce::Colour(channel(R),channel(G),channel(B)));
    }
    return img;
}
inline const juce::Image& knob()
{
    static const auto image=makeKnob();
    return image;
}
} // namespace atelier
