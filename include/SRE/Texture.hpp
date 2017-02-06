#pragma once

#include <cstdint>
#include <vector>

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
    class DllExport TextureBuilder {
    public:
        TextureBuilder& withGenerateMipmaps(bool enable);
        // if true texture sampling is filtered (bi-linear or tri-linear sampling) otherwise use point sampling.
        TextureBuilder& withFilterSampling(bool enable);
        TextureBuilder& withWrappedTextureCoordinates(bool enable);
        TextureBuilder& withFile(const char* filename);
        TextureBuilder& withRGBData(const char* data, int width, int height);
        TextureBuilder& withRGBAData(const char* data, int width, int height);
        TextureBuilder& withWhiteData(int width=2, int height=2);
        Texture* build();
    private:
        std::vector<char> dataOwned;
        const char* data = nullptr;
        const char* filename = nullptr;
        int width = -1;
        int height = -1;
        uint32_t format = -1;
        bool generateMipmaps = false;
        bool filterSampling = true; // true = linear/trilinear sampling, false = point sampling
        bool wrapTextureCoordinates = true;
        TextureBuilder() = default;
        friend class Texture;
    };

    virtual ~Texture();

    static Texture* getWhiteTexture();
    static Texture* getSphereTexture();

    // Create a new texture using the builder pattern
    static TextureBuilder create();

    int getWidth();
    int getHeight();

    // returns true if texture sampling should be filtered (bi-linear or tri-linear sampling) otherwise use point sampling.
    bool isFilterSampling();
    bool isWrapTextureCoordinates();

    // get size of the texture in bytes on GPU
    int getDataSize();
private:
    Texture(const char* rgba, int width, int height, uint32_t format);
    void updateTextureSampler(bool filterSampling, bool wrapTextureCoordinates);
    void invokeGenerateMipmap();
    int width;
    int height;
    uint32_t target;
    bool generateMipmap;
    bool filterSampling = true; // true = linear/trilinear sampling, false = point sampling
    bool wrapTextureCoordinates = true;
    unsigned int textureId;
    friend class Shader;
};
}