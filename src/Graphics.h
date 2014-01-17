#ifndef _GRAPHICS_H
#define _GRAPHICS_H

//#include "GfxAPI.h"
#include "kit/kit.h"
#include "kit/math/common.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <stdexcept>
#include <string>
#include <sstream>

enum class PassType
{
    NONE = -1,
    BASE = 0,
    NORMAL,
    SHADOW, // light perspective
    USER,
};

class Color
{
    static float saturate(float f, float min=0.0f, float max=1.0f){
        if(f>max)
            f=max;
        else if(f<min)
            f=min;
        return f;
    }

public:

    //union {
        //struct { float r, g, b, a; };
        float c[4];
    //};

    float& r() { return c[0]; }
    const float& r() const { return c[0]; }
    float& g() { return c[1]; }
    const float& g() const { return c[1]; }
    float& b() { return c[2]; }
    const float& b() const { return c[2]; }
    float& a() { return c[3]; }
    const float& a() const { return c[3]; }
    glm::vec3 vec3() const {
        return glm::vec3(
            c[0], c[1], c[2]
        );
    }
    glm::vec4 vec4() const {
        return glm::vec4(
            c[0], c[1], c[2], c[3]
        );
    }

    Color()
    {
        for(int i=0; i<4; ++i)
            c[i] = 0.0f;
    }
    Color(const Color& b){
        for(int i=0; i<4; ++i)
            c[i] = b.c[i];
    }
    explicit Color(float s, float _a = 1.0f) {
        set(s, _a);
    }
    Color(float _r, float _g, float _b, float _a = 1.0f) {
        set(_r, _g, _b, _a);
    }
    explicit Color(unsigned char s, unsigned char _a = 255) {
        set(s/255.0f,_a/255.0f);
    }
    Color(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a = 255) {
        set(_r/255.0f, _g/255.0f, _b/255.0f, _a/255.0f);
    }
    Color(std::string s) {
        hex(s);
    }

    Color& operator+=(const Color& rhs) {
        for(int i=0; i<3; ++i) //rgb
            c[i] += rhs.c[i];
        return *this;
    }
    
    Color& operator+=(float f) {
        for(int i=0; i<3; ++i) //rgb
            c[i] += f;
        return *this;
    }

    Color& operator-=(float f) {
        for(int i=0; i<3; ++i) //rgb
            c[i] -= f;
        return *this;
    }

    Color operator*=(float f) {
        for(int i=0; i<4; ++i) //rgba
            c[i] *= f;
        return *this;
    }
    Color operator*=(const Color& b) {
        for(int i=0; i<4; ++i) //rgba
            c[i] *= b.c[i];
        return *this;
    }
    friend Color operator-(const Color& a, const Color& b) {
        Color c;
        for(int i=0; i<4; ++i) //rgba
            c.c[i] = a.c[i] - b.c[i];
        // allow negative colors from subtraction
        return c;
    }
    Color operator~() const {
        Color col;
        for(int i=0; i<3; ++i) //rgb
            col.c[i] = 1.0f - c[i];
        return col;
    }
    Color operator-() const {
        Color c;
        for(int i=0; i<4; ++i) //rgba
            c.c[i] = -c.c[i];
        return c;
    }
    friend Color operator+(const Color& a, const Color& b) {
        Color c;
        for(int i=0; i<4; ++i) //rgba
            c.c[i] = a.c[i] + b.c[i];
        return c;
    }
    friend Color operator*(const Color& a, const Color& b) {
        Color c;
        for(int i=0; i<4; ++i) //rgba
            c.c[i] = a.c[i] * b.c[i];
        return c;
    }

    friend Color operator*(const Color& a, float s) {
        Color c = a;
        for(int i=0; i<4; ++i) //rgba
            c.c[i] *= s;
        return c;
    }
    friend bool operator==(const Color& a, const Color& b) {
        for(unsigned int i=0; i<4; ++i)
            if(!floatcmp(a.c[i], b.c[i]))
                return false;
        return true;
    }
        
    std::string string() const {
        std::ostringstream ss;
        ss << "Color(";
        for(unsigned i=0; i<4; ++i) {
            if(i)
                ss << ", " << c[i];
            else
                ss << c[i];
        }
        ss << ")";
        return ss.str();
    }
    
    void saturate() {
        for(unsigned int i=0; i<4; ++i)
            c[i] = saturate(c[i], 1.0f);
    }

    void set(float _c, float _a=1.0f) {
        const float f = _c;
        for(unsigned int i=0; i<4; ++i)
            c[i] = f;
        c[3]=_a;
    }
    void set(float _r, float _g, float _b) {
        r()=_r; b()=_b; g()=_g; // saturate();
    }
    void set(float _r, float _g, float _b, float _a) {
        r()=_r; b()=_b; g()=_g; a()=_a; //saturate();
    }
    static Color white(float _a = 1.0f) {
        Color c;
        for(int i=0;i<3;++i)
            c.c[i] = 1.0f;
        c.c[3] = _a;
        return c;
    }
    static Color black(float _a = 1.0f) {
        Color c;
        for(int i=0;i<3;++i)
            c.c[i] = 0.0f;
        c.c[3] = _a;
        return c;
    }
    unsigned char red_byte() const { return (unsigned char)std::rint(255*c[0]); }
    unsigned char green_byte() const { return (unsigned char)std::rint(255*c[1]); }
    unsigned char blue_byte() const { return (unsigned char)std::rint(255*c[2]); }
    float* array() const { return (float*)&c[0]; }

    //void allegro(const ALLEGRO_COLOR& ac) {
    //    al_unmap_rgba_f(ac,&r,&g,&b,&a);
    //}
    //ALLEGRO_COLOR allegro() const {
    //    return al_map_rgba_f(r,g,b,a);
    //}
    
    bool hex(std::string s){
        unsigned int v;

        if(boost::starts_with(s, "0x"))
            s = s.substr(2);
        else if(boost::starts_with(s, "#"))
            s = s.substr(1);

        for(size_t i=0;i<s.size();++i) {
            try{
                v = boost::lexical_cast<unsigned int>(std::string("0x") + s.substr(i*2,i+2));
                c[i] = std::rint(v/255.0f);
            }catch(const boost::bad_lexical_cast&){
                return false;
            }catch(const std::out_of_range&){
                return false;
            }
        }
        return true;
    }
};

//template<class V>
//struct Box
//{
//    private:
//        V m_Pos = V();
//        V m_Size = V();
//    public:
//        V pos() const {
//            return m_Pos;
//        }
//        V size() const {
//            return m_Size;
//        }
//        V center() const {
//            return m_Pos + m_Size/2.0f;
//        }
//        V max() const {
//            return m_Pos + m_Size;
//        }
//};

struct Prefab
{
    // triangulated quad vertices
    static std::vector<glm::vec3> quad(
        glm::vec2 min = glm::vec2(0.0f,0.0f),
        glm::vec2 max = glm::vec2(1.0f,1.0f),
        float depth = 0.0f
    );

    // Wrap/UV coordinates for quad above
    static std::vector<glm::vec2> quad_wrap(
        glm::vec2 min = glm::vec2(0.0f, 0.0f),
        glm::vec2 max = glm::vec2(1.0f, 1.0f)
    );

    enum Flags {
        H_FLIP = kit::bit(0),
        V_FLIP = kit::bit(1)
    };

    // Generate wrap/UV coordinates for a tile inside a tileset
    // tile geometry should be triangulated
    static std::vector<glm::vec2> tile_wrap(
        glm::uvec2 tile_size,
        glm::uvec2 tileset_size,
        unsigned int index = 0,
        unsigned int flags = 0
    );
};

enum class Space
{
    LOCAL,
    PARENT,
    WORLD
};

#endif

