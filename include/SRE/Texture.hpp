#pragma once

#include <cstdint>

#include "SRE/impl/Export.hpp"

namespace SRE{
    /**
     * Represent a texture (uploaded to the GPU).
     * In general the width and the height of the texture should be power-of-two (e.g. 256 or 512)
     *
     * Textures can be created from files (png or jpeg). Alternative textures can be created using memory representation
     * of the texture in RGBA (one byte per color channel).
     * The Texture class also provides a white texture using the Texture::getWhiteTexture()
     *
     * A texture object has the following properties:
     * - mipmaps enabled: Optimization, where the texture exists in downscaled versions. This does use more memory, but
     *   in general gives faster texture sampling.
     * - wrap texture coordinates: if enabled the texture is repeated when sampling out the 0.0 .. 1.0 values
     * - filter sampling: if enabled the texture sampling will use interpolation to find the colors between pixel centers
     */
class DllExport Texture {
public:
    static Texture* createFromFile(const char *pngOrJpeg, bool generateMipmaps = false);
    static Texture* createFromMem(const char *pngOrJpeg, int size, bool generateMipmaps = false);
    static Texture* createFromRGBAMem(const char *data, int width, int height, bool generateMipmaps = false);
    static Texture* getWhiteTexture();
    static Texture* getSphereTexture();

    int getWidth();
    int getHeight();

    // returns true if texture sampling should be filtered (bi-linear or tri-linear sampling) otherwise use point sampling.
    bool isFilterSampling();
    // if true texture sampling is filtered (bi-linear or tri-linear sampling) otherwise use point sampling.
    void setFilterSampling(bool enable);
    bool isWrapTextureCoordinates();
    void setWrapTextureCoordinates(bool enable);
private:
    Texture(const char* rgba, int width, int height, uint32_t type,bool generateMipmaps);
    void updateTextureSampler();
    static Texture* whiteTexture;
    static Texture* sphereTexture;

    int width;
    int height;
    bool generateMipmap;
    bool filterSampling = true; // true = linear/trilinear sampling, false = point sampling
    bool wrapTextureCoordinates = true;
    unsigned int textureId;
    friend class Shader;
};
}