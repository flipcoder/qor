#include "Camera.h"
#include "Window.h"
#include <glm/glm.hpp>
using namespace std;

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
        return local_to_world(Box(
            glm::vec3(0.0f, 0.0f, -100.0f),
            glm::vec3(m_Size.x, m_Size.y, 100.0f)
        ));
    };

    on_pend.connect([this]{
        m_ViewMatrix.pend();
        m_OrthoFrustum.pend();
        //m_ViewNeedsUpdate = true;
        //m_FrustumNeedsUpdate = true;
    });
    m_bInited = true;
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
    
    m_WindowResize = window->on_resize.connect([this, window](glm::ivec2 w){
        m_Size = w;
        recalculate_projection();
        pend();
    });
    
    m_Size = window->size();
    recalculate_projection();
    pend();
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

