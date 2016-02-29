#include "Particle.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtx/orthonormalize.hpp>
using namespace std;
using namespace glm;

Particle :: Particle(std::string fn, Cache<Resource, std::string>* cache)
{
    m_pMesh = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(
            vec2(-0.5f, -0.5f),
            vec2(0.5f, 0.5f)
        )),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap()),
            make_shared<MeshNormals>(Prefab::quad_normals())
        },
        make_shared<MeshMaterial>(
            cache->cache_cast<ITexture>(fn)
        )
    );
    m_pMesh->disable_physics();
    add(m_pMesh);
}

void Particle :: logic_self(Freq::Time t)
{
    
}

void Particle :: render_self(Pass* pass) const
{
}

void Particle :: set_render_matrix(Pass* pass) const
{
    mat4 mat(*matrix());
    auto pos = Matrix::translation(mat);
    //auto pos = vec3(0.0f,2.0f,0.0f);
    mat = glm::extractMatrixRotation(*pass->camera()->matrix(Space::WORLD));
    Matrix::translation(mat, pos);
    *matrix() = mat;
    pend();
    mat = *parent_c()->matrix_c(Space::WORLD) * mat;
    pass->matrix(&mat);
}

Particle :: ~Particle()
{
    
}

ParticleSystem :: ParticleSystem(std::string fn, Cache<Resource, std::string>* cache)
{
    m_pParticle = make_shared<Particle>(fn, cache);
}

void ParticleSystem :: logic_self(Freq::Time t)
{
    m_Accum += t;
}

void ParticleSystem :: lazy_logic(Freq::Time t)
{
    // ...
    
    //m_Accum = Freq::Time(0);
}

void ParticleSystem :: render_self(Pass* pass) const
{
    //auto t = m_Timeline.logic(m_Accum);
    //const_cast<ParticleSystem*>(this)->lazy_logic(t);
}

//virtual void set_render_matrix(Pass* pass) const override;

ParticleSystem :: ~ParticleSystem()
{
    
}

