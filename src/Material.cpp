#include "Material.h"
#include "Filesystem.h"
#include <boost/filesystem.hpp>
using namespace std;
namespace fs = boost::filesystem;

Material :: Material(
    const std::string& fn,
    ICache* cache
):
    m_Filename(fn),
    m_pCache(cache)
{
}

Material :: ~Material()
{
}

//unsigned int Material :: id(Pass* pass) const
//{
//    return 0;
//}

void Material :: bind(Pass* pass) const
{
}

/*static*/ bool Material :: supported(const string& fn)
{
    // check if normal map exists
    auto ext = Filesystem::getExtension(fn);
    auto map_fn = Filesystem::cutExtension(fn) + "_NRM." + ext;
    return fs::exists(fs::path(map_fn));
}

