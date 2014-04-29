#ifndef _STATE_MANAGER_H
#define _STATE_MANAGER_H

#include <memory>
#include <vector>
#include <cassert>

// TODO: convert this to smart pointers and use factory resolver
//       instead of new_state overloading

// TODO: add state transitions

template<class tState>
class StateManager
{
    public:
        enum eOperation {
            OP_NONE = 0,
            OP_PUSH,
            OP_POP,
            OP_SWAP,
            OP_CLEAR,
        };
        //enum eScheduledTime {
        //    ST_POLL = 0,
        //    ST_NOW,
        //};
        //enum eUrgency {
        //    U_NORMAL = 0,
        //    U_FORCE
        //};

        StateManager() {}
        virtual ~StateManager() { clear_states_now(); }
        void clear_states_now(){
            m_bFast = true; // no timed transitions
            stack_clear();
        }

        void pop_state() {
            m_Operation = OP_POP;
        }
        void push_state(unsigned id) {
            m_Operation = OP_PUSH;
            m_OperandID = id;
        }
        void change_state(unsigned id) {
            m_Operation = OP_SWAP;
            m_OperandID = id;
        }
        //void restart_state() {
        //  // TODO: ...
        //}
        void clear_states() {
            m_Operation = OP_CLEAR;
        }
        void clear_to_state(unsigned id) {
            m_Operation = OP_CLEAR;
            m_OperandID = id;
        }

        std::shared_ptr<tState> state(unsigned depth=0) {
            return m_States.at(m_States.size()-1-depth);
        }
        const std::shared_ptr<tState> state(unsigned depth=0) const {
            return m_States.at(m_States.size()-1-depth);
        }
        //std::shared_ptr<State> current_state() {
        //    return stack_top();
        //}
        //const std::shared_ptr<State> current_state() const {
        //    return stack_top();
        //}

        bool schedule(eOperation op, unsigned id = 0U) {
            if(pending())
                return false; // already scheduled
            m_Operation = op;
            m_OperandID = id;
            return true;
        }
        
        // returns false if stack is empty
        bool poll_state() {
            //m_bNewState = false;
            m_bEnteringState = false;

            if(m_Operation == OP_NONE)
                return !stack_empty();
            else if(m_Operation == OP_POP) {
                stack_pop();
                if(!stack_empty())
                    m_bEnteringState = true;
            }
            else if(m_Operation == OP_PUSH)
            {
                std::shared_ptr<tState> s = new_state(m_OperandID);
                if(s)
                    m_States.push_back(s);
                //m_bNewState = true;
                m_bEnteringState = true;
            }
            else if(m_Operation == OP_SWAP)
            {
                stack_pop();
                std::shared_ptr<tState> s = new_state(m_OperandID);
                if(s)
                    m_States.push_back(s);
                //m_bNewState = true;
                m_bEnteringState = true;
            }
            else if(m_Operation == OP_CLEAR)
            {
                stack_clear();
                if(m_OperandID != unsigned())
                {
                    std::shared_ptr<tState> s = new_state(m_OperandID);
                    if(s) {
                        m_States.push_back(s);
                        //m_bNewState = true;
                        m_bEnteringState = true;
                    }
                }
            }
            else
               assert(false); // invalid operation

            m_Operation = OP_NONE;

            // check stack again
            return !stack_empty();
        }

        bool pending() const {
            return m_Operation != OP_NONE;
        }

        // factory overload for state creation
        // note: may be NULL
        virtual std::shared_ptr<tState> new_state(unsigned id)=0; //{
        //    return m_StateFactory.create(id);
        //}

        // optional overload
        //virtual void free_state(const std::shared_ptr<tState> s) {
        //    delete s;
        //}

        virtual void configure(std::shared_ptr<tState> s) {}
        
        //tFactory& states() {
        //    return m_StateFactory;
        //}
        
        //bool is_new_state() const {
        //    return m_bNewState;
        //}
        bool is_entering_state() const {
            return m_bEnteringState;
        }
        //bool leaving_state() const {
        //    return m_bLeavingState;
        //}

    private:
        //bool m_bNewState = false;
        bool m_bEnteringState = false;

        //Factory<tState, unsigned, Qor*> m_StateFactory;
        //tFactory m_StateFactory;
        
        std::vector<std::shared_ptr<tState>> m_States;
        std::shared_ptr<tState> m_pPreserved;

        eOperation m_Operation=OP_NONE;
        unsigned m_OperandID=0;

        bool m_bFast = false;

        // stack wrapper methods
        void stack_push(const std::shared_ptr<tState>& s) {
            assert(s);
            m_States.push_back(s);
        }
        void stack_pop() {
            if(m_States.empty())
                return;
            //free_state(m_States.top());
            m_States.pop_back();
        }
        std::shared_ptr<tState> stack_preserve() {
            if(m_States.empty())
                return nullptr;
            auto s = m_States.back();
            m_States.pop_back();
            return s;
        }
        std::shared_ptr<tState> stack_top() {
            if(!stack_empty())
                return m_States.back();
            else
                return nullptr;
        }
        const std::shared_ptr<tState> stack_top() const {
            if(!stack_empty())
                return m_States.back();
            else
                return nullptr;
        }
        bool stack_empty() const {
            return m_States.empty();
        }
        void stack_clear() {
            while(!stack_empty())
                stack_pop();
        }
        
        //virtual bool isNullID(unsigned tid) const {
        //    return tid == unsigned();
        //}
        //virtual unsigned nullID() { return 0; }
};

#endif

