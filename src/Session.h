#ifndef _SESSION_H_F33JXBK6
#define _SESSION_H_F33JXBK6

#include <string>
#include <memory>
#include "State.h"
#include "Profile.h"

/*
 * A game session.  Keeps track of important things inbetween games.
 */
class Session
{
    public:
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

        /*
         * List the loadable profiles detect in default profiles dir
         */
        std::vector<std::string> saved_profiles() const;

    private:
        
        //unsigned m_NextUnused = 0;

        /*
         * Player ID -> Profile map
         */
        //std::map<unsigned, std::shared_ptr<Profile>> m_Profiles;
        kit::shared_index<Profile> m_Profiles;
        Input* m_pInput;
};

#endif

