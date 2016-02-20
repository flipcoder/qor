#ifndef CONSOLE_H
#define CONSOLE_H

#include "Window.h"
#include "Canvas.h"
#include "Input.h"

class Console:
    public Node
{
    public:

        Console(
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
        
    private:

        Window* m_pWindow = nullptr;
        Input* m_pInput = nullptr;
        std::shared_ptr<Canvas> m_pCanvas;
        std::shared_ptr<Canvas> m_pTextCanvas;
        Cache<Resource, std::string>* m_pCache;
};

#endif

