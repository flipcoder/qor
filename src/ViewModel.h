#ifndef _VIEWMODEL_H
#define _VIEWMODEL_H

#include "Node.h"
#include "Camera.h"
#include "Tracker.h"
#include "kit/freq/animation.h"

class ViewModel:
    public Tracker
{
    public:
        ViewModel(
            std::shared_ptr<Camera> camera,
            std::shared_ptr<Node> node
        );
        virtual ~ViewModel();
        
        std::shared_ptr<Node> node(){
            return m_pNode;
        }
        std::shared_ptr<const Node> node() const{
            return m_pNode;
        }
        
        void logic_self(Freq::Time t) override;

        void zoom(bool b);
        void sway(bool b) {
            m_bSway = b;
        }
        bool sway() const {
            return m_bSway;
        }

        bool zoomed() const { return m_bZoomed; }
        void reset();
        void reset_zoom();
        
    private:
        
        bool m_bZoomed = false;
        bool m_bSway = false;
        float m_DefaultFOV;
        float m_ZoomedFOV;
        float m_SwayTime = 0.0f;
        glm::vec3 m_SwayOffset;
        
        std::shared_ptr<Node> m_pNode;
        Camera* m_pCamera;
        Animation<glm::vec3> m_ZoomAnim;
        Animation<float> m_ZoomFOVAnim;
};

#endif

