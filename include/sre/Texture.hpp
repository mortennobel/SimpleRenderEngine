/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <map>

#include "sre/impl/Export.hpp"
#include "sre/Framebuffer.hpp"
#include "sre/Shader.hpp"

namespace sre{

    class RenderPass;

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
class DllExport Texture : public std::enable_shared_from_this<Texture> {
public:
    enum class TextureCubemapSide{
        PositiveX,
        NegativeX,
        PositiveY,
        NegativeY,
        PositiveZ,
        NegativeZ
    };
    class DllExport TextureBuilder {
    public:
        ~TextureBuilder();
        TextureBuilder& withGenerateMipmaps(bool enable);
        TextureBuilder& withFilterSampling(bool enable);                                    // if true texture sampling is filtered (bi-linear or tri-linear sampling) otherwise use point sampling.
        TextureBuilder& withWrappedTextureCoordinates(bool enable);
        TextureBuilder& withFileCubemap(std::string filename, TextureCubemapSide side);     // Must define a cubemap for each side
        TextureBuilder& withFile(std::string filename);                                     // Currently only PNG files supported
        TextureBuilder& withRGBData(const char* data, int width, int height);               // data may be null (for a uninitialized texture)
        TextureBuilder& withRGBAData(const char* data, int width, int height);              // data may be null (for a uninitialized texture)
        TextureBuilder& withWhiteData(int width=2, int height=2);
        TextureBuilder& withWhiteCubemapData(int width=2, int height=2);
        TextureBuilder& withName(const std::string& name);
        std::shared_ptr<Texture> build();
    private:
        TextureBuilder();
        TextureBuilder(const TextureBuilder&) = default;
        int width = -1;
        int height = -1;
        std::string name;
        bool generateMipmaps = false;
        bool filterSampling = true;                                                         // true = linear/trilinear sampling, false = point sampling
        bool wrapTextureCoordinates = true;
        uint32_t target = 0;
        unsigned int textureId = 0;

        friend class Texture;
        friend class RenderPass;
    };

    virtual ~Texture();

    // Create a new texture using the builder pattern
    static TextureBuilder create();

    static std::shared_ptr<Texture> getWhiteTexture();
    static std::shared_ptr<Texture> getSphereTexture();
    static std::shared_ptr<Texture> getDefaultCubemapTexture();

    int getWidth();
    int getHeight();

    bool isFilterSampling();                                                                // returns true if texture sampling is filtered when sampling (bi-linear or tri-linear sampling).
    bool isWrapTextureCoordinates();                                                        // returns false if texture coordinates are clamped otherwise wrapped
    bool isCubemap();                                                                       // is cubemap texture

    const std::string& getName();                                                           // name of the string

    int getDataSize();                                                                      // get size of the texture in bytes on GPU
private:
    Texture(unsigned int textureId, int width, int height, uint32_t target, std::string string);
    void updateTextureSampler(bool filterSampling, bool wrapTextureCoordinates);
    void invokeGenerateMipmap();
    int width;
    int height;
    uint32_t target;
    bool generateMipmap;
    std::string name;
    bool filterSampling = true; // true = linear/trilinear sampling, false = point sampling
    bool wrapTextureCoordinates = true;
    unsigned int textureId;
    friend class Shader;
    friend class Material;
    friend class Framebuffer;
    friend class RenderPass;
    friend class Profiler;
    friend class sre::Framebuffer::FrameBufferBuilder;
};


}