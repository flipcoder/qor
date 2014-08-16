#include "Menu.h"
#include "Canvas.h"
using namespace std;

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
    
    unsigned idx = 0;
    for(auto& op: m_pContext->state().m_Menu->options())
    {
        auto c = make_shared<Canvas>(512, idx * 32);
        add(c);
        ++idx;
    }
}

