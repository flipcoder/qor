#ifndef _SCENE_H
#define _SCENE_H

#include <string>
#include "IResource.h"
#include "Node.h"
#include "kit/cache/cache.h"

class Scene:
    public IResource
{
    public:
        
        Scene(
            const std::string& fn,
            Cache<IResource, std::string>* cache
        );
        
        Scene(const std::tuple<std::string, ICache*>& args):
            Scene(
                std::get<0>(args),
                (Cache<IResource, std::string>*)std::get<1>(args)
            )
        {}
        virtual ~Scene() {}
        
    private:
        
        std::string m_Filename;
        Cache<IResource, std::string>* m_pCache;

        std::shared_ptr<Node> m_pRoot;
};

#endif

