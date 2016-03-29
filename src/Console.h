#ifndef CONSOLE_H
#define CONSOLE_H

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
            Cache<Resource,std::string>* cache
        );
        virtual ~Console() {}

        //Console(const Console&) = default;
        //Console(Console&&) = default;
        //Console& operator=(const Console&) = default;
        //Console& operator=(Console&&) = default;

        virtual void logic_self(Freq::Time) override;

        void write(std::string msg);

        bool input() const {
            return m_bInput;
        }
        
        virtual std::string type() const override { return "console"; }
        
    private:
        
        void redraw();

        boost::circular_buffer<std::string> m_Messages;

        Window* m_pWindow = nullptr;
        Input* m_pInput = nullptr;
        std::shared_ptr<Canvas> m_pCanvas;
        std::shared_ptr<Canvas> m_pTextCanvas;
        Cache<Resource, std::string>* m_pCache;
        Pango::FontDescription m_FontDesc;

        bool m_bInput = false;
        bool m_bDirty = true;

        std::shared_ptr<std::string> m_pInputString;

        Interpreter* m_pInterpreter;
        std::shared_ptr<Interpreter::Context> m_pScript;
};

#endif

