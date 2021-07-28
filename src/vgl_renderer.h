#ifndef __VGL_RENDERER_H__
#define __VGL_RENDERER_H__


#ifdef __APPLE__
#include <GL/glew.h>
#include <OpenGL/glu.h>
#include <GLFW/glfw3.h>
#else
#include <vitasdk.h>
#include <vitaGL.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#define VERTICES_PER_QUAD 4
#define VERTICES_PER_TRI 3
#define VERTICES_PER_PRIM VERTICES_PER_QUAD

#define VERTEX_POS_SIZE 2 // Number of elements in our position (2: (x, y))
#define VERTEX_TEXCOORD_SIZE 2 // Number of elements in our tex coords. (2: (s, v))
#define VERTEX_COLOR_SIZE 4 // The number of elements in our vertex color attribute. (4: (r, g, b, a))

#define VERTEX_POS_OFFSET 0 // How many elements into one block will we find this property?
#define VERTEX_TEXCOORD_OFFSET 2 // 2 elements in (after x, y)
#define VERTEX_COLOR_OFFSET 4 // 4 elements in (after x, y and s, v)

// How many components are in the _vert.
// for just x & y, it's 2. this is used for stride.
// In this case, we have (x, y) (s, v) and (r,g,b,a) available to us.
#define VERTEX_ATTR_ELEM_COUNT 8
#define MAX_VERTICES 1024 // TODO: This should be renamed to MAX_DRAWCALLS. We allocate our VBO with memory to fill MAX_VERTICES * sizeof(DrawCall)

typedef unsigned int GLuint;

typedef struct _vert 
{
    float x, y;
    float s, v; // Tex Coord X, Tex Coord Y
    float _r, _g, _b, _a;
} __attribute__ ((packed)) vert;

typedef struct _DrawCall
{
    struct _vert verts[VERTICES_PER_PRIM];
    GLuint textureID;
} __attribute__ ((packed)) DrawCall;

static const char* GLINVALIDENUM = "GL_INVALID_ENUM";
static const char* GLINVALIDVALUE = "GL_INVALID_VALUE";
static const char* GLINVALIDOP = "GL_INVALID_OPERATION";
static const char* GLOUTOFMEM = "GL_OUT_OF_MEMORY";
static const char* GLSTACKUNDER = "GL_STACK_UNDERFLOW";
static const char* GLSTACKOVER = "GL_STACK_OVERFLOW";
static const char* GLUNKNOWN = "GL_UNKNOWN. Sorry.";

static inline void GET_GL_ERROR(GLenum error, char* output)
{
    switch(error)
    {
    case GL_INVALID_ENUM:
        memcpy(output, GLINVALIDENUM, strlen(GLINVALIDENUM));
        break;
    case GL_INVALID_VALUE:
        memcpy(output, GLINVALIDVALUE, strlen(GLINVALIDVALUE));
        break;
    case GL_INVALID_OPERATION:
        memcpy(output, GLINVALIDVALUE, strlen(GLINVALIDOP));
        break;
    case GL_OUT_OF_MEMORY:
        memcpy(output, GLOUTOFMEM, strlen(GLOUTOFMEM));
        break;
    case GL_STACK_UNDERFLOW:
        memcpy(output, GLSTACKUNDER, strlen(GLSTACKUNDER));
        break;
    case GL_STACK_OVERFLOW:
        memcpy(output, GLSTACKOVER, strlen(GLSTACKOVER));
        break;
    }

    memcpy(output, GLUNKNOWN, strlen(GLUNKNOWN));
}

static inline void CHECK_GL_ERROR(char* prefix) 
{
    GLenum gl_error = 0;
    char error_buffer[128];
    if((gl_error = glGetError()) != GL_NO_ERROR)
    {
        GET_GL_ERROR(gl_error, error_buffer);
        printf("[%s] OPENGL ERROR: %s\n", prefix, error_buffer);
    }
}

static inline void pLogDebug(const char* a, ...)
{
    (void)a;
}

static inline void _printGLVersion()
{
    GLenum enums[] = { GL_VERSION, GL_SHADING_LANGUAGE_VERSION, GL_VENDOR, GL_RENDERER };

    const unsigned char* temp_gl_string;
    for(int i = 0; i < 4; i++)
    {
        temp_gl_string = glGetString(enums[i]);

        switch(enums[i])
        {
        case GL_VERSION:
            printf("\n\nOpenGL Version: %s\n", temp_gl_string);
            break;
        case GL_SHADING_LANGUAGE_VERSION:
#ifdef VITA
            printf("GLSL (CG Shaders for Vita) Ver: %s\n", temp_gl_string);
#else
            printf("GLSL Language Version: %s\n", temp_gl_string);
#endif
            break;
        case GL_VENDOR:
            printf("GL Vendor: %s\n", temp_gl_string);
            break;
        case GL_RENDERER:
            printf("GL Renderer: %s\n", temp_gl_string);
            break;
        }
    }

    free(enums);
}

// ------------------------------------- Functions for the public

// TODO: Combine the 3 inits into one init function?
// TODO: Function prefixes.
int initGL();
int initGLAdv();
int initGLShading();
int deInitGL();

// TODO: Function prefixes for these.
void clear();
void repaint();

void Vita_Draw(float x, float y, float wDst, float hDst);

// ------------------------------------- End Functions for the public

#endif //__VGL_RENDERER_H__