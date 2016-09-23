#include "Session.h"
#include "kit/log/log.h"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "kit/fs/fs.h"
using namespace std;
using namespace boost;
using namespace boost::filesystem;

Session :: Session(std::string appname, Input* input):
    m_App(appname),
    m_pInput(input)
{
    assert(m_pInput);

    // auto-intialize profile if there's only one
    auto profiles = saved_profiles();
    //if(profiles.size()==1)
    plug(profiles.at(0));
}

std::vector<std::string> Session :: saved_profiles() const
{
    std::vector<std::string> profiles;
    vector<path> profile_dirs {
        path("profiles"),
        path(fs::configdir(m_App)) / "profiles"
    };

    for(auto jtr = profile_dirs.begin();
        jtr != profile_dirs.end();
        ++jtr)
    {
        try{
            for(directory_iterator itr(*jtr), e;
                itr != e;
                ++itr)
            {
                auto profile = itr->path();
                //LOG(profile.string());
                if(is_regular_file(profile) &&
                    ends_with(to_lower_copy(profile.string()), ".json"))
                {
                    profiles.push_back(profile.string());
                }
            } 
        }catch(...){
        }
    }
    return profiles;
}

std::shared_ptr<Profile> Session :: dummy_profile(std::string name)
{
    return m_Profiles.at(m_Profiles.emplace(this,name));
}

void Session :: clear_dummy_profiles()
{
    m_Profiles.remove_if([](const std::shared_ptr<Profile>& prof){
        return prof->dummy();
    });
}

void Session :: unplug(Profile* prof)
{
    m_Profiles.remove_if([prof](const std::shared_ptr<Profile>& p){
        return (prof == p.get());
    });
}

