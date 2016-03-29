#ifndef _LIGHT_H
#define _LIGHT_H

#include "Node.h"
#include "Graphics.h"
#include "kit/math/common.h"
#include "kit/meta/meta.h"

class Pass;
class Light:
    public Node
{
    public:

        enum class Type : unsigned
        {
            NONE = 0,
            POINT,
            DIRECTIONAL,
            SPOT
        };
        
        Light(const std::shared_ptr<Meta>& meta);
        Light()//:
            //m_Type(Type::POINT),
            //m_Atten(glm::vec3(1.0f, 0.0f, 0.0f)),
            //m_Flags(0)
        {}
        virtual ~Light() {}
        
        // bind: to be called only by Scene during a render
        //  id is generated at time of render and may change!
        virtual void bind(Pass* pass) const;

        //virtual void logic_self(Freq::Time t) override;
        //virtual void render_self(Pass* pass) const override;
        
        //Color ambient() const { return m_Ambient; }
        Color diffuse() const { return m_Diffuse; }
        Color specular() const { return m_Specular; }
        //glm::vec3 attenuation() const { return m_Atten; }
        Type light_type() const { return m_Type; }
        float dist() const { return m_Dist; }

        //void ambient(const Color& c) { m_Ambient = c; }
        void diffuse(const Color& c) { m_Diffuse = c; }
        void specular(const Color& c) { m_Specular = c; }
        void dist(float f);
        //void atten(const glm::vec3& atten) { m_Atten = atten; }
        void light_type(Type t) { m_Type = t; }

        void flags(unsigned f) {
            m_Flags = f;
        }
        unsigned flags() const {
            return m_Flags;
        }
    
        virtual bool is_light() const override {
            return true;
        }
        
        virtual std::string type() const override { return "light"; }

    private:
        
        //Color m_Ambient;
        Color m_Diffuse = Color(1.0f, 1.0f, 1.0f);
        Color m_Specular = Color(1.0f, 1.0f, 1.0f);
        //glm::vec3 m_Atten; // c, l, q
        float m_Dist = 1.0f;
        float m_Cutoff = 1.0f; // [0,1]
        Light::Type m_Type = Type::POINT;
        enum class Flags : unsigned {
            F_CAST_SHADOWS = kit::bit(0)
        };
        unsigned m_Flags = 0;

        //AABB m_Box;
};

#endif

