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
}

void Tracker :: logic_self(Freq::Time t)
{
    auto target = m_pTarget.lock();
    if(!target)
        return;

    // TODO: biggest problem with this is it depends on which node gets called
    //  first (this one or the target's?)
    // ^ the above can by solved the new actuation callbacks

    glm::mat4 m(*target->matrix_c(Space::WORLD));

    m_Animation.logic(t);
    
    m_Animation.stop(
        // change this matrix depending on focus mode
        m,
        m_FocusTime,
        [](const glm::mat4& a,  const glm::mat4& b, float t) {
            return glm::interpolate(
                a,b,
                t
                //m_Interp
                //Interpolation::out_sine<float>(0.0f, 1.0, t)
            );
            //glm::vec3 A = Matrix::translation(a);
            //glm::vec3 B = Matrix::translation(b);
            //return glm::translate(A + (B-A)*t);
        }
        //[](const glm::mat4& a, const glm::mat4& b) {
        //    return a == b;
        //}
    );

    if(m_Mode == FOLLOW) {
        // only set translation
        Matrix::translation(*matrix(), Matrix::translation(m_Animation.get()));
        *matrix() *= glm::translate(
            m_FocalOffset
        );
    }else if(m_Mode == ORIENT) {
        // only set orientation
        //auto pos = Matrix::translation(*matrix());
        *matrix() = glm::extractMatrixRotation(m_Animation.get());
        //Matrix::reset_translation(*matrix());
        //Matrix::translation(*matrix(), pos);
    }else if(m_Mode == STICK){
        *matrix() = m_Animation.get();
        //*matrix() = glm::translate(
        //    m_Animation.get(),
        //    m_FocalOffset
        //);
    }
    
    pend();
}

