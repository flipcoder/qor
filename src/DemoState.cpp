#include "DemoState.h"
#include "Input.h"
#include "Qor.h"
#include "TileMap.h"
#include "ScreenFader.h"
#include "Sound.h"
#include "Sprite.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include <chrono>
#include <thread>
#include "PlayerInterface3D.h"
#include "Light.h"
#include "Material.h"
//#include <OALWrapper/OAL_Funcs.h>
using namespace std;
using namespace glm;

DemoState :: DemoState(
    Qor* engine
    //std::string fn
):
    m_pQor(engine),
    m_pInput(engine->input()),
    m_pRoot(make_shared<Node>()),
    m_pOrthoRoot(make_shared<Node>()),
    //m_pInterpreter(engine->interpreter()),
    //m_pScript(make_shared<Interpreter::Context>(engine->interpreter())),
    m_pPipeline(engine->pipeline())
{
    m_Shader = m_pPipeline->load_shaders({"detail"});
}

void DemoState :: preload()
{
    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pRoot->add(m_pCamera);
    m_pOrthoCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pOrthoCamera->ortho();
    m_pOrthoRoot->add(m_pOrthoCamera);
    
    auto win = m_pQor->window();

    auto tex = m_pQor->resources()->cache_cast<Texture>("crosshair2.png");
    auto crosshair = make_shared<Mesh>(
        make_shared<MeshGeometry>(
            Prefab::quad(
                -vec2((float)tex->center().x, (float)tex->center().y) / 2.0f,
                vec2((float)tex->center().x, (float)tex->center().y) / 2.0f
            )
        )
    );
    crosshair->add_modifier(make_shared<Wrap>(Prefab::quad_wrap(
        vec2(1.0f, -1.0f)
    )));
    crosshair->material(make_shared<MeshMaterial>(tex));
    crosshair->position(glm::vec3(win->center().x, win->center().y, 0.0f));
    m_pOrthoRoot->add(crosshair);
    
    auto l = make_shared<Light>();
    l->diffuse(Color(0.0f, 0.0f, 1.0f, 1.0f));
    l->atten(glm::vec3(0.0f, 0.1f, 0.01f));
    m_pRoot->add(l);

    //l = make_shared<Light>();
    //l->position(glm::vec3(0.0f, 0.0f, -10.0f));
    //l->diffuse(Color(0.2f, 0.2f, 1.0f, 1.0f));
    //l->specular(Color(0.2f, 0.2f, 1.0f, 1.0f));
    //l->atten(glm::vec3(0.0f, 0.1f, 0.01f));
    //m_pRoot->add(l);

    //l = make_shared<Light>();
    //l->position(glm::vec3(0.0f, 0.0f, -20.0f));
    //l->diffuse(Color(0.2f, 1.0f, 0.2f, 1.0f));
    //l->specular(Color(0.2f, 1.0f, 0.2f, 1.0f));
    //l->atten(glm::vec3(0.0f, 0.1f, 0.01f));
    //m_pRoot->add(l);

    //m_pPipeline = make_shared<Pipeline>(
    //    m_pQor->window(),
    //    m_pQor->resources(),
    //    m_pRoot,
    //    m_pCamera
    //);
    
    m_pRoot->add(m_pQor->make<Mesh>("apartment_scene.obj"));
    m_pController = m_pQor->session()->profile(0)->controller();
    m_pPlayer = kit::init_shared<PlayerInterface3D>(
        m_pController,
        m_pCamera,
        m_pQor->session()->profile(0)->config()
    );
    m_pPlayer->speed(12.0f);
    const bool ads = false;
    m_pViewModel = make_shared<ViewModel>(
        m_pCamera,
        make_shared<Mesh>(
            m_pQor->resource_path("gun_shotgun_sawnoff.obj"),
            m_pQor->resources()
        )
    );
    //m_pViewModel->node()->rotate(0.5f, Axis::Z);
    //m_pViewModel->node()->position(glm::vec3(
    //    ads ? 0.0f : 0.05f,
    //    ads ? -0.04f : -0.06f,
    //    ads ? -0.05f : -0.15f
    //));
    
    m_pRoot->add(m_pViewModel);
    m_pViewModel->model_pos(glm::vec3(
        0.0f, -0.12f, -0.18f
    ));
    m_pViewModel->zoomed_model_pos(glm::vec3(
        0.0f, -0.12f, -0.18f
    ));
    m_pViewModel->reset_zoom();
    
    // TODO: ensure filename contains only valid filename chars
    //m_pScript->execute_file("mods/"+ m_Filename +"/__init__.py");
    //m_pScript->execute_string("preload()");
    m_pPhysics = make_shared<Physics>(m_pRoot.get(), this);
    m_pPhysics->generate(m_pRoot.get(), (unsigned)Physics::GenerateFlag::RECURSIVE);
}

DemoState :: ~DemoState()
{
    //m_pPipeline->partitioner()->clear();
}

void DemoState :: enter()
{
    m_pPipeline->shader(1)->use();
    m_pPipeline->override_shader(PassType::NORMAL, m_Shader);
     
    m_pCamera->perspective();
    m_pInput->relative_mouse(true);

    on_tick.connect(std::move(screen_fader(
        [this](Freq::Time, float fade) {
            m_pPipeline->shader(1)->use();
            int fadev = m_pPipeline->shader(1)->uniform("Brightness");
            if(fadev != -1)
                m_pPipeline->shader(1)->uniform(
                    fadev,
                    glm::vec3(fade,fade,fade)
                );
        },
        [this](Freq::Time){
            if(m_pInput->key(SDLK_ESCAPE))
                return true;
            return false;
        },
        [this](Freq::Time){
            m_pPipeline->shader(1)->use();
            int u = m_pPipeline->shader(1)->uniform("Brightness");
            if(u >= 0)
                m_pPipeline->shader(1)->uniform(u, Color::white().vec3());
            m_pPipeline->blend(false);
            m_pQor->pop_state();
        }
    )));

    //m_pScript->execute_string("enter()");
}

void DemoState :: logic(Freq::Time t)
{
    Actuation::logic(t);
    
    if(m_pInput->key(SDLK_ESCAPE))
        m_pQor->quit();

    if(m_pController->button("zoom").pressed_now())
        m_pViewModel->zoom(not m_pViewModel->zoomed());
    
    if(m_pController->button("fire").pressed_now()) {
        //Sound::play(m_pCamera.get(), "shotgun.wav", m_pQor->resources());
        auto s = m_pQor->make<Sound>("shotgun.wav");
        m_pCamera->add(s);
        s->play();
        s->detach_on_done();
    }

    m_pViewModel->sway(m_pPlayer->move() != glm::vec3(0.0f));
    m_pViewModel->sprint(
        m_pPlayer->move() != glm::vec3(0.0f) && m_pPlayer->sprint()
    );
    
    m_pOrthoRoot->logic(t);
    m_pRoot->logic(t);

    //if(m_pInput->key(SDLK_DOWN))
    //    m_pViewModel->zoomed_model_move(glm::vec3(0.0f, -t.s(), 0.0f));
    //else if(m_pInput->key(SDLK_UP))
    //    m_pViewModel->zoomed_model_move(glm::vec3(0.0f, t.s(), 0.0f));
    //else if(m_pInput->key(SDLK_LEFT))
    //    m_pViewModel->zoomed_model_move(glm::vec3(-t.s(), 0.0f, 0.0f));
    //else if(m_pInput->key(SDLK_RIGHT))
    //    m_pViewModel->zoomed_model_move(glm::vec3(t.s(), 0.0f, 0.0f));
    //else if(m_pInput->key(SDLK_w))
    //    m_pViewModel->zoomed_model_move(glm::vec3(0.0f, 0.0f, t.s()));
    //else if(m_pInput->key(SDLK_r))
    //    m_pViewModel->zoomed_model_move(glm::vec3(0.0f, 0.0f, -t.s()));

    //LOGf("zoomed model pos %s", Vector::to_string(m_pViewModel->zoomed_model_pos()));
}

void DemoState :: render() const
{
    m_pPipeline->override_shader(PassType::NORMAL, m_Shader);
    m_pPipeline->winding(false);
    m_pPipeline->blend(false);
    m_pPipeline->render(
        m_pRoot.get(),
        m_pCamera.get(),
        nullptr,
        Pipeline::LIGHTS
    );
    m_pPipeline->override_shader(PassType::NORMAL, (unsigned)PassType::NONE);
    m_pPipeline->winding(true);
    m_pPipeline->blend(true);
    m_pPipeline->render(
        m_pOrthoRoot.get(),
        m_pOrthoCamera.get(),
        nullptr,
        Pipeline::NO_CLEAR | Pipeline::NO_DEPTH
    );
    m_pPipeline->blend(false);
}

