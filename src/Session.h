#ifndef _SESSION_H_F33JXBK6
#define _SESSION_H_F33JXBK6

#include <string>
#include <memory>
#include "State.h"
#include "Profile.h"

/*
 * Session persists throughout engine states.
 */
class Session:
    public IRealtime
{
    public:
        struct IState:
            public IRealtime
        {
            virtual ~IState(){}
            virtual void logic(Freq::Time t) override {}
        };
    
        Session(Input* input);
        virtual ~Session() {}

        void unplug(unsigned id) {
            m_Profiles.erase(id);
            //m_NextUnused = std::min(id, m_NextUnused);
            //find_unused();
        }

        template<class... Args>
        unsigned plug(Args&&... args)
        {
            auto id = m_Profiles.reserve();
            // TODO: if something throws here, id will be lost until
            // optimize() call on profiles index
            return m_Profiles.emplace_hint(
                id, // first id is hint
                id, // second id forwards into class params
                this,
                m_pInput,
                std::forward<Args>(args)...
            );
            //unsigned id = m_NextUnused;
            //m_Profiles[m_NextUnused] = std::make_shared<Profile>(
            //    m_NextUnused,
            //    this,
            //    m_pInput,
            //    std::forward<Args>(args)...
            //);
            //return find_unused();
        }

        //unsigned find_unused() {
        //    while(true){
        //        if(m_Profiles.find(m_NextUnused++) == m_Profiles.end())
        //            return m_NextUnused;
        //    }
        //    assert(false);
        //}

        std::shared_ptr<Profile> profile(unsigned idx = 0) {
            try{
                return m_Profiles.at(idx);
            }catch(const std::out_of_range&){
                return std::shared_ptr<Profile>();
            }
        }

        std::vector<std::string, std::vector<std::string>> binds();

        typedef typename kit::shared_index<Profile>::const_iterator
            const_iterator;
        typedef typename kit::shared_index<Profile>::iterator iterator;
        iterator begin() { return m_Profiles.begin(); }
        iterator end() { return m_Profiles.end(); }
        const_iterator begin() const { return m_Profiles.begin(); }
        const_iterator end() const { return m_Profiles.end(); }
        const_iterator cbegin() const { return m_Profiles.begin(); }
        const_iterator cend() const { return m_Profiles.end(); }

        /*
         * List the loadable profiles detect in default profiles dir
         */
        std::vector<std::string> saved_profiles() const;

        std::shared_ptr<Meta> meta() { return m_pMeta; }
        std::shared_ptr<const Meta> meta() const { return m_pMeta; }

        IState* state(std::string name) { return m_States[name].get(); }
        void state(std::string name, std::shared_ptr<IState> state) {
            m_States[name] = state;
        }

    private:
        
        //unsigned m_NextUnused = 0;

        /*
         * Player ID -> Profile map
         */
        //std::map<unsigned, std::shared_ptr<Profile>> m_Profiles;
        kit::shared_index<Profile> m_Profiles;
        Input* m_pInput;

        std::shared_ptr<Meta> m_pMeta = std::make_shared<Meta>();

        std::map<std::string, std::shared_ptr<IState>> m_States;
};

#endif

