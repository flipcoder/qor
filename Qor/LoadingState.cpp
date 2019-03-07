#include "LoadingState.h"
#include "Input.h"
#include "Qor.h"
#include "TileMap.h"
#include "Sprite.h"
#include "kit/log/log.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include "Light.h"
using namespace std;
using namespace glm;

LoadingState :: LoadingState(Qor* qor):
    m_pQor(qor),
    m_pWindow(qor->window()),
    m_pInput(qor->input()),
    m_pRoot(std::make_shared<Node>()),
    m_pCamera(make_shared<Camera>(qor->resources(), qor->window())),
    m_pPipeline(qor->pipeline())
{
    //m_bFade = m_pQor->args().value_or("no_loading_fade", "").empty();
    string bg = m_pQor->args().value_or("loading_bg", "");
    auto rc = m_pQor->resources();
    //string shader = m_pQor->args().value_or("loading_shader", "");
    //if(not shader.empty()){
    //    m_Shader = m_pPipeline->load_shaders({shader});
    //    m_pPipeline->override_shader(PassType::NORMAL, m_Shader);
    //}
    
    if(not bg.empty()){
        m_BG = Color(bg);
        m_bFade = false;
    }else{
        if(m_bFade)
            m_BG = Color::white();
        else
            m_BG = Color::black();
    }

    //if(!m_bFade)
    //{
        m_pPipeline->shader(1)->use();
        int fade = m_pPipeline->shader(1)->uniform("Brightness");
        m_pPipeline->shader(1)->uniform(
            fade, Color::white().vec3()
        );
    //}
    m_pRoot->add(m_pCamera);
    
    vec2 win = vec2(m_pWindow->size().x, m_pWindow->size().y);
    const float icon_size = win.x / 24.0f;
    const float half_icon_size = icon_size / 2.0f;

    if(m_pQor->exists("splash.png"))
    {
        m_pLogo = Mesh::quad(
            "splash.png",rc,
            vec2(win.x,win.y),
            //vec3(0.0f,0.0f,0.0f)
            m_pWindow->center3f(),
            vec2(0.5f)
            //vec3(0.0f,0.0f,-1.0f)
        );
        m_pRoot->add(m_pLogo);
        //m_bZoom = true;
    }
    else if(m_pQor->exists("loading.png"))
    {
        m_pLogo = Mesh::quad(
            "loading.png",rc,
            vec2(win.y/2.0f),
            m_pWindow->center3f(),
            vec2(0.5f)
        );
        m_pRoot->add(m_pLogo);
        m_bZoom = true;
    }
    //bg->position(vec3(0.0f,0.0f,-2.0f));
    //m_pLogo->add_modifier(make_shared<Wrap>(Prefab::quad_wrap()));
    //m_pLogo->material(make_shared<MeshMaterial>(
    //    rc->cache_cast<ITexture>(
    //        "logo.png"
    //    )
    //));
    
    m_pWaitIcon = make_shared<Mesh>(
        make_shared<MeshGeometry>(
            Prefab::quad(
                vec2(-half_icon_size),
                vec2(half_icon_size)
            )
        )
    );
    m_pWaitIcon->position(vec3(
        //win.x - icon_size,
        //icon_size,
        m_pWindow->center().x,
        m_pWindow->size().y * 1.0f/8.0f,
        0.0f
    ));
    m_pWaitIcon->add_modifier(make_shared<Wrap>(Prefab::quad_wrap()));
    m_pWaitIcon->material(make_shared<MeshMaterial>(
        rc->cache_cast<ITexture>(
            "load-c.png"
        )
    ));
    m_pRoot->add(m_pWaitIcon);
    
    // loading screen style
    //if(m_bFade)
    //    m_pPipeline->bg_color(m_BG);
    //else
    //    m_pPipeline->bg_color(m_BG);

    //fade_to(Color::white(), m_FadeTime);
    m_Fade.frame(Frame<Color>(
        Color::white(),
        Freq::Time::seconds(0.25f),
        INTERPOLATE(out_sine<Color>)
    ));
    //m_Fade.frame(Frame<Color>(
    //    Color::white(), // wait a while
    //    Freq::Time::seconds(0.25f),
    //    INTERPOLATE(Color, out_sine)
    //));
#ifndef QOR_NO_AUDIO
    try{
        Log::Silencer ls;
        m_pMusic = make_shared<Sound>("loading.ogg", rc);
        m_pRoot->add(m_pMusic);
        m_pMusic->play();
    }catch(...){}
#endif
}

LoadingState :: ~LoadingState()
{
    m_pPipeline->override_shader(PassType::NORMAL, (unsigned)PassType::NONE);
}

//void LoadingState :: fade_to(const Color& c, float t)
//{
//    //m_Fade.set(Freq::Time::seconds(t), ~c, c);
//}

void LoadingState :: logic(Freq::Time t)
{
    Actuation::logic(t);
    m_pCamera->ortho(true);
    m_pPipeline->winding(false);
    m_pPipeline->blend(false);

    if(m_pInput->escape())
        m_pQor->quit();
    
    m_Fade.logic(t);
    m_pRoot->logic(t);
    
    if(not Headless::enabled())
    {
        if(m_bFade)
        {
            m_pPipeline->shader(1)->use();
            int fade = m_pPipeline->shader(1)->uniform("Brightness");
            if(fade >= 0)
                m_pPipeline->shader(1)->uniform(
                    fade,
                    m_Fade.get().vec3()
                );
        }
    }
    m_pQor->do_tasks();
    
    // Loading screen fade style?
    if(m_pLogo)
        if(m_bFade){
            //m_pPipeline->bg_color(m_Fade.get());
            
            //if(m_bZoom){
            //    m_pLogo->reset_orientation();
            //    m_pLogo->scale(m_Fade.get().r());
            //}
            m_pLogo->pend();
        }
        
#ifndef QOR_NO_AUDIO
    if(m_pMusic && m_pMusic->source()) {
        //m_pMusic->source()->gain = m_Fade.get().r();
        //m_pMusic->source()->refresh();
    }
#endif

    *m_pWaitIcon->matrix() *= rotate(
        t.s() * float(K_TAU),
        vec3(0.0f, 0.0f, -1.0f)
    );
    m_pWaitIcon->position(vec3(
        m_pWaitIcon->position().x,
        (m_pWindow->size().y * 1.0f/8.0f) * m_Fade.get().r(),
        m_pWaitIcon->position().z
    ));
    m_pWaitIcon->pend();

    if(m_pQor->state(1)->finished_loading()) {
        if(m_Fade.elapsed()) {
            if(m_Fade.get() == Color::white())
            {
                m_Fade.frame(Frame<Color>(
                    Color::black(),
                    Freq::Time::seconds(0.25f),
                    INTERPOLATE(out_sine<Color>)
                ));
            }
            else
            {
                if(not Headless::enabled())
                {
                    if(m_bFade)
                    {
                        m_pPipeline->shader(1)->use();
                        int u = m_pPipeline->shader(1)->uniform("Brightness");
                        if(u >= 0)
                            m_pPipeline->shader(1)->uniform(
                                u, Color::white().vec3()
                            );
                    }
                }
                
                m_pPipeline->blend(false);
                m_pQor->pop_state();
            }
        }
    }
}

void LoadingState :: render() const
{
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get());
}

