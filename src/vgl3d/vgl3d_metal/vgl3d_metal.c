// HIGHLY EXPERI(METAL)

#include "../vgl3d.h"
#include "../vgl3d_debug.h"

#include <stdlib.h>
#include <stdio.h>

#define VGL_STRIDE (sizeof(float) * VGL_COMPONENTS_PER_VERTEX)

// Define the configuration structure.
typedef struct _VGL3DConfig {

    // TODO: Metal properties.
    uint16_t game_window_width;
    uint16_t game_window_height;

    // Do you want anything "private"?1
    C_PRIVATE_BEGIN(VGL3DConfig)
    mat4 view;
    mat4 proj;
    C_PRIVATE_END
} VGL3DConfig;

#define SELF VGL3DContext* pSelf
#define TSELF VGL3DContext

TSELF* VGL3D_CreateHeap() {

}

inline TSELF VGL3D_Create() {
    VGL3DContext metalContext = (TSELF) {
        .config =                        malloc(sizeof(TSELF)),
        .Begin =                         VGL3D_Begin,
        .End =                           VGL3D_End,
        .DrawQuad =                      VGL3D_DrawQuad,
        .DrawFromVBO =                   VGL3D_DrawFromVBO,
        .InitBackend =                   VGL3D_InitBackend,
        .Log =                           VGL3D_Log,
        .SetClearColor =                 VGL3D_SetClearColor,
        .Clear =                         VGL3D_Clear,
        .SetCamera =                     VGL3D_SetCamera,
        .LoadTextureAt =                 VGL3D_LoadTextureAt,
        .BindTexture =                   VGL3D_BindTexture,
        .DestroyBackend =                VGL3D_DestroyBackend,
        .DestroySelf =                   VGL3D_DestroySelf,
        .DestroyTexture =                VGL3D_DestroyTexture,
        .CreateVBOWithVertexData =       VGL3D_CreateVBOWithVertexData,
        .SetProjectionType =             VGL3D_SetProjectionType,
        .DrawFromVBOTranslation =        VGL3D_DrawFromVBOTranslation,
        .DrawFromVBOTranslationIndices = VGL3D_DrawFromVBOTranslationIndices,
        .private = {
            .curBoundTex = 0,
            .drawingInProgress = 0,
            .doContinue = 1,
            .projectionMatrixType = VGL3D_PROJECTION_IDENTITY,
        },
    };

    metalContext.config->game_window_width = 960.f;
    metalContext.config->game_window_height = 544.f;

    return metalContext;
}

int VGL3D_InitBackend(SELF) {
    // Init Metal backend.
    pSelf->Log(pSelf, "Metal backend initializing....");
    
    pSelf->Log(pSelf, "Metal backend initialized!");

    return 0;
}


#undef TSELF
#undef SELF