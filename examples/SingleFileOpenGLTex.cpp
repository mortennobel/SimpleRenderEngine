//
// Compile for emscripten using
// emcc -Iinclude SingleFileOpenGLTex.cpp \
               -O2 -std=c++14 -s TOTAL_MEMORY=33554432 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' --preload-file examples/data -s USE_SDL=2 -o html/SingleFileOpenGLTex.html
// where the following images must be located in a subfolder
//   - examples/data/test.png
//   - examples/data/cartman.png
//   - examples/data/cube-negx.png
//   - examples/data/cube-negz.png
//
// Tested against emscripten/1.37.14

#include "SDL.h"
#include <SDL_image.h>
#ifdef EMSCRIPTEN
#   include <GLES2/gl2.h>
#   include "emscripten.h"
#else
// OSX only - change on other platforms
#   include <OpenGL/gl3.h>
#endif

#include <iostream>
#include <fstream>
#include <vector>

static bool quitting = false;
static SDL_Window *window = NULL;
static SDL_GLContext gl_context;

GLuint vertexBuffer, vertexArrayObject, shaderProgram;
GLint positionAttribute, uvAttribute;
int textures[4];

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

int LoadGLTextures( const char* filename)
{
    /* Status indicator */
    int Status = false;
    unsigned int texture;

    /* Create storage space for the texture */
    SDL_Surface *TextureImage[1];

    std::ifstream input( filename, std::ios::binary );
    // copies all data into buffer
    std::vector<char> buffer((
                              std::istreambuf_iterator<char>(input)),
                             (std::istreambuf_iterator<char>()));
    std::cout << filename <<" size "<< buffer.size()<<std::endl;

    /* Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit */
    if ( ( TextureImage[0] = IMG_Load( filename ) ) )
    {
        /* Set the status to true */
        Status = true;

        /* Create The Texture */
        glGenTextures( 1, &texture );

        /* Typical Texture Generation Using Data From The Bitmap */
        glBindTexture( GL_TEXTURE_2D, texture );

        std::cout << "Loaded "<<TextureImage[0]->w<<" "<<TextureImage[0]->h<<std::endl;


        // Enforce RGB/RGBA
        int format;
        SDL_Surface* formattedSurf;
        if (TextureImage[0]->format->BytesPerPixel==3) {

            formattedSurf = SDL_ConvertSurfaceFormat(TextureImage[0],
                                                     SDL_PIXELFORMAT_RGB24,
                                                     0);
            format = GL_RGB;
        } else {
            formattedSurf = SDL_ConvertSurfaceFormat(TextureImage[0],
                                                     SDL_PIXELFORMAT_RGBA32,
                                                     0);
            format = GL_RGBA;
        }
        invert_image(formattedSurf->w*formattedSurf->format->BytesPerPixel, formattedSurf->h, (char *) formattedSurf->pixels);

        /* Generate The Texture */
        glTexImage2D( GL_TEXTURE_2D, 0, format, formattedSurf->w,
                      formattedSurf->h, 0, format,
                      GL_UNSIGNED_BYTE, formattedSurf->pixels );

        /* Linear Filtering */
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        SDL_FreeSurface(formattedSurf);
        SDL_FreeSurface( TextureImage[0] );
    } else {
        std::cout << "Cannot load "<<filename <<std::endl;
    }

    return texture;
}

void loadBufferData(){
    // vertex position, uv
    float vertexData[24] = {
            -0.5, -0.5, 0.0, 1.0 ,  0.0, 0.0,
            -0.5,  0.5, 0.0, 1.0 ,  0.0, 1.0,
            0.5,  0.5, 0.0, 1.0 ,  1.0, 1.0,
            0.5, -0.5, 0.0, 1.0 ,  1.0, 0.0,
    };
#ifndef EMSCRIPTEN
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);
#endif
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 32 * sizeof(float), vertexData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(positionAttribute);
    glEnableVertexAttribArray(uvAttribute);
    int vertexSize =sizeof(float)*6;
    glVertexAttribPointer(positionAttribute, 4, GL_FLOAT, GL_FALSE,vertexSize , (const GLvoid *)0);
    glVertexAttribPointer(uvAttribute  , 2, GL_FLOAT, GL_FALSE, vertexSize, (const GLvoid *)(sizeof(float)*4));
}

GLuint initShader(const char* vShader, const char* fShader, const char* outputAttributeName) {
    struct Shader {
        GLenum       type;
        const char*      source;
    }  shaders[2] = {
            { GL_VERTEX_SHADER, vShader },
            { GL_FRAGMENT_SHADER, fShader }
    };

    GLuint program = glCreateProgram();

    for ( int i = 0; i < 2; ++i ) {
        Shader& s = shaders[i];
        GLuint shader = glCreateShader( s.type );
        glShaderSource( shader, 1, (const GLchar**) &s.source, NULL );
        glCompileShader( shader );

        GLint  compiled;
        glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
        if ( !compiled ) {
            std::cerr << " failed to compile:" << std::endl;
            GLint  logSize;
            glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logSize );
            char* logMsg = new char[logSize];
            glGetShaderInfoLog( shader, logSize, NULL, logMsg );
            std::cerr << logMsg << std::endl;
            delete [] logMsg;

            exit( EXIT_FAILURE );
        }

        glAttachShader( program, shader );
    }

    /* Link output */
#ifndef EMSCRIPTEN
    glBindFragDataLocation(program, 0, outputAttributeName);
#endif
    /* link  and error check */
    glLinkProgram(program);

    GLint  linked;
    glGetProgramiv( program, GL_LINK_STATUS, &linked );
    if ( !linked ) {
        std::cerr << "Shader program failed to link" << std::endl;
        GLint  logSize;
        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logSize);
        char* logMsg = new char[logSize];
        glGetProgramInfoLog( program, logSize, NULL, logMsg );
        std::cerr << logMsg << std::endl;
        delete [] logMsg;

        exit( EXIT_FAILURE );
    }

    /* use program object */
    glUseProgram(program);

    return program;
}


void loadShader(){
#ifdef EMSCRIPTEN
    const char *  vert = R"(#version 100
    uniform vec2 offset;
    attribute vec4 position;
    attribute vec2 uv;

    varying vec2 vUV;

    void main (void)
    {
        vUV = uv;
        gl_Position = position + vec4(offset,0.0,0.0);
    })";
#else
    const char *  vert = R"(#version 150
    uniform vec2 offset;
    in vec4 position;
    in vec2 uv;
    
    out vec2 vUV;
    
    void main (void)
    {
        vUV = uv;
        gl_Position = position + vec4(offset,0.0,0.0);
    })";
#endif
#ifdef EMSCRIPTEN
    const char *  frag = R"(#version 100
    precision mediump float;
    
    varying vec2 vUV;

    uniform sampler2D tex;
    
    void main(void)
    {
        gl_FragColor = texture2D(tex,vUV);
    })";
#else
    const char *  frag = R"(#version 150

    in vec2 vUV;
    out vec4 fragColor;

    uniform sampler2D tex;

    void main(void)
    {
        fragColor = texture(tex,vUV);
    })";
#endif
    shaderProgram = initShader(vert,  frag, "fragColor");

    uvAttribute = glGetAttribLocation(shaderProgram, "uv");
    if (uvAttribute < 0) {
        std::cerr << "Shader did not contain the 'color' attribute." << std::endl;
    }
    positionAttribute = glGetAttribLocation(shaderProgram, "position");
    if (positionAttribute < 0) {
        std::cerr << "Shader did not contain the 'position' attribute." << std::endl;
    }
}

void setup() {
    std::cout << "OpenGL version "<<glGetString(GL_VERSION)<<std::endl;
    loadShader();
    loadBufferData();
    textures[0] = LoadGLTextures( "examples/data/test.png");
    textures[1] = LoadGLTextures( "examples/data/cartman.png");
    textures[2] = LoadGLTextures( "examples/data/cube-negx.png");
    textures[3] = LoadGLTextures( "examples/data/cube-negz.png");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);

    glEnable(GL_DEPTH_TEST);
}

void render() {

    SDL_GL_MakeCurrent(window, gl_context);

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 0);
    int textureId = 0;
    for (int x = 0;x<2;x++){
        for (int y = 0;y<2;y++){
            glBindTexture(GL_TEXTURE_2D, textures[textureId]);
            glUniform2f(glGetUniformLocation(shaderProgram, "offset"), -0.5+x, -0.5+y);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            textureId++;
        }
    }


    SDL_GL_SwapWindow(window);

}

void update(){
    SDL_Event event;
    while( SDL_PollEvent(&event) ) {
        if(event.type == SDL_QUIT) {
            quitting = true;
        }
    }

    render();
};

int main(int argc, char *argv[]) {


#ifdef EMSCRIPTEN
    SDL_Renderer *renderer = NULL;
    SDL_CreateWindowAndRenderer(512, 512, SDL_WINDOW_OPENGL, &window, &renderer);
#else
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS) != 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // Use a core profile setup.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    window = SDL_CreateWindow("title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 512, SDL_WINDOW_OPENGL);

    gl_context = SDL_GL_CreateContext(window);
#endif
    setup();

#ifdef EMSCRIPTEN
    // register update as callback
    emscripten_set_main_loop(update, 0, 1);
#else
    while(!quitting) {

        update();
        SDL_Delay(2);

    }
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    exit(0);

#endif


} //main