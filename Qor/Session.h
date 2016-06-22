#ifndef _SESSION_H_F33JXBK6
#define _SESSION_H_F33JXBK6

#include <string>
#include <memory>
#include "State.h"
#include "Profile.h"

/*
 * Session persists throughout engine modules.
 */
class Session:
    public IRealtime
{
    public:
        struct IModule:
            public IRealtime,
            public std::enable_shared_from_this<IModule>
        {
            virtual ~IModule(){}
            virtual void logic(Freq::Time t) override {}
        };
    
        Session(Input* input);
        virtual ~Session() {}

        void unplug(unsigned id) {
            m_Profiles.erase(id);
            //m_NextUnused = std::min(id, m_NextUnused);
            //find_unused();
        }
        void unplug(Profile* prof);

        template<class... Args>
        unsigned plug(Args&&... args)
        {
            auto id = m_Profiles.reserve();
            // TODO: if something throws here, id will be lost until
            // optimize() call on profiles index
            unsigned p = m_Profiles.emplace_hint(
                id, // first id is hint
                id, // second id forwards into class params
                this,
                m_pInput,
                std::forward<Args>(args)...
            );
            m_Profiles.at(p)->active(true);
            return p;
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

        std::shared_ptr<Profile> default_profile() {
            for(auto&& p: m_Profiles)
                if(p.second->name() == "default")
                    return p.second;
            return nullptr;
        }
        
        std::shared_ptr<Profile> profile(unsigned idx = 0) {
            try{
                return m_Profiles.at(idx);
            }catch(const std::out_of_range&){
                return std::shared_ptr<Profile>();
            }
        }

        std::shared_ptr<Profile> active_profile(unsigned idx = 0) {
            int i = 0;
            for(auto&& prof: m_Profiles)
            {
                if(prof.second->active() && i++ == idx)
                    return prof.second;
            }
            return nullptr;
        }

        std::shared_ptr<Profile> dummy_profile(std::string name);
        void clear_dummy_profiles();

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

        template<class T>
        T* module(std::string name) { return (T*)m_Modules[name].get(); }
        
        void module(std::string name, std::shared_ptr<IModule> module) {
            m_Modules[name] = module;
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

        std::map<std::string, std::shared_ptr<IModule>> m_Modules;
};

#endif

