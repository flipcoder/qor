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
            std::shared_ptr<Node> node,
            Freq::Time t
        );
        virtual ~ViewModel();
        
    private:
        std::shared_ptr<Node> m_pNode;
        Camera* m_pCamera;
};

#endif

