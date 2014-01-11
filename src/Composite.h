#ifndef _COMPOSITE_H
#define _COMPOSITE_H

#include <tuple>
#include "Resource.h"
#include "kit/cache/cache.h"
#include "kit/meta/meta.h"

class Composite:
    public Resource
{
    public:
        
        Composite(
            const std::string& fn,
            Cache<Resource, std::string>* cache
        );
        
        Composite(const std::tuple<std::string, ICache*>& args):
            Composite(
                std::get<0>(args),
                (Cache<Resource, std::string>*) std::get<1>(args)
            )
        {}
        
        virtual ~Composite() {}
        
        typedef std::unordered_map<
            std::string,
            std::shared_ptr<Resource>
        >::iterator iterator;
        typedef std::unordered_map<
            std::string,
            std::shared_ptr<Resource>
        >::const_iterator const_iterator;

        iterator begin() {
            return m_Resources.begin();
        }
        iterator end() {
            return m_Resources.end();
        }
        const_iterator begin() const {
            return m_Resources.begin();
        }
        const_iterator end() const {
            return m_Resources.end();
        }

    private:
        
        std::unordered_map<
            std::string,
            std::shared_ptr<Resource>
        > m_Resources;
        
};

#endif

