#include "Game.h"
#include "Qor/BasicPartitioner.h"
#include "Qor/Input.h"
#include "Qor/Qor.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include <chrono>
#include <thread>
using namespace std;
using namespace glm;

Game :: Game(Qor* engine):
    m_pQor(engine),
    m_pInput(engine->input()),
    m_pRoot(make_shared<Node>()),
    m_pPipeline(engine->pipeline())
{}

void Game :: preload()
{
    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pRoot->add(m_pCamera);
}

Game :: ~Game()
{
    m_pPipeline->partitioner()->clear();
}

void Game :: enter()
{
    m_pCamera->ortho();
    m_pPipeline->winding(true);
    m_pPipeline->bg_color(Color::black());
    m_pInput->relative_mouse(false);
}

void Game :: logic(Freq::Time t)
{
    if(m_pInput->key(SDLK_ESCAPE))
        m_pQor->quit();

    m_pRoot->logic(t);
}

void Game :: render() const
{
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get());
}

