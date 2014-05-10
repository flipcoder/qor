#ifndef CANVAS_H_QZMOVUCU
#define CANVAS_H_QZMOVUCU

#include <cairomm/cairomm.h>
#include <pangomm.h>
#include <pangomm/init.h>
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

        Cairo::RefPtr<Cairo::ImageSurface>& surface() {
            return m_Surface;
        }
        Cairo::RefPtr<Cairo::Context>& context() {
            return m_Context;
        }

        std::shared_ptr<Texture> texture() {
            return m_Texture;
        }
        std::shared_ptr<const Texture> texture() const {
            return m_Texture;
        }

    private:

        struct PangoIniter {
            PangoIniter() {
                Pango::init();
            }
        } m_PangoIniter;
        
        bool dirty() const {
            return m_bDirty;
        }
        
        mutable bool m_bDirty = false;
        std::shared_ptr<Texture> m_Texture;
        Cairo::RefPtr<Cairo::ImageSurface> m_Surface;
        Cairo::RefPtr<Cairo::Context> m_Context;
        //Glib::RefPtr<Pango::Context> m_PangoContext;
        Glib::RefPtr<Pango::Layout> m_Layout;
};

#endif

