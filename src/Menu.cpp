#include "Menu.h"
#include "Canvas.h"
#include "Sound.h"
#include "Node.h"
#include "kit/kit.h"
using namespace std;
using namespace glm;

void Menu :: Option :: operator()()
{
    TRY(m_Callback());
}

MenuGUI :: MenuGUI(
    Controller* c,
    MenuContext* ctx,
    Menu* menu,
    IPartitioner* partitioner,
    Canvas* canvas,
    Cache<Resource, std::string>* cache,
    std::string font,
    float* fade
):
    m_pController(c),
    m_pContext(ctx),
    m_pMenu(menu),
    m_pPartitioner(partitioner),
    m_pCanvas(canvas),
    m_pCache(cache),
    m_Font(font),
    m_pFade(fade)
{
    
}

void MenuGUI :: logic_self(Freq::Time t)
{
    auto cairo = m_pCanvas->context();

    if(m_pController->button("up").pressed_now() ||
       m_pController->input()->key("up").pressed_now()
    ){
        if(m_pContext->state().next_option(-1)){
            auto snd = make_shared<Sound>("highlight.wav",m_pCache);
            add(snd);
            snd->source()->play();
            snd->on_tick.connect([snd](Freq::Time){
                if(not snd->source()->playing())
                    snd->detach();
            });
        }
    }
    if(m_pController->button("down").pressed_now() ||
        m_pController->input()->key("down").pressed_now()
    ){
        if(m_pContext->state().next_option(1)){
            auto snd = make_shared<Sound>("highlight.wav",m_pCache);
            add(snd);
            snd->source()->play();
            snd->on_tick.connect([snd](Freq::Time){
                if(not snd->source()->playing())
                    snd->detach();
            });
        }
    }
    if(m_pController->button("select").pressed_now() ||
       m_pController->input()->key("return").pressed_now() ||
       m_pController->input()->key("space").pressed_now()
    ){
        auto snd = make_shared<Sound>("select.wav",m_pCache);
        add(snd);
        snd->source()->play();
        snd->on_tick.connect([snd](Freq::Time){
            if(not snd->source()->playing())
                snd->detach();
        });
        kit::move_on_copy<unique_ptr<bool>> once(kit::make_unique<bool>(false));
        auto cb = make_shared<std::function<void()>>([once, this]{
            if(**once)
                return;
            m_pContext->state().select();
            **once = true;
        });
        m_pContext->with_enter(cb);
        if(cb.unique())
            (*cb)();
    }
    
    if(m_pController->button("back").pressed_now() ||
       m_pController->input()->key("escape").pressed_now()
    ){
        auto snd = make_shared<Sound>("menuback.wav",m_pCache);
        add(snd);
        snd->source()->play();
        snd->on_tick.connect([snd](Freq::Time){
            if(not snd->source()->playing())
                snd->detach();
        });
        kit::move_on_copy<unique_ptr<bool>> once(kit::make_unique<bool>(false));
        auto cb = make_shared<std::function<void()>>([once, this]{
            if(**once)
                return;
            if(*m_pContext)
                m_pContext->pop();
            if(not *m_pContext)
                m_pContext->on_stack_empty();
            **once = true;
        });
        m_pContext->with_leave(cb);
        if(cb.unique())
            (*cb)();
    }

    // clear
    cairo->save();
    cairo->set_operator(Cairo::OPERATOR_CLEAR);
    cairo->paint();
    cairo->restore();
    
    cairo->set_source_rgba(1.0, 1.0, 1.0, 0.5);
    //cairo->select_font_face("Gentium Book Basic", Cairo::FONT_SLANT_NORMAL,
    cairo->select_font_face(
        //"Slackey",
        m_Font,
        Cairo::FONT_SLANT_NORMAL,
        Cairo::FONT_WEIGHT_NORMAL
    );
    float fade = *m_pFade;
    auto textoffset = vec2(fade);
    const float spacing_increase = 64.0f;
    float spacing = spacing_increase;
    
    std::string text = m_pContext->state().m_Menu->name();
    cairo->set_source_rgba(0.2, 0.2, 0.2, 0.5);
    cairo->set_font_size(60.0f + 4.0f * fade);
    m_pCanvas->text(text, vec2(
        -textoffset.x + m_pCanvas->center().x,
        fade * (
            -textoffset.y + m_pCanvas->center().y/2.0f + spacing
        )
    ), Canvas::CENTER);
    cairo->set_source_rgba(0.5, 0.0, 0.0, 1.0);
    m_pCanvas->text(text, vec2(
        -textoffset.x + m_pCanvas->center().x,
        (1.0f-fade) * m_pCanvas->size().y
            - textoffset.y + m_pCanvas->center().y/2.0f + spacing
    ), Canvas::CENTER);

    if(*m_pContext && m_pContext->state().m_Menu)
    {
        unsigned idx = 0;
        for(auto&& opt: m_pContext->state().m_Menu->options())
        {
            text = opt.m_Text;
            cairo->set_source_rgba(1.0, 1.0, 1.0, 0.25 * fade);
            cairo->set_font_size(44.0f + 4.0f * fade);
            m_pCanvas->text(text, vec2(
                -textoffset.x + m_pCanvas->center().x,
                fade * (spacing + textoffset.y + m_pCanvas->size().y/2.0f)
            ), Canvas::CENTER);
            if(m_pContext->state().m_Highlighted == idx)
                cairo->set_source_rgba(0.4, 1.0, 0.4, 1.0 * fade);
            else
                cairo->set_source_rgba(0.0, 0.5, 0.0, 1.0 * fade);
            m_pCanvas->text(text, vec2(
                -textoffset.x + m_pCanvas->center().x,
                (1.0f-fade) * m_pCanvas->size().y +
                    spacing - textoffset.y + m_pCanvas->size().y/2.0f
            ), Canvas::CENTER);
            
            spacing += spacing_increase;
            ++idx;
        }
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

bool MenuContext :: State :: next_option(int delta)
{
    size_t sz = m_Menu->options().size();
    if(delta > 0)
    {
        if(m_Highlighted < sz - delta){
            m_Highlighted += delta;
            return true;
        }else{
            m_Highlighted = sz - 1;
            return false;
        }
    }
    else if(delta < 0)
    {
        if(m_Highlighted >= -delta){
            m_Highlighted += delta;
            return true;
        }else{
            m_Highlighted = 0;
            return false;
        }
    }
    return false;
}

bool MenuContext :: State :: select()
{
    try{
        kit::safe_ptr(m_Menu)->options().at(
            m_Highlighted
        )();
    }catch(...){
        return false;
    }
    return true;
}
