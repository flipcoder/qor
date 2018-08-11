#ifndef CONSOLE_H
#define CONSOLE_H

#ifndef QOR_NO_CAIRO

#include <boost/circular_buffer.hpp>
#include "Window.h"
#include "Canvas.h"
#include "Input.h"
#include "Interpreter.h"

class Console:
    public Node
{
    public:

        Console(
            Interpreter* interp, 
            Window* window,
            Input* input,
            ResourceCache* cache,
            int lines = 5
        );
        virtual ~Console() {}

        //Console(const Console&) = default;
        //Console(Console&&) = default;
        //Console& operator=(const Console&) = default;
        //Console& operator=(Console&&) = default;

        virtual void logic_self(Freq::Time) override;

        void write(std::string msg);
        void listen(std::string cmd_text = "");

        bool input() const {
            return m_bInput;
        }
        
        virtual std::string type() const override { return "console"; }

        struct any_combiner
        {
            typedef bool result_type;
            template<class Itr>
            bool operator()(Itr first, Itr last)
            {
                if(first == last)
                    return false;
                while(first != last){
                    if(*first)
                        return true;
                    ++first;
                }
                return false;
            }
        };
        
        boost::signals2::signal<bool(std::string), any_combiner> on_command;
        
    private:
        
        void redraw();

        boost::circular_buffer<std::string> m_Messages;

        Window* m_pWindow = nullptr;
        Input* m_pInput = nullptr;
        //std::shared_ptr<Canvas> m_pCanvas;
        std::shared_ptr<Canvas> m_pTextCanvas;
        ResourceCache* m_pCache;
        Pango::FontDescription m_FontDesc;

        bool m_bInput = false;
        bool m_bDirty = true;

        std::shared_ptr<std::string> m_pInputString;

        Interpreter* m_pInterpreter;
        std::shared_ptr<Interpreter::Context> m_pScript;

        boost::signals2::scoped_connection m_LogConnection;
};

#endif
#endif

