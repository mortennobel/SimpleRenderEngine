#pragma once

#include <cstdint>

#include "SRE/Export.hpp"

namespace SRE{
class DllExport Texture {
public:
    static Texture* createFromFile(const char *pngOrJpeg, bool generateMipmaps = false);
    static Texture* createFromMem(const char *pngOrJpeg, int size, bool generateMipmaps = false);
    static Texture* createFromRGBAMem(const char *data, int width, int height, bool generateMipmaps = false);
    static Texture* getWhiteTexture();
    static Texture* getFontTexture();

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
    static Texture* fontTexture;
    int width;
    int height;
    bool generateMipmap;
    bool filterSampling = true; // true = linear/trilinear sampling, false = point sampling
    bool wrapTextureCoordinates = true;
    unsigned int textureId;
    friend class Shader;
};
}