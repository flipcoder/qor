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

    private:
};

#endif

