/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#include "sre/Texture.hpp"

#include "sre/impl/GL.hpp"

#include <algorithm>
#include <SDL_surface.h>

#include <SDL_image.h>
#include <SDL_pixels.h>
// define pixel format for old SDL2Image
#ifndef SDL_PIXELFORMAT_RGBA32
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#   define SDL_PIXELFORMAT_RGBA32 SDL_PIXELFORMAT_RGBA8888
#   define SDL_PIXELFORMAT_ARGB32 SDL_PIXELFORMAT_ARGB8888
#   define SDL_PIXELFORMAT_BGRA32 SDL_PIXELFORMAT_BGRA8888
#   define SDL_PIXELFORMAT_ABGR32 SDL_PIXELFORMAT_ABGR8888
#else
#   define SDL_PIXELFORMAT_RGBA32 SDL_PIXELFORMAT_ABGR8888
#   define SDL_PIXELFORMAT_ARGB32 SDL_PIXELFORMAT_BGRA8888
#   define SDL_PIXELFORMAT_BGRA32 SDL_PIXELFORMAT_ARGB8888
#   define SDL_PIXELFORMAT_ABGR32 SDL_PIXELFORMAT_RGBA8888
#endif
#endif
#include <fstream>
#include "sre/RenderStats.hpp"
#include "sre/Renderer.hpp"

#ifndef GL_SRGB_ALPHA
#define GL_SRGB_ALPHA 0x8C42
#endif
#ifndef GL_SRGB
#define GL_SRGB 0x8C40
#endif
#include <iomanip>

#include "sre/Log.hpp"

// anonymous (file local) namespace
namespace {
	static std::vector<char> readAllBytes(char const* filename)
	{
		using namespace std;
		ifstream ifs(filename, std::ios::binary | std::ios::ate);
		ifstream::pos_type pos = ifs.tellg();
		if (pos<0) {
            LOG_ERROR("Cannot read texture from %s",filename);
			return std::vector<char>();
		}
		std::vector<char>  result((size_t)pos);

		ifs.seekg(0, ios::beg);
		ifs.read(&result[0], pos);
        ifs.close();

		return result;
	}

	bool isAlpha(SDL_PixelFormat *format)
	{
		if (SDL_ISPIXELFORMAT_ALPHA(format->format))
		{
			return true;
		}
		if (SDL_ISPIXELFORMAT_INDEXED(format->format))
		{
			int ncolors = format->palette->ncolors;
			SDL_Color *colors = format->palette->colors;
			for (int i=0;i<ncolors;i++)
			{
				if (colors[i].a != 255)
				{
					return true;
				}
			}
		}
		return false;
	}



    int invert_image(int width, int height, void *image_pixels) {
        auto temp_row = std::unique_ptr<char>(new char[width]);
        if (temp_row.get() == nullptr) {
            LOG_ERROR("Not enough memory for image inversion");
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


}

namespace sre {
	std::shared_ptr<Texture> whiteTexture;
    std::shared_ptr<Texture> whiteCubemapTexture;
    std::shared_ptr<Texture> sphereTexture;

	Texture::Texture(unsigned int textureId, int width, int height, uint32_t target, std::string name)
    	: width{ width }, height{ height }, target{ target}, textureId{textureId},name{name} {
        if (! Renderer::instance ){
            LOG_FATAL("Cannot instantiate sre::Texture before sre::Renderer is created.");
        }
		// update stats
		RenderStats& renderStats = Renderer::instance->renderStats;
		renderStats.textureCount++;
        auto datasize = getDataSize();
		renderStats.textureBytes += datasize;
		renderStats.textureBytesAllocated += datasize;

        Renderer::instance->textures.emplace_back(this);
	}

	Texture::~Texture() {
        auto r = Renderer::instance;
        if (r != nullptr){
            // update stats
            RenderStats& renderStats = r->renderStats;
            renderStats.textureCount--;
            auto datasize = getDataSize();
            renderStats.textureBytes -= datasize;
            renderStats.textureBytesDeallocated += datasize;

            r->textures.erase(std::remove(r->textures.begin(), r->textures.end(), this));

            glDeleteTextures(1, &textureId);
        }

    }

    Texture::TextureBuilder &Texture::TextureBuilder::withGenerateMipmaps(bool enable) {
        generateMipmaps = enable;
        return *this;
    }

    GLenum Texture::getFormat(SDL_Surface *image) {
        SDL_PixelFormat *format = image->format;
        auto pixelFormat = format->format;

        const bool alpha = isAlpha(format);

        if (alpha) {
            return GL_RGBA;
        }
        else {
            if (format->BytesPerPixel == 4) {
                return GL_RGBA;
            }
            else if (format->BytesPerPixel == 3) {
                return GL_RGB;
            }
            else {
                LOG_ERROR("Unknown image format. Only PNG is supported.");
                return 0;
            }
        }
    }

    Texture::TextureBuilder &Texture::TextureBuilder::withFilterSampling(bool enable) {
        this->filterSampling = enable;
        return *this;
    }

    Texture::TextureBuilder &Texture::TextureBuilder::withWrappedTextureCoordinates(bool enable) {
        this->wrapUV = enable?Wrap::Repeat:Wrap::ClampToEdge;
        return *this;
    }

    Texture::TextureBuilder &Texture::TextureBuilder::withWrapUV(Texture::Wrap wrap) {
        this->wrapUV = wrap;
        return *this;
    }

    Texture::TextureBuilder &Texture::TextureBuilder::withFile(std::string filename) {
        if (name.length()==0){
            name = filename;
        }
        auto fileData = readAllBytes(filename.c_str());
        GLenum format;
        int width;
        int height;
        int bytesPerPixel;
        fileData = loadFileFromMemory(fileData.data(), (int) fileData.size(), format, this->transparent, width, height,bytesPerPixel);

        textureTypeData[GL_TEXTURE_2D] = {
                width,
                height,
                transparent,
                bytesPerPixel,
                format,
                filename,
                fileData
        };

        return *this;
    }

    Texture::TextureBuilder &Texture::TextureBuilder::withFileCubemap(std::string filename, CubemapSide side){
        auto fileData = readAllBytes(filename.c_str());
        GLenum format;
        int bytesPerPixel;
        int width;
        int height;
        fileData = loadFileFromMemory(fileData.data(), (int) fileData.size(), format,this->transparent, width, height,bytesPerPixel, false);

        textureTypeData[GL_TEXTURE_CUBE_MAP_POSITIVE_X+(unsigned int)side] = {
                width,
                height,
                transparent,
                bytesPerPixel,
                format,
                filename,
                fileData
        };

        return *this;
    }

    Texture::TextureBuilder &Texture::TextureBuilder::withRGBData(const char *data, int width, int height) {

        int bytesPerPixel = 3;
        textureTypeData[GL_TEXTURE_2D] = {
                width,
                height,
                transparent,
                bytesPerPixel,
                GL_RGB,
                "memory"
        };
        if (data != nullptr){
            auto dataSize = static_cast<size_t>(width * height * bytesPerPixel);
            textureTypeData[GL_TEXTURE_2D].data.resize(dataSize);
            memcpy(textureTypeData[GL_TEXTURE_2D].data.data(), data, dataSize);
        }

        return *this;
    }

    Texture::TextureBuilder &Texture::TextureBuilder::withRGBAData(const char *data, int width, int height) {
        int bytesPerPixel = 4;
        textureTypeData[GL_TEXTURE_2D] = {
                width,
                height,
                transparent,
                bytesPerPixel,
                GL_RGBA,
                "memory"
        };

        if (data != nullptr) {
            auto dataSize = static_cast<size_t>(width * height * bytesPerPixel);
            textureTypeData[GL_TEXTURE_2D].data.resize(dataSize);
            memcpy(textureTypeData[GL_TEXTURE_2D].data.data(), data, dataSize);
        }

        return *this;
    }
    Texture::TextureBuilder &Texture::TextureBuilder::withDepth(int width, int height, DepthPrecision precision) {
        depthPrecision = precision;
        textureTypeData[GL_TEXTURE_2D] = {
                width,
                height,
                false,
                0,
                0,
                "DepthTexture"
        };
        return *this;
    }

    std::shared_ptr<Texture> Texture::TextureBuilder::build() {
        if (textureId == 0){
            LOG_FATAL("Texture is already build");
        }
        if (name.length() == 0){
            name = "Unnamed Texture";
        }
        std::map<uint32_t, TextureDefinition>::iterator val;
        TextureDefinition* textureDefPtr;
        if (depthPrecision != DepthPrecision::None){
            if (renderInfo().graphicsAPIVersionES && renderInfo().graphicsAPIVersionMajor <= 2){
                LOG_FATAL("Depth texture not supported");
            } else {
                this->target = GL_TEXTURE_2D;
                GLint internalFormat;
                GLint format = GL_DEPTH_COMPONENT;
                GLenum type = GL_UNSIGNED_BYTE;
                if (depthPrecision == DepthPrecision::I16){
                    internalFormat = GL_DEPTH_COMPONENT16;
                    type = GL_UNSIGNED_SHORT;
                } else if (depthPrecision == DepthPrecision::I24){
                    internalFormat = GL_DEPTH_COMPONENT24;
                    type = GL_UNSIGNED_INT;
#ifndef GL_ES_VERSION_2_0
                } else if (depthPrecision == DepthPrecision::I32){
                    internalFormat = GL_DEPTH_COMPONENT32;
                    type = GL_UNSIGNED_INT;
#endif
                } else if (depthPrecision == DepthPrecision::I24_STENCIL8){
                    internalFormat =  GL_DEPTH24_STENCIL8;
                    type = GL_UNSIGNED_INT;
                    format = GL_DEPTH_STENCIL;
                } else if (depthPrecision == DepthPrecision::F32_STENCIL8){
                    internalFormat = GL_DEPTH32F_STENCIL8;
                    type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
                    format = GL_DEPTH_STENCIL;
                } else if (depthPrecision == DepthPrecision::F32){
                    internalFormat = GL_DEPTH_COMPONENT32F;
                    type = GL_FLOAT;
#ifndef GL_ES_VERSION_2_0
                } else if (depthPrecision == DepthPrecision::STENCIL8){
                    internalFormat = GL_STENCIL_INDEX8;
                    type = GL_UNSIGNED_BYTE;
                    format = GL_STENCIL_INDEX;
#endif
                } else {
                    assert(false && "Invalid depth/stencil format");
                }
                GLint border = 0;

                glBindTexture(target, textureId);
                auto td = textureTypeData.find(GL_TEXTURE_2D);
                textureDefPtr = &td->second;
                glTexImage2D(target, 0, internalFormat, textureDefPtr->width,
                             textureDefPtr->height, border, format, type, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
#ifndef GL_ES_VERSION_2_0
                glm::vec4 ones(1.0, 1.0, 1.0, 1.0);
                glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &ones.x);
#endif
            }
        } else if ((val = textureTypeData.find(GL_TEXTURE_2D)) != textureTypeData.end()){
            auto& textureDef = val->second;
            textureDefPtr = &textureDef;
            this->target = GL_TEXTURE_2D;
            // create texture
            GLint mipmapLevel = 0;

            GLint internalFormat;
            if (samplerColorspace == SamplerColorspace::Linear){
                internalFormat = textureDef.bytesPerPixel==4?GL_SRGB_ALPHA:GL_SRGB;
            } else {
                internalFormat = textureDef.bytesPerPixel==4?GL_RGBA:GL_RGB;
            }

            GLint border = 0;

            bool isPOT = isPowerOfTwo(textureDef.width) && isPowerOfTwo(textureDef.height);
            if (!isPOT && filterSampling){
                LOG_WARNING("Texture %s is not power of two (was %i x %i ). filter sampling ",textureDef.resourcename.c_str(), textureDef.width, textureDef.height);
                filterSampling = false;
            }
            if (!isPOT && generateMipmaps){
                LOG_WARNING("Texture %s is not power of two (was %i x %i ). mipmapping disabled ",textureDef.resourcename.c_str(), textureDef.width, textureDef.height);
                generateMipmaps = false;
            }

            GLenum type = GL_UNSIGNED_BYTE;
            glBindTexture(target, textureId);
            void* dataPtr = textureDef.data.size()>0?textureDef.data.data(): nullptr;
            if (this->dumpDebug){
                textureDef.dumpDebug();
            }
            glTexImage2D(target, mipmapLevel, internalFormat, textureDef.width, textureDef.height, border, textureDef.format, type, dataPtr);
        } else {
            for (int i=0;i<6;i++){
                if ((val = textureTypeData.find(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i)) != textureTypeData.end()) {
                    auto &textureDef = val->second;
                    textureDefPtr = &textureDef;
                    this->target = GL_TEXTURE_CUBE_MAP;
                    GLint mipmapLevel = 0;

                    GLint internalFormat;
                    if (samplerColorspace == SamplerColorspace::Linear){
                        internalFormat = textureDef.bytesPerPixel==4?GL_SRGB_ALPHA:GL_SRGB;
                    } else {
                        internalFormat = textureDef.bytesPerPixel==4?GL_RGBA:GL_RGB;
                    }

                    GLint border = 0;
                    GLenum type = GL_UNSIGNED_BYTE;
                    glBindTexture(target, textureId);
                    void* dataPtr = textureDef.data.size()>0?textureDef.data.data() : nullptr;
                    if (this->dumpDebug){
                        textureDef.dumpDebug();
                    }
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (unsigned int) i, mipmapLevel, internalFormat,
                                 textureDef.width, textureDef.height, border, textureDef.format, type, dataPtr);
                }
            }
        }
        if (target == 0){
            LOG_FATAL("Texture contain no data");
            return {};
        }

        // build texture
        Texture * res = new Texture(textureId, textureDefPtr->width, textureDefPtr->height, target, name);
        res->generateMipmap = this->generateMipmaps;
		res->transparent = this->transparent;
		res->samplerColorspace = this->samplerColorspace;
		res->depthPrecision = this->depthPrecision;
		res->wrapUV = this->wrapUV;
        if (this->generateMipmaps){
            res->invokeGenerateMipmap();
        }
        res->updateTextureSampler(filterSampling, wrapUV);
		
        textureId = 0;
        return std::shared_ptr<Texture>(res);
    }

	Texture::TextureBuilder &Texture::TextureBuilder::withWhiteData(int width, int height) {
		auto one = (char)0xff;
        std::vector<char> dataOwned (width * height * 4, one);
        withRGBAData(dataOwned.data(), width, height);
		return *this;
	}

    Texture::TextureBuilder &Texture::TextureBuilder::withSamplerColorspace(SamplerColorspace samplerColorspace) {
		this->samplerColorspace = samplerColorspace;
		return *this;
	}

    Texture::TextureBuilder &Texture::TextureBuilder::withWhiteCubemapData(int width, int height) {
		auto one = (char)0xff;
        std::vector<char> dataOwned (width * height * 4, one);
        int bytesPerPixel = 4;
        GLenum format = GL_RGBA;
        for (int i=0;i<6;i++){
            textureTypeData[GL_TEXTURE_CUBE_MAP_POSITIVE_X+(unsigned int)i] = {
                    width,
                    height,
                    transparent,
                    bytesPerPixel,
                    format,
                    "CubeWhite",
                    dataOwned
            };
        }


		return *this;
	}

    Texture::TextureBuilder::TextureBuilder() {
        glGenTextures(1, &textureId);
        if (renderInfo().supportTextureSamplerSRGB == false) {
            samplerColorspace = SamplerColorspace::Gamma;
        }
    }

    Texture::TextureBuilder::~TextureBuilder() {
	    if (Renderer::instance){
            if (textureId != 0){
                glDeleteTextures(1, &textureId);
            }
        }
    }

    Texture::TextureBuilder &Texture::TextureBuilder::withName(const std::string& name) {
        this->name = name;
        return *this;
    }

    Texture::TextureBuilder &Texture::TextureBuilder::withDumpDebug() {
        this->dumpDebug = true;
        return *this;
    }


    // returns true if texture sampling should be filtered (bi-linear or tri-linear sampling) otherwise use point sampling.
	bool Texture::isFilterSampling() {
		return filterSampling;
	}

	bool Texture::isWrapTextureCoordinates() {
		return wrapUV == Wrap::Repeat;
	}

	int Texture::getWidth() {
		return width;
	}

	int Texture::getHeight() {
		return height;
	}

	void Texture::updateTextureSampler(bool filterSampling, Wrap wrapTextureCoordinates) {
        this->filterSampling = filterSampling;
        this->wrapUV = wrapTextureCoordinates;
		glBindTexture(target, textureId);
		auto wrapParam = wrapTextureCoordinates == Wrap::Repeat?GL_REPEAT:
                         (wrapTextureCoordinates == Wrap::Mirror ? GL_MIRRORED_REPEAT:
#ifndef GL_ES_VERSION_2_0
                          wrapTextureCoordinates == Wrap::ClampToBorder?GL_CLAMP_TO_BORDER:GL_CLAMP_TO_EDGE);
#else
                          GL_CLAMP_TO_EDGE);
#endif

		glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapParam);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapParam);
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

    std::shared_ptr<Texture> Texture::getWhiteTexture() {
		if (whiteTexture != nullptr) {
			return whiteTexture;
		}
		whiteTexture = create()
                .withWhiteData()
				.withFilterSampling(false)
				.withName("SRE Default White")
                .build();
        return whiteTexture;
	}

    std::shared_ptr<Texture> Texture::getSphereTexture() {
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
                .withName("SRE Default Sphere")
                .build();
		return sphereTexture;
	}

    Texture::TextureBuilder Texture::create() {
        return Texture::TextureBuilder();
    }

    void Texture::invokeGenerateMipmap() {
        if ((!isPowerOfTwo((unsigned int)width) || !isPowerOfTwo((unsigned int)height))) {
            LOG_WARNING("Ignore mipmaps for textures not power of two");
        } else {
            this->generateMipmap = true;
            glGenerateMipmap(target);
        }
    }

	int Texture::getDataSize() {
		int res = width * height * 4;
		if (generateMipmap){
			res += (int)((1.0f/3.0f) * res);
		}
		// six sides
		if (target == GL_TEXTURE_CUBE_MAP){
			res *= 6;
		}
		return res;
	}

    bool Texture::isCubemap() {
        return target == GL_TEXTURE_CUBE_MAP;
    }

    sre::Texture::Wrap Texture::getWrapUV() {
        return wrapUV;
    }

    std::shared_ptr<Texture> Texture::getDefaultCubemapTexture() {
        if (whiteCubemapTexture != nullptr) {
            return whiteCubemapTexture;
        }
        whiteCubemapTexture = create()
                .withWhiteCubemapData()
                .withFilterSampling(false)
                .withName("SRE Default Cubemap")
                .build();
        return whiteCubemapTexture;
    }


    const std::string &Texture::getName() {
        return name;
    }

	bool Texture::isTransparent()
	{
		return transparent;
	}

    bool Texture::isMipmapped() {
        return generateMipmap;
    }

    std::vector<char> Texture::loadFileFromMemory(const char* data, int dataSize, GLenum& format, bool & alpha,int& width, int& height, int& bytesPerPixel, bool invertY){
#ifndef EMSCRIPTEN
        static bool initialized = false;
        if (!initialized) {
            initialized = true;
            int flags = IMG_INIT_PNG;
            int initted = IMG_Init(flags);
            if ((initted & flags) != flags) {
                LOG_ERROR("IMG_Init: Failed to init required png support!\nIMG_Init() returned %s",IMG_GetError());
                // handle error
            }
        }
#endif

        SDL_RWops *source = SDL_RWFromConstMem(data, dataSize);
        SDL_Surface *res_texture = IMG_Load_RW(source, 1);
        if (res_texture == nullptr) {
            LOG_ERROR("Cannot load texture. IMG_Load_RW returned %s", IMG_GetError());
            return {};
        }
        alpha = isAlpha( res_texture->format );

        SDL_Surface *formattedSurf = SDL_ConvertSurfaceFormat(res_texture,
                                                              alpha ?SDL_PIXELFORMAT_RGBA32 : SDL_PIXELFORMAT_RGB24, 0);
        width = formattedSurf->w;
        height = formattedSurf->h;
        format = getFormat(formattedSurf);

        bytesPerPixel = format == GL_RGB ? 3 : 4;
        auto pixels = static_cast<char *>(formattedSurf->pixels);
        if (invertY){
            invert_image(width*bytesPerPixel, height, pixels);
        }

        std::vector<char> res(pixels, pixels+width*bytesPerPixel*height);
        SDL_FreeSurface(formattedSurf);
        SDL_FreeSurface(res_texture);

        return res;
    }

    void Texture::TextureBuilder::TextureDefinition::dumpDebug() {
        std::cout << "Width "<<width<<std::endl;
        std::cout << "Height "<<height<<std::endl;
        std::cout << "Transparent "<<transparent<<std::endl;
        std::cout << "Transparent "<<transparent<<std::endl;
        std::cout << "BytesPerPixel "<<bytesPerPixel<<std::endl;
        std::cout << "Format "<<format<<std::endl;
        std::cout << "Resourcename "<<resourcename<<std::endl;
        std::cout << "Data";
        // store formatting
        std::ios_base::fmtflags oldFlags = std::cout.flags();
        std::streamsize         oldPrec  = std::cout.precision();
        char               oldFill  = std::cout.fill();
        std::cout << std::showbase // show the 0x prefix
                  << std::internal // fill between the prefix and the number
                  << std::setfill('0') // fill with 0s
                  << std::setw(3);
        for (int i=0;i<data.size();i++){
            if (i%(width*bytesPerPixel)==0){
                std::cout << std::endl;
            }

            std::cout << (data[i]&0xFF)<<" ";
        }
        // restore formatting
        std::cout.flags(oldFlags);
        std::cout.precision(oldPrec);
        std::cout.fill(oldFill);
    }

    sre::Texture::SamplerColorspace Texture::getSamplerColorSpace() {
        return samplerColorspace;
    }

    bool Texture::isDepthTexture() {
        return depthPrecision != DepthPrecision::None;
    }

    Texture::DepthPrecision Texture::getDepthPrecision() {
        return depthPrecision;
    }

}
