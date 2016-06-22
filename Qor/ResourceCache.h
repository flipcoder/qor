#ifndef _RESOURCECACHE_H
#define _RESOURCECACHE_H

#include "Resource.h"
#include "kit/cache/cache.h"
#include "kit/meta/meta.h"

class ResourceCache:
    public Cache<Resource, std::string>
{
    public:
        
        ResourceCache() = default;
        ResourceCache(std::string fn):
            Cache<Resource, std::string>(fn)
        {}
        ResourceCache(std::shared_ptr<Meta> cfg):
            Cache<Resource, std::string>(cfg)
        {}
        
        virtual ~ResourceCache() {}
        
        std::shared_ptr<Meta> config() {
            return Cache<Resource, std::string>::config();
        }
        std::shared_ptr<const Meta> config() const {
            return Cache<Resource, std::string>::config();
        }
        std::shared_ptr<Meta> config(std::string fn);

    private:

        std::unordered_map<std::string, std::shared_ptr<Meta>> m_Configs;
};

#endif

