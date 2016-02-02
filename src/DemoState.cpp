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
    //m_pInterpreter(engine->interpreter()),
    //m_pScript(make_shared<Interpreter::Context>(engine->interpreter())),
    m_pPipeline(engine->pipeline())
{
    m_Shader = m_pPipeline->load_shaders({"lit"});
}

void DemoState :: preload()
{
    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pRoot->add(m_pCamera->as_node());
    m_pRoot->add(make_shared<Light>());
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
    const bool ads = false;
    m_pViewModel = make_shared<ViewModel>(
        m_pCamera,
        make_shared<Mesh>(
            m_pQor->resource_path("gun_bullpup.obj"),
            m_pQor->resources()
        )
    );
    //m_pViewModel->node()->rotate(0.5f, Axis::Z);
    m_pViewModel->node()->position(glm::vec3(
        ads ? 0.0f : 0.05f,
        ads ? -0.04f : -0.06f,
        ads ? -0.05f : -0.15f
    ));
    m_pRoot->add(m_pViewModel);

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
            int fadev = m_pPipeline->shader(1)->uniform("LightAmbient");
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
            int u = m_pPipeline->shader(1)->uniform("LightAmbient");
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

    m_pViewModel->sway(m_pPlayer->move() != glm::vec3(0.0f));
    m_pViewModel->sprint(
        m_pPlayer->move() != glm::vec3(0.0f) && m_pPlayer->sprint()
    );
    m_pRoot->logic(t);
}

void DemoState :: render() const
{
    //m_pScript->execute_string("render()");
    m_pPipeline->override_shader(PassType::NORMAL, m_Shader);
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get());
    m_pPipeline->override_shader(PassType::NORMAL, (unsigned)PassType::NONE);
}

