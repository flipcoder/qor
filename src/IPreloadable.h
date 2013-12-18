#ifndef _IPRELOADABLE_H
#define _IPRELOADABLE_H

#include "IRealtime.h"
#include "IRenderable.h"
#include "IPreloadable.h"
#include <boost/optional.hpp>
#include <atomic>

class IPreloadable
{
    private:
        std::atomic<bool> m_bDone = ATOMIC_VAR_INIT(false);
    public:
        virtual ~IPreloadable() {}

        virtual boost::optional<float> progress() {
            //if(done())
            //    return 1.0f;
            return boost::optional<float>();
        }
        virtual void finish_loading() {
            m_bDone = true;
        }
        virtual bool finished_loading() const {
            return m_bDone;
        }
        //virtual bool done(bool b) {
        //    m_bDone = b;
        //}
        virtual void preload() {
            assert(false);
        }
        virtual void prepare() {
            assert(false);
        }

        virtual bool needs_load() const {
            return false;
        }
        virtual bool is_loader() const {
            return false;
        }
};

#endif

