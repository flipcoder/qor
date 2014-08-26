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
        bool zoomed() const { return m_bZoomed; }
        void reset();
        void reset_offset();
        
    private:
        
        bool m_bZoomed = false;
        float m_DefaultFOV;
        float m_ZoomedFOV;
        
        std::shared_ptr<Node> m_pNode;
        Camera* m_pCamera;
};

#endif

