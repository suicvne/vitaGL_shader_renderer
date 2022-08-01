#include "tesla_input.h"

#include <memory.h>
#include <assert.h>
#include <GLFW/glfw3.h>

#define SELF TestMouse* pSelf

typedef struct _TeslaMousePrivate {
    GLFWwindow* WindowListening;
    uint32_t    mouseButtons[8];
    uint32_t    lastMouseButtons[8];
    uint8_t     mouseButtonsLen;
} TeslaMousePrivate;

static TestMouse* TMouse_private_LastCtx = NULL;

void TMouse_private_glfwMouseCallback(GLFWwindow* window, int button, int action, int mods) {
    
    TestMouse* pSelf = TMouse_private_LastCtx;

    pSelf->PrivateData->mouseButtons[button] = action;
    
    pSelf->Log(pSelf, "TMouse callback. Button: %d, Action: %d", button, pSelf->PrivateData->mouseButtons[button]);
}

// TODO: glfwMouseMoveCallback
void TMouse_private_glfwMouseMoveCallback() {}

int TMouse_InitBackend_glfw(SELF, GLFWwindow* existing) { 
    // Store pointer to this context.
    TMouse_private_LastCtx = pSelf;

    if(existing != NULL) {
        pSelf->Log(pSelf, "Using existing GLFWwindow for mouse.");

        pSelf->PrivateData->WindowListening = existing;
        glfwSetMouseButtonCallback(existing, TMouse_private_glfwMouseCallback);
        
        // Log
        pSelf->Log(pSelf, "TMouse Created!");
        return 0;
    } else {
        pSelf->Log(pSelf, "GLFWwindow passed to Mouse was NULL!");
    }

    return 0; 
}

void TMouse_DestroySelf(SELF) { 
    return; 
}

void TMouse_PollInput(SELF) { 
    TMouse_private_LastCtx = pSelf; 

    for(int i = 0; i < pSelf->PrivateData->mouseButtonsLen; i++) {
        if(pSelf->PrivateData->mouseButtons[i] == GLFW_PRESS) {

        }
    }

    for(int i = 0; i < GLFW_KEY_LAST; i++) {
        pSelf->PrivateData->lastMouseButtons[i] = 
            pSelf->PrivateData->mouseButtons[i];
    }

    return; 
}

int TMouse_IsButtonDown(SELF, uint32_t btn) {
    return 0; 
}

int TMouse_IsButtonUp(SELF, uint32_t btn) { 
    return 0; 
}

int TMouse_IsButtonHeld(SELF, uint32_t btn) { 
    return 0; 
}

TestMouse TMouse_Create() {

// #define TMOUSE_INLINE
#ifdef TMOUSE_INLINE
    TestMouse tm = (TestMouse){
        .PrivateData =  malloc(sizeof(TeslaMousePrivate)),
        .InitBackend =  TMouse_InitBackend_glfw,
        .DestroySelf =  TMouse_DestroySelf,
        .IsButtonDown = TMouse_IsButtonDown,
        .IsButtonHeld = TMouse_IsButtonHeld,
        .IsButtonUp =   TMouse_IsButtonUp,
        .pBase.Log =    TInput_Log,
    };
#else
    TestMouse tm;
    tm.PrivateData =  malloc(sizeof(TeslaMousePrivate));
    memset(tm.PrivateData, 0, sizeof(TeslaMousePrivate));
    tm.PrivateData->mouseButtonsLen = 8;

    tm.InitBackend =  TMouse_InitBackend_glfw;
    tm.DestroySelf =  TMouse_DestroySelf;
    tm.IsButtonDown = TMouse_IsButtonDown;
    tm.IsButtonHeld = TMouse_IsButtonHeld;
    tm.IsButtonUp =   TMouse_IsButtonUp;
    tm.pBase.Log =    TInput_Log;
#endif

    assert(tm.PrivateData != NULL);
    assert(tm.InitBackend != NULL);
    assert(tm.InitBackend == TMouse_InitBackend_glfw);

    return tm;
}