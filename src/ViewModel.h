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
        
    private:
        std::shared_ptr<Node> m_pNode;
        Camera* m_pCamera;
};

#endif

