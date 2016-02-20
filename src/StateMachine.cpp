#include "StateMachine.h"

void StateMachine :: change(std::string state)
{
    if(not m_Current.empty())
    {
        auto&& st = m_States.at(state);
        if(not st.on_attempt || not st.on_attempt(m_Current))
            m_States.at(m_Current).on_leave();
        else
        {
            m_States.at(m_Current).on_reject(state);
            return; // rejected
        }
    }
    m_Current = state;
    m_States.at(m_Current).on_enter();
}

void StateMachine :: logic(Freq::Time t)
{
    if(not m_Current.empty())
        m_States.at(m_Current).on_tick(t);
}

void StateMachine :: clear()
{
    m_States.clear();
    m_Current = "";
}

