#include "Particle.h"
#include "Camera.h"
#include "Material.h"
#include <glm/glm.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtx/orthonormalize.hpp>
using namespace std;
using namespace glm;

Particle :: Particle(std::string fn, Cache<Resource, std::string>* cache)
{
    auto mat = cache->cache_as<Material>(fn);
    m_pMesh = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(
            vec2(-0.5f, -0.5f),
            vec2(0.5f, 0.5f)
        )),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap()),
            make_shared<MeshNormals>(Prefab::quad_normals())
        },
        make_shared<MeshMaterial>(mat)
    );
    m_pMesh->disable_physics();
    add(m_pMesh);
}

void Particle :: logic_self(Freq::Time t)
{
    if(not m_Unit.life) {
        // should've been detach already, assume life is unlimited?
        return;
    }
    
    m_Unit.life = Freq::Time::ms(std::max(0, (int)m_Unit.life.ms() - (int)t.ms()));
    if(not m_Unit.life)
        detach();
}

void Particle :: render_self(Pass* pass) const
{
}

void Particle :: set_render_matrix(Pass* pass) const
{
    if(m_Flags & UPRIGHT)
    {
        mat4 mat(*matrix());
        auto normal = Matrix::headingXZ(*pass->camera()->matrix(Space::WORLD));
        auto up = glm::vec3(0.0f, 1.0f, 0.0f);
        auto pos = Matrix::translation(mat);// + normal * m_Offset;
        auto right = glm::cross(normal, up);
        mat = glm::mat4(glm::orthonormalize(glm::mat3(
            right, up, normal
        )));
        Matrix::translation(mat, pos);
        *matrix() = mat;
        *m_pMesh->matrix() = 
            glm::translate(glm::mat4(1.0f),vec3(0.0f, 0.0f, 1.0f)*m_Offset) *
            glm::scale(glm::mat4(1.0f), m_Unit.scale);
        pend();
        //mat = *parent_c()->matrix_c(Space::WORLD) * mat;
        //mat *= glm::scale(m_Unit.scale);
        //pass->matrix(&mat);
    }
    else
    {
        mat4 mat(*matrix());
        auto pos = Matrix::translation(mat);// + normal * m_Offset;
        mat = glm::extractMatrixRotation(*pass->camera()->matrix(Space::WORLD));
        auto normal = Matrix::heading(*pass->camera()->matrix(Space::WORLD));
        Matrix::translation(mat, pos);
        *matrix() = mat;
        *m_pMesh->matrix() =
            glm::translate(glm::mat4(1.0f),vec3(0.0f, 0.0f, 1.0f)*m_Offset) *
            glm::scale(glm::mat4(1.0f), m_Unit.scale);
        pend();
        //mat = *parent_c()->matrix_c(Space::WORLD) * mat;
        //mat *= glm::scale(m_Unit.scale);
        //pass->matrix(&mat);
    }
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

void Particle :: scale(float f, Space s)
{
    m_Unit.scale *= f;
}

void Particle :: rescale(float f)
{
    m_Unit.scale = glm::vec3(f,f,f);
}

glm::vec3 Particle :: scale(Space s) const
{
    return m_Unit.scale;
}

void Particle :: color(Color c)
{
    m_Unit.color = c;
    ((Material*)m_pMesh->material()->texture())->emissive(c);
}

void Particle :: life(Freq::Time t)
{
    m_Unit.life = t;
}

