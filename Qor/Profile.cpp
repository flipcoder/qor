#include "Profile.h"
#include "kit/log/errors.h"
#include "Filesystem.h"
#include "kit/log/log.h"
#include "Session.h"
#include "Input.h"
using namespace std;

Profile :: Profile(
    unsigned int id,
    Session* session,
    Input* input,
    const string& fn
):
    m_pSession(session),
    m_ID(id),
    m_pConfig(std::make_shared<Meta>(
        fn
    )),
    m_pTemp(std::make_shared<Meta>()),
    m_pInput(input),
    m_Filename(fn)
{
    assert(m_pSession);
    reload();
}

Profile :: Profile(Session* session, std::string name = ""):
    m_pConfig(std::make_shared<Meta>()),
    m_pTemp(std::make_shared<Meta>()),
    m_pSession(session)
{
    m_Name = name;
    m_bDummy = true;
    m_bActive = true;
}

void Profile :: reload()
{
    if(m_bDummy)
        return;
    
    auto buf = Filesystem::file_to_buffer(m_Filename);
    if(buf.empty())
        K_ERROR(READ, Filesystem::getFileName(m_Filename));
    
    m_Json = Json::Value();
    Json::Reader reader;
    if(!reader.parse(&buf[0], m_Json))
        K_ERROR(PARSE, Filesystem::getFileName(m_Filename));
    if(!m_Json.isObject())
        K_ERROR(PARSE, Filesystem::getFileName(m_Filename));

    m_Name = m_Json.get("name", Json::Value()).asString();
    
    auto node = m_Json.get("input", Json::Value());
    if(node.isObject())
    {
        auto binds = node.get("binds", Json::Value());
        for(auto bind = binds.begin();
            bind != binds.end();
            ++bind)
        {
            string key = bind.key().asString();
            if((*bind).isString()) {
                m_Binds.emplace_back(make_tuple(
                    key,
                    (*bind).asString()
                ));
            }else if((*bind).isArray()){
                for(auto multibind = (*bind).begin();
                    multibind != (*bind).end();
                    ++multibind)
                {
                    if((*multibind).isString()) {
                        m_Binds.emplace_back(make_tuple(
                            key,
                            (*multibind).asString()
                        ));
                    }else{
                        K_ERRORf(PARSE,"Invalid bind for \"%s\" in %s",key % m_Filename);
                    }
                }
            }else{
                K_ERRORf(PARSE, "Invalid bind for \"%s\" in %s", key % m_Filename);
            }
        }
    }

    m_pController = m_pInput->plug(m_ID);
    for(auto bind: m_Binds)
    {
        auto btn = m_pController->bind(
            std::get<0>(bind), std::get<1>(bind)
        );
    }
}

map<string, vector<string>> Profile :: binds()
{
    map<string, vector<string>> r;
    
    shared_ptr<Meta> binds;
    TRY(binds = config()->
        meta("input")->meta("binds")
    );

    if(binds)
    {
        for(auto&& bind: *binds)
        {
            try{
                // individual action -> key
                r[bind.as<string>()].push_back(bind.key);
            }catch(const boost::bad_any_cast&){
                // many actions -> one key
                auto bind_list = bind.as<shared_ptr<Meta>>();
                for(auto&& key: *bind_list)
                    r[key.as<string>()].push_back(bind.key);
            }
        }
    }
    return r;
}

void Profile :: binds(const map<string, vector<string>>& b)
{
    //for(auto& kv: b)
    //{
    //    auto first = kv.first;
    //    auto second = kv.second;
    //    LOG(first);
    //    Log::Indent li;
    //    for(string& e: second)
    //    {
    //        LOG(e);
    //    }
    //}
}

void Profile :: sync()
{
    m_pConfig->serialize(m_Filename);
}

void Profile :: clear_temp()
{
    m_pTemp = std::make_shared<Meta>();
}

