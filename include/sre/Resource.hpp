/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#pragma once
#include <string>
#include <set>
#include <map>

namespace sre {

    enum ResourceType {
        BuiltIn = 0b001,
        File    = 0b010,
        Memory  = 0b100,
        All     = 0b111,
    };

    // The resource class allows accessing resources in a uniform way. The resources are either built-in resources,
    // file-resources or memory resources. File resources overwrites built-in resources, and memory resources overwrites
    // both built-in and file-resources.
    // The resource class is a key-value map, where each key must be uses a filename notation.
    class Resource {
    public:
                                                                // load resource from built-in, filesystem or memory
                                                                // returns empty string if not found
        static std::string loadText(std::string key, ResourceType filter = ResourceType::All);
                                                                // set memory resource
        static void set(const std::string& key, const std::string& value);
        static void reset();                                    // reset memory resources
                                                                // get keys in resource system
        static std::set<std::string> getKeys(ResourceType filter = ResourceType::All);
    private:
        static std::map<std::string,std::string> memoryOnlyResources;
    };
}