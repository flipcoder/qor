#include "LoadingState.h"
#include "Input.h"
#include "Qor.h"
#include "TileMap.h"
#include "Sprite.h"
#include "kit/log/log.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include <OALWrapper/OAL_Funcs.h>
using namespace std;
using namespace glm;

LoadingState :: LoadingState(Qor* qor):
    m_pQor(qor),
    m_pWindow(qor->window()),
    m_pInput(qor->input()),
    m_pRoot(std::make_shared<Node>()),
    m_pCamera(make_shared<Camera>()),
    m_Fade(qor->timer()->timeline()),
    m_FadeTime(1.0f)
{
    m_pRoot->add(m_pCamera->as_node());
    m_pPipeline = make_shared<BasicPipeline>(
        m_pWindow,
        m_pRoot,
        m_pCamera
    );
    vec2 win = vec2(m_pWindow->size().x, m_pWindow->size().y);
    const float icon_size = win.x / 32.0f;
    const float half_icon_size = icon_size / 2.0f;
    
    //auto bg = make_shared<Mesh>(
    //    make_shared<MeshGeometry>(
    //        Prefab::quad(
    //            vec2(0.0f),
    //            vec2(m_pWindow->size().x, m_pWindow->size().y)
    //        )
    //    )
    //);
    //bg->add_modifier(make_shared<Wrap>(Prefab::quad_wrap()));
    //bg->add_modifier(make_shared<Skin>(
    //    m_pQor->resources()->cache_as<ITexture>(
    //        "background.png"
    //    )
    //));
    //bg->move(vec3(
    //    0.0f, 0.0f, -1.0f
    //));
    //m_pRoot->add(bg);
    
    m_pWaitIcon = make_shared<Mesh>(
        make_shared<MeshGeometry>(
            Prefab::quad(
                vec2(-half_icon_size),
                vec2(half_icon_size)
            )
        )
    );
    m_pWaitIcon->move(vec3(
        win.x - icon_size,
        icon_size,
        //m_pWindow->center().x,
        //m_pWindow->size().y * 1.0f/4.0f,
        0.0f
    ));
    m_pWaitIcon->add_modifier(make_shared<Wrap>(Prefab::quad_wrap()));
    m_pWaitIcon->add_modifier(make_shared<Skin>(
        m_pQor->resources()->cache_as<ITexture>(
            "load-c.png"
        )
    ));
    m_pRoot->add(m_pWaitIcon);
    
    //m_pPipeline->bg_color(Color::white());
    fade_to(Color::white(), m_FadeTime);
}

LoadingState :: ~LoadingState()
{
}

void LoadingState :: fade_to(const Color& c, float t)
{
    m_Fade.set(Freq::Time::seconds(t), ~c, c);
}

void LoadingState :: logic(Freq::Time t)
{ 
    if(m_pInput->key(SDLK_ESCAPE))
        m_pQor->quit();

    m_pRoot->logic(t);
    m_pQor->do_tasks();
    
    //m_pPipeline->shader(1)->uniform(
    //    m_pPipeline->shader(1)->uniform("Fade"),
    //    m_Fade.get().r()
    //);
    m_pPipeline->bg_color(m_Fade.get());

    *m_pWaitIcon->matrix() *= rotate(
        t.s() * 180.0f,
        vec3(0.0f, 0.0f, 1.0f)
    );
    m_pWaitIcon->pend();

    if(m_pQor->state(1)->finished_loading()) {
        if(m_Fade.elapsed()) {
            if(m_Fade.get() == Color::white())
                fade_to(Color::black(), m_FadeTime);
            else
                m_pQor->pop_state();
        }
    }
}

void LoadingState :: render() const
{
    m_pPipeline->render();
}

