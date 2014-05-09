#include "Scene.h"
#include "Mesh.h"
#include "kit/meta/meta.h"
using namespace std;

Scene :: Scene(const string& fn, Cache<Resource, std::string>* cache):
    //Resource(fn),
    m_Filename(fn),
    m_pCache(cache),
    m_pRoot(make_shared<Node>()),
    m_pData(make_shared<Meta<kit::dummy_mutex>>(fn))
{
    std::deque<std::tuple<
        std::shared_ptr<Meta<kit::dummy_mutex>>,
        std::unique_lock<kit::dummy_mutex>,
        std::string
    >> metastack;

    m_pData->each([this, &metastack](
        const std::shared_ptr<Meta<kit::dummy_mutex>>& parent,
        MetaElement& e,
        unsigned level
    ){
        std::vector<std::string> path;
        for(auto&& d: metastack) {
            std::string key = std::get<2>(d);
            if(!key.empty())
                path.push_back(std::get<2>(d));
        }
        path.push_back(e.key);
        LOGf("path element: %s", boost::algorithm::join(path,"/"));

        return MetaLoop::STEP;
    },
        (unsigned)Meta<>::EachFlag::DEFAULTS |
            (unsigned)Meta<>::EachFlag::RECURSIVE,
        &metastack
    );
}

