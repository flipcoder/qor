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
//#include <OALWrapper/OAL_Funcs.h>
using namespace std;
using namespace glm;

BasicState :: BasicState(Qor* engine):
    m_pQor(engine),
    m_pInput(engine->input()),
    m_pRoot(make_shared<Node>()),
    m_pPipeline(engine->pipeline()),
    m_pResources(engine->resources())
{}

void BasicState :: preload()
{
    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pRoot->add(m_pCamera->as_node());
}

BasicState :: ~BasicState()
{
    m_pPipeline->partitioner()->clear();
}

void BasicState :: enter()
{
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

    float speed = 1000.0f * t.s();
    
    if(m_pInput->key(SDLK_UP))
        m_pCamera->move(glm::vec3(0.0f, -speed, 0.0f));
    if(m_pInput->key(SDLK_DOWN))
        m_pCamera->move(glm::vec3(0.0f, speed, 0.0f));
    
    if(m_pInput->key(SDLK_LEFT))
        m_pCamera->move(glm::vec3(-speed, 0.0f, 0.0f));
    if(m_pInput->key(SDLK_RIGHT))
        m_pCamera->move(glm::vec3(speed, 0.0f, 0.0f));

    m_pRoot->logic(t);
}

void BasicState :: render() const
{
    //m_pPipeline->override_shader(PassType::NORMAL, m_Shader);
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get());
}

