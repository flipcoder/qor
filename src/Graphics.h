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
#include <vector>
#include <algorithm>

enum class PassType
{
    NONE = -1,
    BASE = 0,
    NORMAL, // full-bright
    //LIT, // surfaces with lighting
    //DETAIL, // surface detail, lighting
    SHADOW, // light perspective
    USER,
};

enum class PhysicsFlag: unsigned
{
    NONE = 0,
    STATIC = kit::bit(0),
    DYNAMIC = kit::bit(1),
    ACTOR = kit::bit(2)
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

// Axis-aligned bounding box
class Box
{
    public:

        Box();
        
        Box(
            glm::vec3 minimum,
            glm::vec3 maximum
        );
        
        Box(const std::vector<glm::vec3>& points);

        Box(const Box&) = default;
        Box(Box&&) = default;
        Box& operator=(const Box&) = default;
        Box& operator=(Box&&) = default;

        void zero();
        void full();

        static Box Zero() {
            Box b;
            b.zero();
            return b;
        }
        static Box Full() {
            Box b;
            b.full();
            return b;
        }

        const glm::vec3& min() const { return m_Min;}
        glm::vec3& min() { return m_Min;}
        const glm::vec3& max() const { return m_Max; }
        glm::vec3& max() { return m_Max; }

        glm::vec3 size() const {
            return m_Max - m_Min;
        }

        glm::vec3 center() const {
            return (m_Min + m_Max) / 2.0f;
        }

        // get all verts of cube
        std::vector<glm::vec3> verts() const;

        // grow to accomodate point
        Box& operator&=(glm::vec3 point) {
            for(int i=0;i<3;++i)
            {
                if(point[i] < m_Min[i])
                    m_Min[i] = point[i];
                else if(point[i] > m_Max[i])
                    m_Max[i] = point[i];
            }
            return *this;
        }

        bool collision(const glm::vec3& p) const
        {
            return !(
                p.x > m_Max.x ||
                p.x < m_Min.x ||
                p.y > m_Max.y ||
                p.y < m_Min.y ||
                p.z > m_Max.z ||
                p.z < m_Min.z
            );
        }

        bool collision(const Box& rhs) const
        {
            return !(
                rhs.min().x > m_Max.x ||
                rhs.max().x < m_Min.x ||
                rhs.min().y > m_Max.y ||
                rhs.max().y < m_Min.y ||
                rhs.min().z > m_Max.z ||
                rhs.max().z < m_Min.z
            );
        }

        Box intersect(const Box& rhs) const {
            if(not collision(rhs))
                return Box::Zero();

            Box b;
            float vals[4];
            
            vals[0] = min().x;
            vals[1] = max().x;
            vals[2] = rhs.min().x;
            vals[3] = rhs.max().x;
            std::sort(vals, vals + 4, std::less<float>());
            b.min().x = vals[1];
            b.max().x = vals[2];
            
            vals[0] = min().y;
            vals[1] = max().y;
            vals[2] = rhs.min().y;
            vals[3] = rhs.max().y;
            std::sort(vals, vals + 4, std::less<float>());
            b.min().y = vals[1];
            b.max().y = vals[2];

            vals[0] = min().z;
            vals[1] = max().z;
            vals[2] = rhs.min().z;
            vals[3] = rhs.max().z;
            std::sort(vals, vals + 4, std::less<float>());
            b.min().z = vals[1];
            b.max().z = vals[2];

            return b;
        }

        Box normalized() const {
            Box r(*this);
            r.normalize();
            return r;
        }
        void normalize() {
            for(unsigned i=0;i<3;++i)
                if(m_Max[i] < m_Min[i])
                    std::swap(m_Min[i], m_Max[i]);
        }
        //bool quick_valid() const {
        //    return m_Min.x <= m_Max.x;
        //}
        bool quick_full() const {
            return m_Min.x == std::numeric_limits<float>::min();
        }
        bool quick_zero() const {
            return m_Min.x == std::numeric_limits<float>::max();
        }

        //void set_huge() {
        //    m_Min.x = std::numeric_limits<float>::min();
        //}
        
        float volume() const {
            auto sz = size();
            return sz.x * sz.y * sz.z;
        }
        
        operator bool() const {
            return
                m_Min.x <= m_Max.x &&
                m_Min.y <= m_Max.y &&
                m_Min.z <= m_Max.z;
        }

        operator std::string() const {
            return std::string("(") + 
                std::to_string(m_Min.x) + ", " +
                std::to_string(m_Min.y) + ", " +
                std::to_string(m_Min.z) +
            ") <= (" +
                std::to_string(m_Max.x) + ", " +
                std::to_string(m_Max.y) + ", " +
                std::to_string(m_Max.z) +
            ")";
        }
        
    private:
        
        glm::vec3 m_Min;
        glm::vec3 m_Max;

};


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
        glm::vec2 min,
        glm::vec2 max,
        glm::vec2 scale = glm::vec2(1.0f),
        glm::vec2 offset= glm::vec2(0.0f)
    );
    static std::vector<glm::vec2> quad_wrap() {
        return quad_wrap(
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f),
            glm::vec2(0.0f)
        );
    }
    static std::vector<glm::vec2> quad_wrap(
        glm::vec2 scale
    ){
        return quad_wrap(
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            scale
        );
    }

    enum Flag {
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

