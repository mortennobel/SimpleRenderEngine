#include "SRE/Texture.hpp"

#include "SRE/impl/GL.hpp"
#include <stdio.h>
#include <SDL_surface.h>

#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <memory>
#include <SRE/RenderStats.h>
#include <SRE/SimpleRenderEngine.hpp>

namespace {
	static std::vector<char> readAllBytes(char const* filename)
	{
		using namespace std;
		ifstream ifs(filename, std::ios::binary | std::ios::ate);
		ifstream::pos_type pos = ifs.tellg();
		if (pos<0) {
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
	Texture* whiteTexture = nullptr;
	Texture* sphereTexture = nullptr;

	Texture::Texture(const char *data, int width, int height, uint32_t format)
		: width{ width }, height{ height } {

		glGenTextures(1, &textureId);

		target = GL_TEXTURE_2D;
		GLint mipmapLevel = 0;
		GLint internalFormat = format;
		GLint border = 0;
		GLenum type = GL_UNSIGNED_BYTE;
		glBindTexture(target, textureId);
		glTexImage2D(target, mipmapLevel, internalFormat, width, height, border, format, type, data);

		// update stats
		RenderStats& renderStats = SimpleRenderEngine::instance->renderStats;
		renderStats.textureCount++;
		renderStats.textureBytes += getDataSize();
	}

	GLenum getFormat(SDL_Surface *image) {
		SDL_PixelFormat *format = image->format;
		auto pixelFormat = format->format;
		bool isBGR = format->Rshift == 16;

#ifdef EMSCRIPTEN
        GLenum RGB = GL_RGB;
        GLenum RGBA = GL_RGBA;
#else
        GLenum RGB = isBGR ? GL_BGR : GL_RGB;
        GLenum RGBA = isBGR ? GL_BGRA : GL_RGBA;
#endif
		const bool alpha = SDL_ISPIXELFORMAT_ALPHA(pixelFormat);
		if (alpha) {
			return RGBA;
		}
		else {
			if (format->BytesPerPixel == 4) {
				return RGBA;
			}
			else if (format->BytesPerPixel == 3) {
				return RGB;
			}
			else {
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
			memcpy((Uint8 *)temp_row.get(),
				(Uint8 *)(image_pixels)+
				width * index,
				width);

			memcpy(
				(Uint8 *)(image_pixels)+
				width * index,
				(Uint8 *)(image_pixels)+
				width * (height - index - 1),
				width);
			memcpy(
				(Uint8 *)(image_pixels)+
				width * (height - index - 1),
				temp_row.get(),
				width);
		}
		return 0;
	}

	bool isPowerOfTwo(unsigned int x) {
		return ((x != 0) && !(x & (x - 1)));
	}

    Texture::~Texture() {
		// update stats
		RenderStats& renderStats = SimpleRenderEngine::instance->renderStats;
		renderStats.textureCount--;
		renderStats.textureBytes -= getDataSize();

        glDeleteTextures(1, &textureId);
    }



    Texture::TextureBuilder &Texture::TextureBuilder::withGenerateMipmaps(bool enable) {
        generateMipmaps = enable;
        return *this;
    }

    Texture::TextureBuilder &Texture::TextureBuilder::withFilterSampling(bool enable) {
        this->filterSampling = enable;
        return *this;
    }

    Texture::TextureBuilder &Texture::TextureBuilder::withWrappedTextureCoordinates(bool enable) {
        this->wrapTextureCoordinates = enable;
        return *this;
    }

    Texture::TextureBuilder &Texture::TextureBuilder::withFile(const char *filename) {
        this->filename = filename;
        return *this;
    }

    Texture::TextureBuilder &Texture::TextureBuilder::withRGBData(const char *data, int width, int height) {
        this->data = data;
        this->width = width;
        this->height = height;
        this->format = GL_RGB;
        return *this;
    }

    Texture::TextureBuilder &Texture::TextureBuilder::withRGBAData(const char *data, int width, int height) {
        this->data = data;
        this->width = width;
        this->height = height;
        this->format = GL_RGBA;
        return *this;
    }

    std::vector<char> loadFileFromMemory(const char* data, int dataSize, GLenum& format, int& width, int& height){
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

        SDL_RWops *source = SDL_RWFromConstMem(data, dataSize);
        SDL_Surface *res_texture = IMG_Load_RW(source, 1);
        if (res_texture == NULL) {
            std::cerr << "IMG_Load: " << SDL_GetError() << std::endl;
            return {};
        }
        width = res_texture->w;
        height = res_texture->h;
        format = getFormat(res_texture);

        int bytesPerPixel =
#ifndef EMSCRIPTEN
                format == GL_BGR ||
                #endif
                format == GL_RGB ? 3 : 4;
        char *pixels = static_cast<char *>(res_texture->pixels);
        invert_image(width*bytesPerPixel, height, pixels);

        std::vector<char> res(pixels, pixels+width*bytesPerPixel*height);

        SDL_FreeSurface(res_texture);
        return res;
    }

    Texture *Texture::TextureBuilder::build() {
        std::vector<char> fileData;
        if (filename != nullptr){
            fileData = readAllBytes(filename);
            fileData = loadFileFromMemory(fileData.data(), (int) fileData.size(), format, width, height);
            data = fileData.data();
        }
		if (data == nullptr && dataOwned.size()>0){
			data = dataOwned.data();
		}
        Texture* res = new Texture(data, width, height, format);
        if (this->generateMipmaps){
            res->invokeGenerateMipmap();
        }
        res->updateTextureSampler(filterSampling, wrapTextureCoordinates);
        return res;
    }

	Texture::TextureBuilder &Texture::TextureBuilder::withWhiteData(int width, int height) {
		char one = (char)0xff;
		dataOwned = std::vector<char>(width * height * 4, one);
		this->width = width;
		this->height = height;
		format = GL_RGBA;
		return *this;
	}

	// returns true if texture sampling should be filtered (bi-linear or tri-linear sampling) otherwise use point sampling.
	bool Texture::isFilterSampling() {
		return filterSampling;
	}

	bool Texture::isWrapTextureCoordinates() {
		return filterSampling;
	}

	int Texture::getWidth() {
		return width;
	}

	int Texture::getHeight() {
		return height;
	}

	void Texture::updateTextureSampler(bool filterSampling, bool wrapTextureCoordinates) {
        this->filterSampling = filterSampling;
        this->wrapTextureCoordinates = wrapTextureCoordinates;
		glBindTexture(target, textureId);
		glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapTextureCoordinates ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapTextureCoordinates ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		GLuint minification;
		GLuint magnification;
		if (!filterSampling) {
			minification = GL_NEAREST;
			magnification = GL_NEAREST;
		}
		else if (generateMipmap) {
			minification = GL_LINEAR_MIPMAP_LINEAR;
			magnification = GL_LINEAR;
		}
		else {
			minification = GL_LINEAR;
			magnification = GL_LINEAR;
		}
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magnification);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minification);
	}

	Texture *Texture::getWhiteTexture() {
		if (whiteTexture != nullptr) {
			return whiteTexture;
		}
		whiteTexture = create()
                .withWhiteData()
				.withFilterSampling(false)
                .build();
        return whiteTexture;
	}

	Texture *Texture::getSphereTexture() {
		if (sphereTexture != nullptr) {
			return sphereTexture;
		}
		int size = 128;
		char one = (char)0xff;
		std::vector<char> data(size*size * 4, one);
		for (int x = 0; x<size; x++) {
			for (int y = 0; y<size; y++) {
				float distToCenter = glm::clamp(1.0f - 2.0f*glm::length(glm::vec2((x + 0.5f) / size, (y + 0.5f) / size) - glm::vec2(0.5f, 0.5f)), 0.0f, 1.0f);
				data[x*size * 4 + y * 4 + 0] = (char)(255 * distToCenter);
				data[x*size * 4 + y * 4 + 1] = (char)(255 * distToCenter);
				data[x*size * 4 + y * 4 + 2] = (char)(255 * distToCenter);
				data[x*size * 4 + y * 4 + 3] = (char)255;
			}
		}
		sphereTexture = create()
                .withRGBAData(data.data(),size,size)
                .withGenerateMipmaps(true)
                .build();
		return sphereTexture;
	}

    Texture::TextureBuilder Texture::create() {
        return Texture::TextureBuilder();
    }

    void Texture::invokeGenerateMipmap() {
        if ((!isPowerOfTwo((unsigned int)width) || !isPowerOfTwo((unsigned int)height))) {
            std::cerr << "Ignore mipmaps for textures not power of two" << std::endl;
        } else {
            this->generateMipmap = true;
            glGenerateMipmap(target);
        }
    }

	int Texture::getDataSize() {
		int res = width * height * 4; //
		if (generateMipmap){
			res += (int)((1.0f/3.0f) * res);
		}
		// six sides
		if (target == GL_TEXTURE_CUBE_MAP){
			res *= 6;
		}
		return res;
	}
}
