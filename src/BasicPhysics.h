#include "IPhysics.h"
#include "Node.h"
#include "kit/kit.h"

// BasicPhysics uses the BasicPartitioner to sort objects for collision checks

class BasicPhysics:
    public IPhysics
{
    public:
        virtual ~BasicPhysics() {}

    private:
        kit::decision_tree<
            unsigned, Node*
        > m_Tree;
};

