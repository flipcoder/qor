#ifndef PARTICLE_H_FZHEOU7G
#define PARTICLE_H_FZHEOU7G

#include <string>
#include <vector>
#include "Node.h"
#include "Graphics.h"
#include "kit/math/common.h"
#include "Resource.h"
#include "kit/cache/cache.h"

class Particle:
    public Node
{
    public:

        struct Data
        {
            float life = 1.0f;
            float size = 1.0f;
            Color color;
        };
        
        Particle(std::string fn, Cache<Resource, std::string>* cache);
        virtual ~Particle();

    private:

};

class ParticleSystem
{
    public:
        ParticleSystem();
        virtual ~ParticleSystem();

    private:

        Particle::Data m_Reference;
        std::vector<Particle::Data> m_Particles;
        
};

#endif

