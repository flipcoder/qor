#include "Menu.h"
#include "Canvas.h"
#include "Sound.h"
#include "Node.h"
#include "kit/kit.h"
using namespace std;
using namespace glm;

void Menu :: Option :: operator()()
{
    if(m_Callback)
        m_Callback();
}

bool Menu :: Option :: operator()(int ofs)
{
    if(m_AdjustCallback)
        return m_AdjustCallback(ofs);
    return false;
}

MenuGUI :: MenuGUI(
    Controller* c,
    MenuContext* ctx,
    Menu* menu,
    IPartitioner* partitioner,
    Canvas* canvas,
    Cache<Resource, std::string>* cache,
    std::string font,
    float font_size,
    float* fade,
    int max_options_per_screen,
    float spacing,
    Canvas::Align align,
    float x,
    unsigned flags
):
    m_pController(c),
    m_pContext(ctx),
    m_pMenu(menu),
    m_pPartitioner(partitioner),
    m_pCanvas(canvas),
    m_pCache(cache),
    m_Font(font),
    m_FontSize(font_size),
    m_pFade(fade),
    m_MaxOptionsPerScreen(max_options_per_screen),
    m_Spacing(spacing),
    m_Align(align),
    m_X(x),
    m_Flags(flags)
{
}

void MenuGUI :: interface_logic(Freq::Time t)
{
    // if waiting on callback, pause interface logic
    if(m_WaitCount || m_bPause)
        return;
    
    if(m_pController->button("up").pressed_now() ||
       m_pController->input()->key("up").pressed_now()
    ){
        if(m_pContext->state().next_option(-1)){
            auto snd = make_shared<Sound>("highlight.wav",m_pCache);
            add(snd);
            snd->play();
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
            snd->play();
            snd->on_tick.connect([snd](Freq::Time){
                if(not snd->source()->playing())
                    snd->detach();
            });
        }
    }
    if(m_pController->button("left").pressed_now() ||
       m_pController->input()->key("left").pressed_now()
    ){
        if(m_pContext->state().adjust(-1)){
            auto snd = make_shared<Sound>("highlight.wav",m_pCache);
            add(snd);
            snd->play();
            snd->on_tick.connect([snd](Freq::Time){
                if(not snd->source()->playing())
                    snd->detach();
            });
        }
    }
    else if(m_pController->button("right").pressed_now() ||
       m_pController->input()->key("right").pressed_now()
    ){
        if(m_pContext->state().adjust(1)){
            auto snd = make_shared<Sound>("highlight.wav",m_pCache);
            add(snd);
            snd->play();
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
        shared_ptr<Sound> snd;
        if(m_pContext->on_back())
            snd = make_shared<Sound>("menuback.wav",m_pCache);
        else
            snd = make_shared<Sound>("menuselect.wav",m_pCache);
        add(snd);
        snd->play();
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
            --m_WaitCount;
        });
        ++m_WaitCount;
        m_pContext->with_enter(cb);
        if(cb.unique())
            (*cb)();
    }
    else if(m_pController->button("back").pressed_now() ||
       m_pController->input()->key("escape").pressed_now()
    ){
        auto snd = make_shared<Sound>("menuback.wav",m_pCache);
        add(snd);
        snd->play();
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
            --m_WaitCount;
        });
        ++m_WaitCount;
        m_pContext->with_leave(cb);
        if(cb.unique())
            (*cb)();
    }
}

void MenuGUI :: logic_self(Freq::Time t)
{
    if(not m_pContext || m_pContext->empty() || not m_pContext->state().m_Menu)
        return;
    
    interface_logic(t);
    
    auto cairo = m_pCanvas->context();
    
    // clear
    cairo->save();
    cairo->set_operator(Cairo::OPERATOR_CLEAR);
    cairo->paint();
    cairo->restore();

    if(not visible())
        return;
    
    cairo->set_source_rgba(1.0, 1.0, 1.0, 0.5);
    cairo->select_font_face(
        m_Font,
        Cairo::FONT_SLANT_NORMAL,
        Cairo::FONT_WEIGHT_NORMAL
    );
    float fade = *m_pFade;
    auto textoffset = vec2(fade);
    const float spacing_increase = m_FontSize * m_Spacing;
    float spacing = spacing_increase;

    if(not m_pContext || m_pContext->empty() || not m_pContext->state().m_Menu)
        return;
    
    std::string text = m_pContext->state().m_Menu->name();

    float x = m_X;
    if(x < -K_EPSILON)
        x = m_pCanvas->center().x;

    if(not text.empty())
    {
        //cairo->set_source_rgba(0.2, 0.2, 0.2, 0.5);
        cairo->set_font_size(m_FontSize + 4.0f * fade);
        m_pCanvas->text(text, Color(0.2f, 0.5f), vec2(
            -textoffset.x + x,
            fade * (
                -textoffset.y + m_pCanvas->center().y/2.0f + spacing
            )
        ), m_Align);
        //cairo->set_source_rgba(
        //    m_TitleColor.r(),
        //    m_TitleColor.g(),
        //    m_TitleColor.b(),
        //1.0);
        m_pCanvas->text(text, m_TitleColor, vec2(
            -textoffset.x + x,
            (1.0f-fade) * m_pCanvas->size().y
                - textoffset.y + m_pCanvas->center().y/2.0f + spacing
        ), m_Align);
    }

    //unsigned idx = m_pContext->state().m_Highlighted;
    //unsigned endpoint = m_MaxOptionsPerScreen ?
    //    std::min<unsigned>(
    //        m_pContext->state().m_Menu->options().size(),
    //        m_Offset + m_MaxOptionsPerScreen
    //    ) : m_pContext->state().m_Menu->options().size();
    m_Offset = std::max<int>(
        0,
        m_pContext->state().m_Highlighted - m_MaxOptionsPerScreen / 2
    );
    int endpoint = std::max<int>(
        m_pContext->state().m_Highlighted + (m_MaxOptionsPerScreen-1) / 2,
        m_MaxOptionsPerScreen - 1
    );
    for(int idx = m_Offset; idx <= endpoint; ++idx)
    {
    //for(auto&& opt: m_pContext->state().m_Menu->options())
    //{
        if(idx < 0)
            continue;
        if(idx >= m_pContext->state().m_Menu->options().size())
            break;
        auto&& opt = m_pContext->state().m_Menu->options().at(idx);
        
        if(m_Flags & F_BOX){
            m_pCanvas->color(Color(m_OptionColor * (idx%2?0.25f:0.5f), 0.5f));
            m_pCanvas->rectangle(x - 8.0f,
                spacing - spacing_increase + m_pCanvas->size().y/2.0f + 8.0f,
                x + 256.0f, spacing_increase);
            m_pCanvas->context()->fill();
        }
        
        text = *opt.m_pText;
        //cairo->set_source_rgba(1.0, 1.0, 1.0, 0.25 * fade);
        cairo->set_font_size(m_FontSize + 4.0f * fade);
        m_pCanvas->text(text, Color(1.0f, 0.25f * fade), vec2(
            -textoffset.x + x,
            fade * (spacing + textoffset.y + m_pCanvas->size().y/2.0f)
        ), m_Align);
        Color c;
        if(m_pContext->state().m_Highlighted == idx)
            c = Color(m_HighlightColor, 1.0f * fade);
        else
            c = Color(m_OptionColor, 1.0f * fade);
        m_pCanvas->text(text, c, vec2(
            -textoffset.x + x,
            (1.0f-fade) * m_pCanvas->size().y +
                spacing - textoffset.y + m_pCanvas->size().y/2.0f
        ), m_Align);
        
        spacing += spacing_increase;
        //++idx;
    }
    m_pCanvas->refresh();
}

void MenuGUI :: refresh()
{
    children().clear();
    if(!m_pMenu)
        return;
    
    //unsigned idx = 0;
    //for(auto& op: m_pContext->state().m_Menu->options())
    //{
    //    auto c = make_shared<Canvas>(512, idx * 32);
    //    add(c);
    //    ++idx;
    //}
}

void MenuGUI :: consume()
{
    m_pController->button("back").consume();
    m_pController->input()->key("escape").consume();
    m_pController->button("select").consume();
    m_pController->input()->key("return").consume();
    m_pController->input()->key("space").consume();
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

bool MenuContext :: State :: adjust(int ofs)
{
    try{
        kit::safe_ptr(m_Menu)->options().at(
            m_Highlighted
        )(ofs);
    }catch(...){
        return false;
    }
    return true;
}

