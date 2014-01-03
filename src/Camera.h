#ifndef _CAMERA_H_K8EZAF7N
#define _CAMERA_H_K8EZAF7N

#include "Tracker.h"
#include "kit/freq/animation.h"

// TODO: camera should cache it's inverse matrix
//       hook this into Node's pend() func (?)
// Hmm, this won't really speed anything up since the tracker updates every
// logic tick now

class Camera:
    public Tracker
{
    public:

        Camera(
            const std::shared_ptr<Node>& target = std::shared_ptr<Node>()
        ):
            Tracker(target)
        {
        }

        virtual ~Camera() {}

        virtual void logic_self(Freq::Time t) override {
            Tracker::logic_self(t);
        }

        void fov(float f) {
            m_FOV=f;
        }
        float fov() const {
            return m_FOV;
        }

    private:
        float m_FOV = 80.0f;
};

#endif

