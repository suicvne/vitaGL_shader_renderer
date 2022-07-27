#include "vgl3d.h"
#include "vgl3d_gl2es_textures.h"

// =========== Concrete Implementations for OpenGL ES 2.0 Renderer =============

// Struct implementation.
// Configuration and any extra data.
typedef struct _VGL3DConfig {

#ifndef VITA
    GLFWwindow* game_window;
    const char* game_window_title;
    uint16_t game_window_width;
    uint16_t game_window_height;
#endif

    mat4 view;
    mat4 proj;
    mat4 cpu_mvp;
    vec3 camEyePos;
    vec3 camEyeRot;
    GLuint vbo;

    GLuint curShaderID;

} VGL3DConfig;

vec3 _camEyePos = {0.0f, 0.0f, -2.0f};
vec3 _camEyeRot = {0.0f, 0.0f, 0.0f};

#ifndef SELF
#define SELF VGL3DContext* context
#endif

void VGL3D_BindTexture(SELF, VTEX tex) {
    glBindTexture(GL_TEXTURE_2D, tex);
    context->private.curBoundTex = tex;
}

VTEX VGL3D_LoadTextureAt(SELF, const char *path) {
    /* Unused. */
    return -1;
}

void VGL3D_SetCamera(VGL3DContext* context, vec3 pos, vec3 rot_deg) {
    glm_vec3_copy(pos, _camEyePos);
    glm_vec3_copy(rot_deg, _camEyeRot);
}

#ifndef VITA

void glfwError()
{

}

void __key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        const float amt = 30.0f;

        switch(scancode)
        {
            case GLFW_KEY_UP:
            case 126:
                _camEyePos[1] += 0.5f;
                break;
            case GLFW_KEY_DOWN:
            case 125:
                _camEyePos[1] -= 0.5f;
                break;
            case GLFW_KEY_RIGHT:
            case 124:
                _camEyePos[0] += 0.5f;
                break;
            case GLFW_KEY_LEFT:
            case 123:
                _camEyePos[0] -= 0.5f;
                break;
            case 33:
                _camEyePos[2] -= 1.0f;
                break;
            case 30:
                _camEyePos[2] += 1.0f;
                break;
            case 7:
                _camEyeRot[0] += amt;
                break;
            case 6:
                _camEyeRot[0] -= amt;
                break;
            case 0:
                _camEyeRot[1] += amt;
                break;
            case 1:
                _camEyeRot[1] -= amt;
                break;
            case 12:
                _camEyeRot[2] += amt;
                break;
            case 13:
                _camEyeRot[2] -= amt;
                break;
            case 35:
                VGL3D_Log(NULL, "(%.2f, %.2f, %.2f) Rot: (%.2f, %.2f, %.2f)\n", _camEyePos[0], _camEyePos[1], _camEyePos[2], _camEyeRot[0], _camEyeRot[1], _camEyeRot[2]);
                break;
            default:
                printf("key: %d\n", scancode);
        }
    }
}
#endif

inline VGL3DContext VGL3D_Create()
{
    VGL3DContext newContext = (VGL3DContext) {
        .config = malloc(sizeof(VGL3DConfig)),
        .Begin = VGL3D_Begin,
        .End = VGL3D_End,
        .DrawQuad = VGL3D_DrawQuad,
        .InitBackend = VGL3D_InitBackend,
        .Log = VGL3D_Log,
        .SetClearColor = VGL3D_SetClearColor,
        .Clear = VGL3D_Clear,
        .SetCamera = VGL3D_SetCamera,
        .LoadTextureAt = _VGL3D_LoadAndCreateGLTexture,
        .BindTexture = VGL3D_BindTexture,
        .private = {
            .curBoundTex = 0,
            .drawingInProgress = 0,
            .doContinue = 1
        }
    };

#ifndef VITA
    // Desktop GL properties.
    // Vita is force 960 x 544 and obviously no window title available.
    newContext.config->game_window_width = 960.f;
    newContext.config->game_window_height = 544.f;
    newContext.config->game_window_title = "VGL3D_GL2ES";
#endif

    return newContext;
}

inline void VGL3D_Begin(SELF) {
    context->private.drawingInProgress = 1;
    context->private.doContinue = !glfwWindowShouldClose(context->config->game_window);

    // Bind Buffers.

    // Use shaders.

    // Set glVertexAttribPointers

    // Set glUniformMatrix4fv

}

inline void VGL3D_End(SELF) {
    context->private.drawingInProgress = 0;
#ifdef VITA
    vglSwapBuffers(GL_TRUE);
#else
    glfwSwapBuffers(context->config->game_window);
    glfwPollEvents();
#endif
}

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

#ifndef VITA
    memset(error_buffer, 0, sizeof(error_buffer));
    const char* _eb = (char*)error_buffer;
    if(glfwGetError(&_eb))
    {
        printf("\t\tGLFW: %s", error_buffer);
    }
#endif
}

#include <stdio.h>
#include <stdlib.h>

static inline int _VGL3D_ReadShaderFromFile(const char* path, size_t* fsize, char** buffer)
{
    FILE *_file = fopen(path, "r");

    if(_file == NULL)
    {
        return -1;
    }

    fseek(_file, 0, SEEK_END);
    *fsize = ftell(_file);
    fseek(_file, 0, SEEK_SET);

    *buffer = (char*)realloc(*buffer, (*fsize) + 1);
    fread(*buffer, *fsize, 1, _file);

    size_t offset = *fsize;
    (*buffer)[offset] = '\0';

    fclose(_file);
    return 0;
}

GLuint _VGL3D_LoadShader(GLenum type, const char *shaderPath)
{
    // Read
    char *shaderSrc = malloc(1);
    size_t shaderSrcLen;
    _VGL3D_ReadShaderFromFile(shaderPath, &shaderSrcLen, &shaderSrc);
    VGL3D_Log(NULL, "%s: %s\n\n", shaderPath, shaderSrc);

    // Compile
    GLuint shader;
    GLint compiled;
    
    shader = glCreateShader(type);

    if(shader == 0)
    { 
        VGL3D_Log(NULL, "!!!!!!!   Unable to create shader: shader returned ID of %d\n", shader);
        if(shaderSrc != NULL)
            free(shaderSrc);
        return 0;
    }

    glShaderSource(shader, 1, (const GLchar**)(&shaderSrc), NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if(!compiled)
    {
        VGL3D_Log(NULL, "%s NOT COMPILED.\n", (type == GL_VERTEX_SHADER ? "Vertex Shader" : "Fragment Shader"));
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if(infoLen > 1)
        {
            char *infoLog = (char*)malloc(sizeof(char) * infoLen);

            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            VGL3D_Log(NULL, "\n\nError Compiling Shader:\n\n%s\n", infoLog);
            free(infoLog);
        }

        glDeleteShader(shader);

        if(shaderSrc != NULL)
            free(shaderSrc);
        return 0;
    }

    if(shaderSrc != NULL)
        free(shaderSrc);
    return shader;
}

void _VGL3D_UpdateViewProjection(SELF, mat4* oModelMat) {
    
    glm_mat4_identity(context->config->view);
    glm_mat4_identity(context->config->proj);

    // Model
    // Keep for now
    mat4 model = GLM_MAT4_IDENTITY_INIT; 
    if(oModelMat != NULL)
    {
        // Apply transformations.
        glm_mat4_copy(*oModelMat, model);
    }

    // View
    glm_look(
        (vec3){_camEyePos[0], _camEyePos[1], _camEyePos[2]},
        (vec3){0.0f, 0.0f, 1.0f},  // Look ("forward" axis)
        (vec3){0.0f, 1.0f, 0.0f},  // Up axis.
        context->config->view
    );
    glm_rotate_x(context->config->view, glm_rad(_camEyeRot[0]), context->config->view);
    glm_rotate_y(context->config->view, glm_rad(_camEyeRot[1]), context->config->view);
    glm_rotate_z(context->config->view, glm_rad(_camEyeRot[2]), context->config->view);

    // Projection
    glm_perspective_default(
        // (float)context->config->game_window_height / (float)context->config->game_window_width, 
        (float)context->config->game_window_width / (float)context->config->game_window_height, 
        context->config->proj
    );

    // Make cpu_mvp
    mat4* matrices[3];
    matrices[0] = &context->config->proj;
    matrices[1] = &context->config->view;
    matrices[2] = &model;

    glm_mat4_mulN(matrices, 3, context->config->cpu_mvp);
}

void _VGL3D_MakeDefaultViewProjection(SELF) {
    _VGL3D_UpdateViewProjection(context, NULL);
}

int _VGL3D_InitShading(SELF, const char* vPath, const char* fPath) {
    context->Log(context, "Init Shading!\n");
    GLuint vertShaderID = _VGL3D_LoadShader(GL_VERTEX_SHADER, vPath);
    CHECK_GL_ERROR("Vertex Shader");
    GLuint fragShaderID = _VGL3D_LoadShader(GL_FRAGMENT_SHADER, fPath);
    CHECK_GL_ERROR("Fragment Shader");

    GLuint programID = glCreateProgram();
    CHECK_GL_ERROR("Create Program");
    glAttachShader(programID, vertShaderID);
    CHECK_GL_ERROR("Attach Vertex Shader");
    glAttachShader(programID, fragShaderID);
    CHECK_GL_ERROR("Attach Fragment Shader");
    glLinkProgram(programID);
    CHECK_GL_ERROR("Linking Program");
    GLint linked;
    glGetProgramiv(programID, GL_LINK_STATUS, &linked);
    if(!linked)
    {
        context->Log(context, "!!!! Failed to link shader '%s'.\n");
        return -1;
    }

    // Grab indexes and everything else.
    context->Log(context, "DONE Init Shading!\n");

    context->config->curShaderID = programID;

    return 0;
}

int VGL3D_InitBackend(SELF) {
#ifdef VITA // vitaGL initialization.
    vglInit(50 * 1024 * 1024);
    _userHasLibshaccg();
#else // glew/glfw initialization.
    glewExperimental = 1;
    glfwSetErrorCallback(glfwError);

    int glfwReturnVal = glfwInit();
    if(glfwReturnVal == GLFW_FALSE)
    {
        context->Log(context, "Error initializing glfw. Rval: %d\n", glfwReturnVal);
        return -1;
    }

    // TODO Store in configuration.
    // TODO: Grab window size/name from configuration.

    context->config->game_window = glfwCreateWindow(
        context->config->game_window_width,
        context->config->game_window_height,
        context->config->game_window_title, 0, 0
    );

    glfwMakeContextCurrent(context->config->game_window);
    glfwSetKeyCallback(context->config->game_window, __key_callback);
    
    // glfwSetWindowFullscreen(context->config->game_window, GLFW_TRUE);

    int glewRval = glewInit();
    if(glewRval != GLEW_OK)
    {
        context->Log(context, "glewInit failed. Returned: %d\n", glewRval);
        return -1;
    }
    else context->Log(context, "glewInit completed!");
#endif

    // Initialize shading.
    // TODO: Configuration for this? Paths for built in shader? Or bundle these into the renderer?
    int shadingVal = _VGL3D_InitShading(context, "../vert_new.glsl", "../frag_new.glsl");
    if(shadingVal != 0)
    {
        context->Log(context, "Rval from Shading: %d\n", shadingVal);
        return -1;
    }

    _VGL3D_MakeDefaultViewProjection(context);

    // TODO: VGL3D_SetClearColor
    // TODO: VGL3D_SetDepthFunc
    // TODO: VGL3D_SetBlendFunc.
    // TODO: VGL3D_SetViewport

    glClearColor(0.f, 0.f, 0.5f, 1.0f);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0,0,960,544);

    glGenBuffers(1, &context->config->vbo);

#ifdef __APPLE__
    // Wiggle the window so glfw gets the correct position upon opening.
    // It's important this happens AFTER the context is initialized.
    int xpos, ypos;
    glfwGetWindowPos(context->config->game_window, &xpos, &ypos);
    glfwSetWindowPos(context->config->game_window, xpos+5, ypos+5);
    glfwSetWindowPos(context->config->game_window, xpos, ypos);
#endif

    return 0;
}

GLfloat _quad_vertices[] =
{
    -1.0f, -1.0f, 1.0f, // bottom left
    -1.0f, 1.0f, 1.0f,  // top left
    1.0f, 1.0f, 1.0f,   // top right
    1.0f, -1.0f, 1.0f,  // bottom right
};

/*
GLfloat _quad_vertices_packed[] =
{
    // bottom left
    // x     y      z     u     v
    -0.5f, -0.5f, 1.0f,  0.0f, 0.0f,

    // top left
    -0.5f, 0.5f, 1.0f,  0.0f, 1.0f,

    // top right
    0.5f, 0.5f, 1.0f,  1.0f, 1.0f,

    // bottom right
    0.5f, -0.5, 1.0f, 1.0f, 0.0f,
};*/

// Interleaved vertex data.
GLfloat _quad_vertices_packed[] =
{
    // bottom left
    //    Vert Pos      |   Tex Coords
    // x     y      z     u     v
    -0.5f, -0.5f, 1.0f,  0.0f, 1.0f,

    // top left
    -0.5f, 0.5f, 1.0f,  0.0f, 0.0f,

    // top right
    0.5f, 0.5f, 1.0f,  1.0f, 0.0f,

    // bottom right
    0.5f, -0.5, 1.0f, 1.0f, 1.0f,
};

/*
GLfloat _quad_vertices_packed[] =
{
    // bottom left
    // x     y      z    r      g     b     u     v
    -1.0f, -1.0f, 1.0f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

    // top left
    -1.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,

    // top right
    1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f, 1.0f, 1.0f,

    // bottom right
    1.0f, -1.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
};
*/

GLfloat _quad_vertices_h[] =
{
    -0.5f, -0.5f, 1.0f, // bottom left
    -0.5f, 0.5f, 1.0f,  // top left
    0.5f, 0.5f, 1.0f,   // top right
    0.5f, -0.5f, 1.0f,  // bottom right
};

GLubyte _quad_indices[] = 
{
    0,1,2,
    0,2,3
};


void VGL3D_DrawQuad(
    SELF,
    float x, 
    float y, 
    float z, 
    vec3 rot, 
    vec3 scale, 
    vec4 rgba
)
{
    #define USE_SHADERS
    #define USE_PACKED

    #ifdef USE_PACKED
    #define STRIDE (sizeof(float) * 5)
    #else
    #define STRIDE 0
    #endif

    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_translate(model, (vec3){x, y, z});
    glm_rotate_x(model, glm_rad(rot[0]), model);
    glm_rotate_y(model, glm_rad(rot[1]), model);
    glm_rotate_z(model, glm_rad(rot[2]), model);
    glm_scale(model, scale);


    _VGL3D_UpdateViewProjection(context, &model);
    
    // glEnableClientState(GL_VERTEX_ARRAY);

    #ifdef USE_SHADERS
    glUseProgram(context->config->curShaderID);
    #else
    glUseProgram(0);
    #endif

    glBindBuffer(GL_ARRAY_BUFFER, context->config->vbo);
    #ifdef USE_PACKED
    glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * 8) * 4, _quad_vertices_packed, GL_STATIC_DRAW);
    #else
    glBufferData(GL_ARRAY_BUFFER, (3 * 4) * sizeof(float), _quad_vertices_h, GL_STATIC_DRAW);
    #endif

    // Enable shit
    int vPosLoc;
    int vColorLoc;
    int vTexCoordLoc;
    {
        // glBindBuffer(GL_ARRAY_BUFFER, context->config->vbo);
        // glBufferData(GL_ARRAY_BUFFER, (3 * 4) * sizeof(float), _quad_vertices_h, GL_STATIC_DRAW);
        vPosLoc = glGetAttribLocation(context->config->curShaderID, "vPosition");
        CHECK_GL_ERROR("glGetAttribLocation vPosition");

        vTexCoordLoc = glGetAttribLocation(context->config->curShaderID, "vTexCoord");
        CHECK_GL_ERROR("glGetAttribLocation vTexCoord");

        // Enable Vertex Attributes.
        glEnableVertexAttribArray(vPosLoc);
        CHECK_GL_ERROR("glEnableVertexAttribArray vPosLoc");
        
        #ifdef USE_PACKED
        glEnableVertexAttribArray(vTexCoordLoc);
        CHECK_GL_ERROR("glEnableVertexAttribArray vTexCoordLoc");
        #endif
        
        // Offset for vertex positions
        glVertexAttribPointer(vPosLoc, 3, GL_FLOAT, GL_FALSE, STRIDE, 0);
        CHECK_GL_ERROR("glVertexAttribPointer xyz");

        #ifdef USE_PACKED
        glVertexAttribPointer(vTexCoordLoc, 2, GL_FLOAT, GL_FALSE, STRIDE, (void*)(sizeof(float) * 3));
        CHECK_GL_ERROR("glVertexAttribPointer vTexCoord");
        #endif

        // Color data.
        vColorLoc = glGetUniformLocation(context->config->curShaderID, "vColor"); // TODO: Interpolate into vertex data.
        glUniform4fv(vColorLoc, 1, rgba);
        glVertexAttrib4f(vColorLoc, rgba[0], rgba[1], rgba[2], rgba[3]);

        int fUseTexture = glGetUniformLocation(context->config->curShaderID, "useTexture");
        glUniform1i(fUseTexture, (context->private.curBoundTex != 0));
    }

    // Upload to shader
    #ifdef USE_SHADERS
    {
        // Grab uniform locations
        GLint uniMVP = glGetUniformLocation(context->config->curShaderID, "_mvp");

        mat4 i = GLM_MAT4_IDENTITY_INIT;
        // glUniformMatrix4fv(uniMVP, 1, GL_FALSE, (const GLfloat*)i);
        glUniformMatrix4fv(uniMVP, 1, GL_FALSE, (const GLfloat*)context->config->cpu_mvp);
        CHECK_GL_ERROR("glUniformMatrix4fv mvp");
    }
    #endif

    // Tell where in the VBO to start.
    // glVertexPointer(3, GL_FLOAT, 0, 0);

    // Actually finally draw something to the screen.
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, _quad_indices);

    // glDisableClientState(GL_VERTEX_ARRAY);
    CHECK_GL_ERROR("glDrawElements");
    #ifdef USE_SHADERS
    glDisableVertexAttribArray(vPosLoc);
    glDisableVertexAttribArray(vTexCoordLoc);
    #endif
}

void VGL3D_SetClearColor(SELF, vec4 rgba)
{
    glClearColor(rgba[0], rgba[1], rgba[2], rgba[3]);
}

void VGL3D_Clear(SELF)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// =========== Concrete Implementations for OpenGL ES 2.0 Renderer =============
#undef SELF