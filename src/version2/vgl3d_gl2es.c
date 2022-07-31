#include "vgl3d.h"
#include "vgl3d_gl2es_textures.h"
#include "vgl3d_debug.h"

#include <stdio.h>
#include <stdlib.h>

#define VGL_STRIDE (sizeof(float) * VGL_COMPONENTS_PER_VERTEX)

// =========== Concrete Implementations for OpenGL ES 2.0 Renderer =============

/**
 * @brief OpenGL ES 2.0 configuration.
 * - (PC ONLY) GLFWwindow and title.
 * - game_window_width / game_window_height (which is really the resolution.)
 * - Private runtime data:
 *      - current view matrix
 *      - current projection matrix
 *      - (NOT USED) completed MVP.
 *      - The default quad quadVbo handle.
 *      - The default shader program handle.
 */
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
    GLuint quadVbo;
    GLuint curShaderID;

    C_PRIVATE_END

} VGL3DConfig;

vec3 camEyePos_p = {0.0f, 0.0f, -2.0f};
vec3 camEyeRot_p = {0.0f, 0.0f, 0.0f};

#ifndef SELF
#define SELF VGL3DContext* context
#endif

GLFWwindow* VGL3D_GetGlfwWindow_glfw(SELF);

/**
 * @brief Creates a VGL3DContext on the stack.
 * 
 * Initializes default function pointers.
 * Initializes private variables to sane defaults.
 * Sets config->game_window_width/height to sane defaults (960 x 544)
 * (PC only) Sets game_window_title
 * 
 * This function will malloc for sizeof(VGL3DConfig).
 * The size of VGL3DConfig will vary per platform.
 * 
 * @return VGL3DContext: A fully setup and ready-to-use VGL3DContext.
 * 
 * vgl3d_gl2es.c
 */
inline VGL3DContext VGL3D_Create()
{
    // TODO: libGimbal'fy? 
    VGL3DContext newContext = (VGL3DContext) {
        .config =                        malloc(sizeof(VGL3DConfig)),
        .Begin =                         VGL3D_Begin,
        .End =                           VGL3D_End,
        .DrawQuad =                      VGL3D_DrawQuad,
        .DrawFromVBO =                   VGL3D_DrawFromVBO,
        .InitBackend =                   VGL3D_InitBackend,
        .Log =                           VGL3D_Log,
        .SetClearColor =                 VGL3D_SetClearColor,
        .Clear =                         VGL3D_Clear,
        .SetCamera =                     VGL3D_SetCamera,
        .LoadTextureAt =                 VGL3D_LoadAndCreateGLTexture_private,
        .BindTexture =                   VGL3D_BindTexture,
        .DestroyBackend =                VGL3D_DestroyBackend,
        .DestroySelf =                   VGL3D_DestroySelf,
        .DestroyTexture =                VGL3D_DestroyTexture,
        .GetGlfwWindow =                 VGL3D_GetGlfwWindow_glfw,
        .CreateVBOWithVertexData =       VGL3D_CreateVBOWithVertexData,
        .SetProjectionType =             VGL3D_SetProjectionType,
        .DrawFromVBOTranslation =        VGL3D_DrawFromVBOTranslation,
        .DrawFromVBOTranslationIndices = VGL3D_DrawFromVBOTranslationIndices,
        .private = {
            .curBoundTex = 0,
            .drawingInProgress = 0,
            .doContinue = 1,
            .projectionMatrixType = VGL3D_PROJECTION_IDENTITY
        }
    };

    newContext.config->game_window_width = 960.f;
    newContext.config->game_window_height = 544.f;

#ifndef VITA
    // Only makes sense to set this on the desktop.
    newContext.config->game_window_title = "VGL3D_GL2ES";
#endif

    return newContext;
}

GLFWwindow* VGL3D_GetGlfwWindow_glfw(SELF) {
    return context->config->game_window;
}

/**
 * @brief Binds a texture given a VGL3DContext and VTEX texture.
 * VTEX is a texture handle defined per-platform.
 * 
 * @param tex VTEX texture handle.
 * 
 * @example
 * VGL3DContext context = VGL3D_Create();
 * // Usual VGL3D setup
 * VTEX thisTex = context.LoadTextureAt(&context, "/path/to/texture.png");
 * context.BindTexture(&context, thisTex);
 * // Proceeding graphics.Draw calls will use this texture.
 * * vgl3d_gl2es.c
 */
void VGL3D_BindTexture(SELF, VTEX tex) {
    glBindTexture(GL_TEXTURE_2D, tex);
    context->private.curBoundTex = tex;
}

/**
 * @brief Unused for this platform.
 * Overriden in VGL3D_Create with VGL3D_LoadAndCreateGLTexture_private
 * 
 * @param path 
 * @return VTEX Ready-to-use texture handle. Returns -1 if there is a problem.
 * 
 * @example
 * VGL3DContext context = VGL3D_Create();
 * // Usual VGL3D setup
 * VTEX thisTex = context.LoadTextureAt(&context, "/path/to/texture.png");
 * 
 * * vgl3d_gl2es.c
 */
VTEX VGL3D_LoadTextureAt(SELF, const char *path) {
    /* 
    Unused. 
    Replaced by `VGL3D_LoadAndCreateGLTexture_private`
    */
    return -1;
}

/**
 * @brief Sets camera position and rotation given the passed values.
 * 
 * @param SELF = context 
 * @param pos World space position for the camera eye.
 * @param rot_deg X/Y/Z rotation values in degrees.
 * * vgl3d_gl2es.c
 */
void VGL3D_SetCamera(VGL3DContext* context, vec3 pos, vec3 rot_deg) {
    glm_vec3_copy(pos, camEyePos_p);
    glm_vec3_copy(rot_deg, camEyeRot_p);
}

/**
 * @brief Tells VGL3D that we are ready to draw stuff to the screen.
 * 
 * @param SELF = context
 * * vgl3d_gl2es.c
 */
inline void VGL3D_Begin(SELF) {
    context->private.drawingInProgress = 1;

#ifndef VITA
    context->private.doContinue = !glfwWindowShouldClose(context->config->game_window);
#endif
    // TODO Maybe?
    // Bind Buffers.
    // Use shaders.
    // Set glVertexAttribPointers
    // Set glUniformMatrix4fv
}

/**
 * @brief Tells VGL3D that we are done drawing stuff to the screen.
 * For gl2es, this will swap buffers. On desktop glfw, this will also poll events.
 * 
 * @param SELF = context
 * * vgl3d_gl2es.c
 */
inline void VGL3D_End(SELF) {
    context->private.drawingInProgress = 0;
#ifdef VITA
    vglSwapBuffers(GL_FALSE);
#else
    glfwSwapBuffers(context->config->game_window);
    glfwPollEvents();
#endif
}

/**
 * @brief Private function.
 * Uses stdio to open and read the contents of the file into *buffer.
 * 
 * *buffer should be allocated with *something* before passing it in.
 * This function will realloc for the filesize.
 * 
 * 
 * @param path      The absolute path to the shader file to be loaded.
 * @param fsize     A pointer to toss the buffer size into.
 * @param buffer    An allocated buffer to put the contents of the file into. 
 * @return int returns 0 if successful, -1 if unsuccessful.
 * If successful, *buffer will contain null terminated shader text data.
 * 
 * @example
 *  char *shaderSrc = malloc(1);
 *  size_t shaderSrcLen;
 *  VGL3D_ReadShaderFromFile_private(shaderPath, &shaderSrcLen, &shaderSrc);
 */
static inline int VGL3D_ReadShaderFromFile_private(const char* path, size_t* fsize, char** buffer)
{
    // Open the file, reading only.
    FILE *_file = fopen(path, "r");

    // Couldn't open? Return -1 to indicate error.
    if(_file == NULL)
        return -1;

    // Seek to end. Ask for file size. Seek to beginning to read.
    fseek(_file, 0, SEEK_END);
    *fsize = ftell(_file);
    fseek(_file, 0, SEEK_SET);

    // Realloc the buffer
    *buffer = (char*)realloc(*buffer, (*fsize) + 1);
    // Reads *fsize bytes into *buffer.
    fread(*buffer, *fsize, 1, _file);

    // Null terminate. 
    size_t offset = *fsize;
    (*buffer)[offset] = '\0';

    // Close the file.
    fclose(_file);
    return 0;
}

/**
 * @brief Private function.
 * Loads a shader of a given GLenum type at the given path.
 * Reads the file, creates the shader, loads the shader source through OpenGL.
 * Then, we compile the shader, check for errors, and returns the shader handle if successful.
 * 
 * 
 * @param type          Type of shader. GL Vert Shader or GL Frag Shader.
 * @param shaderPath    Absolute path to the shader file to load. Extension doesn't matter.
 * @return GLuint       Returns 0 if there was a problem, or the handle to the created shader.
 */
static inline GLuint VGL3D_LoadShader_private(SELF, GLenum type, const char *shaderPath)
{
    // Read
    char *shaderSrc = malloc(1);
    size_t shaderSrcLen;
    VGL3D_ReadShaderFromFile_private(shaderPath, &shaderSrcLen, &shaderSrc);
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

/**
 * @brief Private function.
 * Updates the view & projection matrices that are stored in VGL3DConfig.
 * 
 * Uses glm_look to set the view matrix position given camEyePos_p.
 * Uses glm_rotate_ to set the view matrix rotation given camEyeRot_p.
 * Uses glm_perspective to set a 90º FOV projection matrix. near=0.1f, far=10000.0f
 * 
 * @param oModelMat (Do not use this parameter. To be removed.)
 */
void VGL3D_UpdateViewProjection_private(SELF, mat4* oModelMat) {
    
    glm_mat4_identity(context->config->private.view);
    // View
    glm_look(
        (vec3){camEyePos_p[0], camEyePos_p[1], camEyePos_p[2]},
        (vec3){0.0f, 0.0f, 1.0f},  // Look ("forward" axis)
        (vec3){0.0f, 1.0f, 0.0f},  // Up axis.
        context->config->private.view
    );

    // x, y, z rotation from camEyeRot_p
    glm_rotate_x(context->config->private.view, glm_rad(camEyeRot_p[0]), context->config->private.view);
    glm_rotate_y(context->config->private.view, glm_rad(camEyeRot_p[1]), context->config->private.view);
    glm_rotate_z(context->config->private.view, glm_rad(camEyeRot_p[2]), context->config->private.view);

    const float closePlane = 0.0001f;
    const float farPlane   = 1000.f;
    switch(context->private.projectionMatrixType) {
        case VGL3D_PROJECTION_ORTHOGRAPHIC:
            glm_ortho(
                (float)context->config->game_window_width / 2.f, 
                (float)-(context->config->game_window_width / 2.f), 
                (float)-(context->config->game_window_height / 2.f), 
                (float)context->config->game_window_height / 2.f, 
                closePlane, farPlane, context->config->private.proj
            );
            break;
        case VGL3D_PROJECTION_PERSPECTIVE:
            glm_perspective(
                glm_rad(45.0f), 
                (float)context->config->game_window_width / (float)context->config->game_window_height, 
                closePlane, farPlane, 
                context->config->private.proj
            );
            break;
        case VGL3D_PROJECTION_IDENTITY:
        default:
            glm_mat4_identity(context->config->private.proj);
            break;
    }
}

/**
 * @brief Initializes the shading system given a vertex shader path and a frag shader path.
 * 
 * Creates the program, attaches the shaders, bind attrib locations, link program.
 * Sets context->config->private.curShaderID to the created full shading program.
 * 
 * @param vPath 
 * @param fPath 
 * @return int Returns 0 on success, -1 or other for error.
 */
int VGL3D_InitShading_private(SELF, const char* vPath, const char* fPath) {
    context->Log(context, "Init Shading!\n");

    // Load vertex shader. Check for errors.
    GLuint vertShaderID = VGL3D_LoadShader_private(context, GL_VERTEX_SHADER, vPath);
    CHECK_GL_ERROR(context, "Vertex Shader");

    // Load frag shader. Check for errors.
    GLuint fragShaderID = VGL3D_LoadShader_private(context, GL_FRAGMENT_SHADER, fPath);
    CHECK_GL_ERROR(context, "Fragment Shader");

    // Early return if we've failed to load one or the other.
    if(vertShaderID == 0 || fragShaderID == 0)
        return -1;

    // Create program and attach shaders.
    GLuint programID = glCreateProgram();
    CHECK_GL_ERROR(context, "Create Program");
    glAttachShader(programID, vertShaderID);
    CHECK_GL_ERROR(context, "Attach Vertex Shader");
    glAttachShader(programID, fragShaderID);
    CHECK_GL_ERROR(context, "Attach Fragment Shader");

    // Bind vertex attribs
    glBindAttribLocation(programID, 0, "vPosition");
    glBindAttribLocation(programID, 1, "vTexCoord");

    // Link program.
    glLinkProgram(programID);
    CHECK_GL_ERROR(context, "Linking Program");

    // Check for errors.
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

// Quad indices. Indices from quad_vertices_packed_private
static uint16_t quad_indices_private[] = 
{
    0,1,2,
    0,2,3
};

// Interleaved vertex data.
#define VGL_COMPONENTS_PER_VERTEX 5
static GLfloat quad_vertices_packed_private[] =
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

/**
 * @brief Destroys a texture handle.
 * 
 * @param texToDestroy The texture handle to free/destroy.
 */
void VGL3D_DestroyTexture (SELF, VTEX texToDestroy) {
    if(texToDestroy == 0)
    {
        context->Log(context, "Note: attempted to destroy texture with invalid texture ID of 0.");
        return;
    }

    glDeleteTextures(1, &texToDestroy);
    CHECK_GL_ERROR(context, "glDeleteTextures");

    context->Log(context, "Destroyed texture with ID %u", texToDestroy);
}

/**
 * @brief Destroys the backend.
 * Calls GL free functions.
 * Deletes vertex buffers and shaders.
 * Terminates glfw on PC.
 * Terminates vitaGL on Vita.
 */
void VGL3D_DestroyBackend(SELF) {
    context->Log(context, "Destroy Backend.");

    // Free OpenGL resources.
    glDeleteBuffers(1, &context->config->private.quadVbo);
    CHECK_GL_ERROR(context, "glDeleteTextures");

    // Destroy shader
    glDeleteProgram(context->config->private.curShaderID);
    CHECK_GL_ERROR(context, "glDeleteProgram");

#ifndef VITA
    // Free glfw window. Terminate glfw.
    glfwDestroyWindow(context->config->game_window);
    glfwTerminate();
#else
    vglEnd();
#endif

    
}

/**
 * @brief Actually destroys the VGL3DContext structure.
 * Calls free on context->config.
 */
void VGL3D_DestroySelf(SELF) {
    context->Log(context, "Destroy self. (internal config malloced)");

    // free config.
    if(context->config != NULL)
    {
        context->Log(context, "Freeing %u byte config.\n", sizeof(VGL3DConfig));
        free(context->config);
    }
}

/**
 * @brief Initializes the rendering backend.
 * For vgl3d_gl2es, this is OpenGL 2.0ES/vitaGL.
 * This is a fairly hefty function:
 * 1. Initializes vitaGL or glfw/glew on PC.
 * 2. Initializes shading system and loads default shaders per platform.
 *      CG shaders for Vita, GLSL for desktop.
 * 3. Updates the view/projection matrix.
 * 4. Enables gl stuff. generates VBO. Buffers the identity quad to the GPU.
 * (OS X ONLY): Wiggles the window to properly set size.
 * 
 * @return int Returns 0 on success, -1 or other on failure. Afterwards, the context is fully ready to use.
 */
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
    // glfwSetKeyCallback(context->config->game_window, __key_callback);
    
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
    int shadingVal = VGL3D_InitShading_private(context, vert_path, frag_path);
#else
    int shadingVal = VGL3D_InitShading_private(context, "../vert_new.glsl", "../frag_new.glsl");
#endif

    if(shadingVal != 0)
    {
        context->Log(context, "Rval from Shading: %d\n", shadingVal);
        return -1;
    }

    VGL3D_UpdateViewProjection_private(context, NULL);

    // TODO: VGL3D_SetClearColor
    // TODO: VGL3D_SetDepthFunc
    // TODO: VGL3D_SetBlendFunc.
    // TODO: VGL3D_SetViewport

    glClearColor(0.f, 0.f, 0.5f, 1.0f);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    // glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_ALWAYS);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0,0,960,544);

    // Generate quadVbo
    glGenBuffers(1, &context->config->private.quadVbo);

    // Bind quadVbo
    glBindBuffer(GL_ARRAY_BUFFER, context->config->private.quadVbo);

    // Buffer the initial data.
    glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * VGL_COMPONENTS_PER_VERTEX) * 4, quad_vertices_packed_private, GL_STATIC_DRAW);

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

    // Bind quadVbo
    glBindBuffer(GL_ARRAY_BUFFER, context->config->private.quadVbo);

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
GLfloat quad_vertices_packed_private[] =
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

static inline void VGL3D_private_InitializeDefaultVertexAttribs(SELF, mat4* modelMat, vec4 rgba) {
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
        glVertexAttribPointer(vPosLoc, 3, GL_FLOAT, GL_FALSE, VGL_STRIDE, 0);
        CHECK_GL_ERROR(context, "glVertexAttribPointer xyz");
        glVertexAttribPointer(vTexCoordLoc, 2, GL_FLOAT, GL_FALSE, VGL_STRIDE, (void*)(sizeof(float) * 3));
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

        glUniformMatrix4fv(uniM, 1, GL_FALSE, (const GLfloat*)(*modelMat));
        glUniformMatrix4fv(uniV, 1, GL_FALSE, (const GLfloat*)context->config->private.view);
        glUniformMatrix4fv(uniP, 1, GL_FALSE, (const GLfloat*)context->config->private.proj);
        CHECK_GL_ERROR(context, "glUniformMatrix4fv mvp");

        // Use this for when you want rgba to be a per-vertex attribute. Not a huge deal to us right at this very moment.
        // glVertexAttrib4f(vColorLoc, rgba[0], rgba[1], rgba[2], rgba[3]);
    }
}

/**
 * @brief Draws a quad given pos, rot, scale, and rgba.
 * This will build a model matrix given the arguments and send it to the GPU.
 * Calls glDrawElements to draw a quad given a VBO.
 * 
 * @param x     x position of the first vertex
 * @param y     y position of the first vertex
 * @param z     z position of the first vertex
 * @param rot   rotation of the quad.
 * @param scale scale of the quad.
 * @param rgba  color of the quad (not per vertex coloring.)
 */
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

    // glBindBuffer -> default quad 
    glBindBuffer(GL_ARRAY_BUFFER, context->config->private.quadVbo);
    CHECK_GL_ERROR(context, "glBindBuffer -> quadVbo");

    // Build mvp with newly created model matrix.
    VGL3D_UpdateViewProjection_private(context, &model);

    // Initialize default vertex attribs.
    VGL3D_private_InitializeDefaultVertexAttribs(context, &model, rgba);

    // Actually finally draw something to the screen.
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, quad_indices_private);
    CHECK_GL_ERROR(context, "glDrawElements");
}

/**
 * @brief Sets the screen clear color.
 * 
 * @param rgba 
 */
void VGL3D_SetClearColor(SELF, vec4 rgba)
{
    glClearColor(rgba[0], rgba[1], rgba[2], rgba[3]);
}

/**
 * @brief Clears the depth buffer and color buffer. 
 * (Clears the screen)
 * 
 */
void VGL3D_Clear(SELF)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

uint32_t VGL3D_CreateVBOWithVertexData(SELF, const float* packedVertexData, size_t nVertices) {
    uint32_t resultingHandle = 0;
    size_t totalBufferSize =   (sizeof(float) * VGL_COMPONENTS_PER_VERTEX) * nVertices;

    context->Log(context, "Creating VBO with %u vertices. Total Size: %u bytes.", nVertices, totalBufferSize);

    glGenBuffers(1, &resultingHandle);
    CHECK_GL_ERROR(context, "VGL3D_CreateVBOWithVertexData/glGenBuffers");

    glBindBuffer(GL_ARRAY_BUFFER, resultingHandle);
    CHECK_GL_ERROR(context, "VGL3D_CreateVBOWithVertexData/glBindBuffer");

    glBufferData(GL_ARRAY_BUFFER, totalBufferSize, packedVertexData, GL_STATIC_DRAW);
    CHECK_GL_ERROR(context, "VGL3D_CreateVBOWithVertexData/glBufferData");

    return resultingHandle;
}

void VGL3D_DrawFromVBO(SELF, uint32_t vboHandle, size_t nVertices) {

    mat4 model = GLM_MAT4_IDENTITY_INIT;
    const float scale = 0.5f;
    const vec4 white = (const vec4){1.0f, 1.0, 1.0f, 1.0f};
    glm_scale(model, (vec3){scale,scale,scale});

    VGL3D_UpdateViewProjection_private(context, &model);

    glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
    VGL3D_private_InitializeDefaultVertexAttribs(context, &model, (float*)white);
    glDrawArrays(GL_TRIANGLES, 0, nVertices);
}

void VGL3D_DrawFromVBOTranslationIndices(SELF, uint32_t vboHandle, size_t nVertices, vec3 pos, vec3 rot, vec3 scale, uint32_t* indices, size_t nIndices) {
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    const vec4 white = (const vec4){1.0f, 1.0, 1.0f, 1.0f};
    // TRS -> Translate, Rotate, Scale
    glm_translate(model, pos);
    glm_rotate_x(model, rot[0], model);
    glm_rotate_y(model, rot[1], model);
    glm_rotate_z(model, rot[2], model);
    glm_scale(model, scale);

    VGL3D_UpdateViewProjection_private(context, &model);

    glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
    VGL3D_private_InitializeDefaultVertexAttribs(context, &model, (float*)white);

    // glDrawElements(GL_TRIANGLES, nVertices, GL_UNSIGNED_INT, indices);
    glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, indices);
}

void VGL3D_DrawFromVBOTranslation(SELF, uint32_t vboHandle, size_t nVertices, vec3 pos, vec3 rot, vec3 scale) {

    mat4 model = GLM_MAT4_IDENTITY_INIT;
    const vec4 white = (const vec4){1.0f, 1.0, 1.0f, 1.0f};
    // TRS -> Translate, Rotate, Scale
    glm_translate(model, pos);
    glm_rotate_x(model, rot[0], model);
    glm_rotate_y(model, rot[1], model);
    glm_rotate_z(model, rot[2], model);
    glm_scale(model, scale);

    VGL3D_UpdateViewProjection_private(context, &model);

    glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
    VGL3D_private_InitializeDefaultVertexAttribs(context, &model, (float*)white);

    // Testing
    // for(int i = 0; i < nVertices; i += 3) {
        // glDrawArrays(GL_TRIANGLES, i * (sizeof(float) * VGL_COMPONENTS_PER_VERTEX), 3);
    // }

    // glDrawElements(GL_TRIANGLES, nVertices, GL_UNSIGNED_SHORT, 0);


    glDrawArrays(GL_TRIANGLES, 0, nVertices);

    // glDrawArrays(GL_TRIANGLE_STRIP, 0, nVertices);
    
}

void VGL3D_VBOBuffer(SELF, uint32_t vboHandle, const float* vertexData, size_t nVertices) {

    const size_t totalBufferSize = (sizeof(float) * VGL_COMPONENTS_PER_VERTEX) * nVertices;
    context->Log(context, "Vbo Handle %u is getting re-buffered with %u bytes of data.", vboHandle, totalBufferSize);

    glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
    CHECK_GL_ERROR(context, "VGL3D_VBOBuffer/glBindBuffer");

    glBufferData(GL_ARRAY_BUFFER, totalBufferSize, vertexData, GL_STATIC_DRAW);
    CHECK_GL_ERROR(context, "VGL3D_VBOBuffer/glBufferData");
}

// =========== Concrete Implementations for OpenGL ES 2.0 Renderer =============
#undef SELF