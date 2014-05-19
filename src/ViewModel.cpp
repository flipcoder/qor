#include "ViewModel.h"

ViewModel :: ViewModel(Camera* camera, std::shared_ptr<Node> node):
    m_pCamera(camera),
    m_pNode(node.get())
{
    assert(node);
    assert(camera);
    assert(node->parent());
    
    if(node->parent() != this)
        add(node);
}

ViewModel :: ~ViewModel()
{
}

