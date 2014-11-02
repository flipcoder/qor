#ifndef MENU_H_PGBCMNRD
#define MENU_H_PGBCMNRD

#include <vector>
#include <stack>
#include "Input.h"
#include "Node.h"
#include "Resource.h"
#include "Graphics.h"
#include "IPartitioner.h"
class Canvas;

class Menu
{
    public:

        Menu() = default;
        Menu(const Menu&) = default;
        Menu(Menu&&) = default;
        Menu& operator=(const Menu&) = default;
        Menu& operator=(Menu&&) = default;
        
        struct Option
        {
            Option(
                std::shared_ptr<std::string> text,
                std::function<void()> cb,
                std::function<bool(int)> adjust = std::function<bool(int)>(),
                std::string desc = "",
                unsigned flags = 0
            ):
                m_pText(kit::safe_ptr(text)),
                m_Callback(cb),
                m_AdjustCallback(adjust),
                m_Description(desc),
                m_Flags(flags)
            {}
            Option(
                std::string text,
                std::function<void()> cb,
                std::function<bool(int)> adjust = std::function<bool(int)>(),
                std::string desc = "",
                unsigned flags = 0
            ):
                m_pText(std::make_shared<std::string>(text)),
                m_Callback(cb),
                m_AdjustCallback(adjust),
                m_Description(desc),
                m_Flags(flags)
            {}


            Option(const Option&) = default;
            Option(Option&&) = default;
            Option& operator=(const Option&) = default;
            Option& operator=(Option&&) = default;

            enum Flags {
                BACK = kit::bit(0)
            };
            
            std::shared_ptr<std::string> m_pText;
            std::function<void()> m_Callback;
            std::function<bool(int)> m_AdjustCallback;
            std::string m_Description;
            unsigned m_Flags = 0;

            void operator()();
            bool operator()(int ofs);
        };
        
        std::vector<Option>& options() {
            return m_Options;
        }
        const std::vector<Option>& options() const {
            return m_Options;
        }

        void name(std::string n) {
            m_Name = n;
        }
        std::string name() const {
            return m_Name;
        }
        
    private:

        std::string m_Name;
        std::vector<Option> m_Options;
};

class MenuContext
{
    public:
        
        MenuContext() = default;
        //MenuContext(Controller* c) {}
        
        MenuContext(const MenuContext&) = default;
        MenuContext(MenuContext&&) = default;
        MenuContext& operator=(const MenuContext&) = default;
        MenuContext& operator=(MenuContext&&) = default;

        struct State
        {
            State(){}
            explicit State(Menu* m):
                m_Menu(m)
            {}
            unsigned m_Highlighted = 0;
            Menu* m_Menu = nullptr;
            bool next_option(int delta);
            bool select();
            bool adjust(int ofs);
        };
        
        operator bool() const {
            return !m_States.empty();
        }
        MenuContext::State& state() {
            return m_States.top();
        }
        
        void clear(Menu* menu) {
            m_States.push(State());
            m_States.top().m_Menu = menu;
        }

        void pop() {
            m_States.pop();
        }
        void push(Menu* m) {
            m_States.emplace(State(m));
        }
        void push(MenuContext::State s) {
            m_States.push(std::move(s));
        }

        boost::signals2::signal<void()> on_stack_empty;
        boost::signals2::signal<void(std::shared_ptr<std::function<void()>>)> with_enter;
        boost::signals2::signal<void(std::shared_ptr<std::function<void()>>)> with_leave;
        
        size_t stack_size() const {
            return m_States.size();
        }
        
        
    private:
        
        // select highlighted menu item, ignoring all callbacks and errors
        //bool select();
        //bool adjust(int ofs);
        
        std::stack<MenuContext::State> m_States;
};

class MenuGUI:
    public Node
{
    public:
        
        MenuGUI(
            Controller* c,
            MenuContext* ctx,
            Menu* menu,
            IPartitioner* partitioner,
            Canvas* canvas,
            Cache<Resource, std::string>* cache,
            std::string m_Font,
            float* fade
        );
        ~MenuGUI() {}
        
        MenuGUI(const MenuGUI&) = default;
        MenuGUI(MenuGUI&&) = default;
        MenuGUI& operator=(const MenuGUI&) = default;
        MenuGUI& operator=(MenuGUI&&) = default;

        virtual void logic_self(Freq::Time t) override;

        void refresh();
        void pause(bool b=true) {m_bPause=b;}
        bool paused() {return m_bPause;}

    private:

        void interface_logic(Freq::Time);
        
        Controller* m_pController;
        MenuContext* m_pContext;
        Menu* m_pMenu;
        IPartitioner* m_pPartitioner;
        Canvas* m_pCanvas;
        Cache<Resource, std::string>* m_pCache;
        float* m_pFade;
        std::string m_Font;
        bool m_bPause = false;

        Color m_TitleColor = Color(1.0f, 1.0f, 1.0f);
        Color m_OptionColor = Color(0.4f, 0.4f, 0.4f);
        Color m_HighlightColor = Color(1.0f, 1.0f, 1.0f);
        
        unsigned m_WaitCount = 0;
};

#endif

