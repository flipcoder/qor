#ifndef _ITEXTURE_H_KRCF74DT
#define _ITEXTURE_H_KRCF74DT

#include "IResource.h"

class Pass;
class ITexture:
    public IResource
{
    public:
        virtual ~ITexture() {}
        virtual unsigned int id(Pass* pass = nullptr) const {
            return 0;
        }
        virtual void bind(Pass* pass) const {
        }
    private:
};

#endif

