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

        Grid() {}
        virtual ~Grid() {}
        //virtual void render() const override;
        Node* at(glm::vec2 loc);

        void add_tile(std::shared_ptr<Node> n, glm::ivec2 loc);
        void size(glm::ivec2 sz) {
            m_Size = sz;
            m_Tiles.resize(m_Size.x * m_Size.y);
        }
        
        virtual bool is_partitioner(Camera* camera) const override { return true; }
        virtual std::vector<const Node*> visible_nodes(Camera* camera) const override;

    private:
        
        std::vector<Node*> m_Tiles;
        glm::ivec2 m_Size; // in tiles, not coordinates
};

#endif

