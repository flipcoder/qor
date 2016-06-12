#ifndef _GRID_H_P55MOVMB
#define _GRID_H_P55MOVMB

#include <boost/any.hpp>
#include "Node.h"
#include "State.h"

/*
 *  Grid should be a way of managing children that are layed out in the form
 *  of a Grid, and this should work in 2D or 3D.
 *
 *  Possible custom render() function needed here to make grid x,y access
 *  into Nodes easier (would be nice to store pointers to connected nodes?)
 *  We'd need a GridNode/ConnectedGridNode class for this.
 */
class Grid:
    public Node
{
    public:

        Grid();
        virtual ~Grid() {}
        //virtual void render() const override;
        Node* at(glm::vec2 loc);

        void add_tile(std::shared_ptr<Node> n, glm::ivec2 loc);
        void size(glm::ivec2 sz) {
            m_Size = sz;
            m_Tiles.reserve(m_Size.x * m_Size.y);
        }
        void tile_size(glm::ivec2 sz) {
            m_TileSize = sz;
        }
        
        virtual bool is_partitioner(Camera* camera) const override { return true; }
        virtual std::vector<const Node*> visible_nodes(Camera* camera) const override;

        virtual std::vector<Node*> all_descendants();
        
        virtual void logic_self(Freq::Time t) override;
        virtual void render_self(Pass* pass) const override;

        virtual std::shared_ptr<Node> tile(int x, int y);
        void remove_tile(Node* tile);

        void set_main_camera(Camera* cam) { m_pMainCamera = cam; }

        std::vector<std::shared_ptr<Node>>& tiles() { return m_Tiles; }
        
        virtual std::vector<Node*> query(
            Box box,
            std::function<bool(Node*)> cond = std::function<bool(Node*)>()
        ) override;
        
    private:
        
        std::vector<std::shared_ptr<Node>> m_Tiles;
        glm::ivec2 m_Size; // in tiles, not coordinates
        glm::ivec2 m_TileSize;

        Camera* m_pMainCamera = nullptr;
};

#endif

