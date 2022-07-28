#include "vgl3d.h"
#include "vgl3d_gl2es_textures.h"

#ifdef VITA
// as always rinne, thank you for this small code snippet
// Checks to make sure the uer has the shader compiler installed.
// If they don't, display a message and exit gracefully from the process.
static int userHasLibshaccg_private(VGL3DContext* context)
{
    SceCommonDialogConfigParam cmnDlgCfgParam;
    sceCommonDialogConfigParamInit(&cmnDlgCfgParam);

    SceIoStat st1, st2;
    if (!(sceIoGetstat("ur0:/data/libshacccg.suprx", &st1) >= 0 || sceIoGetstat("ur0:/data/external/libshacccg.suprx", &st2) >= 0)) {
        SceMsgDialogUserMessageParam msg_param;
        sceClibMemset(&msg_param, 0, sizeof(SceMsgDialogUserMessageParam));
        msg_param.buttonType = SCE_MSG_DIALOG_BUTTON_TYPE_OK;
        msg_param.msg = (const SceChar8*)"Error: Runtime shader compiler (libshacccg.suprx) is not installed.";
        context->Log(context, "\n\n\nError: Runtime shader compiler (libshacccg.suprx) is not installed.\n\n\n");
        SceMsgDialogParam param;
        sceMsgDialogParamInit(&param);
        param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
        param.userMsgParam = &msg_param;
        sceMsgDialogInit(&param);
        while (sceMsgDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
            vglSwapBuffers(GL_TRUE);
        }
        sceKernelExitProcess(0);
    }

    return 1; // TRUE
}
#endif

// =========== Concrete Implementations for OpenGL ES 2.0 Renderer =============

// Struct implementation.
// Configuration and any extra data.
typedef struct _VGL3DConfig {

#ifndef VITA
    GLFWwindow* game_window;
    const char* game_window_title;
#endif
    uint16_t game_window_width;
    uint16_t game_window_height;

    C_PRIVATE_BEGIN(VGL3DConfig)

    mat4 view;
    mat4 proj;
    mat4 cpu_mvp;
    GLuint vbo;
    GLuint curShaderID;

    C_PRIVATE_END

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
    /* 
    Unused. 
    Replaced by `_VGL3D_LoadAndCreateGLTexture`
    */
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
        .config =           malloc(sizeof(VGL3DConfig)),
        .Begin =            VGL3D_Begin,
        .End =              VGL3D_End,
        .DrawQuad =         VGL3D_DrawQuad,
        .InitBackend =      VGL3D_InitBackend,
        .Log =              VGL3D_Log,
        .SetClearColor =    VGL3D_SetClearColor,
        .Clear =            VGL3D_Clear,
        .SetCamera =        VGL3D_SetCamera,
        .LoadTextureAt =    _VGL3D_LoadAndCreateGLTexture,
        .BindTexture =      VGL3D_BindTexture,
        .private = {
            .curBoundTex = 0,
            .drawingInProgress = 0,
            .doContinue = 1
        }
    };

    newContext.config->game_window_width = 960.f;
    newContext.config->game_window_height = 544.f;
#ifndef VITA
    // Desktop GL properties.
    // Vita is force 960 x 544 and obviously no window title available.
    newContext.config->game_window_title = "VGL3D_GL2ES";
#endif

    return newContext;
}

inline void VGL3D_Begin(SELF) {
    context->private.drawingInProgress = 1;

#ifndef VITA
    context->private.doContinue = !glfwWindowShouldClose(context->config->game_window);
#endif

    // Bind Buffers.

    // Use shaders.

    // Set glVertexAttribPointers

    // Set glUniformMatrix4fv

}

inline void VGL3D_End(SELF) {
    context->private.drawingInProgress = 0;
#ifdef VITA
    vglSwapBuffers(GL_FALSE);
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

#define SAFE_GET_GL_ERROR(x, y) \
switch(x) {\
    case GL_INVALID_ENUM:\
        y = GLINVALIDENUM;\
        break;\
    case GL_INVALID_VALUE:\
        y = GLINVALIDVALUE;\
        break;\
    case GL_INVALID_OPERATION:\
        y = GLINVALIDOP;\
        break;\
    case GL_OUT_OF_MEMORY:\
        y = GLOUTOFMEM;\
        break;\
    case GL_STACK_UNDERFLOW:\
        y = GLSTACKUNDER;\
        break;\
    case GL_STACK_OVERFLOW:\
        y = GLSTACKOVER;\
        break;\
    default:\
        y = GLUNKNOWN;\
        break;\
}\

static inline void CHECK_GL_ERROR(SELF, char* prefix) 
{
    GLenum gl_error = 0;
    const char *error_msg = NULL;
    if((gl_error = glGetError()) != GL_NO_ERROR)
    {
        SAFE_GET_GL_ERROR(gl_error, error_msg);
        context->Log(context, "[%s] OPENGL ERROR: %s\n", prefix, error_msg);
    }

// TODO: Fixme.
// #ifndef VITA
//     if(glfwGetError(&error_buffer))
//     {
//         context->Log(context, "\t\tGLFW: %s", error_buffer);
//     }
// #endif
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

GLuint _VGL3D_LoadShader(SELF, GLenum type, const char *shaderPath)
{
    // Read
    char *shaderSrc = malloc(1);
    size_t shaderSrcLen;
    _VGL3D_ReadShaderFromFile(shaderPath, &shaderSrcLen, &shaderSrc);
    context->Log(context, "READ SHADER %s:\n%s\n\n", shaderPath, shaderSrc);

    // Compile
    GLuint shader;
    GLint compiled;
    
    shader = glCreateShader(type);

    if(shader == 0)
    { 
        context->Log(context, "!!!!!!!   Unable to create shader: shader returned ID of %d\n", shader);
        if(shaderSrc != NULL)
            free(shaderSrc);
        return 0;
    }

    glShaderSource(shader, 1, (const GLchar**)(&shaderSrc), NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if(!compiled)
    {
        context->Log(context, "%s NOT COMPILED.\n", (type == GL_VERTEX_SHADER ? "Vertex Shader" : "Fragment Shader"));
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if(infoLen > 1)
        {
            char *infoLog = (char*)malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            context->Log(context, "\n\nError Compiling Shader:\n\n%s\n", infoLog);
            free(infoLog);
        }
        else context->Log(context, "\tNo failure information available. (infoLen: %d)", infoLen);

        glDeleteShader(shader);

        if(shaderSrc != NULL)
            free(shaderSrc);
        return 0;
    }
    else context->Log(context, "%s compiled! WOOOOOO!", (type == GL_VERTEX_SHADER ? "Vertex Shader" : "Fragment Shader"));

    if(shaderSrc != NULL)
        free(shaderSrc);
    return shader;
}

void _VGL3D_UpdateViewProjection(SELF, mat4* oModelMat) {
    
    glm_mat4_identity(context->config->private.view);
    // View
    glm_look(
        (vec3){_camEyePos[0], _camEyePos[1], _camEyePos[2]},
        (vec3){0.0f, 0.0f, 1.0f},  // Look ("forward" axis)
        (vec3){0.0f, 1.0f, 0.0f},  // Up axis.
        context->config->private.view
    );
    glm_rotate_x(context->config->private.view, glm_rad(_camEyeRot[0]), context->config->private.view);
    glm_rotate_y(context->config->private.view, glm_rad(_camEyeRot[1]), context->config->private.view);
    glm_rotate_z(context->config->private.view, glm_rad(_camEyeRot[2]), context->config->private.view);

#ifdef VITA
    glm_perspective(90.0f, 
        (float)context->config->game_window_width / (float)context->config->game_window_height, 
        0.1f, 10000.0f, 
        context->config->private.proj
    );
#else
    // Projection
    glm_perspective(90.0f, 
        (float)context->config->game_window_width / (float)context->config->game_window_height, 
        0.1f, 10000.0f, 
        context->config->private.proj
    );
#endif
#if 0
    // Make cpu_mvp
    mat4* matrices[3];
    
    #ifdef VITA
    matrices[2] = &context->config->private.proj;
    matrices[1] = &context->config->private.view;
    matrices[0] = &model;
    #else
    matrices[0] = &context->config->private.proj;
    matrices[1] = &context->config->private.view;
    matrices[2] = &model;
    #endif

    glm_mat4_mulN(matrices, 3, context->config->private.cpu_mvp);
#endif
}

void _VGL3D_MakeDefaultViewProjection(SELF) {
    _VGL3D_UpdateViewProjection(context, NULL);
}

int _VGL3D_InitShading(SELF, const char* vPath, const char* fPath) {
    context->Log(context, "Init Shading!\n");

    context->Log(context, "Loading vertex shader from '%s'..\n", vPath);
    GLuint vertShaderID = _VGL3D_LoadShader(context, GL_VERTEX_SHADER, vPath);
    CHECK_GL_ERROR(context, "Vertex Shader");

    GLuint fragShaderID = _VGL3D_LoadShader(context, GL_FRAGMENT_SHADER, fPath);
    CHECK_GL_ERROR(context, "Fragment Shader");

    if(vertShaderID == 0 || fragShaderID == 0)
        return -1;

    GLuint programID = glCreateProgram();
    CHECK_GL_ERROR(context, "Create Program");
    glAttachShader(programID, vertShaderID);
    CHECK_GL_ERROR(context, "Attach Vertex Shader");
    glAttachShader(programID, fragShaderID);
    CHECK_GL_ERROR(context, "Attach Fragment Shader");

    // Bind vertex attribs
    glBindAttribLocation(programID, 0, "vPosition");
    glBindAttribLocation(programID, 1, "vTexCoord");

    glLinkProgram(programID);
    CHECK_GL_ERROR(context, "Linking Program");
    GLint linked;
    glGetProgramiv(programID, GL_LINK_STATUS, &linked);
    if(!linked)
    {
        context->Log(context, "!!!! Failed to link shader '%s'.\n");
        return -1;
    }

    // Grab indexes and everything else.
    context->Log(context, "DONE Init Shading!\n");

    context->config->private.curShaderID = programID;

    return 0;
}

uint16_t _quad_indices[] = 
{
    0,1,2,
    0,2,3
};

// Interleaved vertex data.
#define VGL_COMPONENTS_PER_VERTEX 5
GLfloat _quad_vertices_packed[] =
{
    // bottom left
    //    Vert Pos      |   Tex Coords
    // x     y      z     u     v     ignore
    -0.5f, -0.5f, 1.0f,  0.0f, 1.0f,

    // top left
    -0.5f, 0.5f, 1.0f,  0.0f, 0.0f,

    // top right
    0.5f, 0.5f, 1.0f,  1.0f, 0.0f,

    // bottom right
    0.5f, -0.5, 1.0f, 1.0f, 1.0f,
};

int VGL3D_InitBackend(SELF) {
#ifdef VITA // vitaGL initialization.
    vglInit(0x800000);
    userHasLibshaccg_private(context);
    context->Log(context, "vitaGL has initialized.");
#else // glew/glfw initialization.
    glewExperimental = 1;
    glfwSetErrorCallback(glfwError);

    int glfwReturnVal = glfwInit();
    if(glfwReturnVal == GLFW_FALSE)
    {
        context->Log(context, "Error initializing glfw. Rval: %d\n", glfwReturnVal);
        return -1;
    }

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
    #ifdef VITA
        const char* vert_path = "app0:vert_new.cgv";
        const char* frag_path = "app0:frag_new.cgf";
        context->Log(context, "vert shader path: '%s'\n", vert_path);
        context->Log(context, "frag shader path: '%s'\n", frag_path);
        int shadingVal = _VGL3D_InitShading(context, vert_path, frag_path);
    #else
        int shadingVal = _VGL3D_InitShading(context, "../vert_new.glsl", "../frag_new.glsl");
    #endif
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
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0,0,960,544);

    // Generate vbo
    glGenBuffers(1, &context->config->private.vbo);

    // Bind vbo
    glBindBuffer(GL_ARRAY_BUFFER, context->config->private.vbo);

    // Buffer the initial data.
    glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * VGL_COMPONENTS_PER_VERTEX) * 4, _quad_vertices_packed, GL_STATIC_DRAW);

#ifdef __APPLE__
    // Wiggle the window so glfw gets the correct position upon opening.
    // It's important this happens AFTER the context is initialized.
    int xpos, ypos;
    glfwGetWindowPos(context->config->game_window, &xpos, &ypos);
    glfwSetWindowPos(context->config->game_window, xpos+5, ypos+5);
    glfwSetWindowPos(context->config->game_window, xpos, ypos);
#endif

    // Use shader
    glUseProgram(context->config->private.curShaderID);

    // Bind vbo
    glBindBuffer(GL_ARRAY_BUFFER, context->config->private.vbo);

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

GLfloat _quad_vertices_h[] =
{
    -0.5f, -0.5f, 1.0f, // bottom left
    -0.5f, 0.5f, 1.0f,  // top left
    0.5f, 0.5f, 1.0f,   // top right
    0.5f, -0.5f, 1.0f,  // bottom right
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
    #define STRIDE (sizeof(float) * VGL_COMPONENTS_PER_VERTEX)

    // Build model matrix.
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    #ifdef VITA
    glm_translate(model, (vec3){x, y, z});
    glm_rotate_x(model, glm_rad(rot[0]), model);
    glm_rotate_y(model, glm_rad(rot[1]), model);
    glm_rotate_z(model, glm_rad(rot[2]), model);
    glm_scale(model, scale);
    // glm_mat4_transpose(model);
    #else
    glm_translate(model, (vec3){x, y, z});
    glm_rotate_x(model, glm_rad(rot[0]), model);
    glm_rotate_y(model, glm_rad(rot[1]), model);
    glm_rotate_z(model, glm_rad(rot[2]), model);
    glm_scale(model, scale);
    #endif

    // Build mvp with newly created model matrix.
    _VGL3D_UpdateViewProjection(context, &model);

    // Enable shit...again wasteful for now.
    int vPosLoc = 0;
    int vColorLoc;
    int vTexCoordLoc = 1;
    {
        // Enable Vertex Attributes.
        glEnableVertexAttribArray(vPosLoc);
        CHECK_GL_ERROR(context, "glEnableVertexAttribArray vPosLoc");
        glEnableVertexAttribArray(vTexCoordLoc);
        CHECK_GL_ERROR(context, "glEnableVertexAttribArray vTexCoordLoc");
        
        // Offset for vertex positions
        glVertexAttribPointer(vPosLoc, 3, GL_FLOAT, GL_FALSE, STRIDE, 0);
        CHECK_GL_ERROR(context, "glVertexAttribPointer xyz");
        glVertexAttribPointer(vTexCoordLoc, 2, GL_FLOAT, GL_FALSE, STRIDE, (void*)(sizeof(float) * 3));
        CHECK_GL_ERROR(context, "glVertexAttribPointer vTexCoord");

        // Color data/use texture flag.
        vColorLoc = glGetUniformLocation(context->config->private.curShaderID, "vColor"); // TODO: Interpolate into vertex data.
        glUniform4fv(vColorLoc, 1, rgba);
        int fUseTexture = glGetUniformLocation(context->config->private.curShaderID, "useTexture");
        glUniform1i(fUseTexture, (context->private.curBoundTex != 0));

        // GLint uniMVP = glGetUniformLocation(context->config->private.curShaderID, "_mvp");

        GLint uniM = glGetUniformLocation(context->config->private.curShaderID, "_model");
        GLint uniV = glGetUniformLocation(context->config->private.curShaderID, "_view");
        GLint uniP = glGetUniformLocation(context->config->private.curShaderID, "_projection");

        glUniformMatrix4fv(uniM, 1, GL_FALSE, (const GLfloat*)model);
        glUniformMatrix4fv(uniV, 1, GL_FALSE, (const GLfloat*)context->config->private.view);
        glUniformMatrix4fv(uniP, 1, GL_FALSE, (const GLfloat*)context->config->private.proj);
        CHECK_GL_ERROR(context, "glUniformMatrix4fv mvp");

        // Use this for when you want rgba to be a per-vertex attribute. Not a huge deal to us right at this very moment.
        // glVertexAttrib4f(vColorLoc, rgba[0], rgba[1], rgba[2], rgba[3]);
    }

    // Tell where in the VBO to start.
    // glVertexPointer(3, GL_FLOAT, 0, 0);

    // Actually finally draw something to the screen.
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, _quad_indices);
    // glDrawArrays(GL_TRIANGLES, 0, 6);
    CHECK_GL_ERROR(context, "glDrawElements");
    glDisableVertexAttribArray(vPosLoc);
    glDisableVertexAttribArray(vTexCoordLoc);
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