#ifdef __cplusplus
extern "C" {
#endif

#ifndef __VGL_3D_H__
#define __VGL_3D_H__

#if defined(__APPLE__) || defined(PC_BUILD) 
// glew header
#include <GL/glew.h>
#ifndef __APPLE__
// GL header for Linux and presumably Windows. Dunno.
#include <GL/gl.h>
#else
// glu header for Apple Mac OS X.
#include <OpenGL/glu.h>
#endif
// glfw header
#include <GLFW/glfw3.h>
#else
// PS Vita headers.
#include <vitasdk.h>
#include <vitaGL.h>
#include <psp2/gxm.h>
#include <psp2/kernel/clib.h>
#include <psp2/io/stat.h>
#endif

#ifndef GLuint
#define GLuint uint32_t
#endif

#ifndef VTEX
#define VTEX GLuint
#endif

#include <stdarg.h>
#include <cglm/cglm.h>
#include <cglm/clipspace/ortho_lh_zo.h>

#define C_PRIVATE_BEGIN(N)  struct N##_private {
#define C_PRIVATE_END       } private;

#ifndef SELF
#define SELF struct _VGL3D* context
#endif

// Forward declare per platform configuration structure.
struct _VGL3DConfig;

/**
 * @brief VGL3DContext is essentially a fancy vtable.
 * These are your API functions! Call them on this structure to ensure
 * the correct function is called for your rendering platform.
 * 
 * If you need some more functionality, you can even override and hook into these yourself!
 * We have private data, but we're not gonna stop you from fucking with it anyway!
 */
typedef struct _VGL3D {

    // API Functions!!
    int     (*InitBackend)(SELF);
    void    (*Log)(SELF, const char *fmt, ...);
    void    (*Begin)(SELF);
    void    (*End)(SELF);
    void    (*Clear)(SELF);
    void    (*SetClearColor)(SELF, vec4 rgba);
    void    (*DrawQuad)(SELF, float x, float y, float z, vec3 rot, vec3 scale, vec4 rgba);
    void    (*SetCamera)(SELF, vec3 pos, vec3 rot_deg);
    VTEX    (*LoadTextureAt)(SELF, const char *path);
    void    (*BindTexture)(SELF, VTEX tex);
    void    (*DestroyTexture)(SELF, VTEX texToDestroy);
    void    (*DestroyBackend)(SELF);
    void    (*DestroySelf)(SELF);

    // Config. Modifiable
    struct  _VGL3DConfig* config;

    // 'private' data that someone could still fuck with if they wanted to.
    C_PRIVATE_BEGIN(VGL3D)
    uint8_t drawingInProgress;
    uint8_t doContinue;
    VTEX    curBoundTex;
    C_PRIVATE_END;

} VGL3DContext;

#undef SELF

#define SELF VGL3DContext* context

/**
 * @brief Default VGL3D log function.
 * Behaves like printf. 
 * New line is automatically appended to the end.
 * 
 * @param context   VGL3D context
 * @param fmt       Format string.
 * @param ...       va args to fill in the format string.
 */
static inline void VGL3D_Log(SELF, const char *fmt, ...)
{
    char buffer[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 2048, fmt, args);
    printf("[VGL3DLog] %s\n", buffer);
    va_end(args);
}

// ------------------------------ Default/reference function signatures. ------------------------------ 
VGL3DContext    VGL3D_Create();
void            VGL3D_Begin(SELF);
void            VGL3D_End(SELF);
int             VGL3D_InitBackend(SELF);
void            VGL3D_DrawQuad(SELF, float x, float y, float z, vec3 rot, vec3 scale, vec4 rgba);
VTEX            VGL3D_LoadTextureAt(SELF, const char *path);
void            VGL3D_BindTexture(SELF, VTEX tex);
void            VGL3D_SetCamera(SELF, vec3 pos, vec3 rot_deg);
void            VGL3D_SetClearColor(SELF, vec4 rgba);
void            VGL3D_Clear(SELF);
void            VGL3D_DestroyTexture(SELF, VTEX texToDestroy);
void            VGL3D_DestroyBackend(SELF);
void            VGL3D_DestroySelf(SELF);
// ------------------------------ Default/reference function signatures. ------------------------------ 

#undef SELF

#endif
#ifdef __cplusplus
}
#endif