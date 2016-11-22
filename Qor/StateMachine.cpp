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
            //try{
                sl.states[sl.current].on_leave();
            //}catch(const std::out_of_range&){}
        }
        else
        {
            //try{
                sl.states[sl.current].on_reject(state);
            //}catch(const std::out_of_range&){}
            return; // rejected
        }
    }
    sl.current = state;
    sl.states[sl.current].on_enter();
}
void StateMachine :: state(std::string slot, bool state) {
    StateMachine::state(slot, string(state ? "1" : "0"));
}
void StateMachine :: operator()(std::string slot, bool state) {
    StateMachine::state(slot, string(state ? "1": "0"));
}

void StateMachine :: state(std::string slot, const char* state) {
    StateMachine::state(slot, string(state));
}
void StateMachine :: operator()(std::string slot, const char* state) {
    StateMachine::state(slot, string(state));
}

void StateMachine :: state(std::string slot, int state) {
    StateMachine::state(slot,to_string(state));
}
void StateMachine :: operator()(std::string slot, int state) {
    StateMachine::state(slot,to_string(state));
}
void StateMachine :: operator()(std::string slot, std::string state) {
    StateMachine::state(slot,state);
}
bool StateMachine :: is_state(std::string slot) const {
    //try{
        return m_Slots[slot].current != string("0");
    //}catch(const std::out_of_range&){}
    return false;
}

void StateMachine :: logic(Freq::Time t)
{
    for(auto&& slot: m_Slots)
        if(not slot.second.current.empty())
            slot.second.states[slot.second.current].on_tick(t);
}

void StateMachine :: lazy_logic(Freq::Time t)
{
    for(auto&& slot: m_Slots)
        if(not slot.second.current.empty())
            slot.second.states[slot.second.current].on_lazy_tick(t);
}


void StateMachine :: clear()
{
    m_Slots.clear();
}

void StateMachine :: clear(std::string slot)
{
    //try{
        auto&& sl = m_Slots[slot];
        sl.states.clear();
        sl.current = "";
    //}catch(const std::out_of_range&){
    //}
}

std::string StateMachine :: state(std::string slot) const
{
    //try{
        return m_Slots[slot].current;
    //}catch(const std::out_of_range&){
    //    return std::string();
    //}
}
std::string StateMachine :: operator()(std::string slot) const {
    return state(slot);
}

