/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
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

#ifdef None // Fix Linux compile issue
#undef None 
#endif

class SDL_Surface;

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
    enum class CubemapSide{
        PositiveX,
        NegativeX,
        PositiveY,
        NegativeY,
        PositiveZ,
        NegativeZ
    };

    enum class SamplerColorspace {
        Linear,             // Convert values from gamma space to linear space, when gamma correction is enabled. Default behavior.
        Gamma               // Sampler performs no gamma convertions. This is useful for e.g. normal textures where no gamma correction must be performed
    };

    enum class DepthPrecision {
        I16,                // 16 bit integer
        I24,                // 24 bit integer
        I32,                // 32 bit integer
        F32,                // 32 bit float
        I24_STENCIL8,       // 24 bit integer 8 bit stencil
        F32_STENCIL8,       // 32 bit float 8 bit stencil
        STENCIL8,           // 8 bit stencil
        None
    };

    enum class Wrap {
        Repeat,
        ClampToEdge,
        Mirror
    };

    class DllExport TextureBuilder {
    public:
        ~TextureBuilder();
        TextureBuilder& withGenerateMipmaps(bool enable);
        TextureBuilder& withFilterSampling(bool enable);                                    // if true texture sampling is filtered (bi-linear or tri-linear sampling) otherwise use point sampling.
        DEPRECATED("Use with WrapUV")
        TextureBuilder& withWrappedTextureCoordinates(bool enable);
        TextureBuilder& withWrapUV(Wrap wrap);                                              // Define how texture coordinates are sampled outside the [0.0,1.0] range
        TextureBuilder& withFileCubemap(std::string filename, CubemapSide side);            // Must define a cubemap for each side
        TextureBuilder& withFile(std::string filename);                                     // Currently only PNG files supported
        TextureBuilder& withRGBData(const char* data, int width, int height);               // data may be null (for a uninitialized texture)
        TextureBuilder& withRGBAData(const char* data, int width, int height);              // data may be null (for a uninitialized texture)
        TextureBuilder& withWhiteData(int width=2, int height=2);
        TextureBuilder& withSamplerColorspace(SamplerColorspace samplerColorspace);
        TextureBuilder& withWhiteCubemapData(int width=2, int height=2);
        TextureBuilder& withDepth(int width, int height, DepthPrecision precision=DepthPrecision::I16); // Creates a depth texture.
        TextureBuilder& withName(const std::string& name);
        TextureBuilder& withDumpDebug();                                                    // Output debug info on build
        std::shared_ptr<Texture> build();
    private:
        TextureBuilder();
        TextureBuilder(const TextureBuilder&) = default;

        struct TextureDefinition {
            int width = -1;
            int height = -1;
            bool transparent;
            int bytesPerPixel;
            uint32_t format;
            std::string resourcename;
            std::vector<char> data;
            void dumpDebug();
        };
        DepthPrecision depthPrecision = DepthPrecision::None;
        std::string name;
		bool transparent;
        bool generateMipmaps = false;
        bool filterSampling = true;                                                         // true = linear/trilinear sampling, false = point sampling
        Wrap wrapUV = Wrap::Repeat;
        bool dumpDebug = false;
        SamplerColorspace samplerColorspace = SamplerColorspace::Linear;
        uint32_t target = 0;
        unsigned int textureId = 0;

        std::map<uint32_t, TextureDefinition> textureTypeData;

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
    DEPRECATED("Use getWrapUV() instead")
    bool isWrapTextureCoordinates();                                                        // returns false if texture coordinates are clamped otherwise wrapped
    Wrap getWrapUV();
    bool isCubemap();                                                                       // is cubemap texture
    bool isMipmapped();                                                                     // has texture mipmapped enabled
	bool isTransparent();																	// Does texture has alpha channel
    SamplerColorspace getSamplerColorSpace();
    const std::string& getName();                                                           // name of the string

    int getDataSize();                                                                      // get size of the texture in bytes on GPU
    bool isDepthTexture();
    DepthPrecision getDepthPrecision();
private:
    Texture(unsigned int textureId, int width, int height, uint32_t target, std::string string);
    void updateTextureSampler(bool filterSampling, Wrap wrapTextureCoordinates);
    void invokeGenerateMipmap();
    static GLenum getFormat(SDL_Surface *image);
    static std::vector<char> loadFileFromMemory(const char* data, int dataSize, GLenum& format, bool & alpha,int& width, int& height, int& bytesPerPixel, bool invertY = true);
    int width;
    int height;
    uint32_t target;
    bool generateMipmap;
	bool transparent;
    DepthPrecision depthPrecision = DepthPrecision::None;
    std::string name;
    SamplerColorspace samplerColorspace;
    bool filterSampling = true; // true = linear/trilinear sampling, false = point sampling
    Wrap wrapUV;
    unsigned int textureId;
    friend class Shader;
    friend class Material;
    friend class Framebuffer;
    friend class RenderPass;
    friend class Inspector;
    friend class VR;
    friend class Sprite;
    friend class UniformSet;
};


}