#include "kit/log/log.h"
#include "Console.h"
using namespace std;

Console :: Console(Interpreter* interp, Window* window, Input* input, Cache<Resource,std::string>* cache):
    m_pWindow(window),
    m_pInput(input),
    m_pCache(cache),
    m_pInterpreter(interp),
    m_pScript(make_shared<Interpreter::Context>(interp))
{
    auto sw = m_pWindow->size().x;
    auto sh = m_pWindow->size().y;

    m_pCanvas = make_shared<Canvas>(
        m_pWindow->size().x,
        sh
    );
    add(m_pCanvas);
    m_pTextCanvas = make_shared<Canvas>(
        m_pWindow->size().x,
        sh
        //sh / 12.0f
    );
    add(m_pTextCanvas);
    m_Messages = boost::circular_buffer<string>(5);
    m_FontDesc = Pango::FontDescription("Fixed 12");
    m_pTextCanvas->layout()->set_font_description(m_FontDesc);
    m_pInputString = make_shared<string>();
    m_pScript->execute_string("import qor");
}

void Console :: redraw()
{
    // clear black
    auto cairo = m_pCanvas->context();
    cairo->set_source_rgb(0.0f, 0.0f, 0.0f);
    cairo->paint();
    
    // clear transparent
    auto ctext = m_pTextCanvas->context();
    ctext->save();
    ctext->set_operator(Cairo::OPERATOR_SOURCE);
    ctext->set_source_rgba(1.0f, 0.0f, 1.0f, 0.0f);
    //ctext->set_source_rgba(0.0f, 0.0f, 0.0f, 1.0f);
    ctext->paint();
    ctext->restore();
    
    auto layout = m_pTextCanvas->layout();
    layout->set_wrap(Pango::WRAP_WORD);
    
    std::vector<std::string> msgs;
    msgs.reserve(m_Messages.size());
    for(string msg: m_Messages)
        msgs.push_back(msg);

    if(m_bInput)
        msgs.push_back("> " + *m_pInputString);
    
    layout->set_text(boost::join(msgs, "\n"));
    ctext->set_source_rgba(1.0, 1.0, 1.0, 0.75);
    layout->show_in_cairo_context(ctext);
    
    m_pCanvas->dirty(false);
    m_pTextCanvas->dirty(true);
}

void Console :: logic_self(Freq::Time)
{
    if(m_pInput->key(SDLK_BACKQUOTE).pressed_now()) {
        m_bInput = true;
        m_pInput->listen(Input::LISTEN_TEXT, m_pInputString, [&](bool done){
            if(done) {
                m_pScript->execute_string(*m_pInputString);
                write(*m_pInputString);
                *m_pInputString = "";
                m_bInput = false;
            }
            m_bDirty = true;
        });
        m_bDirty = true;
    }
    
    if(m_bDirty) {
        redraw();
        m_bDirty = false;
    }
}

void Console :: write(std::string msg)
{
    LOG(msg);
    m_Messages.push_back(msg);
    m_bDirty = true;
}

