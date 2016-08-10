#include "Texture.hpp"

#if defined(_WIN32)
#   define GLEW_STATIC
#   include <GL/glew.h>
#else
#   include <OpenGL/gl3.h>
#endif
#include <stdio.h>
#include <SDL_surface.h>

#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <memory>

#include "Font.inl"

namespace {
    static std::vector<char> readAllBytes(char const* filename)
    {
        using namespace std;
        ifstream ifs(filename, std::ios::binary|std::ios::ate);
        ifstream::pos_type pos = ifs.tellg();
        if (pos<0){
            std::cerr << "Cannot read " << filename << std::endl;
            return std::vector<char>();
        }
        std::vector<char>  result((size_t)pos);

        ifs.seekg(0, ios::beg);
        ifs.read(&result[0], pos);

        return result;
    }
}

namespace SRE {
    Texture* Texture::whiteTexture = nullptr;
    Texture* Texture::fontTexture = nullptr;

    Texture::Texture(const char *data, int width, int height, uint32_t format, bool generateMipmaps)
            : width{width}, height{height}, generateMipmap{generateMipmaps} {
        glGenTextures(1, &textureId);

        GLenum target = GL_TEXTURE_2D;
        GLint mipmapLevel = 0;
        GLint internalFormat = GL_RGBA;
        GLint border = 0;
        GLenum type = GL_UNSIGNED_BYTE;
        glBindTexture(target, textureId);
        glTexImage2D(target, mipmapLevel, internalFormat, width, height, border, format, type, data);
        updateTextureSampler();
        if (generateMipmaps) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    }

    GLenum getFormat(SDL_Surface *image) {
        SDL_PixelFormat *format = image->format;
        auto pixelFormat = format->format;
        bool isBGR = format->Rshift == 16;

        GLenum RGB = isBGR ? GL_BGR : GL_RGB;
        GLenum RGBA = isBGR ? GL_BGRA : GL_RGBA;

        const bool alpha = SDL_ISPIXELFORMAT_ALPHA(pixelFormat);
        if (alpha) {
            return RGBA;
        } else {
            if (format->BytesPerPixel == 4) {
                return RGBA;
            } else if (format->BytesPerPixel == 3) {
                return RGB;
            } else {
                SDL_SetError("Unknown image format. Only PNG-24 and JPEG is supported.");
                return 0;
            }
        }
    }

    int invert_image(int width, int height, void *image_pixels) {
        auto temp_row = std::unique_ptr<char>(new char[width]);
        if (temp_row.get() == nullptr) {
            SDL_SetError("Not enough memory for image inversion");
            return -1;
        }
        //if height is odd, don't need to swap middle row
        int height_div_2 = height / 2;
        for (int index = 0; index < height_div_2; index++) {
            //uses string.h
            memcpy((Uint8 *) temp_row.get(),
                   (Uint8 *) (image_pixels) +
                   width * index,
                   width);

            memcpy(
                    (Uint8 *) (image_pixels) +
                    width * index,
                    (Uint8 *) (image_pixels) +
                    width * (height - index - 1),
                    width);
            memcpy(
                    (Uint8 *) (image_pixels) +
                    width * (height - index - 1),
                    temp_row.get(),
                    width);
        }
        return 0;
    }

    bool isPowerOfTwo(unsigned int x) {
        return ((x != 0) && !(x & (x - 1)));
    }

    Texture* Texture::createFromFile(const char *pngOrJpeg, bool generateMipmaps){
        auto data = readAllBytes(pngOrJpeg);
        return createFromMem(data.data(), data.size(), generateMipmaps);
    }

    Texture *Texture::createFromMem(const char *pngOrJpeg, int size, bool generateMipmaps) {
        static bool initialized = false;
        if (!initialized) {
            initialized = true;
            int flags = IMG_INIT_PNG | IMG_INIT_JPG;
            int initted = IMG_Init(flags);
            if ((initted & flags) != flags) {
                std::cerr << "IMG_Init: Failed to init required jpg and png support!" << std::endl;
                std::cerr << "IMG_Init: " << IMG_GetError() << std::endl;
                // handle error
            }
        }

        SDL_RWops *source = SDL_RWFromConstMem(pngOrJpeg, size);
        SDL_Surface *res_texture = IMG_Load_RW(source, 1);
        if (res_texture == NULL) {
            std::cerr << "IMG_Load: " << SDL_GetError() << std::endl;
            return nullptr;
        }
        int w = res_texture->w;
        int h = res_texture->h;
        char *pixels = static_cast<char *>(res_texture->pixels);
        invert_image(w*4, h, pixels);
        if (generateMipmaps && (!isPowerOfTwo((unsigned int) w) || !isPowerOfTwo((unsigned int) h))) {
            std::cerr << "Ignore mipmaps for textures not power of two" << std::endl;
            generateMipmaps = false;
        }
        Texture *res = new Texture(pixels, w, h, getFormat(res_texture), generateMipmaps);
        SDL_FreeSurface(res_texture);
        return res;
    }

    Texture *Texture::createFromRGBAMem(const char *data, int width, int height, bool generateMipmaps) {
        return new Texture(data, width, height, GL_RGBA, generateMipmaps);
    }

// returns true if texture sampling should be filtered (bi-linear or tri-linear sampling) otherwise use point sampling.
    bool Texture::isFilterSampling() {
        return filterSampling;
    }

// if true texture sampling is filtered (bi-linear or tri-linear sampling) otherwise use point sampling.
    void Texture::setFilterSampling(bool enable) {
        filterSampling = enable;
        updateTextureSampler();
    }

    bool Texture::isWrapTextureCoordinates() {
        return filterSampling;
    }

    void Texture::setWrapTextureCoordinates(bool enable) {
        wrapTextureCoordinates = enable;
        updateTextureSampler();
    }

    int Texture::getWidth() {
        return width;
    }

    int Texture::getHeight() {
        return height;
    }

    void Texture::updateTextureSampler() {
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapTextureCoordinates ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapTextureCoordinates ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        GLuint minification;
        GLuint magnification;
        if (!filterSampling) {
            minification = GL_NEAREST;
            magnification = GL_NEAREST;
        } else if (generateMipmap) {
            minification = GL_LINEAR_MIPMAP_LINEAR;
            magnification = GL_LINEAR;
        } else {
            minification = GL_LINEAR;
            magnification = GL_LINEAR;
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnification);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minification);
    }

    Texture *Texture::getWhiteTexture() {
        if (whiteTexture != nullptr){
            return whiteTexture;
        }
        char one = (char) 0xff;
        std::vector<char> data(2*2*4, one);
        whiteTexture = createFromRGBAMem(data.data(), 2, 2, true);
        return whiteTexture;
    }

Texture *Texture::getFontTexture() {
    if (fontTexture != nullptr){
        return fontTexture ;
    }

    fontTexture = createFromMem((const char *) font_png, sizeof(font_png), true);
    return fontTexture;
}
}