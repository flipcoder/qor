#include <glm/glm.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include "Tracker.h"
#include "kit/math/matrixops.h"
using namespace std;

void Tracker :: update_tracking()
{
    auto target = m_pTarget.lock();
    if(!target)
        return;

    glm::mat4 m(*target->matrix_c(Space::WORLD));
    Matrix::reset_orientation(m);

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

    glm::mat4 m(*target->matrix_c(Space::WORLD));
    Matrix::reset_orientation(m);

    m_Animation.ensure(
        // change this matrix depending on focus mode
        m,
        m_FocusTime,
        [](const glm::mat4& a,  const glm::mat4& b, float t) {
            return glm::interpolate(a,b,t);
            //glm::vec3 A = Matrix::translation(a);
            //glm::vec3 B = Matrix::translation(b);
            //return glm::translate(A + (B-A)*t);
        },
        [](const glm::mat4& a, const glm::mat4& b) {
            return a == b;
        }
    );
    m_Animation.logic(t);

    *matrix() =  glm::translate(m_Animation.get(),
        m_FocalOffset
    );
    pend();
}

