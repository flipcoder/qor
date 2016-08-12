#include "Camera.h"
#include "Window.h"
#include <glm/glm.hpp>
using namespace std;
using namespace glm;

Camera :: Camera(const std::string& fn, IFactory* factory, ICache* cache):
    m_pResources((Cache<Resource, std::string>*)cache)
{
    init();
}

void Camera :: init()
{
    assert(m_pResources);
    
#ifndef QOR_NO_AUDIO
    
    auto vol_cb = [this] {
        int v = m_pResources->config()->meta("audio")->at<int>("volume");
        m_Listener.gain =  v / 100.0f;
    };
    m_VolumeCon = m_pResources->config()->meta("audio")->on_change("volume", vol_cb);
    vol_cb();
#endif
    
    m_ViewMatrix = [this]{
        return glm::inverse(*matrix_c(Space::WORLD));
    };
    m_OrthoFrustum = [this]{
        auto w = to_world(Box(
            glm::vec3(0.0f, 0.0f, not floatcmp(m_ZNear,0.0f) ?  m_ZNear : -100.0f),
            glm::vec3(m_Size.x * 1.0f, m_Size.y * 1.0f, not floatcmp(m_ZFar,0.0f) ?  m_ZFar : 100.0f)
        ));
        return w;
    };

    on_pend.connect([this]{
        m_ViewMatrix.pend();
        if(m_bOrtho)
            m_OrthoFrustum.pend();
        else
            calculate_perspective_frustum();
    });
    m_bInited = true;
}

void Camera :: calculate_perspective_frustum()
{
    auto mat = *matrix(Space::WORLD);
    auto pos = Matrix::translation(mat);
    vec3 at = normalize(Matrix::heading(mat));
    vec3 up = normalize(Matrix::up(mat));
    vec3 right = normalize(Matrix::right(mat));
    float ratio = m_Size.x / m_Size.y;
    float hnear = 2.0f * tan(m_FOV / 2.0f) * m_ZNear;
    float wnear = hnear * ratio;
    float hfar = 2.0f * tan(m_FOV / 2.0f) * m_ZFar;
    float wfar = hfar * ratio;
    
    vec3 fc = pos + at * m_ZFar;
    vec3 ftl = fc + (up * hfar/2.0f) - (right * wfar/2.0f);
    vec3 ftr = fc + (up * hfar/2.0f) + (right * wfar/2.0f);
    vec3 fbl = fc - (up * hfar/2.0f) - (right * wfar/2.0f);
    vec3 fbr = fc - (up * hfar/2.0f) + (right * wfar/2.0f);

    vec3 nc = pos + at * m_ZNear;
    vec3 ntl = nc + (up * hnear/2.0f) - (right * wnear/2.0f);
    vec3 ntr = nc + (up * hnear/2.0f) + (right * wnear/2.0f);
    vec3 nbl = nc - (up * hnear/2.0f) - (right * wnear/2.0f);
    vec3 nbr = nc - (up * hnear/2.0f) + (right * wnear/2.0f);

    vec3 a = up * normalize((nc + right * wnear / 2.0f) - pos);
}

void Camera :: logic_self(Freq::Time t)
{
    Tracker::logic_self(t);
    
#ifndef QOR_NO_AUDIO
    
    if(m_bListens)
    {
        //auto pos = position();
        //auto wpos = position(Space::WORLD);
        //LOGf("camera mtx: %s", Matrix::to_string(*matrix_c(Space::WORLD)));
        //LOGf("camera local: (%s, %s, %s)", pos.x % pos.y % pos.z);
        //LOGf("camera world: (%s, %s, %s)", wpos.x % wpos.y % wpos.z);
        //auto fo = focal_offset();
        //LOGf("focal offset: (%s, %s, %s)", fo.x % fo.y % fo.z);
        //auto opos = wpos - fo * Matrix::scale(*matrix_c(Space::WORLD));
        m_Listener.pos = position(Space::WORLD);
        //LOGf("camera pos w/ offset: (%s, %s, %s)", opos.x % opos.y % opos.z);
        m_Listener.at = Matrix::heading(*matrix_c(Space::WORLD));
        m_Listener.up = Matrix::up(*matrix_c(Space::WORLD));
        m_Listener.listen();
    }
#endif
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
    else
    {
        
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
    if(Headless::enabled())
        return;
    
    if(m_bOrtho)
    {
        m_ProjectionMatrix = glm::ortho(
            0.0f,
            static_cast<float>(m_Size.x),
            m_bBottomOrigin ? 0.0f : static_cast<float>(m_Size.y),
            m_bBottomOrigin ? static_cast<float>(m_Size.y) : 0.0f,
            m_OrthoFrustum().min().z,
            m_OrthoFrustum().max().z
        );
    }
    else
    {
        float aspect_ratio = (1.0f * m_Size.x) /
            std::max(1.0f, (1.0f * m_Size.y));
        
        m_ProjectionMatrix = glm::perspectiveFov(
            DEG2RADf(m_FOV),
            1.0f * m_Size.x,
            1.0f * m_Size.y,
            (not floatcmp(m_ZNear,0.0f)) ?  m_ZNear : 0.01f,
            (not floatcmp(m_ZFar,0.0f)) ?  m_ZFar : 1000.0f
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

void Camera :: range(float n, float f)
{
    m_ZNear = n;
    m_ZFar = f;
    m_OrthoFrustum.pend();
    recalculate_projection();
}

//bool Camera :: is_subtree_visible(const Node* n) const
//{
//    if(n->has_children())
//    {
//        if(n->skip_child_box_check())
//            return is_visible(n);
//        return true;
//    }
//    return is_visible(n);
//}

bool Camera :: is_visible(const Node* n, Node::LoopCtrl* lc) const
{
    if(lc) *lc = LC_STEP;
    if(not n->visible()){
        if(lc)*lc = LC_SKIP;
        return false;
    }
    if(not n->self_visible()){
        if(lc && n->skip_child_box_check())
            *lc = LC_SKIP;
        return false;
    }
    bool cb_ret = true;
    if(m_IsNodeVisible)
        cb_ret = m_IsNodeVisible(n, lc);
    return cb_ret && in_frustum(n->world_box());
}

