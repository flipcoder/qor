#include "Menu.h"
#include "Canvas.h"
using namespace std;
using namespace glm;

MenuGUI :: MenuGUI(
    Controller* c,
    MenuContext* ctx,
    Menu* menu,
    IPartitioner* partitioner,
    Canvas* canvas,
    float* fade
):
    m_pController(c),
    m_pContext(ctx),
    m_pMenu(menu),
    m_pPartitioner(partitioner),
    m_pCanvas(canvas),
    m_pFade(fade)
{
    
}

void MenuGUI :: logic_self(Freq::Time t)
{
    auto cairo = m_pCanvas->context();

    // clear
    cairo->save();
    cairo->set_operator(Cairo::OPERATOR_CLEAR);
    cairo->paint();
    cairo->restore();
    
    cairo->set_source_rgba(1.0, 1.0, 1.0, 0.5);
    //cairo->select_font_face("Gentium Book Basic", Cairo::FONT_SLANT_NORMAL,
    cairo->select_font_face(
        //"Slackey",
        "Press Start 2P",
        Cairo::FONT_SLANT_NORMAL,
        Cairo::FONT_WEIGHT_NORMAL
    );
    float fade = *m_pFade;
    auto textoffset = vec2(fade);
    
    std::string text = "Qorpse";
    Cairo::TextExtents extents;
    cairo->set_source_rgba(0.2, 0.2, 0.2, 0.5);
    cairo->set_font_size(60.0f + 4.0f * fade);
    cairo->get_text_extents(text, extents);
    cairo->move_to(
        -textoffset.x + m_pCanvas->center().x
            - (extents.width/2 + extents.x_bearing),
        fade * (
            textoffset.y + m_pCanvas->center().y/2.0f
            - (extents.height/2 + extents.y_bearing)
        )
    );
    cairo->show_text(text);
    cairo->set_source_rgba(0.5, 0.0, 0.0, 1.0);
    cairo->move_to(
        textoffset.x + m_pCanvas->center().x
            - (extents.width/2 + extents.x_bearing),
        (1.0f-fade) * (m_pCanvas->size().y)
            + -textoffset.y + m_pCanvas->center().y/2.0f
            - (extents.height/2 + extents.y_bearing)
    );
    cairo->show_text(text);

    float spacing = 0.0f;
    const float spacing_increase = 64.0f;
    if(*m_pContext && m_pContext->state().m_Menu)
        for(auto&& opt: m_pContext->state().m_Menu->options())
        {
            text = opt.m_Text;
            cairo->set_source_rgba(1.0, 1.0, 1.0, 0.25 * fade);
            cairo->set_font_size(44.0f + 4.0f * fade);
            //Cairo::TextExtents extents;
            cairo->get_text_extents(text, extents);
            cairo->move_to(
                -textoffset.x + m_pCanvas->center().x
                - (extents.width/2 + extents.x_bearing),
                fade * (
                    spacing + textoffset.y
                    + m_pCanvas->size().y/2.0f 
                    -(extents.height/2 + extents.y_bearing)
                )
            );
            cairo->show_text(text);
            cairo->set_source_rgba(0.0, 0.5, 0.0, 1.0 * fade);
            cairo->move_to(
                textoffset.x + m_pCanvas->center().x
                    - (extents.width/2 + extents.x_bearing),
                (1.0f-fade) * (m_pCanvas->size().y) + (
                    spacing - textoffset.y +
                    m_pCanvas->size().y/2.0f
                    - (extents.height/2 + extents.y_bearing)
                )
            );
            cairo->show_text(text);
            spacing += spacing_increase;
        }

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

