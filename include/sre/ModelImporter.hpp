/*
 *  SimpleRenderEngine
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#pragma once

#include <vector>
#include "sre/Material.hpp"

namespace sre{


class Mesh;

/**
 * Wavefront OBJ file importer.
 * Both the geometry and materials are loaded (including textures).
 * Only triangular meshes with a vertex count under 65.536 vertices are supported. 
 */
class ModelImporter {
public:
    static std::shared_ptr<Mesh> importObj(std::string path, std::string filename, std::vector<std::shared_ptr<Material>>& outModelMaterials);
                                                        // Load an Obj mesh, materials will be defined in the last parameter.
                                                        // Note that only diffuse color and texture and specular exponent are read from the file
};
}