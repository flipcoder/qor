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
    m_pCamera(make_shared<Camera>())
    //m_Fade(qor->timer()->timeline())
{
    m_pRoot->add(m_pCamera->as_node());
    m_pPipeline = make_shared<BasicPipeline>(
        m_pWindow,
        m_pRoot,
        m_pCamera
    );
    const float icon_size = m_pWindow->size().x / 32.0f;
    const float half_icon_size = icon_size / 2.0f;
    m_pPipeline->bg_color(Color::white());
    m_pWaitIcon = make_shared<Mesh>(
        make_shared<MeshGeometry>(
            Prefab::quad(-half_icon_size, half_icon_size)
        )
    );
    m_pWaitIcon->move(vec3(
        m_pWindow->center().x,
        m_pWindow->size().y * 3.0f/4.0f,
        0.0f
    ));
    m_pWaitIcon->add_modifier(make_shared<Wrap>(Prefab::quad_wrap()));
    m_pWaitIcon->add_modifier(make_shared<Skin>(
        std::dynamic_pointer_cast<ITexture>(m_pQor->resources()->cache(
            "data/vendor/ionicons/png/512/load-c.png"
        ))
    ));
    m_pRoot->add(m_pWaitIcon);
    
    //fade_to(Color::white(), 1.0f);
}

LoadingState :: ~LoadingState()
{
}

void LoadingState :: fade_to(const Color& c, float t)
{
    //m_Fade.set(Freq::Time::seconds(t), ~c, c);
}

void LoadingState :: logic(Freq::Time t)
{ 
    if(m_pInput->key(SDLK_ESCAPE))
        m_pQor->quit();

    m_pRoot->logic(t);
    m_pQor->do_tasks();
    
    //m_pPipeline->bg_color(m_Fade.get());

    *m_pWaitIcon->matrix() *= glm::rotate(
        t.s() * 180.0f,
        vec3(0.0f, 0.0f, 1.0f)
    );
    m_pWaitIcon->pend_transform();
    
    try{
        if(m_pQor->state(1)->finished_loading()) {
            m_pQor->pop_state();
            //if(m_Fade.elapsed()) {
            //    //if(m_Fade.get() == Color::black())
            //    //else
            //    //fade_to(Color::black(), 1.0f);
            //}
        }
    }catch(const std::out_of_range&){}
}

void LoadingState :: render() const
{
    m_pPipeline->render();
}

