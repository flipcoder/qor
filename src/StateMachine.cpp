#include "StateMachine.h"
using namespace std;

void StateMachine :: state(std::string slot, std::string state)
{
    auto itr = m_Slots.find(slot);
    if(itr == m_Slots.end())
        m_Slots[slot] = StateMachineSlot();
    auto&& sl = m_Slots[slot];
    if(not sl.current.empty())
    {
        auto&& st = sl.states[state];
        if(not st.on_attempt || not st.on_attempt(sl.current))
        {
            try{
                sl.states.at(sl.current).on_leave();
            }catch(const std::out_of_range&){}
        }
        else
        {
            try{
                sl.states.at(sl.current).on_reject(state);
            }catch(const std::out_of_range&){}
            return; // rejected
        }
    }
    sl.current = state;
    sl.states[sl.current].on_enter();
}
void StateMachine :: operator()(std::string slot, std::string state) {
    StateMachine::state(slot,state);
}

void StateMachine :: logic(Freq::Time t)
{
    for(auto&& slot: m_Slots)
        if(not slot.second.current.empty())
            slot.second.states.at(slot.second.current).on_tick(t);
}

void StateMachine :: clear()
{
    m_Slots.clear();
}

void StateMachine :: clear(std::string slot)
{
    try{
        auto&& sl = m_Slots.at(slot);
        sl.states.clear();
        sl.current = "";
    }catch(const std::out_of_range&){
    }
}

std::string StateMachine :: state(std::string slot) const
{
    try{
        return m_Slots.at(slot).current;
    }catch(const std::out_of_range&){
        return std::string();
    }
}
std::string StateMachine :: operator()(std::string slot) const {
    return state(slot);
}

