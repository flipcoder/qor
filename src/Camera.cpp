#include "Camera.h"
#include "Window.h"
#include <glm/glm.hpp>
using namespace std;
using namespace glm;

Camera :: Camera(const std::string& fn, IFactory* factory, ICache* cache)
{
    init();
}

void Camera :: init()
{
    m_ViewMatrix = [this]{
        return glm::inverse(*matrix_c(Space::WORLD));
    };
    m_OrthoFrustum = [this]{
        auto w = local_to_world(Box(
            glm::vec3(0.0f, 0.0f, -100.0f),
            glm::vec3(m_Size.x * 1.0f, m_Size.y * 1.0f, 100.0f)
        ));
        //LOGf("camera world box: %s", string(w));
        return w;
    };

    on_pend.connect([this]{
        m_ViewMatrix.pend();
        m_OrthoFrustum.pend();
    });
    m_bInited = true;
}

void Camera :: logic_self(Freq::Time t)
{
    Tracker::logic_self(t);
    
    if(m_bListens)
    {
        //auto pos = position();
        auto wpos = position(Space::WORLD);
        //LOGf("camera mtx: %s", Matrix::to_string(*matrix_c(Space::WORLD)));
        //LOGf("camera local: (%s, %s, %s)", pos.x % pos.y % pos.z);
        //LOGf("camera world: (%s, %s, %s)", wpos.x % wpos.y % wpos.z);
        auto fo = focal_offset();
        //LOGf("focal offset: (%s, %s, %s)", fo.x % fo.y % fo.z);
        auto opos = wpos - fo * Matrix::scale(*matrix_c(Space::WORLD));
        m_Listener.pos = opos;
        //LOGf("camera pos w/ offset: (%s, %s, %s)", opos.x % opos.y % opos.z);
        m_Listener.at = Matrix::heading(*matrix_c(Space::WORLD));
        m_Listener.up = Matrix::up(*matrix_c(Space::WORLD));
        m_Listener.listen();
    }
}

bool Camera :: in_frustum(const Box& box) const
{
    assert(m_bInited);
    
    if(m_bOrtho)
    {
        if(box.quick_zero())
            return false;
        assert(not box.quick_full());
        return m_OrthoFrustum().collision(box);
        //auto w = m_OrthoFrustum();
        //auto r = w.collision(box);
        //if(r)
        //{
        //    LOGf("camera world box: %s", string(w));
        //    LOGf("colliding box: %s", string(box));
        //}
        //else
        //{
        //    LOGf("camera world box: %s", string(w));
        //    LOGf("culled box: %s", string(box));
        //}
        //return r;
    }
    return true;
}

bool Camera :: in_frustum(glm::vec3 point) const
{
    assert(m_bInited);
    
    if(m_bOrtho)
        return m_OrthoFrustum().collision(point);
    return true;
}

const glm::mat4& Camera :: projection() const
{
    return m_ProjectionMatrix;
}

const glm::mat4& Camera :: view() const
{
    return m_ViewMatrix();
}

void Camera :: window(Window* window)
{
    if(not window)
    {
        m_WindowResize.disconnect();
        return;
    }
    
    auto resize  = [this, window]{
        m_Size = window->size();
        recalculate_projection();
        pend();
    };
    m_WindowResize = window->on_resize(resize);
    resize();
}

void Camera :: recalculate_projection()
{
    if(m_bOrtho)
    {
        m_ProjectionMatrix = glm::ortho(
            0.0f,
            static_cast<float>(m_Size.x),
            m_bBottomOrigin ? 0.0f : static_cast<float>(m_Size.y),
            m_bBottomOrigin ? static_cast<float>(m_Size.y) : 0.0f,
            -100.0f,
            100.0f
        );
    }
    else
    {
        float aspect_ratio = (1.0f * m_Size.x) /
            std::max(1.0f, (1.0f * m_Size.y));
        m_ProjectionMatrix = glm::perspective(
            m_FOV,
            aspect_ratio,
            0.01f,
            1000.0f
        );
    }
    
    pend();
}

void Camera :: ortho(bool origin_bottom)
{
    m_bOrtho = true;
    m_bBottomOrigin = origin_bottom;
    recalculate_projection();
}

void Camera :: perspective(float fov)
{
    m_bOrtho = false;
    m_FOV = fov;
    recalculate_projection();
}

//void Camera :: view_update() const
//{
//    if(not m_ViewNeedsUpdate)
//        return;
    
//    m_ViewMatrix = glm::inverse(*matrix_c(Space::WORLD));
    
//    m_ViewNeedsUpdate = false;
//}

//void Camera :: frustum_update() const
//{
//    if(not m_FrustumNeedsUpdate)
//        return;
    
//    if(m_bOrtho)
//    {
//        m_OrthoFrustum = local_to_world(Box(
//            glm::vec3(0.0f, 0.0f, -100.0f),
//            glm::vec3(m_Size.x, m_Size.y, 100.0f)
//        ));
//    }
    
//    m_FrustumNeedsUpdate = false;
//}

