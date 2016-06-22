#ifndef _RESOURCELOCATOR_H_66PTCU1N
#define _RESOURCELOCATOR_H_66PTCU1N

/*
 * Resource locator to eventually replace ResourceCache's path searching code
 * And to be used with new cache system, but not completely necessary yet
 * since ResourceCache loads normal paths just fine, and this will return them.
 */
class ResourceLocator
{
    public:
        ResourceLocator() {}
        virtual ~ResourceLocator() {}
    private:
};

#endif

