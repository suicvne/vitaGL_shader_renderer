#ifndef __VGL_RENDERER_H__
#define __VGL_RENDERER_H__


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

#define VERTEX_ATTRIB_TOTAL_SIZE_1 (VERTEX_ATTR_ELEM_COUNT * sizeof(float)) + (sizeof(void *))

typedef unsigned int GLuint;

typedef struct _vert 
{
    float x, y;
    float s, v; // Tex Coord X, Tex Coord Y
    float _r, _g, _b, _a;

    void *obj_ptr;
} __attribute__ ((packed)) vert;

typedef struct _obj_extra_data 
{
    GLuint textureID;
    float piv_x;
    float piv_y;
    float rot_x;
    float rot_y;
    float rot_z;
    float scale;
} __attribute__ ((packed)) obj_extra_data;

typedef struct _DrawCall
{
    struct _vert verts[VERTICES_PER_PRIM];
    // GLuint textureID;
    // float piv_x;
    // float piv_y;
    // float rot_x; 
    // float rot_y; 
    // float rot_z;
    // float scale;
} __attribute__ ((packed)) DrawCall;

static const char* GLINVALIDENUM = "GL_INVALID_ENUM";
static const char* GLINVALIDVALUE = "GL_INVALID_VALUE";
static const char* GLINVALIDOP = "GL_INVALID_OPERATION";
static const char* GLOUTOFMEM = "GL_OUT_OF_MEMORY";
static const char* GLSTACKUNDER = "GL_STACK_UNDERFLOW";
static const char* GLSTACKOVER = "GL_STACK_OVERFLOW";
static const char* GLUNKNOWN = "GL_UNKNOWN. Sorry.";

static inline void DEBUG_PRINT_OBJ_EX_DATA(struct _obj_extra_data *ex_data)
{
    return;
    if(ex_data == NULL) return;

    printf("\tEX_DATA\n\ttex ID: %d\n\tpiv: %.2f, %.2f\n\trot: %.2f, %.2f, %.2f\n\tscale: %.2f\n\n",
        ex_data->textureID,
        ex_data->piv_x,
        ex_data->piv_y,
        ex_data->rot_x,
        ex_data->rot_y,
        ex_data->rot_z,
        ex_data->scale
    );
}

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

#ifndef VITA
    memset(error_buffer, 0, sizeof(error_buffer));
    const char* _eb = (char*)error_buffer;
    if(glfwGetError(&_eb))
    {
        printf("\t\tGLFW: %s", error_buffer);
    }
#endif
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
int initGL(void (*dbgPrintFn)(const char*, ...));
int initGLAdv();
int initGLShading();
int initGLShading2(char* vertex_shader, char* fragment_shader);
int deInitGL();

// TODO: Function prefixes for these.
void clear();
void repaint();

void Vita_Draw(float x, float y, float wDst, float hDst);


void Vita_DrawRectColor(float x, float y,
                        float wDst, float hDst,
                        float _r,
                        float _g,
                        float _b,
                        float _a
);

void Vita_DrawRectColorExData(float x, float y,
                           float wDst, float hDst,
                           float _r,
                           float _g,
                           float _b,
                           float _a,
                           struct _obj_extra_data *ex_data
);

/*
void Vita_DrawTextureAnimColorRotScale(
    float x,
    float y,
    float wDst,
    float hDst,
    GLuint texId,
    float tex_w,
    float tex_h,
    float src_x,
    float src_y,
    float src_w,
    float src_h,
    float _r,
    float _g,
    float _b,
    float _a,
    float _rot,
    float _scale
);
*/

// Formerly Vita_DrawTextureAnimColorRot

// Change to take ex data
void Vita_DrawTextureAnimColorExData(
    float x,
    float y,
    float wDst,
    float hDst,
    GLuint texId,
    float tex_w,
    float tex_h,
    float src_x,
    float src_y,
    float src_w,
    float src_h,
    float _r,
    float _g,
    float _b,
    float _a,
    struct _obj_extra_data *ex_data
);

void Vita_DrawTextureAnimColor(
    float x,
    float y,
    float wDst,
    float hDst,
    GLuint texId,
    float tex_w,
    float tex_h,
    float src_x,
    float src_y,
    float src_w,
    float src_h,
    float _r,
    float _g,
    float _b,
    float _a
);

// ------------------------------------- End Functions for the public

#endif //__VGL_RENDERER_H__