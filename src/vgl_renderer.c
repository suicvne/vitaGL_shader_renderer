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

static int DISPLAY_WIDTH = 960;
static int DISPLAY_HEIGHT = 544;

static int VERTEX_POS_INDEX = 0;
static int VERTEX_TEXCOORD_INDEX = 1;
static int VERTEX_COLOR_INDEX = 2;
static int VERTEX_MVP_INDEX = 3;

mat4 cpu_mvp;

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

void
Vita_Draw(float x,
          float y,
          float wDst,
          float hDst)
{
    if((_vgl_pending_offset + 1) > _vgl_pending_total_size)
    {
        printf("Ran out of draw calls. %d / %zu\n", _vgl_pending_offset, _vgl_pending_total_size);
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

// TODO: this wasn't really needed, since we're allowing 
// flexibility by grabbing the locations when we compile the shader.
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
    
#ifdef VITA
    VERTEX_POS_INDEX = glGetAttribLocation(programObjectID, "aPosition");
    // TODO: mvp mat4 for CG shader
    // TODO: color float4 for CG shader.
#else
    VERTEX_POS_INDEX = glGetAttribLocation(programObjectID, "vPosition"); // Vertex position.
    VERTEX_MVP_INDEX = glGetUniformLocation(programObjectID, "mvp"); // MVP matrix. In our case, this is an ortho matrix for the Vita's screen.
    VERTEX_COLOR_INDEX = glGetAttribLocation(programObjectID, "vColor"); // Gets passed to the fragment shader.
    // TODO: vec2 for texcoords.
#endif
    
    if(VERTEX_POS_INDEX <= -1)
    {
        printf("VERTEX_POS_INDEX returned invalid value: %d\n", VERTEX_POS_INDEX);
        return -1;
    }
    CHECK_GL_ERROR("VERTEX_POS_INDEX");

    if(VERTEX_MVP_INDEX <= -1)
    {
        printf("VERTEX_MVP_INDEX returned invalid value: %d\n", VERTEX_MVP_INDEX);
        return -1;
    }
    CHECK_GL_ERROR("VERTEX_MVP_INDEX");

    if(VERTEX_COLOR_INDEX <= -1)
    {
        printf("VERTEX_COLOR_INDEX returned invalid value: %d\n", VERTEX_COLOR_INDEX);
        return -1;
    }
    CHECK_GL_ERROR("VERTEX_COLOR_INDEX");
    
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
    glOrtho(0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, -1, 1);

    glm_ortho_lh_zo(0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, -1, 1, cpu_mvp);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

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
    const GLsizei stride = VERTEX_ATTR_ELEM_COUNT * sizeof(GLfloat); // NOT Tightly packed. 4 verts per GL_TRIANGLE_STRIP
    uint32_t draw_calls = Vita_GetTotalCalls();
    if(draw_calls == 0) goto FINISH_DRAWING;
    if(draw_calls > GL_MAX_VERTEX_ATTRIBS)
    {
        printf("Too many calls (%d / %d).\n", draw_calls, GL_MAX_VERTEX_ATTRIBS);
    }

    GLuint _vbo = Vita_GetVertexBufferID(); // Get OpenGL handle to our vbo. (On the GPU)
    
    if(_vbo != 0)
    {
        // Get pointer to the pending drawcalls.
        struct _DrawCall *calls = Vita_GetDrawCallsPending();
        int i = 0;

        uint32_t offset = 0; // OFFSET: byte offset into memory. where to PUT our element.
        uint32_t sizeCopy = 0; // the size of one vertex.
        uint32_t gpuBufferSize = 0; // total size of the GPU buffer, so we make sure not to overload.

        glBindBuffer(GL_ARRAY_BUFFER, _vbo); // Bind our vbo through OpenGL.
        CHECK_GL_ERROR("bind");

                                        // TODO: Texture binding & drawing w texcoords.
                                        // TODO: Could this be done better? 
        for(i = 0; i < draw_calls; i++) // 1 draw call is 4 vertices, we buffer 4 vertices at a time to OGL.
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


            // memory address of our current draw call's vertices. we upload them as is thanks to C-structs!
            void *vertexPtr = &(calls[i].verts); 
/*
            printf("writing (size of calls[i].verts: %d) (size: %d; offset: %d) vertexPtr: %p\n", sizeof(calls[i].verts), sizeCopy, offset, vertexPtr);
            printf("\t%.2f, %.2f\n\t%.2f, %.2f\n\t%.2f, %.2f\n\t%.2f, %.2f\n", 
                ((struct _vert*)vertexPtr)->x, ((struct _vert*)vertexPtr)->y,
                ((struct _vert*)vertexPtr + 1)->x, ((struct _vert*)vertexPtr + 1)->y,
                ((struct _vert*)vertexPtr + 2)->x, ((struct _vert*)vertexPtr + 2)->y,
                ((struct _vert*)vertexPtr + 3)->x, ((struct _vert*)vertexPtr + 3)->y
            );
*/

            // The magic.
            // We buffer in `sizeCopy` bytes at `offset` from our `vertexPtr`
            glBufferSubData(GL_ARRAY_BUFFER, 
                            offset, // offset
                            sizeCopy, // size
                            vertexPtr);
            CHECK_GL_ERROR("buffersub");
        }

        // printf("Handled %d drawcalls. (%d vertice count)\n", draw_calls, draw_calls * VERTICES_PER_QUAD);
    }

// TODO: Is this necessary? Our _vbo should still be bound.
    glBindBuffer(GL_ARRAY_BUFFER, _vbo); // Bind the vbo we've written to.
    glUseProgram(programObjectID); // Begin using our vert/frag shader combo (program)


// ONLY enable these for data that you want to be
// defined/ passed through the vertex attribute array.
    glEnableVertexAttribArray(VERTEX_POS_INDEX); // Enabling the property on the shader side.
    glEnableVertexAttribArray(VERTEX_COLOR_INDEX);

    CHECK_GL_ERROR("enable vertex attrib array 0");

    glVertexAttribPointer(VERTEX_POS_INDEX, 2, GL_FLOAT, GL_FALSE, stride, (void*)0); // Binding the data from the vbo to our vertex attrib.
    CHECK_GL_ERROR("vert attrib ptr arrays");

    glVertexAttribPointer(VERTEX_COLOR_INDEX, 4, GL_FLOAT, GL_FALSE, stride, (void*)(0 + (4 * sizeof(float))));
    CHECK_GL_ERROR("vert attrib ptr color");

    glUniformMatrix4fv(VERTEX_MVP_INDEX, 1, GL_FALSE, cpu_mvp);
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
    glDisableVertexAttribArray(VERTEX_POS_INDEX);
    glDisableVertexAttribArray(VERTEX_COLOR_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);

FINISH_DRAWING:
#ifdef VITA
    vglSwapBuffers(GL_FALSE);
#else
    glfwSwapBuffers(_game_window);
    glfwPollEvents();
#endif

    Vita_ResetTotalCalls();
}