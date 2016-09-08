#include "BasicState.h"
#include "BasicPartitioner.h"
#include "IPartitioner.h"
#include "Input.h"
#include "Qor.h"
#include "TileMap.h"
#include "Sprite.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include <chrono>
#include <thread>
using namespace std;
using namespace glm;

BasicState :: BasicState(Qor* engine):
    m_pQor(engine),
    m_pInput(engine->input()),
    m_pRoot(make_shared<Node>()),
    m_pPipeline(engine->pipeline()),
    m_pResources(engine->resources()),
    m_pCanvas(make_shared<Canvas>(
        engine->window()->size().x, engine->window()->size().y
    )),
    m_pMenuGUI(make_shared<MenuGUI>(
        engine->session()->profile(0)->controller().get(),
        &m_MenuContext,
        &m_MainMenu,
        m_pPipeline->partitioner(),
        m_pCanvas.get(),
        m_pResources,
        "Good Times",
        engine->window()->size().y / 24.0f,
        nullptr,
        5
    ))
    //m_pMenuGUI(make_shared<MenuGUI>(
    //    engine->session()->active_profile(0)->controller().get(),
    //    &m_MenuContext,
    //    &m_MainMenu,
    //    m_pPipeline->partitioner(),
    //    m_pCanvas.get(),
    //    m_pResources,
    //    "PRESS START 2P",
    //    engine->window()->size().y / 30.0f,
    //    nullptr,
    //    7,
    //    1.5f,
    //    Canvas::LEFT,
    //    32.0f,
    //    MenuGUI::F_BOX,
    //    engine->window()
    //))
{}

void BasicState :: preload()
{
    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pRoot->add(m_pCamera->as_node());

    //m_pFont = m_pQor->resources()->cache_as<Font>("PressStart2P-Regular.ttf:30");
    //m_pText = make_shared<Text>(m_pFont);
    //m_pText->set("Hello World!\nAgain down here!\nHELLO?\nOK\nOK2\nReally long text here ok?");
    //auto text2 = make_shared<Text>(m_pFont);
    //text2->set("Testing");
    //m_pRoot->add(text2);
    //m_pText->position(glm::vec3(10.0f, 0.0f, 0.0f));
    //m_pRoot->add(m_pText);
}

BasicState :: ~BasicState()
{
    m_pPipeline->partitioner()->clear();
}

void BasicState :: enter()
{
    m_pPipeline->bg_color(Color::green());
    m_pCamera->ortho();
    m_pPipeline->winding(true);
    
    m_MainMenu.options().emplace_back("OPTION 1", []{
    });
    m_MainMenu.options().emplace_back("OPTION 2", []{
    });
    m_MainMenu.options().emplace_back("OPTION 3", []{
    });
    m_MenuContext.clear(&m_MainMenu);
    m_pRoot->add(m_pMenuGUI);
    
    //LOG("enter");
    //m_pRoot->add(Mesh::line(
    //    vec3(100.0f, 100.0f, 0.0f), vec3(200.0f, 200.0f, 0.0f),
    //    m_pResources->cache_as<Texture>("particle.png")
    //));
}

void BasicState :: logic(Freq::Time t)
{
    if(m_pInput->key(SDLK_ESCAPE))
        m_pQor->quit();

    //float speed = 1000.0f * t.s();
    
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

void BasicState :: render() const
{
    //m_pPipeline->override_shader(PassType::NORMAL, m_Shader);
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get());
}

