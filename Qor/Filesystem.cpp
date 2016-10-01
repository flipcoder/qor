#include "Filesystem.h"
#include <string>
#include "kit/kit.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
//#include <iostream>
using namespace std;

namespace Filesystem {

// takes an absolute or relative path and returns the file name (including ext)
std::string getFileName(const std::string& path)
{
    size_t split_point;
    if((split_point = path.find_last_of("\\/")) != string::npos)
        return path.substr(split_point+1);
    return path;
}

std::string getFileNameNoInternal(std::string path)
{
    auto internals = getInternal(path);
    path = cutInternal(path);
    auto fn = getFileName(path);
    return fn + ":" + internals;
}

std::string getPath(const std::string& path)
{
    size_t split_point;
    if((split_point = path.find_last_of("\\/")) != string::npos)
        return path.substr(0,split_point+1);
    return "";
}

std::string getExtension(const std::string& path)
{
    size_t split_point;
    if((split_point = path.find_last_of(".")) != string::npos)
        return path.substr(split_point+1);
    return "";
}

std::string cutExtension(const std::string& path)
{
    size_t split_point = path.find_last_of(".");
    if(split_point != string::npos)
        return path.substr(0, split_point);
    return "";
}

std::string cutInternal(const std::string& path)
{
    size_t split_point = path.find(":",2);
    if(split_point != string::npos)
        return path.substr(0, split_point);
    return path;
}

bool hasInternal(const std::string& path)
{
    size_t split_point = path.find(":",2);
    if(split_point != string::npos)
        return true;
    return false;
}
std::string getInternal(const std::string& path)
{
    size_t split_point = path.find(":",2);
    if(split_point != string::npos)
        return path.substr(split_point+1);
    return "";
}

std::string changeExtension(const std::string& path, const std::string& ext)
{
    return cutExtension(path) + "." + ext;
}

std::string getFileNameNoExt(std::string path)
{
    size_t split_point;
    if((split_point = path.find_last_of("\\/")) != string::npos)
        path = path.substr(split_point+1);
    return cutExtension(path);
}

bool hasExtension(const std::string& path, std::string ext)
{
    try{
        if(ext.at(0) != '.')
            ext = "." + ext;
    }catch(const std::out_of_range&){}

    return boost::ends_with(boost::to_lower_copy(path), boost::to_lower_copy(ext));
}

bool hasExtension(const std::string& path)
{
    size_t dot, pathsep;

    if(std::string::npos == (dot = path.find_last_of("."))) // get dot location
        return false; // no dot = no extension
    if(std::string::npos == (pathsep = path.find_last_of("\\/")))
        return true; // dot + no pathsep = has extension

    return dot > pathsep; // last dot comes after last pathsep?
}

bool pathCompare(const std::string& a, const std::string& b)
{
    return boost::filesystem::equivalent(
            boost::filesystem::path(a),
            boost::filesystem::path(b));
    //return a==b;
}

boost::filesystem::path locate(
    std::string fn,
    const std::vector<boost::filesystem::path>& paths,
    std::vector<std::string> extensions
)
{
    if(fn.find("\\") != std::string::npos ||
        fn.find("/") != std::string::npos ||
        fn.find(":") != std::string::npos)
    {
        return boost::filesystem::path(fn); // return actual path
    }
    boost::filesystem::path path;
    for(auto& search_path: paths) {
        //Log::get().write(search_path.string());
        path = search_path / fn;
        //Log::get().write(path.string());

        if(path.extension().empty()) {
            for(auto& ext: extensions) { // each extenion
                boost::filesystem::path temp_path = path;
                temp_path.replace_extension("."+ext);
                if(boost::filesystem::exists(temp_path))
                    return temp_path;
            }
        }
        else if(boost::filesystem::exists(path))
            return path;
    }
    return boost::filesystem::path(); // empty
}

std::vector<char> file_to_buffer(const std::string& fn)
{
    ifstream file(fn);
    if(!file)
        return std::vector<char>();
    vector<char> data(
        (istreambuf_iterator<char>(file)),
        istreambuf_iterator<char>()
    );
    data.push_back('\0');
    return data;
}

//need a file to string method
std::string file_to_string(const std::string & fn)
{
    ifstream file(fn);
    if (!file)
        return std::string();
    string data(
        (istreambuf_iterator<char>(file)),
        istreambuf_iterator<char>()
    );
    return data;
}

} // END OF NAMESPACE
