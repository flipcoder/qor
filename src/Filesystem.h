#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include <string>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
//#include "IStaticInstance.h"

// TODO: This file needs massive improvement

namespace Filesystem
{
    std::string getFileName(const std::string& path);
    std::string getPath(const std::string& path);
    std::string getExtension(const std::string& path);
    std::string cutExtension(const std::string& path);
    std::string getFileNameNoExt(std::string path);
    bool hasExtension(const std::string& path, std::string ext);
    bool hasExtension(const std::string& path);
    bool pathCompare(const std::string& a, const std::string& b);
    boost::filesystem::path locate(
        std::string fn,
        const std::vector<boost::filesystem::path>& paths,
        std::vector<std::string> extensions = std::vector<std::string>()
    );
    std::vector<char> file_to_buffer(const std::string& fn);
}

#endif

