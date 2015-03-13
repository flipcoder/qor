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
    
    m_RotateAnim.stop(0.0f);
    reset_zoom();
}

void ViewModel :: logic_self(Freq::Time t)
{
    Tracker::logic_self(t);
    position(target()->position());
    m_ZoomAnim.logic(t);
    m_RotateAnim.logic(t);
    m_ZoomFOVAnim.logic(t);
    m_pNode->position(m_ZoomAnim.get());
    m_pCamera->fov(m_ZoomFOVAnim.get());
    m_pNode->reset_orientation();
    m_pNode->rotate(m_RotateAnim.get(), Axis::Y);
    
    if(m_bSway)
    {
        m_SwayTime += t.s();
        const float SwaySpeed = 1.0f;
        m_SwayTime -= trunc(m_SwayTime);
        m_SwayOffset = glm::vec3(
            -0.01f * sin(m_SwayTime * SwaySpeed * K_TAU),
            0.005f * cos(m_SwayTime * SwaySpeed * 2.0f * K_TAU),
            0.0f
        );
        m_pNode->move(m_SwayOffset);
    }
}

void ViewModel :: sprint(bool b)
{
    if(m_bSprint == b)
        return;

    m_bSprint = b;
    if(b)
        zoom(false);
    
    m_RotateAnim.stop(
        m_bSprint ? (1.0f / 4.0f) : 0.0f,
        Freq::Time(250),
        m_bSprint ? INTERPOLATE(out_sine<float>) : INTERPOLATE(in_sine<float>)
    );
}

void ViewModel :: zoom(bool b)
{
    if(m_bZoomed == b)
        return;
    
    if(m_bSprint && b)
        return;
    
    m_bZoomed = b;
    
    m_ZoomAnim.stop(
        glm::vec3(
            m_bZoomed ? 0.0f : 0.05f,
            m_bZoomed ? -0.02f : -0.06f, // -0.04
            m_bZoomed ? -0.10f : -0.15f // -0.05
        ),
        m_ZoomTime,
        (m_bZoomed ? INTERPOLATE(in_sine<glm::vec3>) : INTERPOLATE(out_sine<glm::vec3>))
    );
    m_ZoomFOVAnim.stop(
        (m_bZoomed ? m_ZoomedFOV : m_DefaultFOV),
        m_ZoomTime,
        (m_bZoomed ? INTERPOLATE(in_sine<float>) : INTERPOLATE(out_sine<float>))
    );
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

