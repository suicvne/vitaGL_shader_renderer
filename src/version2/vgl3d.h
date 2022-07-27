#ifdef __cplusplus
extern "C" {
#endif

#ifndef __VGL_3D_H__
#define __VGL_3D_H__

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
#include <psp2/gxm.h>
#include <psp2/kernel/clib.h>
#include <psp2/io/stat.h>

// as always rinne, thank you for this small code snippet
// Checks to make sure the uer has the shader compiler installed.
// If they don't, display a message and exit gracefully from the process.
static int _userHasLibshaccg()
{
    SceCommonDialogConfigParam cmnDlgCfgParam;
    sceCommonDialogConfigParamInit(&cmnDlgCfgParam);

    SceIoStat st1, st2;
    if (!(sceIoGetstat("ur0:/data/libshacccg.suprx", &st1) >= 0 || sceIoGetstat("ur0:/data/external/libshacccg.suprx", &st2) >= 0)) {
        SceMsgDialogUserMessageParam msg_param;
        sceClibMemset(&msg_param, 0, sizeof(SceMsgDialogUserMessageParam));
        msg_param.buttonType = SCE_MSG_DIALOG_BUTTON_TYPE_OK;
        msg_param.msg = (const SceChar8*)"Error: Runtime shader compiler (libshacccg.suprx) is not installed.";
        _debugPrintf("\n\n\nError: Runtime shader compiler (libshacccg.suprx) is not installed.\n\n\n");
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

#ifndef GLuint
#define GLuint uint32_t
#endif

#ifndef VTEX
#define VTEX GLuint
#endif

#include <stdarg.h>
#include <memory.h>
#include <cglm/cglm.h>
#include <cglm/clipspace/ortho_lh_zo.h>

#define C_PRIVATE_BEGIN(N)  struct N##_private {
#define C_PRIVATE_END       } private;

#ifndef SELF
#define SELF struct _VGL3D* context
#endif

// Forward declare config.
struct _VGL3DConfig;

typedef struct _VGL3D {

    // API Functions!!
    void    (*Begin)(SELF);
    void    (*DrawQuad)(SELF, float x, float y, float z, vec3 rot, vec3 scale, vec4 rgba);
    void    (*End)(SELF);
    int     (*InitBackend)(SELF);
    void    (*Log)(SELF, const char *fmt, ...);
    void    (*SetClearColor)(SELF, vec4 rgba);
    void    (*Clear)(SELF);
    void    (*SetCamera)(SELF, vec3 pos, vec3 rot_deg);
    VTEX    (*LoadTextureAt)(SELF, const char *path);
    void    (*BindTexture)(SELF, VTEX tex);

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

static inline void VGL3D_Log(VGL3DContext* context, const char *fmt, ...)
{
    char buffer[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 2048, fmt, args);
    printf("[VGL3DLog] %s\n", buffer);
    va_end(args);

#ifdef VITA
    // Send to debugNetPrintf on Vita.
#endif
}

/// Public facing function to create a nicely initialized VGL3DContext.
VGL3DContext VGL3D_Create();

void VGL3D_Begin(SELF);
void VGL3D_End(SELF);

/// Initializes the backends necessary to render.
/// PC: (GlFW, GlEW, GL); Vita: VGL
int VGL3D_InitBackend(SELF);

void VGL3D_DrawQuad(
    SELF,
    float x, 
    float y, 
    float z, 
    vec3 rot, 
    vec3 scale, 
    vec4 rgba
);
VTEX VGL3D_LoadTextureAt(SELF, const char *path);
void VGL3D_BindTexture(SELF, VTEX tex);
void VGL3D_SetCamera(SELF, vec3 pos, vec3 rot_deg);
void VGL3D_SetClearColor(SELF, vec4 rgba);
void VGL3D_Clear(SELF);

#undef SELF

#endif

#ifdef __cplusplus
}
#endif