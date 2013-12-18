#include "NodeInterface.h"

void NodeInterface :: event()
{
}

void NodeInterface :: logic(Freq::Time t)
{
    auto node = m_pNode.lock();
    auto input = m_pInput.lock();
}

