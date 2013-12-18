#ifndef _BASICPARTITIONER_H_YPGXO911
#define _BASICPARTITIONER_H_YPGXO911

#include "IPartitioner.h"
#include "kit/kit.h"

class BasicPartitioner:
    public IPartitioner
{
    public:
        BasicPartitioner() {}
        virtual ~BasicPartitioner() {}

    private:

        // The decision tree should most of the time be read-only
        // for parallel partitioning later on to take place
        //
        // TODO: add iteration through decision_trees to allow for
        // assumptions to be made during the decision process
        // (this is good for slightly modified decisions without writing to
        //  the tree directly)
        kit::decision_tree<
            bool, /*BasicPartitioner*,*/ Node*
        > m_Tree;
};

#endif

