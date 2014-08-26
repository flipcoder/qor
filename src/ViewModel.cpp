#include "ViewModel.h"
#include <memory>
using namespace std;

ViewModel :: ViewModel(shared_ptr<Camera> camera, shared_ptr<Node> node):
    Tracker(static_pointer_cast<Node>(camera), Tracker::ORIENT, Freq::Time::ms(50)),
    m_pCamera(camera.get()),
    m_pNode(node)
{
    assert(node);
    assert(camera);
    assert(not node->parent());
    
    if(node->parent() != this)
        add(node);

    m_DefaultFOV = m_pCamera->fov();
    m_ZoomedFOV = m_pCamera->fov() * (2.0f/3.0f);
    
    reset_offset();
}

void ViewModel :: logic_self(Freq::Time t)
{
    Tracker::logic_self(t);
    position(target()->position());
}

void ViewModel :: zoom(bool b)
{
    if(m_bZoomed == b)
        return;
    
    m_bZoomed = b;
    
    if(m_bZoomed)
        m_pCamera->fov(m_ZoomedFOV);
    else
        m_pCamera->fov(m_DefaultFOV);
    
    reset_offset();
}

void ViewModel :: reset()
{
    m_bZoomed = false;
    m_pCamera->fov(m_DefaultFOV);
}

void ViewModel :: reset_offset()
{
    m_pNode->position(glm::vec3(
        m_bZoomed ? 0.0f : 0.05f,
        m_bZoomed ? -0.04f : -0.06f,
        m_bZoomed ? -0.05f : -0.15f
    ));
}

ViewModel :: ~ViewModel()
{
}

