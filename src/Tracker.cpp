#include <glm/glm.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtx/orthonormalize.hpp>
#include "Tracker.h"
#include "kit/math/matrixops.h"
using namespace std;

void Tracker :: update_tracking()
{
    auto target = m_pTarget.lock();
    if(!target)
        return;

    glm::mat4 m(*target->matrix_c(Space::WORLD));
        
    m_Animation.stop(
        m,
        m_FocusTime,
        [](const glm::mat4& a,  const glm::mat4& b, float t) {
            return glm::interpolate(a,b,t);
        }
    );

    sync_tracking();
}

void Tracker :: logic_self(Freq::Time t)
{
    auto target = m_pTarget.lock();
    if(!target)
        return;

    // TODO: biggest problem with this is it depends on which node gets called
    //  first (this one or the target's?)
    // ^ the above can by solved the new actuation callbacks

    m_Animation.logic(t);
    update_tracking();
}

void Tracker :: sync_tracking()
{
    if(m_Mode == FOLLOW) {
        Matrix::translation(*matrix(), Matrix::translation(m_Animation.get()));
        *matrix() *= glm::translate(m_FocalOffset);
    }else if(m_Mode == ORIENT) {
        auto pos = Matrix::translation(*matrix());
        *matrix() = glm::extractMatrixRotation(m_Animation.get());
        Matrix::translation(*matrix(), pos);
    }else if(m_Mode == STICK){
        *matrix() = m_Animation.get();
        *matrix() *= glm::translate(m_FocalOffset);
    }else if(m_Mode == PARALLAX){
        *matrix() = m_Animation.get();
        //position(glm::vec3(
        //    position(Space::WORLD).x * m_ParallaxScale,
        //    position(Space::WORLD).y * m_ParallaxScale,
        //    1.0f
        //));
        //*matrix() *= glm::scale(glm::vec3(m_ParallaxScale, m_ParallaxScale, 1.0f));
    }
    pend();
}

void Tracker :: finish()
{
    auto ft = m_FocusTime;
    sync_tracking();
    m_Animation.finish();
    update_tracking();
    m_FocusTime = ft;
}

