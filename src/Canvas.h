#ifndef CANVAS_H_QZMOVUCU
#define CANVAS_H_QZMOVUCU

#include <cairomm/cairomm.h>
//#include <pangomm.h>
#include "IRenderable.h"
#include "Texture.h"
#include "Pass.h"

/*
 * 2D Canvas for drawing with Cairo
 */

class Canvas:
    public IPipelineRenderable
{
    public:
        
        Canvas(unsigned w, unsigned h);
        virtual ~Canvas();

        virtual void render(Pass* pass) const;

        void dirty(bool b) {
            m_bDirty= b;
        }
        bool dirty() const {
            return m_bDirty;
        }
        
    private:
        
        mutable bool m_bDirty = false;
        std::shared_ptr<Texture> m_Texture;
        Cairo::RefPtr<Cairo::ImageSurface> m_Surface;
        Cairo::RefPtr<Cairo::Context> m_Context;
};

#endif

