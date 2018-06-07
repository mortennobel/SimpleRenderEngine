//
// Created by Morten Nobel-Jørgensen on 6/7/18.
//

#include "sre/Resource.hpp"
#include <iostream>
#include <fstream>
#include <fstream>
#include <sre/Log.hpp>
#include <sre/Resource.hpp>

#include "sre/impl/ShaderSource.inl"

using namespace std;
using namespace sre;

map<string,string> Resource::memoryOnlyResources = {};

std::string sre::Resource::loadText(std::string filename) {
    ifstream in{filename, ios::in | ios::binary};
    if (in && in.is_open())
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        auto size = in.tellg();
        if (size > 0){
            contents.resize((string::size_type)size);
            in.seekg(0, std::ios::beg);
            in.read(&contents[0], contents.size());
        }
        in.close();
        return contents;
    }
    auto res = memoryOnlyResources.find(filename);
    if (res != memoryOnlyResources.end()){
        return res->second;
    }
    res = builtInShaderSource.find(filename);
    if (res != builtInShaderSource.end()){
        return res->second;
    }
    LOG_ERROR("Cannot find shader source %s", filename.c_str());
    return "";
}

void sre::Resource::set(std::string name, std::string value) {
    memoryOnlyResources[name] = value;
}

void Resource::reset() {
    memoryOnlyResources.clear();
}
