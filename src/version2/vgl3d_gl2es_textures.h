#ifndef __VGL3D_TEXTURES__
#define __VGL3D_TEXTURES__

// Forward declare context.
struct _VGL3D;

/* ========== STBI LIBRARY =========== */
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
#include "../stb_image.h"
#endif
/* ========== STBI LIBRARY =========== */

#ifndef SELF
#define SELF struct _VGL3D* context
#endif

/**
 * @brief Loads a PNG texture at a given path into *buffer.
 * w, h, channels are filled in by stbi as it loads the image.
 * 
 * Usage:
 * void* tex_buffer; // Will be allocated as needed by stbi.
 * int w = 0, h = 0, channels = 0;
 * _VGL3D_LoadTextureBufferPNG(&graphics, texPath, &tex_buffer, &w, &h, &channels);
 * // tex_buffer will now contain raw texture data to be sent to GPU.
 * 
 * @param context 
 * @param path 
 * @param buffer 
 * @param w 
 * @param h 
 * @param channels 
 * @return int 
 */
static inline int _VGL3D_LoadTextureBufferPNG(
    SELF,
    const char* path,
    void** buffer,
    int* w,
    int* h,
    int* channels)
{
    stbi_set_flip_vertically_on_load(0);

    stbi_uc* uc = NULL;
    *buffer = stbi_load(path, w, h, channels, STBI_rgb_alpha);

    size_t texBufSize = ((*w) * (*h) * (*channels));
    if(*buffer == NULL)
    {
        context->Log(context, "LoadTextureBufferPNG failed: returned buffer is 0.");
        return -1;
    }

    stbi_image_free(uc);
    context->Log(context, "LoadTextureBufferPNG: %d x %d (%d channels).", *w, *h, *channels);
    return 0;
}

static inline VTEX _VGL3D_MakeGLTexture(SELF, void* buffer, float width, float height) {
    if(buffer == 0 || (width <= 0 || height <= 0)) {
        context->Log(context, "Attempted to make GLTexture with invalid data (buffer: %p; %.2f x %.2f)", buffer, width, height);
        return -1;
    }

    // Create texture handle
    VTEX newGlTex;
    glGenTextures(1, &newGlTex);

    // Bind the texture handle
    glBindTexture(GL_TEXTURE_2D, newGlTex);

    // Upload raw texture from buffer onto the texture currently bound.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*)(buffer));

    // Setup sane default texture parameters.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    context->Log(context, "GLTexture created with handle %d\n", newGlTex);
    return newGlTex;
}

static inline VTEX _VGL3D_LoadAndCreateGLTexture(SELF, const char *path)
{
    void* rawTexBuffer;
    int channels = 0, w = 0, h = 0;

    // Load raw texture data.
    _VGL3D_LoadTextureBufferPNG(context, path, &rawTexBuffer, &w, &h, &channels);
    if(w == 0 && h == 0)
        return 0;
    
    // Make raw texture buffer 
    VTEX newTex = _VGL3D_MakeGLTexture(context, rawTexBuffer, (float)w, (float)h);

    // Free buffer
    if(rawTexBuffer != NULL)
        free(rawTexBuffer);

    // No texture.
    return newTex;
}

#undef SELF
#endif // __VGL3D_TEXTURES__