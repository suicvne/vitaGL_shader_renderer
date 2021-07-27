#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <math.h>

#ifdef __APPLE__
#include <GL/glew.h>
#include <OpenGL/glu.h>
#include <GLFW/glfw3.h>
#else
#include <vitasdk.h>
#include <vitaGL.h>
#endif

#include <cglm/cglm.h>
#include <cglm/clipspace/ortho_lh_zo.h>

#include "SHADERS.h"

#define DISPLAY_WIDTH_DEF 960.f
#define DISPLAY_HEIGHT_DEF 544.f

#ifndef nullptr
#define nullptr 0
#endif

static void pLogDebug(const char* a, ...)
{
    (void)a;
}

#define VERTICES_PER_QUAD 4
#define VERTICES_PER_TRI 3
#define VERTICES_PER_PRIM VERTICES_PER_QUAD

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

#define VERTEX_POS_SIZE 2
#define VERTEX_TEXCOORD_SIZE 2
#define VERTEX_COLOR_SIZE 4

static int VERTEX_POS_INDEX = 0;
static int VERTEX_TEXCOORD_INDEX = 1;
static int VERTEX_COLOR_INDEX = 2;

#define VERTEX_POS_OFFSET 0
#define VERTEX_TEXCOORD_OFFSET 2
#define VERTEX_COLOR_OFFSET 4

// How many components are in the _vert.
// for just x & y, it's 2. this is used for stride.
#define VERTEX_ATTR_ELEM_COUNT 8
#define MAX_VERTICES 1024

#ifndef VITA
static GLFWwindow* _game_window;
#endif

mat4 cpu_mvp;

static const char* GLINVALIDENUM = "GL_INVALID_ENUM";
static const char* GLINVALIDVALUE = "GL_INVALID_VALUE";
static const char* GLINVALIDOP = "GL_INVALID_OPERATION";
static const char* GLOUTOFMEM = "GL_OUT_OF_MEMORY";
static const char* GLSTACKUNDER = "GL_STACK_UNDERFLOW";
static const char* GLSTACKOVER = "GL_STACK_OVERFLOW";
static const char* GLUNKNOWN = "GL_UNKNOWN. Sorry.";

static void GET_GL_ERROR(GLenum error, char* output)
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

    output = GLUNKNOWN;
}

static void CHECK_GL_ERROR(char* prefix) 
{
    GLenum gl_error = 0;
    char error_buffer[128];
    if((gl_error = glGetError()) != GL_NO_ERROR)
    {
        GET_GL_ERROR(gl_error, error_buffer);
        printf("[%s] OPENGL ERROR: %s\n", prefix, error_buffer);
    }
}

static const GLuint VERTEX_SIZE = VERTEX_POS_SIZE + VERTEX_TEXCOORD_SIZE + VERTEX_COLOR_SIZE;

// ------------------------------------------   SHADERS
static DrawCall *_vgl_pending_calls; // ARRAY OF PENDING DRAW CALLS (DrawCall _vgl_pending_calls[MAX_VERTICES];)
static unsigned int _vgl_pending_offset; // INDEX
static size_t _vgl_pending_total_size; // SIZE IN BYTES
static unsigned int _DrawCalls = 0; // DRAW CALL COUNT
// ------------------------------------------ END SHADERS 

// ------------------------------------------   BUFFERS
static GLint _vertexBufferID;
// ------------------------------------------ END BUFFERS

// ------------------------------------------   SHADERS
static GLint vertexShaderID;
static GLint fragmentShaderID;
static GLint programObjectID;
// ------------------------------------------ END SHADERS

static inline void 
Vita_WriteVertices(DrawCall *drawCall, 
                   float x, 
                   float y, 
                   float wDst, 
                   float hDst, 
                   float n_src_x, 
                   float n_src_x2,
                   float n_src_y,
                   float n_src_y2,
                   float _r,
                   float _g,
                   float _b,
                   float _a)
{
    if(drawCall == nullptr) return;

    drawCall->verts[0].x = (x);
    drawCall->verts[0].y = y;
    
    drawCall->verts[1].x = x;
    drawCall->verts[1].y = y + hDst;
    
    drawCall->verts[2].x = x + wDst;
    drawCall->verts[2].y = y;

    if(VERTICES_PER_PRIM == 4)
    {
        drawCall->verts[3].x = x + wDst;
        drawCall->verts[3].y = y + hDst;
    }

    drawCall->verts[0]._r = 1.0f;
    drawCall->verts[0]._g = 0.5f;
    drawCall->verts[0]._b = 0.f;
    drawCall->verts[0]._a = 1.0f;

    drawCall->verts[1]._r = 1.0f;
    drawCall->verts[1]._g = 0.f;
    drawCall->verts[1]._b = 0.5f;
    drawCall->verts[1]._a = 1.0f;

    drawCall->verts[2]._r = 0.0f;
    drawCall->verts[2]._g = 1.f;
    drawCall->verts[2]._b = 0.f;
    drawCall->verts[2]._a = 1.0f;

    drawCall->verts[3]._r = 1.0f;
    drawCall->verts[3]._g = 0.f;
    drawCall->verts[3]._b = 1.f;
    drawCall->verts[3]._a = 1.0f;
}

static inline void
Vita_Draw(float x,
          float y,
          float wDst,
          float hDst)
{
    if((_vgl_pending_offset + 1) > _vgl_pending_total_size)
    {
        printf("Ran out of draw calls. %d / %d\n", _vgl_pending_offset, _vgl_pending_total_size);
        return;
    }
    DrawCall *_curDrawCall = (_vgl_pending_calls + _vgl_pending_offset);

    Vita_WriteVertices(_curDrawCall, x, y, wDst, hDst, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f);
    // _curDrawCall->textureID = texture.texture;
    _vgl_pending_offset += 1;
    _DrawCalls++;

/*
    printf("draw %p; draw with: %.2f, %.2f; offset: %d;", (void*)_curDrawCall, x, y, _vgl_pending_offset);
    printf("\n\tfirst vert: %.2f, %.2f;\n\t2nd: %.2f, %.2f\n\t3rd: %.2f, %.2f\n\t4th: %.2f, %.2f\n", 
        _curDrawCall->verts[0].x, _curDrawCall->verts[0].y, 
        _curDrawCall->verts[1].x, _curDrawCall->verts[1].y,
        _curDrawCall->verts[2].x, _curDrawCall->verts[2].y,
        _curDrawCall->verts[3].x, _curDrawCall->verts[3].y
    );
*/

}

static GLuint Vita_GetVertexBufferID() { return _vertexBufferID; }

unsigned int Vita_GetTotalCalls()
{
    return _DrawCalls;
}

void Vita_ResetTotalCalls()
{
    _DrawCalls = 0;
    _vgl_pending_offset = 0;
}

DrawCall *Vita_GetDrawCallsPending()
{
    return _vgl_pending_calls;
}

GLuint LoadShader(GLenum type, const char *shaderSrc)
{
    GLuint shader;
    GLint compiled;
    
    shader = glCreateShader(type);

    if(shader == 0)
    { 
        printf("!!!!!!!   Unable to create shader: shader returned ID of %d\n", shader);
        return 0;
    }

    glShaderSource(shader, 1, &shaderSrc, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if(!compiled)
    {   
        printf("NOT COMPILED.\n");
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if(infoLen > 1)
        {
            char *infoLog = (char*)malloc(sizeof(char) * infoLen);

            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            printf("\n\nError Compiling Shader:\n\n%s\n", infoLog);
            free(infoLog);
        }

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

// ------------------------------------------    INIT FUNCTIONS

int initGLShading()
{
    vertexShaderID = LoadShader(GL_VERTEX_SHADER, vShaderString);
    if(vertexShaderID == 0)
    {
        printf("ERROR: vertex shader ID: %d\n", vertexShaderID);
        return -1;
    }
    else printf("V Shader ID: %d\n", vertexShaderID);

    CHECK_GL_ERROR("Vertex Shader");

    fragmentShaderID = LoadShader(GL_FRAGMENT_SHADER, vFragmentString);
    if(fragmentShaderID == 0)
    {
        printf("ERROR: frag shader ID: %d\n", fragmentShaderID);
        return -1;
    }
    else printf("F Shader ID: %d\n", fragmentShaderID);

    CHECK_GL_ERROR("Frag Shader");

    programObjectID = glCreateProgram();
    CHECK_GL_ERROR("Make Program Shader");

    if(programObjectID == 0)
    {
        printf("ERROR: Program object is 0.\n");
        return -1;
    }

    CHECK_GL_ERROR("Pre Shader Attach");
    glAttachShader(programObjectID, vertexShaderID);
    CHECK_GL_ERROR("Vertex Shader Attach");

    glAttachShader(programObjectID, fragmentShaderID);  
    CHECK_GL_ERROR("Frag Shader Attach");
#ifdef VITA
    glBindAttribLocation(programObjectID, 0 /*VERTEX_POSITION_INDEX*/, "aPosition");
#else
    glBindAttribLocation(programObjectID, 0, "vPosition");
#endif
    CHECK_GL_ERROR("Bind Attrib Location Shader");

    glLinkProgram(programObjectID);
    CHECK_GL_ERROR("LINK PROGRRAM");

    GLint linked;
    glGetProgramiv(programObjectID, GL_LINK_STATUS, &linked);
    CHECK_GL_ERROR("Get Program Info Value");

    if(!linked)
    {
        // TODO: proper error.
        printf("!!!!!! FAILED TO LINK SHADER!\n");
        return -1;
    }
    
    VERTEX_POS_INDEX = glGetAttribLocation(programObjectID, "vPosition");
    
    
    return 0;
}

int initGLAdv()
{
    _vgl_pending_total_size = sizeof(DrawCall) * MAX_VERTICES;
    _vgl_pending_calls = (DrawCall*)malloc(_vgl_pending_total_size);

    // Generate vbo
    glGenBuffers(1, &_vertexBufferID);
    printf("Gen Buffers with ID %d\n", _vertexBufferID);
    CHECK_GL_ERROR("GEN BUFFERS");

    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferID);
    printf("bind buffers\n");
    CHECK_GL_ERROR("BIND BUFFERS");

    // Initial data buffer
    glBufferData(GL_ARRAY_BUFFER, _vgl_pending_total_size, 0, GL_DYNAMIC_DRAW);
    printf("Initial Buffer Data with %d bytes\n", _vgl_pending_total_size);
    CHECK_GL_ERROR("INITIAL BUFFER DATA");
}

static void glfwError(int id, const char* description)
{
    printf("[GLFW] ERROR ID %d: %s\n", id, description);
}


int initGL()
{
#ifdef VITA
    vglInit(0x100000);
#endif
#ifndef VITA
    glewExperimental = 1;
    glfwSetErrorCallback(&glfwError);
    int glfwReturnVal = glfwInit();
    if(glfwReturnVal == GLFW_FALSE)
    {
        printf("[main] glfwReturnVal is == GLFW_FALSE. glfw init failed.");
        return -1;
    }
    printf("[main] GLFWInit: %d\n", glfwReturnVal);

    printf("Window Pointer B4: %x\n", _game_window);
    _game_window = glfwCreateWindow(960,544,"Test", 0, 0);
    printf("Window Pointer: %x\n", _game_window);
    glfwMakeContextCurrent(_game_window);

    

    int glewReturnVal = glewInit();
    if(glewReturnVal != GLEW_OK)
    {
        printf("[main] GLEWINIT FAILED!\n");
        return -1;
    }
    printf("[main] GLEWInit: %d\n", glewReturnVal);
    printf("Window Pointer BEFORE RETURN: %x\n", _game_window);
#endif

    glClearColor(.1f, .5f, .1f, 1.0f);

    // glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, DISPLAY_WIDTH_DEF, DISPLAY_HEIGHT_DEF, 0, -1, 1);

    glm_ortho_lh_zo(0, DISPLAY_WIDTH_DEF, DISPLAY_HEIGHT_DEF, 0, -1, 1, cpu_mvp);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glViewport(
        0,
        -DISPLAY_HEIGHT_DEF,
        DISPLAY_WIDTH_DEF * 2,
        DISPLAY_HEIGHT_DEF * 2
    );

    return 0;
}

// ------------------------------------------ END INIT FUNCTIONS

void clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
    Vita_ResetTotalCalls();
}

void repaint()
{
    const GLsizei stride = VERTEX_ATTR_ELEM_COUNT * sizeof(GLfloat); // Tightly packed. 4 verts per GL_TRIANGLE_STRIP
    uint32_t draw_calls = Vita_GetTotalCalls();
    if(draw_calls == 0) goto FINISH_DRAWING;
    if(draw_calls > GL_MAX_VERTEX_ATTRIBS)
    {
        printf("Too many calls (%d / %d).\n", draw_calls, GL_MAX_VERTEX_ATTRIBS);
    }

    GLuint _vbo = Vita_GetVertexBufferID();
    
    // glEnableClientState(GL_VERTEX_ARRAY);
    if(_vbo != 0)
    {
        
        struct _DrawCall *calls = Vita_GetDrawCallsPending();
        int i = 0;

        uint32_t offset = 0;
        uint32_t sizeCopy = 0;
        uint32_t gpuBufferSize = 0;

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        CHECK_GL_ERROR("bind");
        for(i = 0; i < draw_calls; i++)
        {
            
             offset = i * sizeof(calls[i].verts);
             sizeCopy = sizeof(calls[i].verts);
            
            glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &gpuBufferSize);
            if(offset < 0 || sizeCopy < 0)
            {
                printf("Error: %s%s\n", offset < 0 ? "Offset < 0" : "", sizeCopy < 0 ? ", sizeCopy < 0" : "");
                continue;
            }
            
            if(offset + sizeCopy > gpuBufferSize)
            {
                printf("offset + sizeCopy > GL_BUFFERSIZE: %d > %d\n", (offset + sizeCopy), gpuBufferSize);
                continue;
            }


            void *vertexPtr = &(calls[i].verts);
// /*
            printf("writing (size of calls[i].verts: %d) (size: %d; offset: %d) vertexPtr: %p\n", sizeof(calls[i].verts), sizeCopy, offset, vertexPtr);
            printf("\t%.2f, %.2f\n\t%.2f, %.2f\n\t%.2f, %.2f\n\t%.2f, %.2f\n", 
                ((struct _vert*)vertexPtr)->x, ((struct _vert*)vertexPtr)->y,
                ((struct _vert*)vertexPtr + 1)->x, ((struct _vert*)vertexPtr + 1)->y,
                ((struct _vert*)vertexPtr + 2)->x, ((struct _vert*)vertexPtr + 2)->y,
                ((struct _vert*)vertexPtr + 3)->x, ((struct _vert*)vertexPtr + 3)->y
            );
// */

            glBufferSubData(GL_ARRAY_BUFFER, 
                            offset, // offset
                            sizeCopy, // size
                            vertexPtr);
            CHECK_GL_ERROR("buffersub");
        }

        printf("Handled %d drawcalls. (%d vertice count)\n", draw_calls, draw_calls * VERTICES_PER_QUAD);
    }

    
    glBindBuffer(GL_ARRAY_BUFFER, _vbo); // Bind the vbo we've written to.
    glUseProgram(programObjectID); // Begin using our vert/frag shader combo (program)

    // Get index of our shader property. TODO: Do this at the beginning of our program (in initGLShading).
#ifdef VITA
    GLint vPosProp = glGetAttribLocation(programObjectID, "aPosition");
    // TODO: mvp mat4 for CG shader
#else
    GLint vPosProp = glGetAttribLocation(programObjectID, "vPosition");
    GLint vMvpProp = glGetUniformLocation(programObjectID, "mvp");
    GLint vColorProp = glGetAttribLocation(programObjectID, "vColor");
    // GLint fColorProp = glGetUniformLocation(programObjectID, "fColor");
#endif

    if(vPosProp <= -1)
    {
        printf("vPosProp returned invalid value: %d\n", vPosProp);
        return;
    }
    CHECK_GL_ERROR("vPosProp");

    if(vMvpProp <= -1)
    {
        printf("vMvpProp returned invalid value: %d\n", vMvpProp);
        return;
    }

    glEnableVertexAttribArray(vPosProp); // Enabling the property on the shader side.
    glEnableVertexAttribArray(vColorProp);

    CHECK_GL_ERROR("enable vertex attrib array 0");

    glVertexAttribPointer(vPosProp, 2, GL_FLOAT, GL_FALSE, stride, (void*)0); // Binding the data from the vbo to our vertex attrib.
    CHECK_GL_ERROR("vert attrib ptr arrays");

    glVertexAttribPointer(vColorProp, 4, GL_FLOAT, GL_FALSE, stride, (void*)(0 + (4 * sizeof(float))));
    CHECK_GL_ERROR("vert attrib ptr color");

    glUniformMatrix4fv(vMvpProp, 1, GL_FALSE, cpu_mvp);
    CHECK_GL_ERROR("glUniformMatrix4fv");

    // glUniform4f(fColorProp, (rand() % 255) / 255.f, (rand() % 255) / 255.f, rand() * 1.0f, 1.0f);
    // CHECK_GL_ERROR("glUniform4f");

    // This is a "hack around".
    // Ideally, I'd be able to batch this all at once.
    int i;
    for(i = 0; i < draw_calls; i++)
    {
        glDrawArrays(GL_TRIANGLE_STRIP, i * VERTICES_PER_PRIM, VERTICES_PER_PRIM);
    }
    
    // Reverting state.
    glDisableVertexAttribArray(vPosProp);
    glDisableVertexAttribArray(vColorProp);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glDisableVertexAttribArray(vPosProp);

FINISH_DRAWING:
#ifdef VITA
    vglSwapBuffers(GL_FALSE);
#else
    glfwSwapBuffers(_game_window);
    glfwPollEvents();
#endif

    Vita_ResetTotalCalls();
}


const GLint _scr_offset_x = (DISPLAY_WIDTH_DEF);
const GLint _scr_offset_y = (DISPLAY_HEIGHT_DEF);

float _ticks = 0;

int main()
{
    initGL();

    int shadingErrorCode = 0;

    if((shadingErrorCode = initGLShading()) != 0)
    {
        printf("ERROR INIT GL SHADING! %d\n", shadingErrorCode);
        return -1;
    }
    
    initGLAdv();
    

    int run = 1;

    while(run == 1)
    {
        // clear current vbo
        clear();

        // for(int i = 0; i < 1000; i++)
        // {
        //     Vita_Draw(
        //         (sin(_ticks) * 34) + 272.f + (sin(_ticks / 2) * (18.f * i)), 
        //         (cos(_ticks) * 34) + 256.f + (sin(16.f) * i), 
        //         30, 
        //         30
        //     );
        // }

        Vita_Draw(abs(cos(_ticks * .5f) * (300 + 0)), 0, 32, 32);
        Vita_Draw(abs(cos(_ticks * .5f) * (300 + 20)), 32, 64, 64);
        Vita_Draw(abs(cos(_ticks * .5f) * (300 + 40)), sin(_ticks) * (32 + 64) , 128, 128);

        // Draw from vbo, swap to next vbo
        repaint();

        _ticks += .077f;
    }


    return 0;
}
