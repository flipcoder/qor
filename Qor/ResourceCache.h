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
            Cache<Resource,std::string>(fn)
        {}
        ResourceCache(Meta::ptr cfg):
            Cache<Resource,std::string>(cfg)
        {}
        
        virtual ~ResourceCache() {}
        
        Meta::ptr config() {
            return Cache<Resource, std::string>::config();
        }
        Meta::cptr config() const {
            return Cache<Resource, std::string>::config();
        }
        Meta::ptr config(std::string fn);

    private:

        std::unordered_map<std::string, Meta::ptr> m_Configs;
};

#endif

