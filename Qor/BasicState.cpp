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
    m_pRTTRoot(make_shared<Node>()),
    m_pPipeline(engine->pipeline()),
    m_pResources(engine->resources())
{}

void BasicState :: preload()
{
    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pRoot->add(m_pCamera->as_node());
    m_pRTTCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pRTTRoot->add(m_pRTTCamera->as_node());
}

BasicState :: ~BasicState()
{
    m_pPipeline->partitioner()->clear();
}

void BasicState :: enter()
{
    float sw = m_pQor->window()->size().x;
    float sh = m_pQor->window()->size().y;

    //m_pPipeline->bg_color(Color::green());
    m_pCamera->ortho();
    m_pRTTCamera->ortho();
    m_pPipeline->winding(true);
    m_pRenderBuffer = std::make_shared<RenderBuffer>(100,100);

    auto mat = make_shared<MeshMaterial>("logo.png", m_pQor->resources());
    auto mesh = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(vec2(0.0f, 0.0f), vec2(sw, sh))),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap(vec2(0.0f,1.0f), vec2(1.0f,0.0f)))
        }, mat
    );
    m_pRTTRoot->add(mesh);
    
    mesh = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(vec2(0.0f, 0.0f), vec2(sw, sh))),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap(vec2(0.0f,1.0f), vec2(1.0f,0.0f)))
        }, std::make_shared<MeshMaterial>(m_pRenderBuffer->texture())
    );
    m_pRoot->add(mesh);
}

void BasicState :: logic(Freq::Time t)
{
    if(m_pInput->key(SDLK_ESCAPE))
        m_pQor->quit();

    m_pRoot->logic(t);
}

void BasicState :: render() const
{
    float sw = m_pQor->window()->size().x;
    float sh = m_pQor->window()->size().y;
    
    m_pRenderBuffer->push();
    m_pPipeline->render(m_pRTTRoot.get(), m_pRTTCamera.get());
    RenderBuffer::pop();

    glViewport(0,0,sw,sh);
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get());
}

