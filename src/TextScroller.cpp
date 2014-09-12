#include "TextScroller.h"
#include "GLTask.h"
using namespace std;
using namespace glm;

TextScroller :: TextScroller(
    Window* window,
    Controller* ctrl,
    std::string font,
    Cache<Resource, std::string>* resources,
    Mode mode
):
    m_pWindow(window),
    m_pController(ctrl),
    m_Font(font),
    m_Mode(mode)
{
    auto sh = m_pWindow->size().y;
    auto dh = sh / 6.0f;
    m_fActiveY =  0.0f;
    m_fInactiveY = -dh;
    m_Drop.stop(m_fInactiveY);

    vec2 margin = vec2(
        m_pWindow->size().x / 32.0f,
        m_pWindow->size().y / 32.0f
    );
    
    m_pCanvas = make_shared<Canvas>(
        m_pWindow->size().x,
        sh / 6.0f
    );
    add(m_pCanvas);
    m_pTextCanvas = make_shared<Canvas>(
        m_pWindow->size().x + margin.x,
        sh / 6.0f
    );
    add(m_pTextCanvas);
    m_pTextCanvas->context()->move_to(margin.x, margin.y);
    auto layout = m_pTextCanvas->layout();
    layout->set_width((m_pWindow->size().x - margin.x * 2.0f) * Pango::SCALE);
    
    //m_Sounds["select"] = make_shared<Sound>("select.wav", resources);
    //for(auto&& snd: m_Sounds)
    //    add(snd.second);

    //GL_TASK_ASYNC_START()
    on_tick.connect([this](Freq::Time){
        // clear black
        auto cairo = m_pCanvas->context();
        cairo->set_source_rgb(0.0f, 0.0f, 0.0f);
        cairo->paint();
        
        // clear transparent
        auto ctext = m_pTextCanvas->context();
        ctext->save();
        ctext->set_operator(Cairo::OPERATOR_SOURCE);
        //ctext->set_source_rgba(1.0f, 0.0f, 1.0f, 1.0f);
        ctext->set_source_rgba(0.0f, 0.0f, 0.0f, 0.0f);
        ctext->paint();
        ctext->restore();

        if(!m_Messages.empty()) {
            auto layout = m_pTextCanvas->layout();
            layout->set_wrap(Pango::WRAP_WORD);
            layout->set_text(m_Messages.front().msg);
            auto fontdesc = Pango::FontDescription((
                boost::format("%s %s") %
                //boost::format("Special Elite %s") %
                    m_Font %
                    (m_pCanvas->size().y / 6.0f)
            ).str());
            layout->set_font_description(fontdesc);
            ctext->set_source_rgba(1.0, 1.0, 1.0, 0.75);
            layout->show_in_cairo_context(ctext);
        }
    });
    //GL_TASK_ASYNC_END()
}

void TextScroller :: logic_self(Freq::Time t)
{
    m_Drop.logic(t);
    
    if(!m_Messages.empty())
    {
        bool advance = false;
        if(m_Mode ==  WAIT)
            advance = m_pController->button("select").consume();
        else if(m_Mode ==  TIMED)
            advance = m_AutoSkip.elapsed();
            
        if(advance)
        {
            auto msg = m_Messages.front();
            m_Messages.pop();
            try{
                msg.on_end(t);
            }catch(const std::bad_function_call&){}
            //m_Sounds["select"]->source()->play();

            if(m_Messages.empty()){
                clear();
            }else{
                try{
                    m_Messages.front().on_show(t);
                    if(m_Mode == TIMED)
                        m_AutoSkip.set(m_AutoSkipTime);
                }catch(const std::bad_function_call&){}
            }
        }
    }
    position(glm::vec3(0.0f, m_Drop.get(), 0.0f));
}

void TextScroller :: write(
    std::string msg,
    std::function<void(Freq::Time)> show,
    std::function<void(Freq::Time)> end,
    std::function<void(Freq::Time)> tick

){
    m_Messages.emplace(Message{
        std::move(msg),
        std::move(show),
        std::move(end),
        std::move(tick)
    });
    
    if(m_Messages.size() != 1)
        return;
    
    // animate down window
    m_Drop.abort();
    m_Drop.frame(Frame<float>(
        m_fActiveY,
        Freq::Time::seconds(1.0f),
        INTERPOLATE(in_sine<float>)
    ));
    
    //on_tick.connect([this](Freq::Time){
    //    if(m_Drop.elapsed()) {
    //    }
    //});
}

void TextScroller :: clear()
{
    kit::clear(m_Messages);
    m_Drop.abort();
    m_Drop.frame(Frame<float>(
        m_fInactiveY,
        Freq::Time::seconds(1.0f),
        INTERPOLATE(out_sine<float>)
    ));
}

void TextScroller :: render_self(Pass* pass) const
{
}

