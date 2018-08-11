#ifndef TEXT_H_SRN0YM4G
#define TEXT_H_SRN0YM4G

#include <string>
#include <SDL2/SDL_ttf.h>
#include "kit/cache/icache.h"
#include "kit/reactive/reactive.h"
#include "Node.h"
#include "Resource.h"
#include "Texture.h"
#include "Mesh.h"

class Font:
    public Resource
{
    public:

        friend class Text;

        Font();
        Font(const std::string& fn, ICache* c);
        Font(const std::tuple<std::string, ICache*>& args);
        virtual ~Font();

        Font(const Font&) = default;
        Font(Font&&) = default;
        Font& operator=(const Font&) = default;
        Font& operator=(Font&&) = default;

        TTF_Font* font() { return m_pFont; }

        int size() const { return m_Size; }

    private:

        TTF_Font* m_pFont = nullptr;
        glm::vec2 m_WindowSize;
        int m_Size;
};

class Text:
    public Node
{
    public:

        enum Align
        {
            LEFT = 0,
            CENTER = 1,
            RIGHT = 2
        };

        static void init();
        static void deinit();

        Text(const std::shared_ptr<Font>& font);
        virtual ~Text();

        Text(const Text&) = default;
        Text(Text&&) = default;
        Text& operator=(const Text&) = default;
        Text& operator=(Text&&) = default;

        virtual void logic_self(Freq::Time t) override;

        void redraw();

        void set(std::string tx);
        void align(Align a);
        void color(Color c);

        glm::uvec2 size() const { return m_pTexture->size(); }
        glm::uvec2 img_size() const { return m_ImgSize(); }

    private:

        bool m_bDirty = true;

        std::string m_Text;
        std::shared_ptr<Font> m_pFont;
        Color m_Color = Color::white();
        Align m_Align = LEFT;
        glm::vec2 m_Pos;
        //SDL_Surface* m_pSurface = nullptr;
        float m_LineSpacing = 0.0f;
        
        friend class kit::lazy<glm::ivec2>;
        kit::lazy<glm::ivec2> m_ImgSize;

        std::shared_ptr<Texture> m_pTexture;
        std::shared_ptr<Mesh> m_pMesh;
};

#endif

