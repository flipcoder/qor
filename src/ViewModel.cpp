#include "ViewModel.h"
#include <memory>
using namespace std;

ViewModel :: ViewModel(shared_ptr<Camera> camera, shared_ptr<Node> node):
    Tracker(static_pointer_cast<Node>(camera), Tracker::STICK, Freq::Time::ms(100)),
    m_pCamera(camera.get()),
    m_pNode(node)
{
    assert(node);
    assert(camera);
    assert(not node->parent());
    
    if(node->parent() != this)
        add(node);
}

ViewModel :: ~ViewModel()
{
}

