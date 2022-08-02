
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PSD
#define STBI_ONLY_BMP
#define STBI_ONLY_GIF
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_PNM
#define STBI_ONLY_TGA
#include "stb_image.h"


#if defined(__APPLE__) || defined(PC_BUILD) 
#include <GL/glew.h>

#ifndef __APPLE__
#include <GL/gl.h>
#else
#include <OpenGL/glu.h>
#endif

#include <GLFW/glfw3.h>
#else
#include <vitasdk.h>
#include <vitaGL.h>
#endif

/**
 * Vita_LoadTextureGL:
 *  Loads a single image from a buffer into OpenGL.
 * 
 * returns:
 *  GLuint with the texture handle, 0 if the image failed to load.
 *  After this function returns, it is safe to free the image buffer.
 */
static inline GLuint Vita_LoadTextureGL(void* buffer, 
    float width, 
    float height,
    void (*debugPrintf)(const char*, ...)
)
{
    if(buffer == 0 || (width <= 0 || height <= 0))
    {
        if(buffer == 0)
            debugPrintf("[load_texture] buffer == 0. (%x, %p)\n", buffer, buffer);
        if(width <= 0)
            debugPrintf("[load_texture] Width <= 0 (%.2f)\n", width);
        if(height <= 0)
            debugPrintf("[load_texture] Height <= 0 (%.2f)\n", height);

        return 0;
    }

    GLuint returnValue;
    glGenTextures(1, &returnValue);

    glBindTexture(GL_TEXTURE_2D, returnValue);

    glTexImage2D(GL_TEXTURE_2D, 0, 
                    GL_RGBA, 
                    width, height, 
                    0, 
                    GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*)(buffer));

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    debugPrintf("Wrap: GL_CLAMP_TO_BORDER\n");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    debugPrintf("[load_texture] OK! GLuint: %u\n", returnValue);
    return returnValue;
}

static inline int Vita_LoadTextureBuffer(const char* path, 
    void** buffer, 
    int* w, 
    int* h, 
    int* channels, 
    void(*debugPrintf)(const char*, ...)
)
{
    stbi_set_flip_vertically_on_load(0);

    stbi_uc* uc = NULL;
    *buffer = stbi_load(path, w, h, channels, STBI_rgb_alpha);
    
    size_t newSize = ((*w) * (*h) * (*channels));
    debugPrintf("[Vita_LoadTexture] src size:%d --- Reallocating buffer from size %d to %d (img size: %d x %d)\n", sizeof(uc), sizeof(void*), newSize, *w, *h);

    // *buffer = realloc(*buffer, newSize);
    // memcpy(*buffer, uc, newSize);
    if(*buffer == 0)
    {
        debugPrintf("Vita_LoadTexture failed: returned buffer is 0 (returned buffer is at %x)\n", (*buffer));
        return -1;
    }

    stbi_image_free(uc);
    debugPrintf("[Vita_LoadTexture] %s has %d channels.\n", path, *channels);
    debugPrintf("[Vita_LoadTexture] buffer: %p\n", (*buffer));
    return 0;
}

#endif