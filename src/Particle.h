#ifndef PARTICLE_H_FZHEOU7G
#define PARTICLE_H_FZHEOU7G

#include <string>
#include <vector>
#include "Node.h"
#include "Graphics.h"
#include "Texture.h"
#include "kit/math/common.h"
#include "Resource.h"
#include "kit/cache/cache.h"

class Particle:
    public Node
{
    public:

        struct Unit
        {
            bool active = false;
            float life = 1.0f;
            float size = 1.0f;
            Color color; // opacity in alpha
            glm::vec3 pos;
            glm::vec3 vel;
        };
        
        Particle(std::string fn, Cache<Resource, std::string>* cache);
        virtual ~Particle();

        //void timeline(const std::shared_ptr<Freq::Timeline>& tl){
        //    m_pTimeline = tl;
        //}
        
        virtual void logic_self(Freq::Time t) override;
        virtual void render_self(Pass* pass) const override;

    private:

        // separate timeline to accumulate ticks
        // and advance only on render() (lazy logic)
        //std::shared_ptr<Freq::Timeline> m_pTimeline;
};

class ParticleSystem:
    public Node
{
    public:

        struct Description
        {
            bool active = true;
            Particle::Unit ref;
            glm::vec3 pos_variance;
            glm::vec3 velocity_variance;
            Color color_delta;
            //glm::vec3 variance_bias;
            float spawn_rate = 1.0f;
        };

        ParticleSystem(std::string fn, Cache<Resource, std::string>* cache);
        virtual ~ParticleSystem();

    private:

        //Particle::Unit m_Reference;
        std::vector<Particle::Unit> m_Particles;
        Description m_Description;
        
        virtual void logic_self(Freq::Time t) override;
        virtual void render_self(Pass* pass) const override;

        void lazy_logic(Freq::Time t);
        
        mutable Freq::Time m_Accum;
        mutable Freq::Timeline m_Timeline;
};

#endif

