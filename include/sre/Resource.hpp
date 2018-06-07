/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#pragma once
#include <string>
#include <vector>
#include <map>

namespace sre {
    // The resource class allows accessing resources in a uniform way. The resources are either built-in resources, file-resources
    // or memory resources. File resources overwrites built-in resources, and memory resources overwrites both built-in and file-resources.
    // The resource class is a key-value map, where each key must be uses a filename notation.
    class Resource {
    public:
        static std::string loadText(std::string key);           // load resource from built-in, filesystem or memory
        static void set(std::string key, std::string value);    // set memory resource
        static void reset();                                    // reset memory resources
    private:
        static std::map<std::string,std::string> memoryOnlyResources;
    };
}