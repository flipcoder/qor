#ifndef _PROFILE_H_4UYYSMKF
#define _PROFILE_H_4UYYSMKF

#include <string>
#include <unordered_map>
#include <json/json.h>
#include <tuple>
#include "Input.h"
#include "kit/meta/meta.h"

class Session;

class Profile
{
    public:
        Profile(
            unsigned int id,
            Session* session,
            Input* input,
            const std::string& fn
        );
        
        Profile(Session* session, std::string name);
        
        virtual ~Profile() {}
        
        std::shared_ptr<Controller> controller() { return m_pController; }

        const Json::Value& json() const { return m_Json; }
        Json::Value& json() { return m_Json; }

        std::shared_ptr<Meta>& config() {
            return m_pConfig;
        }
        std::shared_ptr<Meta>& temp() {
            return m_pTemp;
        }

        std::map<std::string, std::vector<std::string>> binds();
        void binds(const std::map<std::string, std::vector<std::string>>& b);
        void reload();

        void sync();

        std::string name() const { return m_Name; }
        
        bool dummy() const { return m_bDummy; }
        Session* session() { return m_pSession; }
       
        void clear_temp();

        bool active() const { return m_bActive; }
        void active(bool b) { m_bActive=b; }
        
    private:

        bool m_bActive = false;

        std::string m_Name;
        int m_ID = -1;
        Session* m_pSession = nullptr;
        std::vector<std::tuple<std::string,std::string>> m_Binds;
        std::shared_ptr<Controller> m_pController;

        Json::Value m_Json;
        std::shared_ptr<Meta> m_pConfig;
        std::shared_ptr<Meta> m_pTemp;
        Input* m_pInput = nullptr;
        std::string m_Filename;
        bool m_bDummy = false;
};

#endif

