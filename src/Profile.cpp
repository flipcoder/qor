#include "Profile.h"
#include "kit/log/errors.h"
#include "Filesystem.h"
#include "kit/log/log.h"
#include "Session.h"
#include "Input.h"

Profile :: Profile(
    unsigned int id,
    Session* session,
    Input* input,
    const std::string& fn
):
    m_pSession(session),
    m_ID(id)
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
            m_Binds[bind.key().asString()] =  (*bind).asString();
        }
    }

    m_pController = input->plug(id);
    for(auto bind: m_Binds)
    {
        auto btn = m_pController->bind(bind.first, bind.second);
    }
}

