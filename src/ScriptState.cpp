#include "ScriptState.h"
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
//#include <OALWrapper/OAL_Funcs.h>
using namespace std;
using namespace glm;

ScriptState :: ScriptState(
    Qor* engine
    //std::string fn
):
    m_pQor(engine),
    m_pInput(engine->input()),
    m_pRoot(make_shared<Node>()),
    m_pInterpreter(engine->interpreter()),
    m_pScript(make_shared<Interpreter::Context>(engine->interpreter())),
    m_pPipeline(engine->pipeline())
{
}

ScriptState :: ScriptState(
    Qor* engine,
    std::string fn
):
    ScriptState(engine)
{
    m_Filename = fn;
}

void ScriptState :: preload()
{
    m_pCamera = make_shared<Camera>(m_pQor->resources(),m_pQor->window());
    m_pRoot->add(m_pCamera->as_node());
    //m_pPipeline = make_shared<Pipeline>(
    //    m_pQor->window(),
    //    m_pQor->resources(),
    //    m_pRoot,
    //    m_pCamera
    //);
#ifndef QOR_NO_PHYSICS
    //m_pPhysics = make_shared<Physics>(m_pRoot.get(), this);
#endif
    if(m_Filename.empty())
        m_Filename = m_pQor->args().value_or("mod", "demo");
    // TODO: ensure filename contains only valid filename chars
    m_pScript->execute_file("mods/"+ m_Filename +"/__init__.py");
    m_pScript->execute_string("preload()");

#ifndef QOR_NO_PHYSICS
    //m_pPhysics->generate(m_pRoot.get());
#endif
}

ScriptState :: ~ScriptState()
{
    m_pScript->execute_string("unload()");
}

void ScriptState :: enter()
{
    m_pCamera->perspective();
    
    on_tick.connect(std::move(screen_fader(
        [this](Freq::Time, float fade) {
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
            m_pPipeline->shader(1)->uniform(
                m_pPipeline->shader(1)->uniform("LightAmbient"),
                Color::white().vec3()
            );
            m_pPipeline->blend(false);
            m_pQor->pop_state();
        }
    )));
    

    m_pScript->execute_string("enter()");
}

void ScriptState :: logic(Freq::Time t)
{
    Actuation::logic(t);
    
#ifndef QOR_NO_PHYSICS
    //m_pPhysics->sync(m_pRoot.get());
    //m_pPhysics->logic(t);
#endif
    
    m_pScript->execute_string((
        boost::format("logic(%s)") % t.s()
    ).str());

    float speed = 1000.0f * t.s();
    //if(m_pInput->key(SDLK_r))
    //{
    //    *m_pCamera->matrix() = glm::scale(
    //        *m_pCamera->matrix(), glm::vec3(1.0f-t.s(), 1.0f-t.s(), 1.0f)
    //    );
    //    m_pCamera->pend();
    //}
    //if(m_pInput->key(SDLK_w))
    //{
    //    *m_pCamera->matrix() = glm::scale(
    //        *m_pCamera->matrix(), glm::vec3(1.0f+t.s(), 1.0f+t.s(), 1.0f)
    //    );
    //    m_pCamera->pend();
    //}

    //if(m_pInput->key(SDLK_UP))
    //    m_pCamera->move(glm::vec3(0.0f, -speed, 0.0f));
    //if(m_pInput->key(SDLK_DOWN))
    //    m_pCamera->move(glm::vec3(0.0f, speed, 0.0f));
    
    //if(m_pInput->key(SDLK_LEFT))
    //    m_pCamera->move(glm::vec3(-speed, 0.0f, 0.0f));
    //if(m_pInput->key(SDLK_RIGHT))
    //    m_pCamera->move(glm::vec3(speed, 0.0f, 0.0f));

    m_pRoot->logic(t);
}

void ScriptState :: render() const
{
    m_pScript->execute_string("render()");
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get());
}

