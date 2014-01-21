#ifndef _LIGHT_H
#define _LIGHT_H

#include "Graphics.h"
#include "Node.h"
#include "kit/math/common.h"

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
        
        Light():
            m_Type(Type::POINT),
            m_Atten(glm::vec3(1.0f, 0.0f, 0.0f)),
            m_Flags(0)
        {
            // TODO: Use light cutoffs for AABB, instead of temp:
            //m_Box.min = glm::vec3(-0.5f);
            //m_Box.max = glm::vec3(0.5f);
        }
        virtual ~Light() {}
        
        // bind: to be called only by Scene during a render
        //  id is generated at time of render and may change!
        virtual void bind(unsigned id) {}

        virtual void logic_self(unsigned advance);
        virtual void render_self(IPartitioner* partitioner, unsigned flags) const;
        
        Color ambient() const { return m_Ambient; }
        Color diffuse() const { return m_Diffuse; }
        Color specular() const { return m_Specular; }
        glm::vec3 attenuation() const { return m_Atten; }
        Type type() const { return m_Type; }

        void ambient(const Color& c) { m_Ambient = c; }
        void diffuse(const Color& c) { m_Diffuse = c; }
        void specular(const Color& c) { m_Specular = c; }
        void atten(const glm::vec3& atten) { m_Atten = atten; }
        void type(Type t) { m_Type = t; }

        void flags(unsigned f) {
            m_Flags = f;
        }
        unsigned flags() const {
            return m_Flags;
        }
    
        virtual bool is_light() const override {
            return true;
        }

    private:
        
        Color m_Ambient;
        Color m_Diffuse;
        Color m_Specular;
        glm::vec3 m_Atten; // c, l, q
        Light::Type m_Type;
        enum class Flags : unsigned {
            F_CAST_SHADOWS = kit::bit(0)
        };
        unsigned m_Flags;

        //AABB m_Box;
};

#endif

