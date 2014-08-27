#include "ViewModel.h"
#include <memory>
using namespace std;

ViewModel :: ViewModel(shared_ptr<Camera> camera, shared_ptr<Node> node):
    Tracker(static_pointer_cast<Node>(camera), Tracker::ORIENT, Freq::Time::ms(25)),
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
    
    reset_zoom();
}

void ViewModel :: logic_self(Freq::Time t)
{
    Tracker::logic_self(t);
    position(target()->position());
    m_ZoomAnim.logic(t);
    m_ZoomFOVAnim.logic(t);
    m_pNode->position(m_ZoomAnim.get());
    m_pCamera->fov(m_ZoomFOVAnim.get());
    
    m_SwayTime += t.s();
    m_SwayTime -= trunc(m_SwayTime);
    m_SwayOffset = glm::vec3(
        -0.01f * sin(m_SwayTime * K_TAU),
        0.005f * cos(m_SwayTime * 2.0f * K_TAU),
        0.0f
    );
    m_pNode->move(m_SwayOffset);
}

void ViewModel :: zoom(bool b)
{
    if(m_bZoomed == b)
        return;
    
    m_bZoomed = b;
    
    //if(m_bZoomed)
    //    m_pCamera->fov(m_ZoomedFOV);
    //else
    //    m_pCamera->fov(m_DefaultFOV);
    m_ZoomAnim.stop(
        glm::vec3(
            m_bZoomed ? 0.0f : 0.05f,
            m_bZoomed ? -0.04f : -0.06f,
            m_bZoomed ? -0.05f : -0.15f
        ),
        Freq::Time(250),
        (m_bZoomed ? INTERPOLATE(in_sine<glm::vec3>) : INTERPOLATE(out_sine<glm::vec3>))
    );
    m_ZoomFOVAnim.stop(
        (m_bZoomed ? m_ZoomedFOV : m_DefaultFOV),
        Freq::Time(250),
        (m_bZoomed ? INTERPOLATE(in_sine<float>) : INTERPOLATE(out_sine<float>))
    );
    
    //reset_offset();
}

void ViewModel :: reset()
{
    m_bZoomed = false;
    m_pCamera->fov(m_DefaultFOV);
}

void ViewModel :: reset_zoom()
{
    m_ZoomAnim.stop(glm::vec3(
        m_bZoomed ? 0.0f : 0.05f,
        m_bZoomed ? -0.04f : -0.06f,
        m_bZoomed ? -0.05f : -0.15f
    ));
    m_ZoomFOVAnim.stop(m_DefaultFOV);
}

ViewModel :: ~ViewModel()
{
}

