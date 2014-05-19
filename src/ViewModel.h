#ifndef _VIEWMODEL_H
#define _VIEWMODEL_H

#include "Node.h"
#include "Camera.h"

class ViewModel:
    public Node
{
    public:
        ViewModel(
            Camera* camera, std::shared_ptr<Node> node
        );
        virtual ~ViewModel();

    private:
        Node* m_pNode;
        Camera* m_pCamera;
};

#endif

