#ifndef MENU_H_PGBCMNRD
#define MENU_H_PGBCMNRD

#include <vector>
#include <stack>
#include "Input.h"
#include "Node.h"
#include "Resource.h"
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
                std::string text,
                std::function<void()> cb,
                std::string desc = ""
            ):
                m_Text(text),
                m_Callback(cb),
                m_Description(desc)
            {}

            Option(const Option&) = default;
            Option(Option&&) = default;
            Option& operator=(const Option&) = default;
            Option& operator=(Option&&) = default;
            
            std::string m_Text;
            std::function<void()> m_Callback;
            std::string m_Description;

            void operator()();
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
            unsigned m_Highlighted = 0;
            Menu* m_Menu = nullptr;
            bool next_option(int delta);
            bool select();
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
        bool select();
        
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
        
    private:
        
        Controller* m_pController;
        MenuContext* m_pContext;
        Menu* m_pMenu;
        IPartitioner* m_pPartitioner;
        Canvas* m_pCanvas;
        Cache<Resource, std::string>* m_pCache; 
        float* m_pFade;
        std::string m_Font;
};

#endif

