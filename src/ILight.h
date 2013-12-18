#ifndef _ILIGHT_H_KRCF74DT
#define _ILIGHT_H_KRCF74DT

#include "IRealtime.h"

class Pass;
class ILight:
    public IRealtime
{
    public:
        virtual ~ILight() {}

        //virtual unsigned id(Pass* pass = nullptr) const = 0;
        //virtual unsigned int& id_ref() const = 0;

        virtual void logic(Freq::Time t) override = 0;
    private:
};

#endif

