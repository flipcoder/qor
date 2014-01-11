#include "Composite.h"
using namespace std;

Composite :: Composite(
    const std::string& fn,
    Cache<Resource, std::string>* cache
):
    Resource(fn)
{
    
}

