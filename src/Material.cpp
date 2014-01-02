#include "Material.h"
#include "Filesystem.h"
#include "kit/log/log.h"
#include <boost/filesystem.hpp>
#include <vector>
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
    string ext = Filesystem::getExtension(fn);
    string cut = Filesystem::cutExtension(fn);
    unsigned compat = 0U;
    vector<string> types = {
        "NRM",
        "DISP",
        "SPEC",
        "OCC"
    };
    for(auto&& t: types) {
        if(fs::exists(
            fs::path(cut + "_" + t + "." + ext)
        )){
            ++compat;
        }
    }
    // all detail maps exist
    if(compat == types.size())
        return true;
    // partial compatibility probably means user forgot one, so we'll warn
    if(compat)
        WARNINGf("Material \"%s\" lacking %s out of %s detail maps",
            Filesystem::getFileName(fn) %
            (types.size() - compat) %
            types.size()
        );
    return false;
}

