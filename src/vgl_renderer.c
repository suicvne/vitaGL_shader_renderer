#include "vgl_renderer.h"

#include <cglm/cglm.h>
#include <cglm/clipspace/ortho_lh_zo.h>

#include "SHADERS.h"

#ifndef nullptr
#define nullptr 0
#endif

#ifndef VITA
static GLFWwindow* _game_window;
#endif

static void (*_debugPrintf)(const char*, ...);

static int DISPLAY_WIDTH = 960;
static int DISPLAY_HEIGHT = 544;

static int VERTEX_POS_INDEX = 0;
static int VERTEX_TEXCOORD_INDEX = 0;
static int VERTEX_COLOR_INDEX = 0;
static int VERTEX_MVP_INDEX = 3;
static int UNIFORM_ROTMAT_INDEX = 4;
static int UNIFORM_SCALE_INDEX = 5;

mat4 cpu_mvp;
mat4 _rot;
mat4 _rot_arb;
mat4 _scale;
mat4 _scale_arb;

static const GLuint VERTEX_SIZE = VERTEX_POS_SIZE + VERTEX_TEXCOORD_SIZE + VERTEX_COLOR_SIZE;

// ------------------------------------------   SHADERS
static DrawCall *_vgl_pending_calls; // ARRAY OF PENDING DRAW CALLS (DrawCall _vgl_pending_calls[MAX_VERTICES];)
static unsigned int _vgl_pending_offset; // INDEX
static size_t _vgl_pending_total_size; // SIZE IN BYTES
static unsigned int _DrawCalls = 0; // DRAW CALL COUNT
// ------------------------------------------ END SHADERS 

// ------------------------------------------   BUFFERS
static GLuint _vertexBufferID;
// ------------------------------------------ END BUFFERS

// ------------------------------------------   SHADERS
static GLint vertexShaderID;
static GLint fragmentShaderID;
static GLint programObjectID;
// ------------------------------------------ END SHADERS

/**
 * _Vita_GetAvailableDrawCall():
 *  Returns the first available draw call, or if you've asked
 *  for too many, returns NULL (0);
 */
static inline DrawCall *_Vita_GetAvailableDrawCall()
{
    if((_vgl_pending_offset + 1) > _vgl_pending_total_size)
    {
        _debugPrintf("Ran out of draw calls. %d / %zu\n", _vgl_pending_offset, _vgl_pending_total_size);
        return NULL;
    }

    return (_vgl_pending_calls + _vgl_pending_offset);
}

/**
 * _Vita_DoneWithDrawCall():
 *  Concludes the draw call by incrementing 
 *  _vgl_pending_offset and _DrawCalls variables.
 */
static inline void _Vita_DoneWithDrawCall()
{
    _vgl_pending_offset += 1;
    _DrawCalls++;
}


static inline void
_Vita_WriteVertices4xColor(DrawCall *drawCall,
                          float x,
                          float y,
                          float wDst, float hDst,
                          float n_src_x, float n_src_x2,
                          float n_src_y, float n_src_y2,
                          float rgba0[4], 
                          float rgba1[4], 
                          float rgba2[4],
                          float rgba3[4])
{
    if(drawCall == nullptr) return;

    drawCall->verts[0].x = x;
    drawCall->verts[0].y = y;
    drawCall->verts[0].s = n_src_x; // Tex Coord X
    drawCall->verts[0].v = n_src_y; // Tex Coord Y
    
    drawCall->verts[1].x = x;
    drawCall->verts[1].y = y + hDst;
    drawCall->verts[1].s = n_src_x; // Tex Coord X
    drawCall->verts[1].v = n_src_y2; // Tex Coord Y
    
    drawCall->verts[2].x = x + wDst;
    drawCall->verts[2].y = y;
    drawCall->verts[2].s = n_src_x2; // Tex Coord X
    drawCall->verts[2].v = n_src_y; // Tex Coord Y

    if(VERTICES_PER_PRIM == 4)
    {
        drawCall->verts[3].x = x + wDst;
        drawCall->verts[3].y = y + hDst;
        drawCall->verts[3].s = n_src_x2; // Tex Coord X
        drawCall->verts[3].v = n_src_y2; // Tex Coord Y
    }

    drawCall->verts[0]._r = rgba0[0];
    drawCall->verts[0]._g = rgba0[1];
    drawCall->verts[0]._b = rgba0[2];
    drawCall->verts[0]._a = rgba0[3];

    drawCall->verts[1]._r = rgba1[0];
    drawCall->verts[1]._g = rgba1[1];
    drawCall->verts[1]._b = rgba1[2];
    drawCall->verts[1]._a = rgba1[3];

    drawCall->verts[2]._r = rgba2[0];
    drawCall->verts[2]._g = rgba2[1];
    drawCall->verts[2]._b = rgba2[2];
    drawCall->verts[2]._a = rgba2[3];

    drawCall->verts[3]._r = rgba3[0];
    drawCall->verts[3]._g = rgba3[1];
    drawCall->verts[3]._b = rgba3[2];
    drawCall->verts[3]._a = rgba3[3];
}

static inline void 
_Vita_WriteVertices(DrawCall *drawCall, 
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
    float rgba0[4] = {_r, _g, _b, _a};
    _Vita_WriteVertices4xColor(drawCall, 
                              x, y, wDst, hDst, 
                              n_src_x, n_src_x2, n_src_y, n_src_y2, 
                              rgba0, rgba0, rgba0, rgba0);
}

void Vita_DrawRect4xColor(float x, float y,
                          float wDst, float hDst,
                          float rgba0[4],
                          float rgba1[4],
                          float rgba2[4],
                          float rgba3[4])
{
    DrawCall *_curDrawCall = _Vita_GetAvailableDrawCall();
    if(_curDrawCall == 0) {return;}

    _Vita_WriteVertices4xColor(_curDrawCall, x, y, wDst, hDst, 0.f, 1.f, 0.f, 1.f, rgba0, rgba1, rgba2, rgba3);

    _Vita_DoneWithDrawCall();
}

void Vita_DrawRectColor(float x, float y,
                        float wDst, float hDst,
                        float _r, 
                        float _g,
                        float _b, 
                        float _a)
{
    float rgba0[4] = {_r, _g, _b, _a};
    Vita_DrawRect4xColor(x, y, wDst, hDst, rgba0, rgba0, rgba0, rgba0);
}

void Vita_DrawRectColorExData(float x, float y,
                           float wDst, float hDst,
                           float _r,
                           float _g,
                           float _b,
                           float _a,
                           obj_extra_data *ex_data)
{
    float rgba0[4] = {_r, _g, _b, _a};
    DrawCall *_curDrawCall = _Vita_GetAvailableDrawCall();

    if(ex_data != NULL)
        ex_data->textureID = 0;

    _curDrawCall->verts[0].obj_ptr = ex_data;
    _curDrawCall->verts[1].obj_ptr = ex_data;
    _curDrawCall->verts[2].obj_ptr = ex_data;
    _curDrawCall->verts[3].obj_ptr = ex_data;


    // _curDrawCall->scale = 1.0f;
    // _curDrawCall->rot_x = 0;
    // _curDrawCall->rot_y = 0;
    // _curDrawCall->rot_z = rot;
    // _curDrawCall->piv_x = x + (wDst * .5f);
    // _curDrawCall->piv_y = y + (hDst * .5f);

    _Vita_WriteVertices4xColor(_curDrawCall, x, y, wDst, hDst, 1.f, 1.f, 1.f, 1.f, rgba0, rgba0, rgba0, rgba0);

    _Vita_DoneWithDrawCall();

}


void Vita_Draw(float x,
               float y,
               float wDst,
               float hDst)
{
    
    DrawCall *_curDrawCall = _Vita_GetAvailableDrawCall();
    
    for(int i = 0; i < VERTICES_PER_PRIM; i++)
        _curDrawCall->verts[i].obj_ptr = NULL;
#if 0
    _curDrawCall->scale = 1.0f;
#endif

    _Vita_WriteVertices(_curDrawCall, x, y, wDst, hDst, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f);
    
    _Vita_DoneWithDrawCall();
}

// DEPRECATED
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
        float _scale)
{
    DrawCall *_curDrawCall = _Vita_GetAvailableDrawCall();
    if(_curDrawCall == NULL) return;

    
#if 0
    _curDrawCall->textureID = texId;
    _curDrawCall->piv_x = x + (wDst * .5f);
    _curDrawCall->piv_y = y + (hDst * .5f);
    _curDrawCall->rot_z = _rot;
    _curDrawCall->scale = _scale;
#endif

#ifdef HALF_TEX
    tex_w = float(texture.w) / float(2),
    tex_h = float(texture.h) / float(2);
#else
#endif
    float n_src_x = (src_x / tex_w);
    float n_src_x2 = ((src_x + src_w) / tex_w);
    float n_src_y = (src_y / tex_h);
    float n_src_y2 = ((src_y + src_h) / tex_h);

    _Vita_WriteVertices(
        _curDrawCall, 
        x, 
        y, 
        wDst, 
        hDst, 
        n_src_x, 
        n_src_x2, 
        n_src_y, 
        n_src_y2, 
        _r, _g, _b, _a);
    
    _Vita_DoneWithDrawCall();
}

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
        obj_extra_data *ex_data)
{

    DrawCall *_curDrawCall = _Vita_GetAvailableDrawCall();
    if(_curDrawCall == NULL) return;

    if(ex_data != NULL && ex_data->textureID != texId)
        ex_data->textureID = texId;

    for(int i = 0; i < VERTICES_PER_PRIM; i++)
        _curDrawCall->verts[i].obj_ptr = (void*)ex_data;
    
#if 0
    _curDrawCall->piv_x = x + (wDst * .5f);
    _curDrawCall->piv_y = y + (hDst * .5f);
#endif

    float n_src_x = (src_x / tex_w);
    float n_src_x2 = ((src_x + src_w) / tex_w);
    float n_src_y = (src_y / tex_h);
    float n_src_y2 = ((src_y + src_h) / tex_h);

    _Vita_WriteVertices(
        _curDrawCall, 
        x, 
        y, 
        wDst, 
        hDst, 
        n_src_x, 
        n_src_x2, 
        n_src_y, 
        n_src_y2, 
        _r, _g, _b, _a);
    
    _Vita_DoneWithDrawCall();

    // Vita_DrawTextureAnimColorRotScale(x, y, wDst, hDst, texId, tex_w, tex_h, src_x, src_y, src_w, src_h, _r, _g, _b, _a, _rot, 1.f);
}

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
        float _a)
{
    Vita_DrawTextureAnimColorExData(x, y, wDst, hDst, texId, tex_w, tex_h, src_x, src_y, src_w, src_h, _r, _g, _b, _a, NULL);
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
        _debugPrintf("!!!!!!!   Unable to create shader: shader returned ID of %d\n", shader);
        return 0;
    }

    glShaderSource(shader, 1, &shaderSrc, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if(!compiled)
    {   
        
        _debugPrintf("%s NOT COMPILED.\n", (type == GL_VERTEX_SHADER ? "Vertex Shader" : "Fragment Shader"));
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if(infoLen > 1)
        {
            char *infoLog = (char*)malloc(sizeof(char) * infoLen);

            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            _debugPrintf("\n\nError Compiling Shader:\n\n%s\n", infoLog);
            free(infoLog);
        }

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

// ------------------------------------------    INIT FUNCTIONS

int initGLShading2(char* _vShaderString, char* _fShaderString)
{
    _debugPrintf("(NOTE): Init GL Shading 2. Initializing GL Shading with shader strings passed to us externally (by the user).\n");
    vertexShaderID = LoadShader(GL_VERTEX_SHADER, _vShaderString);
    if(vertexShaderID == 0)
    {
        _debugPrintf("ERROR: vertex shader ID: %d\n", vertexShaderID);
        return -1;
    }
    else _debugPrintf("V Shader ID: %d\n", vertexShaderID);

    CHECK_GL_ERROR("Vertex Shader");

    fragmentShaderID = LoadShader(GL_FRAGMENT_SHADER, _fShaderString);
    if(fragmentShaderID == 0)
    {
        _debugPrintf("ERROR: frag shader ID: %d\n", fragmentShaderID);
        return -1;
    }
    else _debugPrintf("F Shader ID: %d\n", fragmentShaderID);

    CHECK_GL_ERROR("Frag Shader");

    programObjectID = glCreateProgram();
    CHECK_GL_ERROR("Make Program Shader");

    if(programObjectID == 0)
    {
        _debugPrintf("ERROR: Program object is 0.\n");
        return -1;
    }

    CHECK_GL_ERROR("Pre Shader Attach");
    glAttachShader(programObjectID, vertexShaderID);
    CHECK_GL_ERROR("Vertex Shader Attach");

    glAttachShader(programObjectID, fragmentShaderID);  
    CHECK_GL_ERROR("Frag Shader Attach");

    glLinkProgram(programObjectID);
    CHECK_GL_ERROR("LINK PROGRRAM");

    GLint linked;
    glGetProgramiv(programObjectID, GL_LINK_STATUS, &linked);
    CHECK_GL_ERROR("Get Program Info Value");

    if(!linked)
    {
        // TODO: proper error.
        _debugPrintf("!!!!!! FAILED TO LINK SHADER!\n");
        return -1;
    }
    
#ifdef VITA
    VERTEX_POS_INDEX = glGetAttribLocation(programObjectID, "aPosition");
    VERTEX_TEXCOORD_INDEX = glGetAttribLocation(programObjectID, "vTexCoord");
    VERTEX_COLOR_INDEX = glGetAttribLocation(programObjectID, "vColor");
    // TODO: mvp mat4 for CG shader
    // TODO: color float4 for CG shader.
    // Uniforms
    VERTEX_MVP_INDEX = glGetUniformLocation(programObjectID, "mvp"); // MVP matrix. In our case, this is an ortho matrix for the Vita's screen.

    UNIFORM_ROTMAT_INDEX = glGetUniformLocation(programObjectID, "_rot");
    UNIFORM_SCALE_INDEX = glGetUniformLocation(programObjectID, "_scale");
    
    glm_mat4_identity(_rot);
    glm_mat4_identity(_rot_arb);

    glm_mat4_identity(_scale);
    glm_mat4_identity(_scale_arb);
#else
    // Attribs
    VERTEX_POS_INDEX = glGetAttribLocation(programObjectID, "vPosition"); // Vertex position.
    VERTEX_TEXCOORD_INDEX = glGetAttribLocation(programObjectID, "vTexCoord"); // Vertex Tex Coord.
    VERTEX_COLOR_INDEX = glGetAttribLocation(programObjectID, "vColor"); // Gets passed to the fragment shader.

    // Uniforms
    VERTEX_MVP_INDEX = glGetUniformLocation(programObjectID, "mvp"); // MVP matrix. In our case, this is an ortho matrix for the Vita's screen.

    UNIFORM_ROTMAT_INDEX = glGetUniformLocation(programObjectID, "_rot");
    UNIFORM_SCALE_INDEX = glGetUniformLocation(programObjectID, "_scale");
    
    glm_mat4_identity(_rot);
    glm_mat4_identity(_rot_arb);

    glm_mat4_identity(_scale);
    glm_mat4_identity(_scale_arb);
    
    // TODO: vec2 for texcoords.
#endif
#ifndef VITA
    if(VERTEX_POS_INDEX <= -1)
    {
        _debugPrintf("VERTEX_POS_INDEX returned invalid value: %d\n", VERTEX_POS_INDEX);
        return -1;
    }
    CHECK_GL_ERROR("VERTEX_POS_INDEX");

    if(VERTEX_TEXCOORD_INDEX <= -1)
    {
        _debugPrintf("VERTEX_TEXCOORD_INDEX returned invalid value: %d\n", VERTEX_TEXCOORD_INDEX);
        return -1;
    }
    CHECK_GL_ERROR("VERTEX_TEXCOORD_INDEX");

    if(VERTEX_MVP_INDEX <= -1)
    {
        _debugPrintf("VERTEX_MVP_INDEX returned invalid value: %d\n", VERTEX_MVP_INDEX);
        return -1;
    }
    CHECK_GL_ERROR("VERTEX_MVP_INDEX");

    if(VERTEX_COLOR_INDEX <= -1)
    {
        _debugPrintf("VERTEX_COLOR_INDEX returned invalid value: %d\n", VERTEX_COLOR_INDEX);
        return -1;
    }
    CHECK_GL_ERROR("VERTEX_COLOR_INDEX");
#endif
    _debugPrintf(
        "[Attrib Location Report]\n\nVERTEX_POS_INDEX: %d\nVERTEX_TEXCOORD_INDEX: %d\nVERTEX_MVP_INDEX: %d\nVERTEX_COLOR_INDEX: %d\n",
        VERTEX_POS_INDEX, VERTEX_TEXCOORD_INDEX, VERTEX_MVP_INDEX, VERTEX_COLOR_INDEX
    );

    
    return 0;
}

int initGLShading()
{
    return initGLShading2((char *)vShaderString, (char *)vFragmentString);
}

int initGLAdv()
{
    _vgl_pending_total_size = sizeof(DrawCall) * MAX_VERTICES;
    _vgl_pending_calls = (DrawCall*)malloc(_vgl_pending_total_size);
    memset(_vgl_pending_calls, 0, _vgl_pending_total_size);

    // Generate vbo
    glGenBuffers(1, &_vertexBufferID);
    _debugPrintf("Gen Buffers with ID %d\n", _vertexBufferID);
    CHECK_GL_ERROR("GEN BUFFERS");

    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferID);
    _debugPrintf("bind buffers\n");
    CHECK_GL_ERROR("BIND BUFFERS");

    // Initial data buffer
    glBufferData(GL_ARRAY_BUFFER, _vgl_pending_total_size, 0, GL_DYNAMIC_DRAW);
    _debugPrintf("Initial Buffer Data with %ld bytes (%.2f MB)\n", _vgl_pending_total_size, (_vgl_pending_total_size / 1024.f) / 1024.f);
    CHECK_GL_ERROR("INITIAL BUFFER DATA");

    return 0;
}

int deInitGL()
{
    free(_vgl_pending_calls);
    
#ifdef VITA
    vglEnd();
#else
    glfwTerminate();
#endif

    return 0;
}

static void glfwError(int id, const char* description)
{
    _debugPrintf("[GLFW] ERROR ID %d: %s\n", id, description);
}

int initGL(void (*dbgPrintFn)(const char*, ...))
{
    if(dbgPrintFn == NULL)
    {
        return -1;
    }
    
    _debugPrintf = dbgPrintFn;
#ifdef VITA
    vglInit(0x100000);
#endif
#ifndef VITA
    glewExperimental = 1;
    glfwSetErrorCallback(&glfwError);
    int glfwReturnVal = glfwInit();
    if(glfwReturnVal == GLFW_FALSE)
    {
        _debugPrintf("[main] glfwReturnVal is == GLFW_FALSE. glfw init failed.");
        return -1;
    }
    _debugPrintf("[main] GLFWInit: %d\n", glfwReturnVal);

    _game_window = glfwCreateWindow(960,544,"Test", 0, 0);
    _debugPrintf("Window Pointer: %p\n", _game_window);
    glfwMakeContextCurrent(_game_window);

    

    int glewReturnVal = glewInit();
    if(glewReturnVal != GLEW_OK)
    {
        _debugPrintf("[main] GLEWINIT FAILED!\n");
        return -1;
    }
    _debugPrintf("[main] GLEWInit: %d\n", glewReturnVal);
#endif

    glClearColor(.1f, .5f, .1f, 1.0f);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glMatrixMode(GL_PROJECTION);
    // glLoadIdentity();
    // glOrtho(0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, -1, 1);

#ifdef __APPLE__
    // TODO: Check for retina on Apple machines.
    glm_ortho_lh_zo(0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, -1, 1, cpu_mvp);
#else
    glm_ortho_lh_zo(0, DISPLAY_WIDTH * 2, DISPLAY_HEIGHT * 2, 0, -1, 1, cpu_mvp);
#endif

    // glMatrixMode(GL_MODELVIEW);
    // glLoadIdentity();

    glViewport(
        0,
        -DISPLAY_HEIGHT,
        DISPLAY_WIDTH * 2,
        DISPLAY_HEIGHT * 2
    );

    return 0;
}

// ------------------------------------------ END INIT FUNCTIONS

/**
 * clear(): 
 *  Clears the screen's color buffer using glClear.
 *  Then, calls Vita_ResetTotalCalls() to reset the 
 *  number of draw calls in our queue.
 */
void clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
    Vita_ResetTotalCalls();
}

/**
 * repaint():
 *  Repaint does the following.
 *      1. Buffers the CPU calculated vertices into the GPU.
 *      2. Sets up the vertex attrib pointers for the shader based on the data.
 *      3. Binds the buffers and the default shader and calls glDrawArrays for
 *         each vertex. This sounds bad, but is surprisingly performant 
 *         due to the fact that the data already exists on the GPU at this time.
 *          
 *         Still, this was a workaround due to the fact that glDrawArrays 
 *         tries to render the full list as one quad and the fact that glDrawElements
 *         requires an indices list. 
 */
void repaint()
{
    const GLsizei stride = VERTEX_ATTRIB_TOTAL_SIZE_1; // NOT Tightly packed. 4 verts per GL_TRIANGLE_STRIP
    uint32_t draw_calls = Vita_GetTotalCalls();
    if(draw_calls == 0) goto FINISH_DRAWING;
    if(draw_calls > GL_MAX_VERTEX_ATTRIBS)
    {
        _debugPrintf("Too many calls (%d / %d).\n", draw_calls, GL_MAX_VERTEX_ATTRIBS);
    }

    GLuint _vbo = Vita_GetVertexBufferID(); // Get OpenGL handle to our vbo. (On the GPU)
    
    if(_vbo != 0)
    {
        // Get pointer to the pending drawcalls.
        struct _DrawCall *calls = Vita_GetDrawCallsPending();
        int i = 0;

        uint32_t offset = 0; // OFFSET: byte offset into memory. where to PUT our element.
        uint32_t sizeCopy = 0; // the size of one vertex.
        GLint gpuBufferSize = 0; // total size of the GPU buffer, so we make sure not to overload.

        glBindBuffer(GL_ARRAY_BUFFER, _vbo); // Bind our vbo through OpenGL.
        CHECK_GL_ERROR("bind");

        glBufferData(GL_ARRAY_BUFFER, draw_calls * sizeof(DrawCall), calls, GL_DYNAMIC_DRAW);
        // _debugPrintf("Handled %d drawcalls. (%d vertice count)\n", draw_calls, draw_calls * VERTICES_PER_QUAD);
    }
    else return;

    // TODO: Is this necessary? Our _vbo should still be bound.
    glBindBuffer(GL_ARRAY_BUFFER, _vbo); // Bind the vbo we've written to.
    glUseProgram(programObjectID); // Begin using our vert/frag shader combo (program)


    // ONLY enable these for data that you want to be
    // defined/ passed through the vertex attribute array.
    glEnableVertexAttribArray(VERTEX_POS_INDEX); // Enabling the property on the shader side.
    glEnableVertexAttribArray(VERTEX_TEXCOORD_INDEX); // Enabling TEX_COORD_INDEX
    glEnableVertexAttribArray(VERTEX_COLOR_INDEX); // Enabling color index

    CHECK_GL_ERROR("enable vertex attrib array 0");

    glVertexAttribPointer(VERTEX_POS_INDEX, 2, GL_FLOAT, GL_FALSE, stride, (void*)0); // Binding the data from the vbo to our vertex attrib.
    CHECK_GL_ERROR("vert attrib ptr arrays");

    glVertexAttribPointer(VERTEX_TEXCOORD_INDEX, 2, GL_FLOAT, GL_FALSE, stride, (void*)(0 + (2 * sizeof(float))));
    CHECK_GL_ERROR("vert attrib ptr tex coord.");

    glVertexAttribPointer(VERTEX_COLOR_INDEX, 4, GL_FLOAT, GL_FALSE, stride, (void*)(0 + (4 * sizeof(float))));
    CHECK_GL_ERROR("vert attrib ptr color");

    glUniformMatrix4fv(VERTEX_MVP_INDEX, 1, GL_FALSE, (const GLfloat*)cpu_mvp);
    CHECK_GL_ERROR("glUniformMatrix4fv");

    // This is a "hack around".
    // Ideally, I'd be able to batch this all at once.
    int i;
    GLint _locUseTexture = glGetUniformLocation(programObjectID, "useTexture");
    GLuint _curBoundTex = 0;
    DrawCall _curDrawCall;

    for(i = 0; i < draw_calls; i++)
    {
        _curDrawCall = _vgl_pending_calls[i];

        if(_curDrawCall.verts != NULL
            && _curDrawCall.verts[0].obj_ptr != NULL)
        {
            // _debugPrintf("!!!!!\t!!!!! HAS extra data ptr!\n");
            DEBUG_PRINT_OBJ_EX_DATA(((obj_extra_data *)_curDrawCall.verts[0].obj_ptr));
            obj_extra_data ex_data = *((obj_extra_data *)_curDrawCall.verts[0].obj_ptr); 
    
            // Only re-bind texture when it's different
            // from what's currently bound.
            if(_curBoundTex != ex_data.textureID)
            {
            
                if(ex_data.textureID == 0)
                    glUniform1i(_locUseTexture, 0);
                else
                    glUniform1i(_locUseTexture, 1);
                

                // _debugPrintf("[vgl_renderer] repaint(): TODO change bind texture from id %u to id %u\n", _curBoundTex, ex_data.textureID);
                glBindTexture(GL_TEXTURE_2D, ex_data.textureID);
                _curBoundTex = ex_data.textureID;
                
            }


            glm_mat4_identity(_rot_arb);
            glm_rotate_atm(
                _rot_arb, 
                (vec3){ex_data.piv_x, ex_data.piv_y, 0.f}, 
                glm_rad(ex_data.rot_z), 
                (vec3){0.f, 0.f, 1.f}
            );

            vec3 refVector = {ex_data.piv_x, ex_data.piv_y, 0.f};
            vec3 nRefVector = {-ex_data.piv_x, -ex_data.piv_y, 0.f};

            mat4 transRefTo;
            mat4 transRefFrom;
            mat4 transfScale;
            mat4 _temp1;
            glm_mat4_identity(transRefTo);
            glm_mat4_identity(transRefFrom);
            glm_mat4_identity(transfScale);
            glm_mat4_identity(_temp1);
            glm_translate(transRefTo, nRefVector);
            glm_translate(transRefFrom, refVector);
            
            glm_scale(transfScale, (vec3){ex_data.scale, ex_data.scale, ex_data.scale});
            glm_mat4_mul(transRefFrom, transfScale, _temp1);
            glm_mat4_mul(_temp1, transRefTo, _scale_arb);    
        }

        glUniformMatrix4fv(UNIFORM_SCALE_INDEX, 1, GL_FALSE, (const GLfloat *)_scale_arb);
        glUniformMatrix4fv(UNIFORM_ROTMAT_INDEX, 1, GL_FALSE, (const GLfloat*)_rot_arb);

        glDrawArrays(GL_TRIANGLE_STRIP, i * VERTICES_PER_PRIM, VERTICES_PER_PRIM);

        glm_mat4_identity(_scale_arb);
        glm_mat4_identity(_rot_arb);
    }

    glFlush();
    
    // Revert shader state. 
    glUniform1i(_locUseTexture, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Reverting state.
    glDisableVertexAttribArray(VERTEX_POS_INDEX);
    glDisableVertexAttribArray(VERTEX_TEXCOORD_INDEX);
    glDisableVertexAttribArray(VERTEX_COLOR_INDEX);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);

    
    glFinish();

FINISH_DRAWING:
#ifdef VITA
    vglSwapBuffers(GL_TRUE);
#else
    glfwSwapBuffers(_game_window);
    glfwPollEvents();
#endif

    Vita_ResetTotalCalls();
}