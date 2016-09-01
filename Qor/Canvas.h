#ifndef CANVAS_H_QZMOVUCU
#define CANVAS_H_QZMOVUCU

#ifndef QOR_NO_CAIRO

#include <cairomm/cairomm.h>
#include <pangomm.h>
#include <pangomm/init.h>
#include "Node.h"
#include "Texture.h"
#include "Pass.h"


/*
 * 2D Canvas for drawing with Cairo
 */

class Canvas:
    public Node
{
    public:
        
        Canvas(unsigned w, unsigned h);
        virtual ~Canvas();

        virtual void logic_self(Freq::Time t) override;
        virtual void render_self(Pass* pass) const override;

        //void resize(unsigned w, unsigned h);
        
        void refresh() {
            m_bDirty = true;
        }
        void dirty(bool b) {
            m_bDirty = b;
        }

        Cairo::RefPtr<Cairo::ImageSurface>& surface() {
            return m_Surface;
        }
        Cairo::RefPtr<Cairo::Context>& context() {
            return m_Context;
        }
        Glib::RefPtr<Pango::Layout>& layout() {
            return m_Layout;
        }

        std::shared_ptr<Texture> texture() {
            return m_Texture;
        }
        std::shared_ptr<const Texture> texture() const {
            return m_Texture;
        }

        glm::vec2 size() const {
            return glm::vec2(m_Texture->size().x, m_Texture->size().y);
        }
        glm::vec2 center() const {
            return glm::vec2(
                m_Texture->size().x/2.0f, m_Texture->size().y/2.0f
            );
        }

        enum Align {
            LEFT = 0,
            CENTER,
            RIGHT
        };
        void text(std::string text,
            Color c = Color::white(),
            glm::vec2 pos = glm::vec2(0.0f),
            Canvas::Align align = Align::LEFT
        );

        void clear(Color c = Color::clear());
        void font(std::string fn, int sz);

        void rectangle(
            float x, float y, float width, float height, float radius = 0.0f
        );
        
        virtual std::string type() const override { return "canvas"; }
       
        void color(Color c);
        
        boost::signals2::signal<void()> on_redraw;
        
    private:
        
        struct PangoIniter {
            PangoIniter() {
                Pango::init();
            }
        };
        struct PangoInitOnce {
            PangoInitOnce() {
                static PangoIniter init_once;
            }
        } m_PangoInit;
        
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
#endif

