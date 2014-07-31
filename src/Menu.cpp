#include "Menu.h"

MenuGUI :: MenuGUI(
    Controller* c,
    MenuContext* ctx,
    Menu* menu,
    IPartitioner* partitioner
):
    m_pController(c),
    m_pContext(ctx),
    m_pMenu(menu),
    m_pPartitioner(partitioner)
{
    
}

void MenuGUI :: logic_self(Freq::Time t)
{
    
}

void MenuGUI :: refresh()
{
    children().clear();
    if(!m_pMenu)
        return;
    
    for(auto& op: m_pContext->state().m_Menu->options())
    {
        //add()
    }
}

