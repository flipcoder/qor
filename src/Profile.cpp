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
    ))
{
    assert(m_pSession);

    auto buf = Filesystem::file_to_buffer(fn);
    if(buf.empty())
        ERROR(READ, Filesystem::getFileName(fn));

    m_Json = Json::Value();
    Json::Reader reader;
    if(!reader.parse(&buf[0], m_Json))
        ERROR(PARSE, Filesystem::getFileName(fn));
    if(!m_Json.isObject())
        ERROR(PARSE, Filesystem::getFileName(fn));

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
                        ERRORf(PARSE,"Invalid bind for \"%s\" in %s",key % fn);
                    }
                }
            }else{
                ERRORf(PARSE, "Invalid bind for \"%s\" in %s", key % fn);
            }
        }
    }

    m_pController = input->plug(id);
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
    
}

