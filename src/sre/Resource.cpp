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

std::string sre::Resource::loadText(std::string filename, ResourceType filter) {
    auto res = memoryOnlyResources.find(filename);
    if ((filter & ResourceType::Memory) && res != memoryOnlyResources.end()){
        return res->second;
    }
    if (filter & ResourceType::File){
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
    }
    if (filter & ResourceType::BuiltIn) {
        res = builtInShaderSource.find(filename);
        if (res != builtInShaderSource.end()) {
            return res->second;
        }
    }
    return "";
}

void sre::Resource::set(const std::string& name, const std::string& value) {
    memoryOnlyResources[name] = value;
}

void Resource::reset() {
    memoryOnlyResources.clear();
}

set<string> Resource::getKeys(ResourceType filter) {
    std::set<string> res;
    if (filter & ResourceType::Memory){
        for (auto& keyValue : memoryOnlyResources){
            res.insert(keyValue.first);
        }
    }
    if (filter & ResourceType::BuiltIn) {
        for (auto &keyValue : builtInShaderSource) {
            res.insert(keyValue.first);
        }
    }
    return res;
}
