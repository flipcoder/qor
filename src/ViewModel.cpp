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
    
    ///if(node->parent() != this)
    add(node);

    m_DefaultFOV = m_pCamera->fov();
    m_ZoomedFOV = m_pCamera->fov() * (2.0f/3.0f);
    
    m_SprintLowerAnim.stop(0.0f);
    m_SprintRotateAnim.stop(0.0f);
    m_RecoilAnim.stop(0.0f);
    m_EquipAnim.stop(0.0f);
    reset_zoom();
}

void ViewModel :: logic_self(Freq::Time t)
{
    Tracker::logic_self(t);
    position(target()->position(Space::WORLD));
    
    m_ZoomAnim.logic(t);
    m_SprintLowerAnim.logic(t);
    m_SprintRotateAnim.logic(t);
    m_ZoomFOVAnim.logic(t);
    m_RecoilAnim.logic(t);
    m_EquipAnim.logic(t);

    m_pNode->position(
        m_ZoomAnim.get() +
        glm::vec3(0.0f, m_SprintLowerAnim.get() + m_EquipAnim.get(), 0.0f));
    m_pCamera->fov(m_ZoomFOVAnim.get());
    m_pNode->reset_orientation();
    m_pNode->rotate(m_SprintRotateAnim.get(), Axis::Y);
    
    m_SwayOffset = glm::vec3(0.0f);
    if(m_bSway && not m_bZoomed)
    {
        m_SwayTime += t.s();
        const float SwaySpeed = 1.0f;
        m_SwayTime -= trunc(m_SwayTime);
        m_SwayOffset = glm::vec3(
            -0.01f * sin(m_SwayTime * SwaySpeed * K_TAU),
            0.005f * cos(m_SwayTime * SwaySpeed * 2.0f * K_TAU),
            0.01f * -sin(m_SwayTime * SwaySpeed * 2.0f * K_TAU)
        );
    }
    m_SwayOffset += glm::vec3(
        0.0f, 0.0f, m_RecoilAnim.get()
    );
    m_pNode->move(m_SwayOffset);
}

void ViewModel :: sprint(bool b)
{
    if(m_bEquipping)
        return;
    if(m_bSprint == b)
        return;

    m_bSprint = b;
    if(b)
        zoom(false);
    
    m_SprintLowerAnim.stop(
        m_bSprint ? -0.15 : 0.0f,
        Freq::Time(250),
        m_bSprint ? INTERPOLATE(out_sine<float>) : INTERPOLATE(in_sine<float>)
    );
    m_SprintRotateAnim.stop(
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
        m_bZoomed ? m_ZoomedModelPos : m_ModelPos,
        m_ZoomTime,
        (m_bZoomed ? INTERPOLATE(in_sine<glm::vec3>) : INTERPOLATE(out_sine<glm::vec3>))
    );
    m_ZoomFOVAnim.stop(
        (m_bZoomed ? m_ZoomedFOV : m_DefaultFOV),
        m_ZoomTime,
        (m_bZoomed ? INTERPOLATE(in_sine<float>) : INTERPOLATE(out_sine<float>))
    );
}
void ViewModel :: fast_zoom(bool b)
{
    zoom(b);
    reset_zoom();
}

void ViewModel :: reset()
{
    m_bZoomed = false;
    m_pCamera->fov(m_DefaultFOV);
}

void ViewModel :: reset_zoom()
{
    m_ZoomAnim.stop(m_bZoomed ? m_ZoomedModelPos : m_ModelPos);
    //m_bZoomed ? 0.0f : 0.05f,
    //m_bZoomed ? -0.04f : -0.06f,
    //m_bZoomed ? -0.05f : -0.15f
    m_ZoomFOVAnim.stop(m_DefaultFOV);
}

void ViewModel :: recoil(Freq::Time out, Freq::Time in, float dist)
{
    m_RecoilAnim.stop(0.0f);
    m_RecoilAnim.frame(Frame<float>(
        dist,
        out,
        INTERPOLATE(in_sine<float>)
    ));
    m_RecoilAnim.frame(Frame<float>(
        0.0f,
        in,
        INTERPOLATE(in_sine<float>)
    ));
}

void ViewModel :: equip(bool r, std::function<void()> cb)
{
    //if(m_bEquipping)
    //    return;
    if(r == m_bEquip) // redundant
        return;
    m_bEquipping = true;
    m_bEquip = false;

    m_onEquip = cb;
    
    m_EquipAnim.abort();
    m_EquipAnim.frame(Frame<float>(
        r ? 0.0f : -0.5f,
        m_EquipTime,
        r ? INTERPOLATE(in_sine<float>) : INTERPOLATE(out_sine<float>),
        [&,r]{
            m_bEquipping = false;
            m_bEquip = r;
            auto cbc = std::move(m_onEquip);
            if(cbc)
                cbc();
        }
    ));
}

void ViewModel :: fast_equip(bool r)
{
    m_bEquip = r;
    m_bEquipping = false;
    m_EquipAnim.stop(r ? 0.0f : -0.5f);
}

bool ViewModel :: idle() const
{
    return m_SprintRotateAnim.elapsed() &&
        not m_bSprint &&
        not recoil() &&
        not m_bEquipping;
}

ViewModel :: ~ViewModel()
{
}

